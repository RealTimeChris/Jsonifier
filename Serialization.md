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

On the Bool test, which serializes individual bools one at a time in a loop, this writer runs at 1,251–2,548 MB/s freshly allocated on all five builds, 11.90–16.18× Glaze's 91–214 MB/s. simdjson's reflection writer runs 249 MB/s on Linux / GCC and 213 MB/s on macOS / GCC, so the lead over it is 8.58× and 8.62×. With the string reused, the reflection writer closes that gap entirely: it edges ahead on Linux / GCC (3,232 against Jsonifier's 3,080 MB/s, 0.95×) and trails on macOS / GCC (2,549 against 3,181 MB/s, 1.25×), which puts most of its freshly allocated cost in allocation rather than in the write.

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
- **Divide by 10^8** for longer values: take the high bits of a 128-bit product with 12379400392853802749 and shift right 90 in total. GCC and Clang use `__uint128_t`, MSVC uses `_umul128`, and other compilers fall back to `mulhiPortable`.

So a 20-digit `uint64_t` is two 10^8 splits, four 10^4 splits and five 4-byte stores, with no loop and no per-digit work. A 3-digit group, as at the front of a 19-digit value, is one 4-byte table load followed by a 2-byte and a 1-byte store, so nothing is written past the 3 digits.

On the Int64 and Uint64 tests, which serialize individual values one at a time in a loop, Jsonifier is ahead of Glaze freshly allocated on every build: Int64 by 1.04× on Linux / Clang, 1.16× on Linux / GCC, 1.29× on MSVC, 1.52× on macOS / GCC and 5.97× on macOS / Clang, and Uint64 by 1.06× on Linux / Clang, 1.09× on Linux / GCC, 1.36× on MSVC, 1.49× on macOS / GCC and 6.14× on macOS / Clang. Against simdjson's reflection writer freshly allocated, Jsonifier takes Int64 on both GCC builds, 981 against 949 MB/s on Linux / GCC (1.03×) and 763 against 689 on macOS / GCC (1.11×), and Uint64 on Linux / GCC, 957 against 909 (1.05×), but loses Uint64 on macOS / GCC, 735 against 762 (0.96×). With the string reused, Jsonifier is ahead of the fastest other library on both tests on every build, by 1.31× (Int64, Linux / GCC) to 5.15× (Uint64, Windows / MSVC), and the macOS / GCC Uint64 result turns into a 1.70× lead (5,008 against 2,943 MB/s).

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

Jsonifier was fastest in 228 of the 235 write tests on five builds, tied none and lost 7. With the output string reused across iterations it was fastest in 231 of 235, tied 1 and lost 3. Each build runs 47 write tests: five single-value tests, ten corpus documents in minified and prettified form, the same ten documents as "Small" cut-down copies (at most 5 KiB minified) in both forms, and the Minify and Prettify reformatting tests. Jsonifier wins every full-size document test on every build in both runs. Freshly allocated, five of the seven losses are against simdjson's reflection writer, which exists only on GCC: Double, String and Uint64 on macOS / GCC, and minified Marine IK Small and Marine IK Reverse Small on Linux / GCC. The other two are reformatting tests lost to Glaze: Prettify on Windows / MSVC and Minify on macOS / Clang. Reused, every loss and the tie are against the reflection writer: Bool and minified Marine IK Reverse Small on Linux / GCC and String on macOS / GCC lose, and minified Marine IK Small on Linux / GCC ties. Linux / Clang wins all 47 tests in both runs. Prettified documents are where the lead is widest: 1.45–10.65× over Glaze freshly allocated across the five builds, and 16–60× over the reflection writer on the two GCC builds.

These results come from the [Json-Performance](https://github.com/nihilai-collective/Json-Performance) sweep published October 22, 2026, with Jsonifier [da96147](https://github.com/nihilai-collective/jsonifier/commit/da96147), Glaze [628bec1](https://github.com/stephenberry/glaze/commit/628bec1), simdjson [70f83c9](https://github.com/simdjson/simdjson/commit/70f83c9) and BenchmarkSuite [ab75024](https://github.com/nihilai-collective/benchmarksuite/commit/ab75024) on every build. Every test runs twice: once with a freshly allocated output string per iteration, and once with a reused one (see Method). Sampling and tie rules are listed under Method below and match those in *Two Stages, On Demand*. `simdjson (reflection)` is simdjson 5's C++26 `to_json`. It only runs on GCC, which has P2996 reflection. Its writer has no single-pass pretty mode, and its prettified results appear in the tables at a small fraction of the minified throughput. The Minify and Prettify tests reformat existing JSON text instead of serializing objects; they compare Jsonifier against Glaze and simdjson On Demand and are counted with the rest.

### Method

**Hardware and builds.**

| Build | CPU | OS | Jsonifier | Glaze (string escape / float write) | simdjson |
| --- | --- | --- | --- | --- | --- |
| Linux / Clang 24.0 | Intel Core i9-14900KF | Linux 7.0.0-38 | AVX2 | AVX2 / SSE4.1 | haswell |
| Linux / GCC 16.2 | Intel Core i9-14900KF | Linux 7.0.0-38 | AVX2 | AVX2 / SSE4.1 | haswell |
| macOS / Clang 23.1 | Apple M1 (virtual) | macOS 25.6.0 | NEON | NEON / NEON | arm64 |
| macOS / GCC 16.2 | Apple M1 (virtual) | macOS 25.6.0 | NEON | NEON / NEON | arm64 |
| Windows / MSVC 19.44 | Intel Core i9-14900KF | Windows 10.0.26200 | AVX2 | AVX2 / SSE4.1 | haswell |

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
| Windows / MSVC 19.44 (i9-14900KF, AVX2) | 47 | 46 | 0 | 1 |
| Linux / Clang 24.0 (i9-14900KF, AVX2) | 47 | 47 | 0 | 0 |
| Linux / GCC 16.2 (i9-14900KF, AVX2) | 47 | 45 | 0 | 2 |
| macOS / GCC 16.2 (Apple M1, NEON) | 47 | 44 | 0 | 3 |
| macOS / Clang 23.1 (Apple M1, NEON) | 47 | 46 | 0 | 1 |
| **Total** | **235** | **228** | **0** | **7** |

Every write test per build follows, in MB/s, freshly allocated run first and reused run after it. Results marked n/c did not converge. Bool, Double, Int64, String and Uint64 serialize individual values one at a time in a loop. The rest serialize whole documents, and each build's second table holds the two reformatting tests. *Lead* is Jsonifier's throughput divided by the fastest other library's, so a value under 1 is a loss.

### Windows / MSVC 19.44 (i9-14900KF, AVX2)

| Test | Jsonifier | Glaze | Lead |
| --- | --- | --- | --- |
| Bool | 1,251 | 91 | 13.78× |
| Double | 218 | 152 | 1.43× |
| Int64 | 506 | 393 | 1.29× |
| String | 1,166 | 901 | 1.29× |
| Uint64 | 518 | 381 | 1.36× |
| Canada (minified) | 1,358 | 809 | 1.68× |
| Canada (prettified) | 3,142 | 1,425 | 2.21× |
| CitmCatalog (minified) | 8,270 | 4,351 | 1.90× |
| CitmCatalog (prettified) | 4,602 | 1,950 | 2.36× |
| Discord (minified) | 10,080 | 4,539 | 2.22× |
| Discord (prettified) | 12,813 | 4,343 | 2.95× |
| Google Maps Response (minified) | 8,818 | 3,338 | 2.64× |
| Google Maps Response (prettified) | 14,084 | 5,286 | 2.66× |
| Instruments (minified) | 12,292 | 4,225 | 2.91× |
| Instruments (prettified) | 16,409 | 4,971 | 3.30× |
| Marine IK Reverse (minified) | 864 | 680 | 1.27× |
| Marine IK Reverse (prettified) | 3,095 | 1,831 | 1.69× |
| Marine IK (minified) | 883 | 602 | 1.47× |
| Marine IK (prettified) | 3,072 | 1,260 | 2.44× |
| Mesh (minified) | 1,450 | 912 | 1.59× |
| Mesh (prettified) | 2,307 | 1,523 | 1.51× |
| Random (minified) | 3,372 | 2,886 | 1.17× |
| Random (prettified) | 4,038 | 2,262 | 1.78× |
| Twitter (minified) | 13,984 | 4,302 | 3.25× |
| Twitter (prettified) | 15,604 | 3,436 | 4.54× |
| Canada Small (minified) | 2,009 | 1,142 | 1.76× |
| Canada Small (prettified) | 5,376 | 2,882 | 1.87× |
| CitmCatalog Small (minified) | 6,816 | 3,534 | 1.93× |
| CitmCatalog Small (prettified) | 12,440 | 4,367 | 2.85× |
| Discord Small (minified) | 9,224 | 3,584 | 2.57× |
| Discord Small (prettified) | 11,685 | 3,732 | 3.13× |
| Google Maps Response Small (minified) | 8,740 | 3,184 | 2.75× |
| Google Maps Response Small (prettified) | 15,731 | 5,130 | 3.07× |
| Instruments Small (minified) | 8,905 | 3,797 | 2.35× |
| Instruments Small (prettified) | 16,046 | 4,092 | 3.92× |
| Marine IK Reverse Small (minified) | 1,560 | 833 | 1.87× |
| Marine IK Reverse Small (prettified) | 7,004 | 2,747 | 2.55× |
| Marine IK Small (minified) | 1,564 | 909 | 1.72× |
| Marine IK Small (prettified) | 6,561 | 2,796 | 2.35× |
| Mesh Small (minified) | 2,006 | 1,129 | 1.78× |
| Mesh Small (prettified) | 4,065 | 1,901 | 2.14× |
| Random Small (minified) | 8,269 | 3,297 | 2.51× |
| Random Small (prettified) | 14,028 | 4,275 | 3.28× |
| Twitter Small (minified) | 9,797 | 4,296 | 2.28× |
| Twitter Small (prettified) | 12,706 | 3,371 | 3.77× |

| Reformatting test | Jsonifier | Glaze | simdjson (ondemand) | Lead |
| --- | --- | --- | --- | --- |
| Minify | 3,052 | 1,938 | 1,155 | 1.57× |
| Prettify | 2,354 | 2,370 | 921 | 0.99× |

### Linux / Clang 24.0 (i9-14900KF, AVX2)

| Test | Jsonifier | Glaze | Lead |
| --- | --- | --- | --- |
| Bool | 2,548 | 214 | 11.90× |
| Double | 344 | 308 | 1.12× |
| Int64 | 942 | 905 | 1.04× |
| String | 4,047 | 2,013 | 2.01× |
| Uint64 | 930 | 878 | 1.06× |
| Canada (minified) | 1,595 | 961 | 1.66× |
| Canada (prettified) | 4,723 | 3,264 | 1.45× |
| CitmCatalog (minified) | 10,648 | 5,693 | 1.87× |
| CitmCatalog (prettified) | 17,631 | 5,705 | 3.09× |
| Discord (minified) | 14,479 | 6,161 | 2.35× |
| Discord (prettified) | 18,075 | 5,028 | 3.59× |
| Google Maps Response (minified) | 11,957 | 4,100 | 2.92× |
| Google Maps Response (prettified) | 18,804 | 5,047 | 3.73× |
| Instruments (minified) | 18,420 | 5,579 | 3.30× |
| Instruments (prettified) | 23,079 | 5,713 | 4.04× |
| Marine IK Reverse (minified) | 846 | 618 | 1.37× |
| Marine IK Reverse (prettified) | 6,133 | 3,714 | 1.65× |
| Marine IK (minified) | 866 | 601 | 1.44× |
| Marine IK (prettified) | 6,156 | 2,905 | 2.12× |
| Mesh (minified) | 1,769 | 1,013 | 1.75× |
| Mesh (prettified) | 4,090 | 1,708 | 2.39× |
| Random (minified) | 14,283 | 3,335 | 4.28× |
| Random (prettified) | 18,954 | 4,015 | 4.72× |
| Twitter (minified) | 20,314 | 6,045 | 3.36× |
| Twitter (prettified) | 22,799 | 5,462 | 4.17× |
| Canada Small (minified) | 1,481 | 965 | 1.53× |
| Canada Small (prettified) | 4,411 | 3,156 | 1.40× |
| CitmCatalog Small (minified) | 8,521 | 3,468 | 2.46× |
| CitmCatalog Small (prettified) | 16,571 | 3,956 | 4.19× |
| Discord Small (minified) | 10,840 | 3,541 | 3.06× |
| Discord Small (prettified) | 14,900 | 2,882 | 5.17× |
| Google Maps Response Small (minified) | 10,894 | 3,568 | 3.05× |
| Google Maps Response Small (prettified) | 20,686 | 4,494 | 4.60× |
| Instruments Small (minified) | 12,370 | 2,888 | 4.28× |
| Instruments Small (prettified) | 21,554 | 4,039 | 5.34× |
| Marine IK Reverse Small (minified) | 1,262 | 793 | 1.59× |
| Marine IK Reverse Small (prettified) | 7,063 | 3,899 | 1.81× |
| Marine IK Small (minified) | 1,437 | 811 | 1.77× |
| Marine IK Small (prettified) | 7,474 | 3,730 | 2.00× |
| Mesh Small (minified) | 1,702 | 970 | 1.76× |
| Mesh Small (prettified) | 4,056 | 1,654 | 2.45× |
| Random Small (minified) | 11,437 | 3,560 | 3.21× |
| Random Small (prettified) | 19,898 | 4,034 | 4.93× |
| Twitter Small (minified) | 8,781 | 4,763 | 1.84× |
| Twitter Small (prettified) | 11,938 | 3,637 | 3.28× |

| Reformatting test | Jsonifier | Glaze | simdjson (ondemand) | Lead |
| --- | --- | --- | --- | --- |
| Minify | 3,727 | 2,569 | 1,373 | 1.45× |
| Prettify | 5,369 | 4,191 | 1,473 | 1.28× |

### Linux / GCC 16.2 (i9-14900KF, AVX2)

| Test | Jsonifier | Glaze | simdjson (reflection) | Lead |
| --- | --- | --- | --- | --- |
| Bool | 2,140 | 164 | 249 | 8.58× |
| Double | 352 | 312 | 343 | 1.03× |
| Int64 | 981 | 849 | 949 | 1.03× |
| String | 3,934 | 2,029 | 3,578 | 1.10× |
| Uint64 | 957 | 881 | 909 | 1.05× |
| Canada (minified) | 2,144 | 1,451 | 1,039 | 1.48× |
| Canada (prettified) | 6,290 | 3,273 | 385 | 1.92× |
| CitmCatalog (minified) | 12,762 | 6,245 | 11,330 | 1.13× |
| CitmCatalog (prettified) | 20,180 | 6,002 | 568 | 3.36× |
| Discord (minified) | 15,594 | 6,212 | 11,248 | 1.39× |
| Discord (prettified) | 20,090 | 6,255 | 510 | 3.21× |
| Google Maps Response (minified) | 16,324 | 4,191 | 10,344 | 1.58× |
| Google Maps Response (prettified) | 21,012 | 6,895 | 535 | 3.05× |
| Instruments (minified) | 20,615 | 5,512 | 15,853 | 1.30× |
| Instruments (prettified) | 26,171 | 6,005 | 483 | 4.36× |
| Marine IK Reverse (minified) | 1,322 | 994 | 884 | 1.33× |
| Marine IK Reverse (prettified) | 6,593 | 3,746 | 272 | 1.76× |
| Marine IK (minified) | 1,286 | 965 | 886 | 1.33× |
| Marine IK (prettified) | 6,454 | 3,131 | 274 | 2.06× |
| Mesh (minified) | 2,456 | 1,632 | 1,303 | 1.50× |
| Mesh (prettified) | 4,612 | 2,680 | 282 | 1.72× |
| Random (minified) | 15,606 | 3,702 | 11,572 | 1.35× |
| Random (prettified) | 20,273 | 4,804 | 510 | 4.22× |
| Twitter (minified) | 18,863 | 6,707 | 12,962 | 1.46× |
| Twitter (prettified) | 24,173 | 4,654 | 526 | 5.19× |
| Canada Small (minified) | 2,189 | 1,546 | 1,078 | 1.42× |
| Canada Small (prettified) | 6,662 | 3,462 | 409 | 1.92× |
| CitmCatalog Small (minified) | 8,631 | 3,653 | 6,902 | 1.25× |
| CitmCatalog Small (prettified) | 18,277 | 4,614 | 538 | 3.96× |
| Discord Small (minified) | 10,275 | 4,254 | 6,527 | 1.57× |
| Discord Small (prettified) | 15,244 | 4,094 | 530 | 3.72× |
| Google Maps Response Small (minified) | 13,153 | 3,958 | 7,929 | 1.66× |
| Google Maps Response Small (prettified) | 26,416 | 6,053 | 541 | 4.36× |
| Instruments Small (minified) | 12,301 | 3,911 | 7,913 | 1.55× |
| Instruments Small (prettified) | 21,186 | 3,546 | 516 | 5.97× |
| Marine IK Reverse Small (minified) | 1,274 | 1,213 | 1,501 | 0.85× |
| Marine IK Reverse Small (prettified) | 7,177 | 3,649 | 532 | 1.97× |
| Marine IK Small (minified) | 1,355 | 1,208 | 1,485 | 0.91× |
| Marine IK Small (prettified) | 6,918 | 3,538 | 507 | 1.96× |
| Mesh Small (minified) | 2,165 | 1,354 | 1,370 | 1.58× |
| Mesh Small (prettified) | 4,544 | 2,131 | 300 | 2.13× |
| Random Small (minified) | 12,940 | 3,953 | 7,565 | 1.71× |
| Random Small (prettified) | 21,051 | 4,905 | 555 | 4.29× |
| Twitter Small (minified) | 8,084 | 5,120 | 6,149 | 1.31× |
| Twitter Small (prettified) | 11,385 | 3,547 | 536 | 3.21× |

| Reformatting test | Jsonifier | Glaze | simdjson (ondemand) | Lead |
| --- | --- | --- | --- | --- |
| Minify | 3,241 | 2,413 | 1,404 | 1.34× |
| Prettify | 4,307 | 3,689 | 1,495 | 1.17× |

### macOS / GCC 16.2 (Apple M1, NEON)

| Test | Jsonifier | Glaze | simdjson (reflection) | Lead |
| --- | --- | --- | --- | --- |
| Bool | 1,838 | 114 | 213 | 8.62× |
| Double | 245 | 177 | 287 | 0.85× |
| Int64 | 763 | 503 | 689 | 1.11× |
| String | 1,180 | 987 | 1,219 | 0.97× |
| Uint64 | 735 | 495 | 762 | 0.96× |
| Canada (minified) | 2,219 | 1,522 | 759 | 1.46× |
| Canada (prettified) | 6,458 | 3,153 (n/c) | 240 | 2.05× |
| CitmCatalog (minified) | 8,424 | 1,946 | 7,086 | 1.19× |
| CitmCatalog (prettified) | 18,280 | 3,265 | 316 | 5.60× |
| Discord (minified) | 11,483 | 2,711 | 8,516 | 1.35× |
| Discord (prettified) | 13,193 | 2,491 | 328 | 5.30× |
| Google Maps Response (minified) | 10,326 | 1,625 | 7,837 | 1.32× |
| Google Maps Response (prettified) | 18,804 | 2,602 | 313 | 7.23× |
| Instruments (minified) | 11,837 | 1,866 | 7,188 | 1.65× |
| Instruments (prettified) | 16,863 | 1,741 | 322 | 9.69× |
| Marine IK Reverse (minified) | 1,234 | 765 (n/c) | 718 (n/c) | 1.61× |
| Marine IK Reverse (prettified) | 6,209 | 2,789 | 253 | 2.23× |
| Marine IK (minified) | 1,247 | 758 | 691 | 1.64× |
| Marine IK (prettified) | 6,126 | 2,539 (n/c) | 259 | 2.41× |
| Mesh (minified) | 2,238 | 1,099 | 944 | 2.04× |
| Mesh (prettified) | 4,492 | 1,435 | 204 | 3.13× |
| Random (minified) | 11,096 | 1,908 (n/c) | 7,453 | 1.49× |
| Random (prettified) | 13,632 | 2,386 | 308 | 5.71× |
| Twitter (minified) | 13,221 | 3,524 | 9,507 | 1.39× |
| Twitter (prettified) | 17,231 | 3,667 | 375 | 4.70× |
| Canada Small (minified) | 2,090 | 1,532 | 841 | 1.36× |
| Canada Small (prettified) | 6,194 | 2,675 | 267 | 2.32× |
| CitmCatalog Small (minified) | 8,008 | 1,614 | 6,566 | 1.22× |
| CitmCatalog Small (prettified) | 15,635 | 2,742 | 322 | 5.70× |
| Discord Small (minified) | 11,213 | 2,469 | 7,687 | 1.46× |
| Discord Small (prettified) | 12,170 | 2,320 | 346 | 5.24× |
| Google Maps Response Small (minified) | 8,597 | 1,600 | 7,454 | 1.15× |
| Google Maps Response Small (prettified) | 19,453 | 2,702 | 297 | 7.20× |
| Instruments Small (minified) | 12,309 | 1,436 | 7,323 | 1.68× |
| Instruments Small (prettified) | 16,436 | 2,101 | 295 | 7.82× |
| Marine IK Reverse Small (minified) | 1,604 | 942 | 1,378 | 1.16× |
| Marine IK Reverse Small (prettified) | 7,592 | 2,801 | 281 | 2.71× |
| Marine IK Small (minified) | 1,816 | 897 | 1,429 | 1.27× |
| Marine IK Small (prettified) | 7,600 | 2,730 | 268 | 2.78× |
| Mesh Small (minified) | 2,007 | 933 | 1,083 | 1.85× |
| Mesh Small (prettified) | 4,493 | 1,266 | 195 | 3.55× |
| Random Small (minified) | 10,253 | 1,777 | 8,479 | 1.21× |
| Random Small (prettified) | 13,086 | 2,339 | 255 | 5.59× |
| Twitter Small (minified) | 10,160 | 3,102 | 7,664 | 1.33× |
| Twitter Small (prettified) | 13,680 | 2,691 | 382 | 5.08× |

| Reformatting test | Jsonifier | Glaze | simdjson (ondemand) | Lead |
| --- | --- | --- | --- | --- |
| Minify | 2,129 | 1,444 | 860 | 1.47× |
| Prettify | 2,894 | 2,083 | 626 | 1.39× |

### macOS / Clang 23.1 (Apple M1, NEON)

| Test | Jsonifier | Glaze | Lead |
| --- | --- | --- | --- |
| Bool | 1,387 | 96 | 14.41× |
| Double | 261 | 183 | 1.43× |
| Int64 | 2,694 | 452 | 5.97× |
| String | 1,026 | 878 | 1.17× |
| Uint64 | 2,684 | 437 | 6.14× |
| Canada (minified) | 1,855 | 1,358 | 1.37× |
| Canada (prettified) | 5,548 | 2,373 | 2.34× |
| CitmCatalog (minified) | 7,923 | 1,626 | 4.87× |
| CitmCatalog (prettified) | 16,993 | 2,281 | 7.45× |
| Discord (minified) | 10,813 | 2,100 | 5.15× |
| Discord (prettified) | 16,673 | 2,408 | 6.93× |
| Google Maps Response (minified) | 9,299 | 1,457 | 6.38× |
| Google Maps Response (prettified) | 19,826 | 2,585 | 7.67× |
| Instruments (minified) | 13,037 | 1,749 | 7.45× |
| Instruments (prettified) | 20,489 | 1,923 | 10.65× |
| Marine IK Reverse (minified) | 1,461 | 823 | 1.78× |
| Marine IK Reverse (prettified) | 6,107 | 2,894 | 2.11× |
| Marine IK (minified) | 1,309 | 707 | 1.85× |
| Marine IK (prettified) | 5,333 | 2,574 | 2.07× |
| Mesh (minified) | 2,522 | 1,202 | 2.10× |
| Mesh (prettified) | 4,535 | 1,877 | 2.42× |
| Random (minified) | 9,001 | 1,741 | 5.17× |
| Random (prettified) | 11,855 | 1,990 | 5.96× |
| Twitter (minified) | 13,362 | 3,483 | 3.84× |
| Twitter (prettified) | 17,882 | 3,557 | 5.03× |
| Canada Small (minified) | 2,317 | 1,295 | 1.79× |
| Canada Small (prettified) | 6,815 | 2,959 | 2.30× |
| CitmCatalog Small (minified) | 7,693 | 1,539 | 5.00× |
| CitmCatalog Small (prettified) | 15,034 | 2,048 | 7.34× |
| Discord Small (minified) | 9,475 | 2,263 | 4.19× |
| Discord Small (prettified) | 14,545 | 2,530 | 5.75× |
| Google Maps Response Small (minified) | 8,831 | 1,577 | 5.60× |
| Google Maps Response Small (prettified) | 15,791 | 2,200 | 7.18× |
| Instruments Small (minified) | 12,301 | 1,671 | 7.36× |
| Instruments Small (prettified) | 21,601 | 1,879 | 11.50× |
| Marine IK Reverse Small (minified) | 1,654 | 935 | 1.77× |
| Marine IK Reverse Small (prettified) | 6,636 | 2,590 | 2.56× |
| Marine IK Small (minified) | 1,683 | 802 | 2.10× |
| Marine IK Small (prettified) | 6,869 | 2,742 | 2.51× |
| Mesh Small (minified) | 2,120 | 1,014 | 2.09× |
| Mesh Small (prettified) | 4,345 | 1,500 | 2.90× |
| Random Small (minified) | 8,306 | 1,454 | 5.71× |
| Random Small (prettified) | 11,598 | 2,028 | 5.72× |
| Twitter Small (minified) | 9,327 | 2,704 | 3.45× |
| Twitter Small (prettified) | 12,630 | 2,841 | 4.45× |

| Reformatting test | Jsonifier | Glaze | simdjson (ondemand) | Lead |
| --- | --- | --- | --- | --- |
| Minify | 1,284 (n/c) | 1,444 (n/c) | 694 | 0.89× |
| Prettify | 2,968 | 1,483 | 491 | 2.00× |

### Reused output strings

With the string's capacity kept across iterations, Jsonifier is fastest in 231 of the 235 write tests, ties 1 and loses 3, all on GCC against the reflection writer: Bool on Linux / GCC, 3,080 vs 3,232 MB/s (0.95×), minified Marine IK Reverse Small on Linux / GCC, 1,319 vs 1,525 MB/s (0.86×), and String on macOS / GCC, 8,831 vs 9,676 MB/s (0.91×). Minified Marine IK Small on Linux / GCC is the tie (1,402 vs 1,495 MB/s). Thirteen tests across the five builds have at least one non-converged result (five freshly allocated, eight reused): twelve on the two macOS builds and one on Windows / MSVC, where Glaze did not converge on minified Random Small reused. Each is marked n/c in the tables. Four of them are Jsonifier's own: freshly allocated Minify on macOS / Clang, a loss, and three reused wins, prettified Marine IK Reverse Small on macOS / GCC and prettified Canada and Twitter on macOS / Clang.

| Platform / compiler | Write tests | Jsonifier fastest | Tied | Jsonifier lost |
| --- | --- | --- | --- | --- |
| Windows / MSVC 19.44 (i9-14900KF, AVX2) | 47 | 47 | 0 | 0 |
| Linux / Clang 24.0 (i9-14900KF, AVX2) | 47 | 47 | 0 | 0 |
| Linux / GCC 16.2 (i9-14900KF, AVX2) | 47 | 44 | 1 | 2 |
| macOS / GCC 16.2 (Apple M1, NEON) | 47 | 46 | 0 | 1 |
| macOS / Clang 23.1 (Apple M1, NEON) | 47 | 47 | 0 | 0 |
| **Total** | **235** | **231** | **1** | **3** |

#### Windows / MSVC 19.44 (i9-14900KF, AVX2)

| Test | Jsonifier | Glaze | Lead |
| --- | --- | --- | --- |
| Bool | 1,948 | 254 | 7.66× |
| Double | 828 | 312 | 2.65× |
| Int64 | 4,854 | 1,033 | 4.70× |
| String | 7,054 | 2,357 | 2.99× |
| Uint64 | 5,326 | 1,035 | 5.15× |
| Canada (minified) | 1,683 | 1,094 | 1.54× |
| Canada (prettified) | 5,200 | 3,092 | 1.68× |
| CitmCatalog (minified) | 8,414 | 4,863 | 1.73× |
| CitmCatalog (prettified) | 16,188 | 7,528 | 2.15× |
| Discord (minified) | 10,816 | 5,019 | 2.16× |
| Discord (prettified) | 13,134 | 5,098 | 2.58× |
| Google Maps Response (minified) | 9,430 | 4,060 | 2.32× |
| Google Maps Response (prettified) | 14,806 | 6,186 | 2.39× |
| Instruments (minified) | 12,918 | 4,498 | 2.87× |
| Instruments (prettified) | 16,688 | 5,175 | 3.22× |
| Marine IK Reverse (minified) | 995 | 741 | 1.34× |
| Marine IK Reverse (prettified) | 4,832 | 3,363 | 1.44× |
| Marine IK (minified) | 1,014 | 756 | 1.34× |
| Marine IK (prettified) | 4,970 | 3,356 | 1.48× |
| Mesh (minified) | 1,841 | 1,274 | 1.45× |
| Mesh (prettified) | 3,579 | 2,149 | 1.67× |
| Random (minified) | 9,121 | 3,213 | 2.84× |
| Random (prettified) | 12,710 | 4,514 | 2.82× |
| Twitter (minified) | 13,568 | 5,337 | 2.54× |
| Twitter (prettified) | 15,768 | 3,937 | 4.00× |
| Canada Small (minified) | 2,016 | 1,232 | 1.64× |
| Canada Small (prettified) | 5,674 | 3,274 | 1.73× |
| CitmCatalog Small (minified) | 7,998 | 4,571 | 1.75× |
| CitmCatalog Small (prettified) | 15,748 | 6,142 | 2.56× |
| Discord Small (minified) | 10,622 | 5,076 | 2.09× |
| Discord Small (prettified) | 13,241 | 4,623 | 2.86× |
| Google Maps Response Small (minified) | 9,529 | 4,012 | 2.38× |
| Google Maps Response Small (prettified) | 19,129 | 6,710 | 2.85× |
| Instruments Small (minified) | 11,417 | 3,860 | 2.96× |
| Instruments Small (prettified) | 17,932 | 4,029 | 4.45× |
| Marine IK Reverse Small (minified) | 1,613 | 980 | 1.65× |
| Marine IK Reverse Small (prettified) | 7,291 | 3,443 | 2.12× |
| Marine IK Small (minified) | 1,643 | 972 | 1.69× |
| Marine IK Small (prettified) | 6,834 | 3,343 | 2.04× |
| Mesh Small (minified) | 2,150 | 1,179 | 1.82× |
| Mesh Small (prettified) | 4,241 | 2,205 | 1.92× |
| Random Small (minified) | 9,969 | 4,121 (n/c) | 2.42× |
| Random Small (prettified) | 17,502 | 5,594 | 3.13× |
| Twitter Small (minified) | 11,725 | 5,892 | 1.99× |
| Twitter Small (prettified) | 14,837 | 5,408 | 2.74× |

| Reformatting test | Jsonifier | Glaze | simdjson (ondemand) | Lead |
| --- | --- | --- | --- | --- |
| Minify | 3,129 | 1,885 | 1,159 | 1.66× |
| Prettify | 3,648 | 2,376 | 933 | 1.54× |

#### Linux / Clang 24.0 (i9-14900KF, AVX2)

| Test | Jsonifier | Glaze | Lead |
| --- | --- | --- | --- |
| Bool | 4,588 | 835 | 5.50× |
| Double | 911 | 719 | 1.27× |
| Int64 | 5,484 | 2,437 | 2.25× |
| String | 12,215 | 5,691 | 2.15× |
| Uint64 | 5,594 | 2,582 | 2.17× |
| Canada (minified) | 1,601 | 996 | 1.61× |
| Canada (prettified) | 4,719 | 3,901 | 1.21× |
| CitmCatalog (minified) | 10,654 | 6,571 | 1.62× |
| CitmCatalog (prettified) | 17,851 | 7,705 | 2.32× |
| Discord (minified) | 14,832 | 6,361 | 2.33× |
| Discord (prettified) | 18,495 | 5,768 | 3.21× |
| Google Maps Response (minified) | 12,490 | 4,487 | 2.78× |
| Google Maps Response (prettified) | 19,173 | 5,436 | 3.53× |
| Instruments (minified) | 18,522 | 6,305 | 2.94× |
| Instruments (prettified) | 23,146 | 6,778 | 3.42× |
| Marine IK Reverse (minified) | 846 | 619 | 1.37× |
| Marine IK Reverse (prettified) | 6,135 | 4,080 | 1.50× |
| Marine IK (minified) | 868 | 623 | 1.39× |
| Marine IK (prettified) | 6,131 | 4,058 | 1.51× |
| Mesh (minified) | 1,773 | 1,054 | 1.68× |
| Mesh (prettified) | 4,083 | 1,786 | 2.29× |
| Random (minified) | 14,293 | 3,829 | 3.73× |
| Random (prettified) | 18,944 | 4,632 | 4.09× |
| Twitter (minified) | 20,488 | 7,294 | 2.81× |
| Twitter (prettified) | 22,879 | 6,489 | 3.53× |
| Canada Small (minified) | 1,513 | 1,007 | 1.50× |
| Canada Small (prettified) | 4,369 | 3,471 | 1.26× |
| CitmCatalog Small (minified) | 9,439 | 3,989 | 2.37× |
| CitmCatalog Small (prettified) | 18,119 | 4,604 | 3.94× |
| Discord Small (minified) | 11,940 | 4,086 | 2.92× |
| Discord Small (prettified) | 16,537 | 3,348 | 4.94× |
| Google Maps Response Small (minified) | 11,778 | 3,868 | 3.05× |
| Google Maps Response Small (prettified) | 23,001 | 5,042 | 4.56× |
| Instruments Small (minified) | 13,864 | 3,891 | 3.56× |
| Instruments Small (prettified) | 23,887 | 4,534 | 5.27× |
| Marine IK Reverse Small (minified) | 1,382 | 846 | 1.63× |
| Marine IK Reverse Small (prettified) | 7,127 | 4,477 | 1.59× |
| Marine IK Small (minified) | 1,468 | 829 | 1.77× |
| Marine IK Small (prettified) | 7,640 | 4,393 | 1.74× |
| Mesh Small (minified) | 1,804 | 997 | 1.81× |
| Mesh Small (prettified) | 4,233 | 1,775 | 2.39× |
| Random Small (minified) | 12,650 | 3,967 | 3.19× |
| Random Small (prettified) | 23,644 | 4,625 | 5.11× |
| Twitter Small (minified) | 10,001 | 5,582 | 1.79× |
| Twitter Small (prettified) | 13,210 | 4,647 | 2.84× |

| Reformatting test | Jsonifier | Glaze | simdjson (ondemand) | Lead |
| --- | --- | --- | --- | --- |
| Minify | 3,759 | 2,566 | 1,369 | 1.46× |
| Prettify | 5,338 | 4,193 | 1,487 | 1.27× |

#### Linux / GCC 16.2 (i9-14900KF, AVX2)

| Test | Jsonifier | Glaze | simdjson (reflection) | Lead |
| --- | --- | --- | --- | --- |
| Bool | 3,080 | 686 | 3,232 | 0.95× |
| Double | 888 | 717 | 651 | 1.24× |
| Int64 | 5,611 | 2,511 | 4,283 | 1.31× |
| String | 11,315 | 6,149 | 9,463 | 1.20× |
| Uint64 | 6,382 | 2,795 | 4,399 | 1.45× |
| Canada (minified) | 2,135 | 1,543 | 1,044 | 1.38× |
| Canada (prettified) | 6,276 | 3,853 | 382 | 1.63× |
| CitmCatalog (minified) | 12,828 | 7,320 | 11,327 | 1.13× |
| CitmCatalog (prettified) | 20,558 | 8,281 | 557 | 2.48× |
| Discord (minified) | 15,478 | 6,792 | 11,431 | 1.35× |
| Discord (prettified) | 20,398 | 7,188 | 512 | 2.84× |
| Google Maps Response (minified) | 17,156 | 4,605 | 10,596 | 1.62× |
| Google Maps Response (prettified) | 22,196 | 7,573 | 533 | 2.93× |
| Instruments (minified) | 20,601 | 6,265 | 15,772 | 1.31× |
| Instruments (prettified) | 26,388 | 7,249 | 477 | 3.64× |
| Marine IK Reverse (minified) | 1,322 | 1,002 | 881 | 1.32× |
| Marine IK Reverse (prettified) | 6,603 | 4,114 | 271 | 1.60× |
| Marine IK (minified) | 1,287 | 1,007 | 887 | 1.28× |
| Marine IK (prettified) | 6,423 | 4,118 | 295 | 1.56× |
| Mesh (minified) | 2,474 | 1,763 | 1,316 | 1.40× |
| Mesh (prettified) | 4,617 | 2,874 | 276 | 1.61× |
| Random (minified) | 15,633 | 4,103 | 11,540 | 1.35× |
| Random (prettified) | 20,492 | 5,704 | 513 | 3.59× |
| Twitter (minified) | 19,004 | 7,697 | 13,090 | 1.45× |
| Twitter (prettified) | 24,266 | 5,325 | 529 | 4.56× |
| Canada Small (minified) | 2,218 | 1,696 | 1,085 | 1.31× |
| Canada Small (prettified) | 6,813 | 3,733 | 412 | 1.83× |
| CitmCatalog Small (minified) | 9,233 | 4,100 | 7,311 | 1.26× |
| CitmCatalog Small (prettified) | 19,614 | 5,288 | 541 | 3.71× |
| Discord Small (minified) | 11,606 | 4,987 | 7,300 | 1.59× |
| Discord Small (prettified) | 16,894 | 4,612 | 533 | 3.66× |
| Google Maps Response Small (minified) | 14,872 | 4,235 | 8,691 | 1.71× |
| Google Maps Response Small (prettified) | 28,166 | 7,098 | 547 | 3.97× |
| Instruments Small (minified) | 13,667 | 4,697 | 8,460 | 1.62× |
| Instruments Small (prettified) | 23,095 | 3,413 | 518 | 6.77× |
| Marine IK Reverse Small (minified) | 1,319 | 1,287 | 1,525 | 0.86× |
| Marine IK Reverse Small (prettified) | 7,413 | 4,013 | 532 | 1.85× |
| Marine IK Small (minified) | 1,402 | 1,311 | 1,495 | 0.94× |
| Marine IK Small (prettified) | 7,213 | 3,925 | 507 | 1.84× |
| Mesh Small (minified) | 2,316 | 1,462 | 1,437 | 1.58× |
| Mesh Small (prettified) | 4,922 | 2,091 | 299 | 2.35× |
| Random Small (minified) | 14,330 | 4,658 | 8,248 | 1.74× |
| Random Small (prettified) | 24,988 | 5,705 | 552 | 4.38× |
| Twitter Small (minified) | 9,330 | 6,291 | 6,805 | 1.37× |
| Twitter Small (prettified) | 13,528 | 4,299 | 544 | 3.15× |

| Reformatting test | Jsonifier | Glaze | simdjson (ondemand) | Lead |
| --- | --- | --- | --- | --- |
| Minify | 3,254 | 2,393 | 1,402 | 1.36× |
| Prettify | 4,314 | 3,694 | 1,493 | 1.17× |

#### macOS / GCC 16.2 (Apple M1, NEON)

| Test | Jsonifier | Glaze | simdjson (reflection) | Lead |
| --- | --- | --- | --- | --- |
| Bool | 3,181 | 807 | 2,549 | 1.25× |
| Double | 735 | 461 | 639 | 1.15× |
| Int64 | 4,944 | 1,866 | 2,990 | 1.65× |
| String | 8,831 | 4,560 | 9,676 | 0.91× |
| Uint64 | 5,008 | 2,132 | 2,943 | 1.70× |
| Canada (minified) | 2,094 | 1,420 | 716 | 1.48× |
| Canada (prettified) | 6,014 | 3,559 | 241 | 1.69× |
| CitmCatalog (minified) | 8,484 | 1,985 | 7,241 | 1.17× |
| CitmCatalog (prettified) | 18,546 | 4,053 | 318 | 4.58× |
| Discord (minified) | 12,184 | 2,998 | 8,691 | 1.40× |
| Discord (prettified) | 13,711 | 2,789 | 341 | 4.92× |
| Google Maps Response (minified) | 10,592 | 1,899 | 8,440 | 1.26× |
| Google Maps Response (prettified) | 21,721 | 3,098 | 312 | 7.01× |
| Instruments (minified) | 13,789 | 2,000 | 7,865 | 1.75× |
| Instruments (prettified) | 16,395 | 1,639 | 315 | 10.00× |
| Marine IK Reverse (minified) | 1,297 | 775 | 712 | 1.67× |
| Marine IK Reverse (prettified) | 5,739 | 2,868 | 259 | 2.00× |
| Marine IK (minified) | 1,218 | 807 | 664 | 1.51× |
| Marine IK (prettified) | 6,009 | 2,909 | 267 (n/c) | 2.07× |
| Mesh (minified) | 2,365 | 1,127 | 967 | 2.10× |
| Mesh (prettified) | 4,391 | 1,532 | 201 | 2.87× |
| Random (minified) | 10,272 | 1,842 | 7,649 | 1.34× |
| Random (prettified) | 13,358 | 2,493 | 289 | 5.36× |
| Twitter (minified) | 13,566 | 4,150 | 9,114 | 1.49× |
| Twitter (prettified) | 16,393 | 4,031 | 398 | 4.07× |
| Canada Small (minified) | 2,318 | 1,567 | 787 | 1.48× |
| Canada Small (prettified) | 6,860 | 2,981 | 266 | 2.30× |
| CitmCatalog Small (minified) | 9,191 | 1,813 | 6,503 | 1.41× |
| CitmCatalog Small (prettified) | 17,394 | 3,115 | 324 | 5.58× |
| Discord Small (minified) | 11,912 | 2,792 | 8,521 | 1.40× |
| Discord Small (prettified) | 12,822 | 2,487 | 315 | 5.16× |
| Google Maps Response Small (minified) | 9,944 | 1,866 | 7,534 | 1.32× |
| Google Maps Response Small (prettified) | 20,266 | 2,913 | 306 | 6.96× |
| Instruments Small (minified) | 14,659 | 1,875 | 7,334 | 2.00× |
| Instruments Small (prettified) | 17,452 | 1,538 | 327 | 11.35× |
| Marine IK Reverse Small (minified) | 1,674 | 935 | 1,538 | 1.09× |
| Marine IK Reverse Small (prettified) | 6,889 (n/c) | 3,429 | 281 | 2.01× |
| Marine IK Small (minified) | 1,660 | 936 | 1,352 | 1.23× |
| Marine IK Small (prettified) | 7,413 | 3,417 | 269 | 2.17× |
| Mesh Small (minified) | 2,181 | 1,077 | 1,130 | 1.93× |
| Mesh Small (prettified) | 5,012 | 1,456 | 210 | 3.44× |
| Random Small (minified) | 11,145 | 2,066 | 7,867 | 1.42× |
| Random Small (prettified) | 15,139 | 2,674 | 295 | 5.66× |
| Twitter Small (minified) | 12,905 | 3,644 | 7,593 | 1.70× |
| Twitter Small (prettified) | 14,637 | 3,780 | 353 | 3.87× |

| Reformatting test | Jsonifier | Glaze | simdjson (ondemand) | Lead |
| --- | --- | --- | --- | --- |
| Minify | 2,112 | 1,598 | 855 | 1.32× |
| Prettify | 3,080 | 2,026 | 628 | 1.52× |

#### macOS / Clang 23.1 (Apple M1, NEON)

| Test | Jsonifier | Glaze | Lead |
| --- | --- | --- | --- |
| Bool | 2,140 | 644 | 3.32× |
| Double | 774 | 549 | 1.41× |
| Int64 | 3,234 | 1,933 | 1.67× |
| String | 6,982 | 1,250 | 5.58× |
| Uint64 | 2,673 | 1,815 | 1.47× |
| Canada (minified) | 1,945 | 1,232 (n/c) | 1.58× |
| Canada (prettified) | 4,283 (n/c) | 2,838 | 1.51× |
| CitmCatalog (minified) | 6,587 | 1,564 | 4.21× |
| CitmCatalog (prettified) | 16,381 | 2,760 | 5.93× |
| Discord (minified) | 10,442 | 2,384 | 4.38× |
| Discord (prettified) | 17,429 | 2,801 | 6.22× |
| Google Maps Response (minified) | 9,771 | 1,341 | 7.29× |
| Google Maps Response (prettified) | 22,357 | 2,961 | 7.55× |
| Instruments (minified) | 11,288 | 1,977 | 5.71× |
| Instruments (prettified) | 19,848 | 2,054 | 9.66× |
| Marine IK Reverse (minified) | 1,468 | 824 | 1.78× |
| Marine IK Reverse (prettified) | 5,577 | 3,046 | 1.83× |
| Marine IK (minified) | 1,384 | 689 | 2.01× |
| Marine IK (prettified) | 6,052 | 2,634 | 2.30× |
| Mesh (minified) | 2,655 | 1,235 | 2.15× |
| Mesh (prettified) | 4,779 | 1,859 | 2.57× |
| Random (minified) | 8,319 | 1,940 | 4.29× |
| Random (prettified) | 11,767 | 2,288 | 5.14× |
| Twitter (minified) | 14,071 | 3,487 | 4.03× |
| Twitter (prettified) | 15,530 (n/c) | 3,799 | 4.09× |
| Canada Small (minified) | 2,023 | 1,501 | 1.35× |
| Canada Small (prettified) | 6,518 | 3,405 | 1.91× |
| CitmCatalog Small (minified) | 7,226 | 1,798 | 4.02× |
| CitmCatalog Small (prettified) | 18,669 | 2,655 | 7.03× |
| Discord Small (minified) | 10,965 | 2,718 | 4.03× |
| Discord Small (prettified) | 16,329 | 2,973 | 5.49× |
| Google Maps Response Small (minified) | 7,274 | 1,751 | 4.15× |
| Google Maps Response Small (prettified) | 21,276 | 2,543 | 8.37× |
| Instruments Small (minified) | 12,698 | 1,980 | 6.41× |
| Instruments Small (prettified) | 22,279 | 2,130 | 10.46× |
| Marine IK Reverse Small (minified) | 1,901 | 1,022 | 1.86× |
| Marine IK Reverse Small (prettified) | 7,926 | 3,257 | 2.43× |
| Marine IK Small (minified) | 1,742 | 1,021 | 1.71× |
| Marine IK Small (prettified) | 6,881 | 3,062 | 2.25× |
| Mesh Small (minified) | 2,376 | 1,102 | 2.16× |
| Mesh Small (prettified) | 4,829 | 1,829 | 2.64× |
| Random Small (minified) | 8,993 | 1,796 | 5.01× |
| Random Small (prettified) | 12,737 | 1,944 (n/c) | 6.55× |
| Twitter Small (minified) | 11,899 | 3,367 | 3.53× |
| Twitter Small (prettified) | 16,182 | 3,734 | 4.33× |

| Reformatting test | Jsonifier | Glaze | simdjson (ondemand) | Lead |
| --- | --- | --- | --- | --- |
| Minify | 2,407 | 1,549 | 725 | 1.55× |
| Prettify | 2,971 | 1,851 | 514 (n/c) | 1.60× |

### Losses and ties

The prettified lead comes from the indent tables: a newline plus indent is a few 8-byte stores, not a loop. Linux / Clang wins every test in both runs. The seven freshly allocated losses:

- **Double, macOS / GCC.** The reflection writer runs 287 vs Jsonifier's 245 MB/s (0.85×). Jsonifier is ahead of Glaze (177 MB/s, 1.38×), and ahead of the reflection writer reused (735 vs 639 MB/s, 1.15×).
- **String, macOS / GCC.** The reflection writer runs 1,219 vs 1,180 MB/s (0.97×), and reused 9,676 vs 8,831 (0.91×), so this loss holds in both runs.
- **Uint64, macOS / GCC.** The reflection writer runs 762 vs 735 MB/s (0.96×). Reused, Jsonifier wins by 1.70× (5,008 vs 2,943).
- **Marine IK Small (minified), Linux / GCC.** The reflection writer runs 1,485 vs 1,355 MB/s (0.91×). Reused it is a tie (1,495 vs 1,402).
- **Marine IK Reverse Small (minified), Linux / GCC.** The reflection writer runs 1,501 vs 1,274 MB/s (0.85×), and reused 1,525 vs 1,319 (0.86×), a loss in both runs.
- **Prettify, Windows / MSVC.** Glaze runs 2,370 vs Jsonifier's 2,354 MB/s (0.99×), a gap Welch's test still separates. Reused, Jsonifier wins by 1.54× (3,648 vs 2,376).
- **Minify, macOS / Clang.** Glaze runs 1,444 vs 1,284 MB/s (0.89×), with neither result converged. Reused, Jsonifier wins by 1.55× (2,407 vs 1,549).

The one reused loss not listed above is Bool on Linux / GCC, where the reflection writer runs 3,232 vs Jsonifier's 3,080 MB/s (0.95×); freshly allocated, Jsonifier leads it by 8.58× (2,140 vs 249).

Every full-size document is ahead of the reflection writer on both GCC builds in both runs. Freshly allocated, minified documents lead it by 1.13–2.92× and prettified documents by 16–60×. The Small documents follow the full-size ones apart from the two minified Marine IK copies on Linux / GCC: Jsonifier wins 98 of the 100 freshly allocated and 98 of the 100 reused (one loss, one tie). Not profiled yet: why the reflection writer edges ahead on the number-heavy Marine IK Small documents on Linux / GCC and on String, Double and Uint64 on macOS / GCC is not known, and this paper does not guess at its algorithm.

**Reformatting.** Jsonifier wins Minify on four of the five builds freshly allocated, by 1.34–1.57×, and loses it on macOS / Clang; reused it wins on all five, by 1.32–1.66×. It wins Prettify on four of the five builds freshly allocated, by 1.17–2.00×, and loses it narrowly on Windows / MSVC; reused it wins on all five, by 1.17–1.60×. Glaze is the next-fastest library in every case.

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

The gap is a freshly allocated one. Freshly allocated, Jsonifier runs String at 1,180 MB/s on macOS / GCC (1.20× Glaze's 987, 0.97× the reflection writer's 1,219) and 1,026 MB/s on macOS / Clang (1.17× Glaze's 878). The AVX2 builds run the same test at 1,166–4,047 MB/s and lead Glaze by 1.29× (Windows / MSVC), 2.01× (Linux / Clang) and 1.94× (Linux / GCC). With the string reused the M1 lead over Glaze is 5.58× on macOS / Clang (6,982 vs 1,250 MB/s) and 1.94× on macOS / GCC (8,831 vs 4,560), against the 1.84–2.99× the AVX2 builds show reused. On macOS / GCC the reflection writer is ahead in both runs, at 9,676 MB/s reused.

So most of the M1 shortfall sits in the part of the freshly allocated run that the reused run removes, the allocation and release of the output string, rather than in the copy loop. That is an inference from the two runs, not a profile. Two features of the general loop remain candidates for whatever is left:

- Each block stops at its first escapable byte, writes that one escape, and reloads from the next byte. A string dense in escapes therefore pays one vector load per escape.
- Finding the first escapable byte needs a byte-mask extraction (`opBitMaskRaw`). x86 has a single instruction for this; NEON has to emulate it.

Neither has been isolated.

### Does simdjson's writer win doubles on GCC?

On macOS / GCC it does, freshly allocated: 287 vs Jsonifier's 245 MB/s (0.85×), separated by Welch's test. On Linux / GCC Jsonifier is ahead, 352 vs 343 MB/s (1.03×). Reused, Jsonifier is ahead on both, 888 vs 651 MB/s (1.36×) on Linux / GCC, where Glaze is the runner-up at 717, and 735 vs 639 (1.15×) on macOS / GCC. Jsonifier beats Glaze on the Double test on all five builds freshly allocated, by 1.12–1.43×. Why the reflection writer is faster freshly allocated on macOS / GCC has not been measured, and this paper does not guess at its algorithm.

### What to check on MSVC

The Windows build has landed (table above), but its assembly has not been inspected yet. Three things are worth confirming there:

1. The 10^8 reciprocal uses `_umul128` and stays in registers, with no stack temporary.
2. `resize_and_overwrite` compiles without iterator-debug checks. Release builds default to `_ITERATOR_DEBUG_LEVEL=0`, so this should hold unless the project overrides it.
3. The fixed-size scalar stores stay scalar and are not merged into wider unaligned vector stores that split cache lines.

## Limitations

- **Over-allocation.** The size bound is worst case: every number reserves 32 bytes and, unless `prescanStrings` is set, every string `6n + 2`. A string-heavy document asks for up to about 6× its output size before shrinking. That is cheap with `resize_and_overwrite` but costs peak memory on large payloads. `prescanStrings` tightens the string part to the exact escaped length at the cost of an extra pass over each string; numbers still reserve 32 bytes each.
- **Excluded keys are a runtime lookup.** Types with `jsonifierExcludedKeys` do a set lookup per member in both passes.
- **Registration required.** Like the parser, the fast path needs `jsonifier::core<T>`; there is no reflection-free serializer for arbitrary types.
