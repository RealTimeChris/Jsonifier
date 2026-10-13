# Jsonifier Serialization: Design and Performance

**Nihilai Collective Corp: Engineering Papers**  
*Nihilai Collective Corp*  
*Jsonifier, October 2026*  

---

## Abstract

Jsonifier serializes a reflection-registered C++ object in two passes: an exact-or-over size estimate, one buffer resize, then a single branch-light write pass that never checks capacity. Every byte the schema already knows (key names, quotes, colons, commas, indentation) is folded at compile time into packed integer constants and emitted with fixed-width stores, so the runtime work left is the data itself: strings, numbers and container lengths.

This paper describes that design as it stands in `include/jsonifier-incl/serializing/` (`serializer.hpp`, `serialize_impl.hpp`) and its helpers for strings (`string_utils.hpp`), integers (`i_to_str.hpp`) and floats (`d_to_str.hpp`).

## Architecture

The whole pipeline is resolved at compile time from `jsonifier::core<T>` and `serialize_options`: one sizing walk, one resize, one write walk.

```mermaid
flowchart LR
    A["serializeJson<br/>entry point"] --> B["Size pass<br/>staticSize shortcuts"]
    B --> C["One resize<br/>bound + 64 bytes slack"]
    C --> D["Write pass<br/>fixed-width overstores"]
    D -- "POD values inlined, containers outlined" --> E["Member headers<br/>one packed store each"]
    D --> F["Strings<br/>SIMD, SWAR, scalar"]
    D --> G["Integers<br/>4 digits per store"]
    D --> H["Floats<br/>zmij shortest round-trip"]
```

The `serialize<options>` dispatcher is a force-inlined (`JSONIFIER_INLINE`) forwarder for every value type: it strips cv and reference qualifiers and calls that type's `serialize_impl` specialization directly. Each registered type gets its own `serialize_impl` specialization: objects, maps, vectors, fixed arrays, tuples, strings, chars, enums, numbers, bools, pointers and optionals, raw JSON, null, and variants (via `visit`).

The serializer is compiled once, for the single instruction-set tier the build was configured for. That tier is chosen at CMake configure time: voided-hw-detection runs a detector on the build machine (CPUID and XCR0 on x86, `HWCAP` or `sysctl` on ARM), or `JSONIFIER_CPU_INSTRUCTIONS` names the target explicitly, and CMake attaches the matching compiler flags. `serializeJson` calls `serialize<options>::impl` directly, with no CPUID read, no cached backend value and no per-call check. There are no function pointers or virtual calls, and everything described below is compile-time code for that one tier. ARM builds work the same way, with NEON or SVE2 as the tier.

## Key techniques

### Size first, then write without bounds checks

`get_size_impl` computes an upper bound for the output before a byte is written. Fixed-width types report a `staticSize` (numbers and enums 32, `bool` 5, `char` 8, null 4), so a `vector<double>` is sized as `n × 32` plus separators with no loop. Strings are sized at `6n + 2` by default, the worst case where every byte becomes a `\u00XX` escape. With `prescanStrings` set, each string is scanned instead and sized at its exact escaped length plus 2 for the quotes (see *Pre-scanning strings*). Object keys contribute `key.size() + 3` (or `+ 4` when prettified) as a compile-time constant.

`serializeJson` adds 64 bytes of slack and calls `resize_and_overwrite` when the buffer has it, so no zero-fill happens. The write pass then trusts the bound completely: no growth checks, no reallocation, and every writer may overstore (write 8 bytes, advance 4) into the slack.

### Writing through `resize_and_overwrite`

When the output buffer has C++23's `resize_and_overwrite` (detected by the `resize_and_overwriteable_types` concept), the whole write pass runs inside it. `std::string::resize_and_overwrite(n, op)` grows the buffer to at least `n` without initializing it, calls `op(data, n)`, and sets the final size to whatever `op` returns. One call replaces the usual three steps: zero-filling resize, write, shrinking resize.

The `op` Jsonifier passes is `serialize_writer_ro`, a small functor that holds a reference to the object:

```cpp
template<serialize_options options, typename value_type> struct serialize_writer_ro {
	JSONIFIER_INLINE uint64_t operator()(write_buffer_ptr ptrNew, uint64_t) noexcept {
		const write_buffer_ptr bufferPtr = serialize<options>::impl(object, ptrNew, 0);
		return static_cast<uint64_t>(bufferPtr - ptrNew);
	}
	value_type& object;
};
```

- **`operator()` is force-inlined.** `JSONIFIER_INLINE` makes the compiler expand the serializer into the standard library's call site, instead of leaving an out-of-line function that the library calls. The write pass then runs as part of `resize_and_overwrite`'s own body, with the buffer pointer in a register and no extra call frame. Whether `resize_and_overwrite` itself inlines into `serializeJson` is still the compiler's choice.
- **The functor is a reference and a function.** It holds only `value_type& object`, and its copy and move operations are deleted, so passing it costs nothing and the object is never copied.
- **The length comes straight from the pointer.** `operator()` returns the end pointer minus the start, so the final size is set from the write itself, with no second pass and no separate size variable.

Buffers without `resize_and_overwrite` take a fallback: `resize` to the bound if the buffer is smaller, write, then `resize` down to the written length. That path zero-fills on growth.

Single values skip the size pass entirely. A lone number reserves its type's maximum digit count, a lone string `6n + 2` plus one SIMD step, and a lone `bool` 8 bytes, each passed straight to the same `resize_and_overwrite` call.

### Schema text as packed integers

`packed_blitter` turns `"key":` into one constant at compile time. Literals up to 8 bytes become a single 2, 4 or 8-byte integer; longer ones become an array of `uint64_t` rounded up to 16 or a multiple of 32. Writing a member header is one `pow2MemcpyWrapper` store and a pointer bump by the true length.

### Member iteration: CABERIHT

Object members are walked with CABERIHT, Constexpresh Aggregated Bases for Efficient Runtime Iteration of Heterogeneous Types. It is CAFBERIHT without the filter; the full pattern, with compile-time filtering, is described in [its own paper](https://nihilai-collective.net/cafberiht). Each member becomes a base class, and one fold expression over those bases writes the whole object in declaration order.

1. **One base per member.** `get_serialize_base` expands an index sequence over the `core<T>::parseValue` tuple and wraps each entry in `json_entity_serialize<options, entity>`. Each base carries its member's `name`, `memberPtr` and `isItLast` as compile-time constants.
2. **Aggregate the bases.** `serialize_map<bases...>` inherits from every one of them, so a type with 12 members gets one type with 12 bases.
3. **Fold over them.** `iterateValues` is a single fold, `((bufferPtr = bases::processIndex(value, bufferPtr, indent)), ...)`. Each `processIndex` is a static function of its own base, so every call is direct and resolved at compile time.

Inside each `processIndex`, nothing about the member is decided at runtime:

- the header `"name":` is that base's own `packed_blitter` constant;
- the value type is known, so `serialize<options>::impl` picks its writer with no dispatch;
- the trailing comma comes from `if constexpr (!isItLast)`, not an index check.

The sizing pass uses the same pattern: `size_getter_map` aggregates `json_entity_size` bases and folds `processIndex` over them, so both passes walk members the same way.

The result is no virtual calls, no `std::tuple` `get<I>` recursion, no runtime member index and no loop over members. A whole object becomes straight-line code: a run of fixed stores for headers and commas, with the value writers in between. The CAFBERIHT paper measures this same fold-over-bases dispatch as zero overhead against a hand-written baseline by assembly diff.

That is the distinction from CAFBERIHT. There, the bases are also filtered at compile time by tag, so only the components that match join the aggregate. Jsonifier's serializer keeps every registered member, so it has no filter step: the aggregate is exactly the members in `core<T>`. Runtime key exclusion (`jsonifierExcludedKeys`) is a separate mechanism, a set lookup inside each `processIndex` (see Limitations).

### Copies have a compile-time power-of-2 size

Nearly every copy on the write path has a power-of-2 size fixed at compile time: 2, 4 or 8 bytes. `pow2MemcpyWrapper<size>` takes the size as a template argument and `static_assert`s `std::has_single_bit(size)`, so a copy of any other size will not compile. A `memcpy` of 2, 4 or 8 bytes known at compile time lowers to a single unaligned load and store, with no call, no length branch and no tail loop.

What this does to the generated code:

| Copy | Typical x86-64 | Typical ARM64 |
| --- | --- | --- |
| `std::memcpy`, runtime length | `call memcpy` (or an inline length dispatch with branches and a tail loop) | `bl memcpy` |
| `jsonifierMemcpy`, runtime length under 256 bytes | at most four compare-and-branch steps on the length (no jump table), then two overlapping 1 to 128-byte copies, each split into register-width load–store pairs | the same, with `ldr`/`str` or `ldr q`/`str q` pairs |
| `jsonifierMemcpy`, runtime length of 256 bytes or more | `call memcpy` | `bl memcpy` |
| Compile-time 7 bytes | two overlapping 4-byte moves, or 4 + 2 + 1 | several loads and stores |
| Compile-time 2 / 4 / 8 bytes | one `mov` load, one `mov` store | one `ldrh`/`ldr` and one `strh`/`str` |
| Compile-time 2 / 4 / 8 bytes of constant data | one immediate store, no load (`mov qword ptr [rdi], imm`) | a constant materialized in a register, then one `str` |

- **No call.** A call to `memcpy` clobbers every caller-saved register, so live values have to be spilled and reloaded around it. Without the call, the buffer pointer and the values being written stay in registers for the whole member loop.
- **No length branches.** A runtime `memcpy` has to dispatch on the length before it copies anything. A fixed-size copy has no branch to predict or mispredict.
- **Constants become immediates.** Key headers, `true`/`false` and the empty `{}`/`[]`/`""` literals are compile-time data. Their copies can fold into immediate stores, so writing a member header needs no memory read at all.
- **Runtime lengths stay short.** Copies whose length is only known at run time, such as escape sequences and the runs that Minify and Prettify carry over, go through `jsonifierMemcpy` (`utility.hpp`). It sorts the length into a power-of-2 bucket `b`, where `b ≤ length < 2b`, and copies the first `b` bytes and the last `b` bytes. The two copies overlap, so any length from 2 to 255 bytes takes one fixed-size copy pair (a single byte takes one store), with no loop and no tail. Buckets wider than the vector register are split into register-width moves: the 128-byte bucket is 8 moves on AVX2, 4 on AVX-512 and 16 on SSE or NEON. Only lengths of 256 bytes or more reach `std::memcpy`. `jsonifierMemcpyUpTo<maxBytes>` takes a compile-time upper bound and leaves out every bucket above it, so a caller whose length is bounded never pays for the cases it cannot hit.
- **The bucket is chosen by a compare tree, not a jump table.** `dispatchBuckets` is a balanced binary tree of `length < 2^k` tests generated at compile time, so the bucket is found in at most four conditional branches on the length itself, with no `lzcnt` and no indirect jump. An earlier version switched on `std::bit_width(length)`. GCC compiled that switch to a jump table that cost three taken branches per copy against Clang's one or two, and on Linux / GCC it lost to glibc's `memcpy` in 4 of its 18 benchmark tests. The tree compiles to the same branch-only shape on GCC and Clang, and in the [memcpy-decomp](https://github.com/nihilai-collective/memcpy-decomp) harness it beats `std::memcpy` on every range tested, from 1–15 to 1–129 bytes, on all five platforms.
- **Inlining and scheduling.** Each copy is one or two instructions, so a whole object's serializer inlines into straight-line code. The compiler can then interleave stores from neighbouring members with the value work between them.

**What actually reaches `jsonifierMemcpy`.** We counted the length of every call to `jsonifierMemcpy` and `jsonifierMemcpyUpTo`, separately, over one instrumented run. The counters were a temporary patch and are not in the library.

| Length | `jsonifierMemcpy` calls | Share | `jsonifierMemcpyUpTo` calls | Share |
| --- | --- | --- | --- | --- |
| 0 | 13,040 | 1.16% | 0 | 0.00% |
| 1 | 4,012 | 0.36% | 938 | 0.22% |
| 2–3 | 9,592 | 0.85% | 1,891 | 0.44% |
| 4–7 | 22,641 | 2.01% | 8,303 | 1.93% |
| 8–15 | 185,573 | 16.47% | 411,279 | 95.82% |
| 16–31 | 735,745 | 65.29% | 6,797 | 1.58% |
| 32–63 | 149,125 | 13.23% | 0 | 0.00% |
| 64–127 | 6,552 | 0.58% | 0 | 0.00% |
| 128–255 | 312 | 0.03% | 0 | 0.00% |
| 256 or more (`std::memcpy`) | 351 | 0.03% | 1 | 0.00% |
| **Total** | **1,126,943** | | **429,209** | |

Two lengths dominate `jsonifierMemcpy`: exactly 16 bytes (659,358 calls, 58.5%) and exactly 32 bytes (146,188 calls, 13.0%), with a smaller spike at exactly 64 (3,688 calls). That matches the member-header blit, whose length `lengthToCopy` is the key literal rounded up to 16, 32 or 64 bytes. Those calls carry a template-constant length, so they fold to fixed-size vector stores. Of the 1,126,943 calls, 99.69% are 64 bytes or less, and only 351 (0.03%) are 256 bytes or more and reach `std::memcpy`. Under the earlier cutoff, where everything from 64 bytes up went to `std::memcpy`, that would have been 7,215 calls (0.64%), about 20 times as many. The 6,864 calls from 64 to 255 bytes now take the two-copy path instead.

`jsonifierMemcpyUpTo` is narrower still. 95.8% of its 429,209 calls fall in the 8–15 bucket, led by 10 bytes (31.8%) and 9 bytes (22.4%), and every call but one is 31 bytes or less. The one exception was a single 339-byte copy, which took the `std::memcpy` fallback.

That is why the writers round up and overstore instead of copying exact lengths. A 5-byte `false` is one 8-byte store. A 3-digit group is a 4-byte load followed by a 2-byte and a 1-byte store. A 7-byte key literal is one 8-byte store. The pointer then advances by the true length, and the size pass's slack absorbs the extra bytes.

Key literals longer than 8 bytes copy 16 bytes, or a multiple of 32, through the plain wrapper. That size is still a compile-time constant. The only copies whose length is known only at runtime are off the common path: escape sequences inside strings (only when an escapable byte is found), raw JSON passthrough, and the memset for indents deeper than 8 levels.

### Booleans: one subtraction, one store

A `bool` becomes JSON with no branch and no table lookup. Two 64-bit constants are built at compile time, each in the native byte order:

- `falseVInt` holds the bytes `false` packed into one word (435728179558, `0x65736C6166` on little-endian).
- `trueVInt` holds the difference between the `false` word and the `true` word (434025983730), not the `true` word itself.

The writer computes `state = falseVInt - value × trueVInt`. When `value` is 0 the result is the `false` word. When it is 1 the difference cancels and the `true` word is left. One 8-byte store writes it, and the pointer advances by `5 - value`, so `true` moves 4 bytes and `false` moves 5. The extra bytes land in the slack the size pass reserved (5 bytes per bool plus 64 at the end) and are overwritten by whatever comes next.

On the Bool test, which serializes individual bools one at a time in a loop, this writer runs at 1,204–2,311 MB/s freshly allocated on all five builds, 10.44–15.43× Glaze's 93–211 MB/s. simdjson's reflection writer runs 252 MB/s on Linux / GCC and 155 MB/s on macOS / GCC, so the lead over it is 8.49× and 10.44×. With the string reused, the reflection writer closes most of that gap (2,911 against Jsonifier's 3,090 MB/s, 1.06×, on Linux / GCC and 2,425 against 3,030 MB/s, 1.25×, on macOS / GCC), which puts most of its freshly allocated cost in allocation rather than in the write.

### Strings: picked by length

`string_serializer::impl` sorts every string into one of three length classes before it touches a byte. In all three the escapable bytes are the same: `"`, `\` and anything below 0x20.

1. **Under 16 bytes: one page-safe vector.** If the source does not sit within 16 bytes of the end of a 4 KiB page, `pageSafeShortImpl` loads a full 16-byte vector, masks its escape bits to the string's true length and, when nothing is set, stores all 16 bytes and advances by the length. A short clean key or value is one load, one test and one store, with no branch on its length. Reading past the string's end cannot fault, because the page check guarantees the 16 bytes are mapped; under ASan the load moves into a `JSONIFIER_NO_SANITIZE_ADDRESS` helper so the sanitizer does not flag it. Near a page end the string falls back to two overlapping 4- or 2-byte loads (`smallImpl`) below 8 bytes, or SWAR on 64-bit words (`swarFinish`) from 8 to 15.
2. **16 to 64 bytes: head and tail.** `mediumImpl` loads one vector from the start of the string and one ending at its last byte, tests both and, when both are clean, stores both. The two overlap in the middle, so any length in the range is covered with no loop and no remainder: 16-byte vectors up to 32 bytes, then 32-byte vectors on AVX2, or two 16-byte head-and-tail pairs over 32-byte halves where the widest register is 16 bytes. A string with any escapable byte returns `false` and takes the general path.
3. **Everything else: copy first, check second.** The general path walks the widest available SIMD width down to the narrowest, storing each block unconditionally and testing it afterwards. A clean block advances the full width; a dirty one advances to the first escapable byte, writes its escape from a 256-entry table and resumes. The last partial block is one 16-byte load ending at the string's last byte (`overlappedFinish`), shifted down to the bytes not yet written.

### Integers: exact length first, then 4 digits per store

Integers go through `to_chars` in `i_to_str.hpp` in three steps.

1. **Width.** In `serialize_impl`, every integer narrower than 64 bits is widened to `uint64_t` or `int64_t`, so one writer per signedness covers all eight integer types.
2. **Sign.** A negative value writes `'-'` and then takes its absolute value without a branch: `(v ^ (v >> 63)) - (v >> 63)` on the unsigned type. Because the result is unsigned, `INT64_MIN` works too.
3. **Length.** A balanced tree of comparisons against powers of ten picks the exact digit count, 1 to 20, in at most 5 compares. Each count has its own fixed writer (`to_chars_internal<v_type, N>`), so the writer knows at compile time where every digit goes and never reverses or shifts its output.

The writers use three lookup tables built at compile time: 100 two-digit entries, 1,000 three-digit entries and 10,000 four-digit entries, each digit group pre-packed as characters. Writing four digits is one 4-byte copy from the 10,000-entry table.

Divisions are replaced by multiplies:

- **Divide by 10^4** inside an 8-digit block: multiply by 3518437209 and shift right 45.
- **Divide by 10^8** for longer values: take the high bits of a 128-bit product with 12379400392853802749 and shift right 90 in total. GCC and Clang use `__uint128_t`, MSVC uses `_umul128`, and other compilers fall back to a portable `mulhi`.

So a 20-digit `uint64_t` is two 10^8 splits, four 10^4 splits and five 4-byte stores, with no loop and no per-digit work. A 3-digit group, as at the front of a 19-digit value, is one 4-byte table load followed by a 2-byte and a 1-byte store, so nothing is written past the 3 digits.

On the Int64 and Uint64 tests, which serialize individual values one at a time in a loop, Jsonifier is ahead of Glaze freshly allocated on every build: Int64 by 1.09× on Linux / Clang, 1.17× on Linux / GCC, 1.34× on MSVC, 1.43× on macOS / GCC and 6.56× on macOS / Clang, and Uint64 by 1.10× on Linux / GCC, 1.13× on Linux / Clang, 1.41× on MSVC, 1.53× on macOS / GCC and 6.65× on macOS / Clang. Against simdjson's reflection writer freshly allocated, Jsonifier takes both tests on both GCC builds: Int64 959 against 945 MB/s on Linux / GCC (1.01×) and 643 against 594 on macOS / GCC (1.08×), Uint64 969 against 890 (1.09×) and 717 against 540 (1.33×). With the string reused, Jsonifier is ahead of the fastest other library on both tests on every build, by 1.32× (Int64, Linux / GCC) to 5.10× (Int64, macOS / Clang).

### Floats

Doubles go through `zmij::detail::write`, a shortest-round-trip formatter vendored in `zmij.hpp`.

## Minified and prettified output

`serialize_options` carries `prettify`, `indentSize` (default 3), `indentChar` (default space) and `prescanStrings` (default false) as template parameters, so the two modes compile to separate code with no runtime mode check.

| Piece | Minified | Prettified |
| --- | --- | --- |
| Member header | `"key":` packed store | `"key": ` packed store |
| Separator | one `,` byte | `indent_table<",\n">` blit |
| Open / close | one `{` / `}` byte | `"{\n"` + indent, `"\n"` + indent, then `}` |
| Size estimate per member | `key + 3` | `key + 4`, plus `2 + indent` per separator |

`indent_table` prebuilds, at compile time, the prefix (`,\n`, `{\n` or `\n`) followed by 8 levels of indent characters as a packed `uint64_t` array. A newline plus indent is a short run of 8-byte stores from that table, rounded up into the slack. Nesting deeper than 8 levels falls back to `memset` for the remainder, marked `[[unlikely]]`.

This prettify path is the direct one, used when serializing an object. Prettifying existing JSON text is a separate path (`prettifier.hpp`) built on the stage-1 structural index described in *Two Stages, On Demand*.

## Performance results

Jsonifier was fastest in 232 of the 235 write tests on five builds, tied 2 and lost 1. With the output string reused across iterations it was also fastest in 232 of 235, tied none and lost 3. Each build runs 47 write tests: five single-value tests, ten corpus documents in minified and prettified form, the same ten documents as "Small" cut-down copies (at most 5 KiB minified) in both forms, and the Minify and Prettify reformatting tests. Jsonifier wins every full-size document test on every build freshly allocated, and every one reused except minified CitmCatalog on macOS / GCC. Every loss and tie in either run is on macOS / GCC against simdjson's reflection writer, which exists only on GCC: freshly allocated it loses minified Instruments Small and ties Double and minified Random Small; reused it loses String, minified CitmCatalog and minified Twitter Small. Windows / MSVC, Linux / Clang, Linux / GCC and macOS / Clang win all 47 tests in both runs. Prettified documents are where the lead is widest: 1.43–10.03× over Glaze freshly allocated across the five builds, and 16–63× over the reflection writer on the two GCC builds.

These results come from the [Json-Performance](https://github.com/nihilai-collective/Json-Performance) sweep published October 21, 2026, with Jsonifier [769b6b0](https://github.com/nihilai-collective/jsonifier/commit/769b6b0), Glaze [628bec1](https://github.com/stephenberry/glaze/commit/628bec1) and BenchmarkSuite [e19dd21](https://github.com/nihilai-collective/benchmarksuite/commit/e19dd21) on every build. simdjson is [1f24df3](https://github.com/simdjson/simdjson/commit/1f24df3) on Linux / Clang, Linux / GCC and macOS / Clang, [45fecc2](https://github.com/simdjson/simdjson/commit/45fecc2) on Windows / MSVC, and [683e686](https://github.com/simdjson/simdjson/commit/683e686) on macOS / GCC. Every test runs twice: once with a freshly allocated output string per iteration, and once with a reused one (see Method). Sampling and tie rules are listed under Method below and match those in *Two Stages, On Demand*. `simdjson (reflection)` is simdjson 5's C++26 `to_json`. It only runs on GCC, which has P2996 reflection. Its writer has no single-pass pretty mode, and its prettified results appear in the tables at a small fraction of the minified throughput. The Minify and Prettify tests reformat existing JSON text instead of serializing objects; they compare Jsonifier against Glaze and simdjson On Demand and are counted with the rest.

### Method

**Hardware and builds.**

| Build | CPU | OS | Jsonifier | Glaze (string escape / float write) | simdjson |
| --- | --- | --- | --- | --- | --- |
| Linux / Clang 24.0 | Intel Core i9-14900KF | Linux 7.0.0-38 | AVX2 | AVX2 / SSE4.1 | haswell |
| Linux / GCC 16.2 | Intel Core i9-14900KF | Linux 7.0.0-38 | AVX2 | AVX2 / SSE4.1 | haswell |
| macOS / Clang 23.1 | Apple M1 (virtual) | macOS 25.6.0 | NEON | NEON / NEON | arm64 |
| macOS / GCC 16.2 | Apple M1 (virtual) | macOS 25.6.0 | NEON | NEON / NEON | arm64 |
| Windows / MSVC 19.51 | Intel Core i9-14900KF | Windows 10.0.26200 | AVX2 | AVX2 / SSE4.1 | haswell |

Each library picks its own instruction set at build or run time; the table lists what produced these results. Glaze reports a backend per subsystem, so its string escaper and float writer can differ within one build.

**What is timed.** Every test runs twice. In the freshly allocated run, each iteration constructs a new output string, serializes into it, and destroys it, all inside the timed region, so allocation and deallocation are part of every measurement. In the reused run, labelled "(Reused)" in the sweep, the string is created once and held across iterations; it is cleared, keeping its capacity, outside the timed region before each iteration, so only the serialize work is measured. Both runs are tabulated below, freshly allocated first. The "Small" documents are cut-down copies of each corpus document, at most 5 KiB minified, and measure per-call overhead more than steady-state throughput. CPU caches are cleared before iterations. All libraries serialize the same test data.

**Throughput.** MB/s counts output bytes: each iteration is credited with the size of the JSON string that library produced, and throughput is total bytes over total time in the kept epoch window. MB here means 2²⁰ bytes. Output sizes can differ slightly between libraries for the same data, because each formats floats in its own way; on the Double test, for example, Glaze writes 1,798 bytes, Jsonifier 1,811 and simdjson's reflection writer 1,997. Each library is credited with its own output size.

**Sampling.**

1. Iterations start at 100 and double each epoch (100, 200, 400, …) up to 100,000.
2. Each epoch runs all its iterations and evaluates a trailing window of max(iterations / 10, 30) samples, capped at 100,000.
3. Sampling never stops early: epochs continue until 5 seconds have elapsed or the iteration cap is reached.
4. Every epoch after the first is scored by its relative standard error plus its epoch-over-epoch mean shift, and the lowest-scoring epoch is kept as the result.

**Convergence.** A kept epoch counts as converged only if its RSE is under 5% and its mean shift under 2.5% on the three x86 builds, or under 10% and 5% on the virtualized M1. Non-converged results are included in the rankings and in the win, tie and loss counts. Each one is marked n/c in the tables below, and a test with a non-converged result should be read with reduced statistical confidence.

**Ranking.** Variance is Bessel-corrected. Two libraries tie when Welch's t-test (two-sided, p < 0.05) cannot separate their kept epochs. Jsonifier is counted as a win when it ranks first and no other library is statistically tied with it, as a tie when it is statistically tied with another library at the top, and as a loss when another library ranks above it. Win, tie and loss counts include non-converged results, which are marked n/c.

**Caveats.** Keeping the quietest epoch favours each library's least-disturbed stretch, which raises absolute throughput somewhat, but the rule applies identically to every library. The M1 numbers were taken in a virtual machine, so absolute MB/s may differ from bare metal; within a build, every library ran in the same environment.

| Platform / compiler | Write tests | Jsonifier fastest | Tied | Jsonifier lost |
| --- | --- | --- | --- | --- |
| Windows / MSVC 19.51 (i9-14900KF, AVX2) | 47 | 47 | 0 | 0 |
| Linux / Clang 24.0 (i9-14900KF, AVX2) | 47 | 47 | 0 | 0 |
| Linux / GCC 16.2 (i9-14900KF, AVX2) | 47 | 47 | 0 | 0 |
| macOS / GCC 16.2 (Apple M1, NEON) | 47 | 44 | 2 | 1 |
| macOS / Clang 23.1 (Apple M1, NEON) | 47 | 47 | 0 | 0 |
| **Total** | **235** | **232** | **2** | **1** |

Every write test per build follows, in MB/s, freshly allocated run first and reused run after it. Results marked n/c did not converge. Bool, Double, Int64, String and Uint64 serialize individual values one at a time in a loop. The rest serialize whole documents, and each build's second table holds the two reformatting tests. *Lead* is Jsonifier's throughput divided by the fastest other library's, so a value under 1 is a loss.

### Windows / MSVC 19.51 (i9-14900KF, AVX2)

| Test | Jsonifier | Glaze | Lead |
| --- | --- | --- | --- |
| Bool | 1,204 | 93 | 12.98× |
| Double | 214 | 152 | 1.41× |
| Int64 | 508 | 378 | 1.34× |
| String | 1,218 | 899 | 1.35× |
| Uint64 | 527 | 373 | 1.41× |
| Canada (minified) | 1,302 | 887 | 1.47× |
| Canada (prettified) | 2,961 | 1,500 | 1.97× |
| CitmCatalog (minified) | 8,414 | 4,378 | 1.92× |
| CitmCatalog (prettified) | 4,566 | 1,884 | 2.42× |
| Discord (minified) | 10,781 | 3,964 | 2.72× |
| Discord (prettified) | 13,212 | 3,902 | 3.39× |
| Google Maps Response (minified) | 9,200 | 3,387 | 2.72× |
| Google Maps Response (prettified) | 14,487 | 5,082 | 2.85× |
| Instruments (minified) | 13,266 | 4,492 | 2.95× |
| Instruments (prettified) | 18,130 | 4,140 | 4.38× |
| Marine IK Reverse (minified) | 887 | 722 | 1.23× |
| Marine IK Reverse (prettified) | 3,258 | 1,951 | 1.67× |
| Marine IK (minified) | 897 | 636 | 1.41× |
| Marine IK (prettified) | 3,139 | 1,370 | 2.29× |
| Mesh (minified) | 1,494 | 989 | 1.51× |
| Mesh (prettified) | 2,317 | 1,563 | 1.48× |
| Random (minified) | 3,554 | 2,878 | 1.23× |
| Random (prettified) | 4,208 | 2,132 | 1.97× |
| Twitter (minified) | 14,157 | 4,155 | 3.41× |
| Twitter (prettified) | 16,328 | 2,767 | 5.90× |
| Canada Small (minified) | 1,860 | 1,085 | 1.71× |
| Canada Small (prettified) | 5,418 | 2,948 | 1.84× |
| CitmCatalog Small (minified) | 7,423 | 3,529 | 2.10× |
| CitmCatalog Small (prettified) | 12,552 | 4,130 | 3.04× |
| Discord Small (minified) | 7,736 | 3,446 | 2.24× |
| Discord Small (prettified) | 11,724 | 3,511 | 3.34× |
| Google Maps Response Small (minified) | 8,686 | 3,404 | 2.55× |
| Google Maps Response Small (prettified) | 16,742 | 4,708 | 3.56× |
| Instruments Small (minified) | 10,462 | 3,201 (n/c) | 3.27× |
| Instruments Small (prettified) | 14,701 | 3,270 | 4.50× |
| Marine IK Reverse Small (minified) | 1,476 | 953 | 1.55× |
| Marine IK Reverse Small (prettified) | 6,792 | 2,842 | 2.39× |
| Marine IK Small (minified) | 1,494 | 956 | 1.56× |
| Marine IK Small (prettified) | 6,819 | 2,970 | 2.30× |
| Mesh Small (minified) | 1,944 | 1,306 | 1.49× |
| Mesh Small (prettified) | 3,950 | 1,956 | 2.02× |
| Random Small (minified) | 8,475 | 3,229 | 2.62× |
| Random Small (prettified) | 13,620 | 3,719 | 3.66× |
| Twitter Small (minified) | 7,971 | 4,389 | 1.82× |
| Twitter Small (prettified) | 12,315 | 3,267 | 3.77× |

| Reformatting test | Jsonifier | Glaze | simdjson (ondemand) | Lead |
| --- | --- | --- | --- | --- |
| Minify | 2,919 | 1,945 | 1,098 | 1.50× |
| Prettify | 2,289 | 2,223 | 883 | 1.03× |

### Linux / Clang 24.0 (i9-14900KF, AVX2)

| Test | Jsonifier | Glaze | Lead |
| --- | --- | --- | --- |
| Bool | 2,311 | 211 | 10.95× |
| Double | 342 | 287 | 1.19× |
| Int64 | 952 | 872 | 1.09× |
| String | 4,431 | 2,009 | 2.21× |
| Uint64 | 958 | 846 | 1.13× |
| Canada (minified) | 1,608 | 959 | 1.68× |
| Canada (prettified) | 4,683 | 3,284 | 1.43× |
| CitmCatalog (minified) | 10,800 | 5,594 | 1.93× |
| CitmCatalog (prettified) | 17,777 | 5,708 | 3.11× |
| Discord (minified) | 14,448 | 6,003 | 2.41× |
| Discord (prettified) | 18,121 | 5,049 | 3.59× |
| Google Maps Response (minified) | 11,989 | 4,129 | 2.90× |
| Google Maps Response (prettified) | 18,516 | 5,233 | 3.54× |
| Instruments (minified) | 18,584 | 5,311 | 3.50× |
| Instruments (prettified) | 23,080 | 5,647 | 4.09× |
| Marine IK Reverse (minified) | 840 | 612 | 1.37× |
| Marine IK Reverse (prettified) | 6,107 | 3,643 | 1.68× |
| Marine IK (minified) | 895 | 594 | 1.51× |
| Marine IK (prettified) | 6,145 | 2,782 | 2.21× |
| Mesh (minified) | 1,760 | 1,026 | 1.71× |
| Mesh (prettified) | 4,097 | 1,714 | 2.39× |
| Random (minified) | 14,225 | 3,530 | 4.03× |
| Random (prettified) | 18,652 | 3,960 | 4.71× |
| Twitter (minified) | 20,080 | 5,701 | 3.52× |
| Twitter (prettified) | 22,713 | 5,491 | 4.14× |
| Canada Small (minified) | 1,590 | 968 | 1.64× |
| Canada Small (prettified) | 4,999 | 3,243 | 1.54× |
| CitmCatalog Small (minified) | 8,445 | 3,514 | 2.40× |
| CitmCatalog Small (prettified) | 17,424 | 3,908 | 4.46× |
| Discord Small (minified) | 10,717 | 3,424 | 3.13× |
| Discord Small (prettified) | 13,601 | 3,323 | 4.09× |
| Google Maps Response Small (minified) | 10,714 | 3,913 | 2.74× |
| Google Maps Response Small (prettified) | 21,765 | 4,674 | 4.66× |
| Instruments Small (minified) | 12,981 | 3,505 | 3.70× |
| Instruments Small (prettified) | 22,353 | 3,984 | 5.61× |
| Marine IK Reverse Small (minified) | 1,317 | 819 | 1.61× |
| Marine IK Reverse Small (prettified) | 7,518 | 3,914 | 1.92× |
| Marine IK Small (minified) | 1,394 | 817 | 1.71× |
| Marine IK Small (prettified) | 7,416 | 3,879 | 1.91× |
| Mesh Small (minified) | 1,673 | 963 | 1.74× |
| Mesh Small (prettified) | 3,789 | 1,649 | 2.30× |
| Random Small (minified) | 12,619 | 3,589 | 3.52× |
| Random Small (prettified) | 20,640 | 3,961 | 5.21× |
| Twitter Small (minified) | 8,567 | 4,628 | 1.85× |
| Twitter Small (prettified) | 11,731 | 4,030 | 2.91× |

| Reformatting test | Jsonifier | Glaze | simdjson (ondemand) | Lead |
| --- | --- | --- | --- | --- |
| Minify | 3,723 | 2,881 | 1,340 | 1.29× |
| Prettify | 5,116 | 4,014 | 1,485 | 1.27× |

### Linux / GCC 16.2 (i9-14900KF, AVX2)

| Test | Jsonifier | Glaze | simdjson (reflection) | Lead |
| --- | --- | --- | --- | --- |
| Bool | 2,139 | 205 | 252 | 8.49× |
| Double | 339 | 305 | 337 | 1.00× |
| Int64 | 959 | 823 | 945 | 1.01× |
| String | 3,926 | 2,006 | 3,816 | 1.03× |
| Uint64 | 969 | 878 | 890 | 1.09× |
| Canada (minified) | 2,090 | 1,419 | 1,032 | 1.47× |
| Canada (prettified) | 6,009 | 3,227 | 378 | 1.86× |
| CitmCatalog (minified) | 12,266 | 6,155 | 11,064 | 1.11× |
| CitmCatalog (prettified) | 18,408 | 5,938 | 538 | 3.10× |
| Discord (minified) | 14,125 | 6,181 | 11,354 | 1.24× |
| Discord (prettified) | 19,077 | 5,967 | 511 | 3.20× |
| Google Maps Response (minified) | 11,069 | 4,070 | 9,850 | 1.12× |
| Google Maps Response (prettified) | 18,052 | 6,903 | 525 | 2.62× |
| Instruments (minified) | 18,960 | 5,404 | 15,564 | 1.22× |
| Instruments (prettified) | 24,412 | 5,954 | 479 | 4.10× |
| Marine IK Reverse (minified) | 1,340 | 983 | 868 | 1.36× |
| Marine IK Reverse (prettified) | 6,560 | 3,588 | 266 | 1.83× |
| Marine IK (minified) | 1,334 | 973 | 886 | 1.37× |
| Marine IK (prettified) | 6,523 | 3,130 | 269 | 2.08× |
| Mesh (minified) | 2,415 | 1,656 | 1,302 | 1.46× |
| Mesh (prettified) | 4,494 | 2,656 | 261 | 1.69× |
| Random (minified) | 13,594 | 3,626 | 10,871 | 1.25× |
| Random (prettified) | 18,000 | 4,791 | 513 | 3.76× |
| Twitter (minified) | 19,170 | 6,903 | 12,810 | 1.50× |
| Twitter (prettified) | 23,384 | 4,788 | 523 | 4.88× |
| Canada Small (minified) | 2,033 | 1,407 | 1,080 | 1.45× |
| Canada Small (prettified) | 5,629 | 3,441 | 405 | 1.64× |
| CitmCatalog Small (minified) | 9,129 | 3,817 | 6,828 | 1.34× |
| CitmCatalog Small (prettified) | 17,880 | 4,325 | 526 | 4.13× |
| Discord Small (minified) | 10,258 | 4,071 | 6,665 | 1.54× |
| Discord Small (prettified) | 14,450 | 3,782 | 530 | 3.82× |
| Google Maps Response Small (minified) | 9,955 | 3,762 | 7,904 | 1.26× |
| Google Maps Response Small (prettified) | 21,144 | 5,919 | 529 | 3.57× |
| Instruments Small (minified) | 14,613 | 3,724 | 7,969 | 1.83× |
| Instruments Small (prettified) | 24,672 | 3,636 | 506 | 6.78× |
| Marine IK Reverse Small (minified) | 1,748 | 1,232 | 1,466 | 1.19× |
| Marine IK Reverse Small (prettified) | 7,455 | 3,902 | 516 | 1.91× |
| Marine IK Small (minified) | 1,784 | 1,191 | 1,464 | 1.22× |
| Marine IK Small (prettified) | 7,745 | 3,652 | 494 | 2.12× |
| Mesh Small (minified) | 1,955 | 1,412 | 1,372 | 1.38× |
| Mesh Small (prettified) | 4,369 | 2,061 | 285 | 2.12× |
| Random Small (minified) | 11,600 | 3,928 | 7,597 | 1.53× |
| Random Small (prettified) | 20,255 | 4,684 | 545 | 4.32× |
| Twitter Small (minified) | 8,560 | 5,266 | 6,092 | 1.41× |
| Twitter Small (prettified) | 12,011 | 3,137 | 544 | 3.83× |

| Reformatting test | Jsonifier | Glaze | simdjson (ondemand) | Lead |
| --- | --- | --- | --- | --- |
| Minify | 3,272 | 2,375 | 1,350 | 1.38× |
| Prettify | 4,343 | 3,673 | 1,502 | 1.18× |

### macOS / GCC 16.2 (Apple M1, NEON)

| Test | Jsonifier | Glaze | simdjson (reflection) | Lead |
| --- | --- | --- | --- | --- |
| Bool | 1,622 | 107 | 155 | 10.44× |
| Double | 207 (n/c) | 161 | 233 | 0.89× |
| Int64 | 643 | 448 | 594 | 1.08× |
| String | 1,136 | 942 | 986 | 1.15× |
| Uint64 | 717 | 469 | 540 | 1.33× |
| Canada (minified) | 1,573 (n/c) | 1,430 (n/c) | 633 | 1.10× |
| Canada (prettified) | 4,305 (n/c) | 2,505 | 203 (n/c) | 1.72× |
| CitmCatalog (minified) | 5,614 | 1,435 | 4,750 | 1.18× |
| CitmCatalog (prettified) | 17,454 (n/c) | 3,129 (n/c) | 288 | 5.58× |
| Discord (minified) | 11,023 | 2,550 | 8,222 | 1.34× |
| Discord (prettified) | 12,025 | 2,236 | 303 | 5.38× |
| Google Maps Response (minified) | 9,098 | 1,485 (n/c) | 7,442 | 1.22× |
| Google Maps Response (prettified) | 16,545 | 2,602 | 276 | 6.36× |
| Instruments (minified) | 12,974 | 1,717 | 7,064 | 1.84× |
| Instruments (prettified) | 14,744 | 1,640 | 233 | 8.99× |
| Marine IK Reverse (minified) | 1,033 | 685 | 545 | 1.51× |
| Marine IK Reverse (prettified) | 5,700 | 2,621 | 199 (n/c) | 2.17× |
| Marine IK (minified) | 1,143 | 683 | 485 (n/c) | 1.67× |
| Marine IK (prettified) | 5,532 | 1,960 | 230 (n/c) | 2.82× |
| Mesh (minified) | 1,994 (n/c) | 762 | 859 (n/c) | 2.32× |
| Mesh (prettified) | 4,219 | 1,401 | 125 (n/c) | 3.01× |
| Random (minified) | 9,588 | 1,721 | 6,883 | 1.39× |
| Random (prettified) | 11,898 | 2,163 | 288 | 5.50× |
| Twitter (minified) | 10,619 | 2,658 | 7,748 | 1.37× |
| Twitter (prettified) | 12,056 | 3,131 | 292 (n/c) | 3.85× |
| Canada Small (minified) | 2,124 | 820 | 297 | 2.59× |
| Canada Small (prettified) | 5,984 | 2,381 (n/c) | 193 | 2.51× |
| CitmCatalog Small (minified) | 6,730 | 1,617 | 5,963 | 1.13× |
| CitmCatalog Small (prettified) | 13,365 | 2,051 | 236 | 6.52× |
| Discord Small (minified) | 10,312 | 2,320 | 7,383 | 1.40× |
| Discord Small (prettified) | 11,689 | 2,342 | 304 | 4.99× |
| Google Maps Response Small (minified) | 8,781 | 1,593 | 5,504 | 1.60× |
| Google Maps Response Small (prettified) | 15,157 | 2,515 | 259 | 6.03× |
| Instruments Small (minified) | 4,460 | 1,435 | 6,646 | 0.67× |
| Instruments Small (prettified) | 14,810 | 1,991 | 272 | 7.44× |
| Marine IK Reverse Small (minified) | 1,673 | 676 | 950 (n/c) | 1.76× |
| Marine IK Reverse Small (prettified) | 6,159 | 2,236 | 257 | 2.76× |
| Marine IK Small (minified) | 1,399 | 840 | 1,200 | 1.17× |
| Marine IK Small (prettified) | 6,715 | 2,859 | 238 | 2.35× |
| Mesh Small (minified) | 1,710 | 392 | 990 | 1.73× |
| Mesh Small (prettified) | 3,983 | 1,138 | 196 | 3.50× |
| Random Small (minified) | 9,411 (n/c) | 773 | 7,784 | 1.21× |
| Random Small (prettified) | 4,500 | 1,918 | 210 | 2.35× |
| Twitter Small (minified) | 9,201 | 1,260 | 7,144 | 1.29× |
| Twitter Small (prettified) | 12,435 (n/c) | 1,064 | 357 (n/c) | 11.69× |

| Reformatting test | Jsonifier | Glaze | simdjson (ondemand) | Lead |
| --- | --- | --- | --- | --- |
| Minify | 2,037 | 1,401 | 778 | 1.45× |
| Prettify | 2,896 | 1,613 (n/c) | 468 | 1.80× |

### macOS / Clang 23.1 (Apple M1, NEON)

| Test | Jsonifier | Glaze | Lead |
| --- | --- | --- | --- |
| Bool | 1,759 | 114 | 15.43× |
| Double | 261 | 177 | 1.47× |
| Int64 | 2,912 | 444 | 6.56× |
| String | 1,015 | 948 | 1.07× |
| Uint64 | 2,913 | 438 | 6.65× |
| Canada (minified) | 1,850 (n/c) | 1,392 | 1.33× |
| Canada (prettified) | 6,103 | 2,742 | 2.23× |
| CitmCatalog (minified) | 7,298 | 1,690 | 4.32× |
| CitmCatalog (prettified) | 17,051 | 2,625 | 6.50× |
| Discord (minified) | 10,926 | 2,324 | 4.70× |
| Discord (prettified) | 16,799 | 2,524 | 6.66× |
| Google Maps Response (minified) | 9,626 | 1,105 (n/c) | 8.71× |
| Google Maps Response (prettified) | 19,892 | 2,679 | 7.43× |
| Instruments (minified) | 12,222 | 1,847 | 6.62× |
| Instruments (prettified) | 21,700 | 2,164 | 10.03× |
| Marine IK Reverse (minified) | 1,465 | 666 | 2.20× |
| Marine IK Reverse (prettified) | 6,586 | 2,911 | 2.26× |
| Marine IK (minified) | 1,306 (n/c) | 800 | 1.63× |
| Marine IK (prettified) | 6,213 | 2,229 | 2.79× |
| Mesh (minified) | 2,415 | 1,074 | 2.25× |
| Mesh (prettified) | 4,113 | 1,847 | 2.23× |
| Random (minified) | 7,005 | 1,602 (n/c) | 4.37× |
| Random (prettified) | 11,263 | 2,188 | 5.15× |
| Twitter (minified) | 12,795 | 3,440 | 3.72× |
| Twitter (prettified) | 10,833 | 3,509 | 3.09× |
| Canada Small (minified) | 2,233 | 1,096 | 2.04× |
| Canada Small (prettified) | 6,381 | 2,581 | 2.47× |
| CitmCatalog Small (minified) | 7,857 | 1,576 | 4.98× |
| CitmCatalog Small (prettified) | 14,519 | 2,290 | 6.34× |
| Discord Small (minified) | 9,339 | 2,289 | 4.08× |
| Discord Small (prettified) | 14,684 | 2,397 | 6.13× |
| Google Maps Response Small (minified) | 8,202 | 1,453 | 5.64× |
| Google Maps Response Small (prettified) | 18,106 | 2,353 | 7.70× |
| Instruments Small (minified) | 10,346 | 1,584 | 6.53× |
| Instruments Small (prettified) | 21,230 | 1,799 | 11.80× |
| Marine IK Reverse Small (minified) | 1,662 | 878 | 1.89× |
| Marine IK Reverse Small (prettified) | 6,386 | 2,172 | 2.94× |
| Marine IK Small (minified) | 1,612 | 869 | 1.86× |
| Marine IK Small (prettified) | 6,108 | 2,669 | 2.29× |
| Mesh Small (minified) | 2,146 | 965 | 2.22× |
| Mesh Small (prettified) | 4,032 | 1,479 | 2.73× |
| Random Small (minified) | 8,179 | 1,552 | 5.27× |
| Random Small (prettified) | 4,654 | 1,842 (n/c) | 2.53× |
| Twitter Small (minified) | 8,565 | 2,778 | 3.08× |
| Twitter Small (prettified) | 12,461 | 2,495 | 5.00× |

| Reformatting test | Jsonifier | Glaze | simdjson (ondemand) | Lead |
| --- | --- | --- | --- | --- |
| Minify | 2,291 | 1,539 | 728 | 1.49× |
| Prettify | 3,069 | 2,088 | 539 | 1.47× |

### Reused output strings

With the string's capacity kept across iterations, Jsonifier is fastest in 232 of the 235 write tests and loses 3, all on macOS / GCC against the reflection writer: String, 7,612 vs 8,679 MB/s (0.88×), minified CitmCatalog, 6,068 vs 6,425 MB/s (0.94×), and minified Twitter Small, 6,189 vs 8,278 MB/s (0.75×). Forty-two tests across the five builds have at least one non-converged result (twenty-two freshly allocated, twenty reused): forty on the two macOS builds and two on Windows / MSVC, where Glaze did not converge on Instruments Small, minified freshly allocated and prettified reused. Each is marked n/c in the tables. Sixteen of them are Jsonifier's own, all on macOS / GCC except two on macOS / Clang, and all wins except the freshly allocated Double and minified Random Small ties and the reused minified Twitter Small loss, all on macOS / GCC.

| Platform / compiler | Write tests | Jsonifier fastest | Tied | Jsonifier lost |
| --- | --- | --- | --- | --- |
| Windows / MSVC 19.51 (i9-14900KF, AVX2) | 47 | 47 | 0 | 0 |
| Linux / Clang 24.0 (i9-14900KF, AVX2) | 47 | 47 | 0 | 0 |
| Linux / GCC 16.2 (i9-14900KF, AVX2) | 47 | 47 | 0 | 0 |
| macOS / GCC 16.2 (Apple M1, NEON) | 47 | 44 | 0 | 3 |
| macOS / Clang 23.1 (Apple M1, NEON) | 47 | 47 | 0 | 0 |
| **Total** | **235** | **232** | **0** | **3** |

#### Windows / MSVC 19.51 (i9-14900KF, AVX2)

| Test | Jsonifier | Glaze | Lead |
| --- | --- | --- | --- |
| Bool | 1,768 | 291 | 6.08× |
| Double | 821 | 382 | 2.15× |
| Int64 | 4,597 | 1,064 | 4.32× |
| String | 8,433 | 2,414 | 3.49× |
| Uint64 | 5,179 | 1,074 | 4.82× |
| Canada (minified) | 1,580 | 1,216 | 1.30× |
| Canada (prettified) | 4,676 | 3,222 | 1.45× |
| CitmCatalog (minified) | 8,579 | 4,885 | 1.76× |
| CitmCatalog (prettified) | 16,087 | 7,263 | 2.22× |
| Discord (minified) | 11,204 | 4,777 | 2.35× |
| Discord (prettified) | 13,785 | 4,493 | 3.07× |
| Google Maps Response (minified) | 10,378 | 4,295 | 2.42× |
| Google Maps Response (prettified) | 15,447 | 6,082 | 2.54× |
| Instruments (minified) | 14,095 | 4,993 | 2.82× |
| Instruments (prettified) | 18,447 | 4,641 | 3.97× |
| Marine IK Reverse (minified) | 1,003 | 781 | 1.28× |
| Marine IK Reverse (prettified) | 4,988 | 3,524 | 1.42× |
| Marine IK (minified) | 1,019 | 799 | 1.28× |
| Marine IK (prettified) | 4,989 | 3,559 | 1.40× |
| Mesh (minified) | 1,878 | 1,405 | 1.34× |
| Mesh (prettified) | 3,467 | 2,289 | 1.51× |
| Random (minified) | 10,224 | 3,059 | 3.34× |
| Random (prettified) | 13,719 | 4,086 | 3.36× |
| Twitter (minified) | 14,405 | 4,907 | 2.94× |
| Twitter (prettified) | 16,395 | 3,001 | 5.46× |
| Canada Small (minified) | 1,917 | 1,185 | 1.62× |
| Canada Small (prettified) | 5,754 | 3,277 | 1.76× |
| CitmCatalog Small (minified) | 8,068 | 4,315 | 1.87× |
| CitmCatalog Small (prettified) | 15,138 | 5,408 | 2.80× |
| Discord Small (minified) | 10,228 | 4,432 | 2.31× |
| Discord Small (prettified) | 13,021 | 3,935 | 3.31× |
| Google Maps Response Small (minified) | 9,835 | 4,121 | 2.39× |
| Google Maps Response Small (prettified) | 20,189 | 6,035 | 3.35× |
| Instruments Small (minified) | 12,304 | 5,588 | 2.20× |
| Instruments Small (prettified) | 18,530 | 3,952 (n/c) | 4.69× |
| Marine IK Reverse Small (minified) | 1,613 | 1,020 | 1.58× |
| Marine IK Reverse Small (prettified) | 6,969 | 3,437 | 2.03× |
| Marine IK Small (minified) | 1,590 | 992 | 1.60× |
| Marine IK Small (prettified) | 7,121 | 3,402 | 2.09× |
| Mesh Small (minified) | 2,118 | 1,402 | 1.51× |
| Mesh Small (prettified) | 4,251 | 2,288 | 1.86× |
| Random Small (minified) | 11,029 | 3,770 | 2.93× |
| Random Small (prettified) | 18,255 | 4,315 | 4.23× |
| Twitter Small (minified) | 11,080 | 5,487 | 2.02× |
| Twitter Small (prettified) | 14,000 | 3,384 | 4.14× |

| Reformatting test | Jsonifier | Glaze | simdjson (ondemand) | Lead |
| --- | --- | --- | --- | --- |
| Minify | 2,939 | 1,959 | 1,093 | 1.50× |
| Prettify | 3,589 | 2,218 | 861 | 1.62× |

#### Linux / Clang 24.0 (i9-14900KF, AVX2)

| Test | Jsonifier | Glaze | Lead |
| --- | --- | --- | --- |
| Bool | 4,279 | 896 | 4.78× |
| Double | 906 | 615 | 1.47× |
| Int64 | 5,312 | 2,350 | 2.26× |
| String | 11,726 | 5,574 | 2.10× |
| Uint64 | 5,634 | 2,569 | 2.19× |
| Canada (minified) | 1,603 | 1,000 | 1.60× |
| Canada (prettified) | 4,684 | 3,942 | 1.19× |
| CitmCatalog (minified) | 10,754 | 6,492 | 1.66× |
| CitmCatalog (prettified) | 17,947 | 7,682 | 2.34× |
| Discord (minified) | 14,773 | 6,140 | 2.41× |
| Discord (prettified) | 18,537 | 5,803 | 3.19× |
| Google Maps Response (minified) | 12,553 | 4,520 | 2.78× |
| Google Maps Response (prettified) | 19,263 | 5,711 | 3.37× |
| Instruments (minified) | 18,590 | 6,343 | 2.93× |
| Instruments (prettified) | 23,767 | 6,737 | 3.53× |
| Marine IK Reverse (minified) | 837 | 611 | 1.37× |
| Marine IK Reverse (prettified) | 6,033 | 4,025 | 1.50× |
| Marine IK (minified) | 897 | 624 | 1.44× |
| Marine IK (prettified) | 6,145 | 4,009 | 1.53× |
| Mesh (minified) | 1,762 | 1,055 | 1.67× |
| Mesh (prettified) | 4,139 | 1,811 | 2.29× |
| Random (minified) | 14,287 | 3,769 | 3.79× |
| Random (prettified) | 18,763 | 4,569 | 4.11× |
| Twitter (minified) | 20,392 | 6,820 | 2.99× |
| Twitter (prettified) | 22,774 | 6,374 | 3.57× |
| Canada Small (minified) | 1,606 | 1,004 | 1.60× |
| Canada Small (prettified) | 5,038 | 3,736 | 1.35× |
| CitmCatalog Small (minified) | 9,282 | 3,830 | 2.42× |
| CitmCatalog Small (prettified) | 18,680 | 4,642 | 4.02× |
| Discord Small (minified) | 11,716 | 3,586 | 3.27× |
| Discord Small (prettified) | 16,228 | 3,615 | 4.49× |
| Google Maps Response Small (minified) | 11,832 | 4,164 | 2.84× |
| Google Maps Response Small (prettified) | 23,519 | 5,301 | 4.44× |
| Instruments Small (minified) | 14,206 | 3,723 | 3.82× |
| Instruments Small (prettified) | 24,429 | 4,525 | 5.40× |
| Marine IK Reverse Small (minified) | 1,351 | 843 | 1.60× |
| Marine IK Reverse Small (prettified) | 7,592 | 4,356 | 1.74× |
| Marine IK Small (minified) | 1,458 | 832 | 1.75× |
| Marine IK Small (prettified) | 7,651 | 4,305 | 1.78× |
| Mesh Small (minified) | 1,752 | 989 | 1.77× |
| Mesh Small (prettified) | 4,312 | 1,799 | 2.40× |
| Random Small (minified) | 13,923 | 3,946 | 3.53× |
| Random Small (prettified) | 23,800 | 4,646 | 5.12× |
| Twitter Small (minified) | 10,102 | 5,269 | 1.92× |
| Twitter Small (prettified) | 13,852 | 4,800 | 2.89× |

| Reformatting test | Jsonifier | Glaze | simdjson (ondemand) | Lead |
| --- | --- | --- | --- | --- |
| Minify | 3,716 | 2,843 | 1,333 | 1.31× |
| Prettify | 5,106 | 3,963 | 1,467 | 1.29× |

#### Linux / GCC 16.2 (i9-14900KF, AVX2)

| Test | Jsonifier | Glaze | simdjson (reflection) | Lead |
| --- | --- | --- | --- | --- |
| Bool | 3,090 | 677 | 2,911 | 1.06× |
| Double | 835 | 731 | 643 | 1.14× |
| Int64 | 5,479 | 2,482 | 4,147 | 1.32× |
| String | 11,121 | 6,116 | 9,240 | 1.20× |
| Uint64 | 6,243 | 2,697 | 4,289 | 1.46× |
| Canada (minified) | 2,092 | 1,509 | 1,030 | 1.39× |
| Canada (prettified) | 6,015 | 3,821 | 381 | 1.57× |
| CitmCatalog (minified) | 12,414 | 7,235 | 11,182 | 1.11× |
| CitmCatalog (prettified) | 18,417 | 8,220 | 524 | 2.24× |
| Discord (minified) | 14,192 | 6,648 | 11,377 | 1.25× |
| Discord (prettified) | 19,195 | 7,225 | 509 | 2.66× |
| Google Maps Response (minified) | 11,420 | 4,457 | 10,246 | 1.11× |
| Google Maps Response (prettified) | 18,853 | 7,637 | 524 | 2.47× |
| Instruments (minified) | 19,384 | 6,259 | 15,279 | 1.27× |
| Instruments (prettified) | 24,545 | 7,415 | 474 | 3.31× |
| Marine IK Reverse (minified) | 1,338 | 991 | 873 | 1.35× |
| Marine IK Reverse (prettified) | 6,580 | 4,048 | 265 | 1.63× |
| Marine IK (minified) | 1,337 | 1,026 | 886 | 1.30× |
| Marine IK (prettified) | 6,535 | 4,117 | 285 | 1.59× |
| Mesh (minified) | 2,419 | 1,732 | 1,302 | 1.40× |
| Mesh (prettified) | 4,474 | 2,857 | 257 | 1.57× |
| Random (minified) | 13,736 | 4,046 | 11,517 | 1.19× |
| Random (prettified) | 17,982 | 5,664 | 511 | 3.17× |
| Twitter (minified) | 19,276 | 7,775 | 13,043 | 1.48× |
| Twitter (prettified) | 23,520 | 5,392 | 524 | 4.36× |
| Canada Small (minified) | 2,060 | 1,609 | 1,068 | 1.28× |
| Canada Small (prettified) | 5,720 | 3,771 | 401 | 1.52× |
| CitmCatalog Small (minified) | 9,845 | 4,229 | 7,238 | 1.36× |
| CitmCatalog Small (prettified) | 20,817 | 5,028 | 527 | 4.14× |
| Discord Small (minified) | 11,821 | 4,716 | 7,111 | 1.66× |
| Discord Small (prettified) | 16,323 | 4,209 | 529 | 3.88× |
| Google Maps Response Small (minified) | 10,878 | 4,142 | 8,437 | 1.29× |
| Google Maps Response Small (prettified) | 23,660 | 7,052 | 527 | 3.36× |
| Instruments Small (minified) | 16,422 | 4,647 | 8,365 | 1.96× |
| Instruments Small (prettified) | 29,057 | 4,075 | 505 | 7.13× |
| Marine IK Reverse Small (minified) | 1,797 | 1,300 | 1,510 | 1.19× |
| Marine IK Reverse Small (prettified) | 7,656 | 4,260 | 515 | 1.80× |
| Marine IK Small (minified) | 1,820 | 1,278 | 1,497 | 1.22× |
| Marine IK Small (prettified) | 7,885 | 4,018 | 503 | 1.96× |
| Mesh Small (minified) | 2,063 | 1,451 | 1,422 | 1.42× |
| Mesh Small (prettified) | 4,588 | 1,791 | 285 | 2.56× |
| Random Small (minified) | 12,378 | 4,549 | 8,264 | 1.50× |
| Random Small (prettified) | 22,347 | 5,640 | 547 | 3.96× |
| Twitter Small (minified) | 9,722 | 6,227 | 6,993 | 1.39× |
| Twitter Small (prettified) | 14,155 | 3,939 | 536 | 3.59× |

| Reformatting test | Jsonifier | Glaze | simdjson (ondemand) | Lead |
| --- | --- | --- | --- | --- |
| Minify | 3,308 | 2,381 | 1,349 | 1.39× |
| Prettify | 4,312 | 3,681 | 1,521 | 1.17× |

#### macOS / GCC 16.2 (Apple M1, NEON)

| Test | Jsonifier | Glaze | simdjson (reflection) | Lead |
| --- | --- | --- | --- | --- |
| Bool | 3,030 (n/c) | 755 | 2,425 | 1.25× |
| Double | 746 | 427 | 600 | 1.24× |
| Int64 | 4,592 | 1,926 | 2,430 | 1.89× |
| String | 7,612 | 3,642 (n/c) | 8,679 (n/c) | 0.88× |
| Uint64 | 4,184 (n/c) | 1,855 | 2,930 | 1.43× |
| Canada (minified) | 1,867 (n/c) | 1,482 | 647 | 1.26× |
| Canada (prettified) | 5,503 | 2,636 | 201 (n/c) | 2.09× |
| CitmCatalog (minified) | 6,068 | 1,670 | 6,425 | 0.94× |
| CitmCatalog (prettified) | 16,795 | 3,349 | 211 (n/c) | 5.01× |
| Discord (minified) | 11,389 | 2,567 | 8,282 | 1.38× |
| Discord (prettified) | 12,848 | 2,464 | 208 (n/c) | 5.22× |
| Google Maps Response (minified) | 10,145 | 1,527 | 7,808 | 1.30× |
| Google Maps Response (prettified) | 19,590 | 2,608 | 287 | 7.51× |
| Instruments (minified) | 13,177 | 1,865 | 5,927 (n/c) | 2.22× |
| Instruments (prettified) | 15,132 | 1,623 | 299 | 9.32× |
| Marine IK Reverse (minified) | 1,033 (n/c) | 729 (n/c) | 547 (n/c) | 1.42× |
| Marine IK Reverse (prettified) | 5,712 | 2,685 | 221 | 2.13× |
| Marine IK (minified) | 1,173 | 752 | 549 | 1.56× |
| Marine IK (prettified) | 5,379 | 2,671 | 223 | 2.01× |
| Mesh (minified) | 1,806 | 957 | 888 | 1.89× |
| Mesh (prettified) | 3,322 | 1,445 | 149 (n/c) | 2.30× |
| Random (minified) | 9,699 | 1,678 | 6,838 | 1.42× |
| Random (prettified) | 10,684 | 2,260 | 242 | 4.73× |
| Twitter (minified) | 13,108 | 3,835 | 8,563 | 1.53× |
| Twitter (prettified) | 11,265 | 3,183 | 306 (n/c) | 3.54× |
| Canada Small (minified) | 2,031 | 1,490 | 795 (n/c) | 1.36× |
| Canada Small (prettified) | 6,293 | 2,936 | 208 | 2.14× |
| CitmCatalog Small (minified) | 8,422 | 1,811 | 6,462 | 1.30× |
| CitmCatalog Small (prettified) | 15,991 | 2,371 | 302 | 6.75× |
| Discord Small (minified) | 9,739 | 2,490 | 6,744 | 1.44× |
| Discord Small (prettified) | 12,851 | 2,470 | 297 | 5.20× |
| Google Maps Response Small (minified) | 9,398 | 1,707 | 7,668 | 1.23× |
| Google Maps Response Small (prettified) | 18,065 | 2,629 | 216 | 6.87× |
| Instruments Small (minified) | 13,985 (n/c) | 1,657 | 7,050 (n/c) | 1.98× |
| Instruments Small (prettified) | 16,119 | 1,507 | 194 | 10.70× |
| Marine IK Reverse Small (minified) | 1,631 | 843 | 1,399 (n/c) | 1.17× |
| Marine IK Reverse Small (prettified) | 6,021 | 3,327 | 256 | 1.81× |
| Marine IK Small (minified) | 1,677 | 848 | 1,380 | 1.22× |
| Marine IK Small (prettified) | 7,077 | 3,057 | 177 | 2.31× |
| Mesh Small (minified) | 1,901 (n/c) | 858 | 964 | 1.97× |
| Mesh Small (prettified) | 4,646 | 1,440 | 198 | 3.23× |
| Random Small (minified) | 10,462 | 1,932 | 8,389 | 1.25× |
| Random Small (prettified) | 4,685 | 1,659 | 140 | 2.82× |
| Twitter Small (minified) | 6,189 (n/c) | 3,217 | 8,278 | 0.75× |
| Twitter Small (prettified) | 4,866 | 1,374 | 349 | 3.54× |

| Reformatting test | Jsonifier | Glaze | simdjson (ondemand) | Lead |
| --- | --- | --- | --- | --- |
| Minify | 1,712 | 1,434 | 739 | 1.19× |
| Prettify | 2,160 | 1,311 | 434 (n/c) | 1.65× |

#### macOS / Clang 23.1 (Apple M1, NEON)

| Test | Jsonifier | Glaze | Lead |
| --- | --- | --- | --- |
| Bool | 2,779 | 529 | 5.25× |
| Double | 784 | 586 | 1.34× |
| Int64 | 3,274 | 641 | 5.10× |
| String | 7,007 | 4,048 | 1.73× |
| Uint64 | 3,081 | 2,044 | 1.51× |
| Canada (minified) | 2,222 | 1,484 | 1.50× |
| Canada (prettified) | 5,995 | 2,960 | 2.03× |
| CitmCatalog (minified) | 7,206 | 1,801 | 4.00× |
| CitmCatalog (prettified) | 17,005 | 3,010 | 5.65× |
| Discord (minified) | 12,433 | 2,902 | 4.28× |
| Discord (prettified) | 17,202 | 2,613 | 6.58× |
| Google Maps Response (minified) | 10,117 | 1,743 | 5.80× |
| Google Maps Response (prettified) | 22,685 | 2,974 | 7.63× |
| Instruments (minified) | 13,894 | 1,977 | 7.03× |
| Instruments (prettified) | 20,988 | 2,369 | 8.86× |
| Marine IK Reverse (minified) | 1,467 | 822 | 1.79× |
| Marine IK Reverse (prettified) | 6,619 | 3,048 | 2.17× |
| Marine IK (minified) | 1,254 | 823 | 1.52× |
| Marine IK (prettified) | 5,766 | 2,751 | 2.10× |
| Mesh (minified) | 2,559 | 1,223 (n/c) | 2.09× |
| Mesh (prettified) | 3,974 | 1,907 | 2.08× |
| Random (minified) | 8,207 | 1,746 | 4.70× |
| Random (prettified) | 9,580 | 2,193 | 4.37× |
| Twitter (minified) | 13,800 | 4,126 | 3.34× |
| Twitter (prettified) | 13,139 | 3,594 | 3.66× |
| Canada Small (minified) | 1,894 | 1,328 | 1.43× |
| Canada Small (prettified) | 6,559 | 3,119 | 2.10× |
| CitmCatalog Small (minified) | 8,077 | 1,901 | 4.25× |
| CitmCatalog Small (prettified) | 17,454 | 2,571 | 6.79× |
| Discord Small (minified) | 10,719 | 2,521 | 4.25× |
| Discord Small (prettified) | 15,310 | 2,764 | 5.54× |
| Google Maps Response Small (minified) | 8,843 | 1,596 | 5.54× |
| Google Maps Response Small (prettified) | 20,284 | 2,243 | 9.04× |
| Instruments Small (minified) | 12,628 | 1,860 | 6.79× |
| Instruments Small (prettified) | 23,988 | 2,127 | 11.28× |
| Marine IK Reverse Small (minified) | 1,697 | 946 | 1.79× |
| Marine IK Reverse Small (prettified) | 7,949 | 2,790 | 2.85× |
| Marine IK Small (minified) | 1,883 | 890 | 2.12× |
| Marine IK Small (prettified) | 6,681 | 3,142 | 2.13× |
| Mesh Small (minified) | 2,557 | 1,060 | 2.41× |
| Mesh Small (prettified) | 4,006 | 1,759 | 2.28× |
| Random Small (minified) | 8,623 | 1,774 | 4.86× |
| Random Small (prettified) | 12,803 | 2,325 | 5.51× |
| Twitter Small (minified) | 11,416 | 3,004 | 3.80× |
| Twitter Small (prettified) | 15,310 | 3,181 (n/c) | 4.81× |

| Reformatting test | Jsonifier | Glaze | simdjson (ondemand) | Lead |
| --- | --- | --- | --- | --- |
| Minify | 2,312 | 1,558 | 729 | 1.48× |
| Prettify | 3,194 | 2,087 | 596 | 1.53× |

### Losses and ties

The prettified lead comes from the indent tables: a newline plus indent is a few 8-byte stores, not a loop. Windows / MSVC, Linux / Clang, Linux / GCC and macOS / Clang win every test in both runs. The one freshly allocated loss is on macOS / GCC:

- **Instruments Small (minified), macOS / GCC.** Jsonifier runs 4,460 vs the reflection writer's 6,646 MB/s (0.67×). It is ahead of Glaze (1,435 MB/s, 3.11×), and ahead of the reflection writer reused (13,985 vs 7,050 MB/s, 1.98×, neither converged).

The two freshly allocated ties are both on macOS / GCC, and in both Jsonifier's result did not converge. On Double the reflection writer posts 233 against Jsonifier's 207 MB/s (0.89×); on minified Random Small Jsonifier posts 9,411 against 7,784 MB/s (1.21×). Welch's test separates neither, and reused Jsonifier wins both (746 vs 600 MB/s, 1.24×, and 10,462 vs 8,389, 1.25×). The three reused losses, all on macOS / GCC:

- **String, macOS / GCC.** The reflection writer runs 8,679 vs Jsonifier's 7,612 MB/s (0.88×); neither the reflection writer's nor Glaze's result converged. Freshly allocated, Jsonifier wins (1,136 vs 986 MB/s, 1.15×).
- **CitmCatalog (minified), macOS / GCC.** The reflection writer runs 6,425 vs Jsonifier's 6,068 MB/s (0.94×). Freshly allocated, Jsonifier wins (5,614 vs 4,750 MB/s, 1.18×).
- **Twitter Small (minified), macOS / GCC.** The reflection writer runs 8,278 vs Jsonifier's 6,189 MB/s (0.75×), with Jsonifier's result not converged. Freshly allocated, Jsonifier wins (9,201 vs 7,144 MB/s, 1.29×).

Every full-size document is ahead of the reflection writer on both GCC builds freshly allocated, and reused on all but minified CitmCatalog on macOS / GCC. Freshly allocated, minified documents lead it by 1.11–2.48× and prettified documents by 16–63×. The Small documents follow the full-size ones: Jsonifier wins 98 of the 100 freshly allocated (the Instruments Small loss and Random Small tie above) and 99 of the 100 reused (the Twitter Small loss). Not profiled yet: why the reflection writer edges ahead on some minified documents and on String and Double on macOS / GCC is not known, and this paper does not guess at its algorithm.

**Reformatting.** Jsonifier wins Minify on all five builds in both runs, by 1.29–1.50× freshly allocated and 1.19–1.50× reused over the next-fastest library (Glaze in every case). It wins Prettify on all five builds in both runs, by 1.03–1.80× freshly allocated and 1.17–1.65× reused.

## Questions from external review

An outside review of a draft raised five concerns. Each is answered here against the code as it stands; where the answer is "not measured yet", it says so.

### Can an overstore run past the buffer?

No, because every overstore lands inside the string's own size, not past it. `serializeJson` resizes the buffer to the full bound plus 64 bytes before writing, so the write pass works inside memory the string already owns. No write ever reaches capacity or the allocator's edge, and ASan sees only in-bounds accesses.

The bound covers the largest overstore of each writer:

| Writer | Largest write beyond its true length | Covered by |
| --- | --- | --- |
| Bool | 3 bytes (8 written, 5 reserved) | 64-byte tail slack |
| Integer or float | none past the 32 bytes reserved per number | `staticSize` of 32 |
| Key literal over 16 bytes | up to 31 bytes (rounded to a multiple of 32) | 64-byte tail slack |
| Indent run | up to 7 bytes (rounded to 8) | 64-byte tail slack |
| String SIMD block | one vector width, stored before the escape check | the `6n + 2` reservation, which the copied bytes never exceed |
| Short string (under 16 bytes) | up to 15 bytes (one 16-byte store) | 64-byte tail slack |

The largest single overstore is 31 bytes, under the 64-byte slack. That slack is a fixed constant, not derived from the widest writer, so a future writer with a wider overstore must raise it.

### Does `6n + 2` blow up memory on huge strings?

Yes, the reservation is real. A 100 MB string field asks for about 600 MB before the final shrink, and the string keeps that capacity afterwards unless the caller calls `shrink_to_fit`. How much of it is touched depends on the buffer:

- With `resize_and_overwrite`, nothing is zero-filled. On Linux and macOS, pages that are never written are usually never faulted in, so the cost is mostly address space. On Windows, the allocation still counts against the commit limit.
- Without `resize_and_overwrite`, `resize` zero-fills the whole bound, so every page is touched.

Setting `prescanStrings` in `serialize_options` removes the 6× factor, at the price of one extra pass over every string's bytes during sizing. See *Pre-scanning strings* below. Its throughput cost has not been measured.

### Pre-scanning strings

`prescanStrings` replaces the `6n + 2` string bound with a scan that returns the exact number of bytes the writer will emit for the string, plus 2 for the quotes. The option is off by default and is a compile-time `if constexpr` in the string `get_size_impl`, so the default path is unchanged.

The scan (`string_serializer::escapedSize`) reads 8 bytes at a time and applies the same SWAR flag test the writer's tail path uses (`flagMask`): a word with no `"`, `\` or control byte adds 8 and moves on. A flagged word is summed byte by byte from `escapeTableSizes`, the same table the writer uses, where `\b \t \n \f \r " \\` take 2 bytes, other control bytes take 6, and everything else takes 1. The tail under 8 bytes is summed the same way. A NUL byte is counted as 1, although the writer's table emits nothing for it, so the bound can exceed the output by a byte but never fall short.

The 64-byte slack in `serializeJson` is unchanged, so the overstore argument above still holds. The option only affects strings: numbers, booleans and keys keep their existing bounds.

### Is it portable to big-endian targets?

The code has big-endian paths, but none are tested. `packed_blitter`, the bool constants, the digit tables and the SWAR string tail each branch on `std::endian::native` at compile time and use `std::byteswap` where needed. The CI matrix covers only little-endian x86 and ARM, so those branches have never run.

### Why is the String test slower on NEON than on AVX2?

The gap is a freshly allocated one. Freshly allocated, Jsonifier runs String at 1,136 MB/s on macOS / GCC (1.21× Glaze's 942, 1.15× the reflection writer's 986) and 1,015 MB/s on macOS / Clang (1.07× Glaze's 948). The AVX2 builds run the same test at 1,218–4,431 MB/s and lead Glaze by 1.35× (Windows / MSVC), 2.21× (Linux / Clang) and 1.96× (Linux / GCC). With the string reused the M1 lead over Glaze is 1.73× on macOS / Clang (7,007 vs 4,048 MB/s) and 2.09× on macOS / GCC (7,612 vs 3,642, Glaze not converged), against the 1.82–3.49× the AVX2 builds show reused. On macOS / GCC the reflection writer is ahead reused, at 8,679 MB/s (not converged).

So most of the M1 shortfall sits in the part of the freshly allocated run that the reused run removes, the allocation and release of the output string, rather than in the copy loop. That is an inference from the two runs, not a profile. Two features of the general loop remain candidates for whatever is left:

- Each block stops at its first escapable byte, writes that one escape, and reloads from the next byte. A string dense in escapes therefore pays one vector load per escape.
- Finding the first escapable byte needs a byte-mask extraction (`opBitMaskRaw`). x86 has a single instruction for this; NEON has to emulate it.

Neither has been isolated.

### Does simdjson's writer win doubles on GCC?

On neither build does it win outright. On macOS / GCC it posts the higher freshly allocated mean, 233 vs Jsonifier's 207 MB/s (0.89×), but Jsonifier's result did not converge and Welch's test counts it a tie. On Linux / GCC Jsonifier is ahead, 339 vs 337 MB/s (1.00×, separated by Welch's test). Reused, Jsonifier is ahead on both, 835 vs 643 MB/s (1.30×) on Linux / GCC and 746 vs 600 (1.24×) on macOS / GCC. Jsonifier beats Glaze on the Double test on all five builds freshly allocated, by 1.11–1.47×. Why the reflection writer posts the higher freshly allocated mean on macOS / GCC has not been measured, and this paper does not guess at its algorithm.

### What to check on MSVC

The Windows build has landed (table above), but its assembly has not been inspected yet. Three things are worth confirming there:

1. The 10^8 reciprocal uses `_umul128` and stays in registers, with no stack temporary.
2. `resize_and_overwrite` compiles without iterator-debug checks. Release builds default to `_ITERATOR_DEBUG_LEVEL=0`, so this should hold unless the project overrides it.
3. The fixed-size scalar stores stay scalar and are not merged into wider unaligned vector stores that split cache lines.

## Limitations

- **Over-allocation.** The size bound is worst case: every number reserves 32 bytes and, unless `prescanStrings` is set, every string `6n + 2`. A string-heavy document asks for up to about 6× its output size before shrinking. That is cheap with `resize_and_overwrite` but costs peak memory on large payloads. `prescanStrings` tightens the string part to the exact escaped length at the cost of an extra pass over each string; numbers still reserve 32 bytes each.
- **Excluded keys are a runtime lookup.** Types with `jsonifierExcludedKeys` do a set lookup per member in both passes.
- **Registration required.** Like the parser, the fast path needs `jsonifier::core<T>`; there is no reflection-free serializer for arbitrary types.
