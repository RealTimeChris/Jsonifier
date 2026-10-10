# Jsonifier Serialization: Design and Performance

**Nihilai Collective Corp — Engineering Papers**  
*Nihilai Collective Corp*  
*October 2026 — Jsonifier*  

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

The `serialize<options>` dispatcher forces inlining (`JSONIFIER_INLINE`) only for POD values and optionals of them (`inline_contained_v`). Objects, maps and vectors go through a plain `inline` function, so the compiler can keep large container bodies out of line instead of expanding every nested type into its caller. Each registered type gets its own `serialize_impl` specialization: objects, maps, vectors, fixed arrays, tuples, strings, chars, enums, numbers, bools, pointers and optionals, raw JSON, null, and variants (via `visit`).

On x86 the whole serializer is compiled once per instruction-set tier, and `serializeJson` picks the tier at run time. CPUID and XCR0 are read once on first use and the result is cached; every call after that compares a cached enum against the compiled tiers in a fold expression and calls that tier's serializer directly. There are no function pointers or virtual calls, and everything described below is compile-time code inside each tier. ARM builds compile one backend.

## Key techniques

### Size first, then write without bounds checks

`get_size_impl` computes an upper bound for the output before a byte is written. Fixed-width types report a `staticSize` (numbers and enums 32, `bool` 5, `char` 8, null 4), so a `vector<double>` is sized as `n × 32` plus separators with no loop. Strings are sized at `6n + 2` by default, the worst case where every byte becomes a `\u00XX` escape. With `prescanStrings` set, each string is scanned instead and sized at its exact escaped length plus 2 for the quotes (see *Pre-scanning strings*). Object keys contribute `key.size() + 3` (or `+ 4` when prettified) as a compile-time constant.

`serializeJson` adds 64 bytes of slack and calls `resize_and_overwrite` when the buffer has it, so no zero-fill happens. The write pass then trusts the bound completely: no growth checks, no reallocation, and every writer may overstore (write 8 bytes, advance 4) into the slack.

### Writing through `resize_and_overwrite`

When the output buffer has C++23's `resize_and_overwrite` (detected by the `has_resize_and_overwrite` concept), the whole write pass runs inside it. `std::string::resize_and_overwrite(n, op)` grows the buffer to at least `n` without initializing it, calls `op(data, n)`, and sets the final size to whatever `op` returns. One call replaces the usual three steps: zero-filling resize, write, shrinking resize.

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

Object members are walked with CABERIHT, Constexpr Aggregated Bases for Efficient Runtime Iteration of Heterogeneous Types. It is CAFBERIHT without the filter; the full pattern, with compile-time filtering, is described in [its own paper](https://nihilai-collective.net/cafberiht). Each member becomes a base class, and one fold expression over those bases writes the whole object in declaration order.

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
| Runtime length | `call memcpy` (or an inline length dispatch with branches and a tail loop) | `bl memcpy` |
| Compile-time 7 bytes | two overlapping 4-byte moves, or 4 + 2 + 1 | several loads and stores |
| Compile-time 2 / 4 / 8 bytes | one `mov` load, one `mov` store | one `ldrh`/`ldr` and one `strh`/`str` |
| Compile-time 2 / 4 / 8 bytes of constant data | one immediate store, no load (`mov qword ptr [rdi], imm`) | a constant materialized in a register, then one `str` |

- **No call.** A call to `memcpy` clobbers every caller-saved register, so live values have to be spilled and reloaded around it. Without the call, the buffer pointer and the values being written stay in registers for the whole member loop.
- **No length branches.** A runtime `memcpy` has to dispatch on the length before it copies anything. A fixed-size copy has no branch to predict or mispredict.
- **Constants become immediates.** Key headers, `true`/`false` and the empty `{}`/`[]`/`""` literals are compile-time data. Their copies can fold into immediate stores, so writing a member header needs no memory read at all.
- **Inlining and scheduling.** Each copy is one or two instructions, so a whole object's serializer inlines into straight-line code. The compiler can then interleave stores from neighbouring members with the value work between them.

That is why the writers round up and overstore instead of copying exact lengths. A 5-byte `false` is one 8-byte store. A 3-digit group is a 4-byte load followed by a 2-byte and a 1-byte store. A 7-byte key literal is one 8-byte store. The pointer then advances by the true length, and the size pass's slack absorbs the extra bytes.

Key literals longer than 8 bytes copy 16 bytes, or a multiple of 32, through the plain wrapper. That size is still a compile-time constant. The only copies whose length is known only at runtime are off the common path: escape sequences inside strings (only when an escapable byte is found), raw JSON passthrough, and the memset for indents deeper than 8 levels.

### Booleans: one subtraction, one store

A `bool` becomes JSON with no branch and no table lookup. Two 64-bit constants are built at compile time, each in the native byte order:

- `falseVInt` holds the bytes `false` packed into one word (435728179558, `0x65736C6166` on little-endian).
- `trueVInt` holds the difference between the `false` word and the `true` word (434025983730), not the `true` word itself.

The writer computes `state = falseVInt - value × trueVInt`. When `value` is 0 the result is the `false` word. When it is 1 the difference cancels and the `true` word is left. One 8-byte store writes it, and the pointer advances by `5 - value`, so `true` moves 4 bytes and `false` moves 5. The extra bytes land in the slack the size pass reserved (5 bytes per bool plus 64 at the end) and are overwritten by whatever comes next.

On the Bool test, which serializes individual bools one at a time in a loop, this writer runs at 1,081–1,992 MB/s freshly allocated on all five builds, 8.5–14.4× Glaze's 89–209 MB/s. simdjson's reflection writer runs 249 MB/s on Linux / GCC and 197 MB/s on macOS / GCC, so the lead over it is 7.0× and 8.1×. With the string reused, the reflection writer passes Jsonifier on both GCC builds (3,074 against Jsonifier's 2,620 MB/s, 0.85×, on Linux / GCC and 2,738 against 2,164 MB/s, 0.79×, on macOS / GCC), which puts most of its freshly allocated cost in allocation rather than in the write.

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

On the Int64 and Uint64 tests, which serialize individual values one at a time in a loop, Jsonifier is ahead of Glaze freshly allocated on every build but one: Int64 by 1.05× on Linux / Clang, 1.13× on Linux / GCC, 1.38× on macOS / GCC, 1.39× on MSVC and 6.52× on macOS / Clang, and Uint64 by 1.00×, 1.30×, 1.38× and 5.65× on Linux / GCC, MSVC, macOS / GCC and macOS / Clang. The exception is Uint64 on Linux / Clang, where Glaze runs 915 against Jsonifier's 882 MB/s (0.96×). simdjson's reflection writer is ahead of it freshly allocated on Int64 on both GCC builds (956 against 923 MB/s on Linux / GCC, 733 against 686 on macOS / GCC) and on Uint64 on macOS / GCC (766 against 705), while Jsonifier takes Uint64 on Linux / GCC (893 against 879, 1.02×). With the string reused, Jsonifier is ahead of the fastest other library on both tests on every build, by 1.14× (Uint64, Linux / GCC) to 4.64× (Uint64, MSVC).

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

Jsonifier was fastest in 118 of the 125 converged serialization tests on five builds, with no ties and 7 losses. With the output string reused across iterations it was fastest in 122 of 125, with no ties and 3 losses. Every non-win is a single-value test; Jsonifier wins every converged document test on every build in both runs. Six of the seven freshly allocated losses, and all three reused losses, are against simdjson's reflection writer, which exists only on GCC: Double and Int64 on Linux / GCC, and Double, Int64, String and Uint64 on macOS / GCC. The seventh is Uint64 on Linux / Clang, where Glaze runs 915 MB/s against Jsonifier's 882; it is the only test in either run in which Jsonifier is behind Glaze. Reused, the reflection writer takes Bool on both GCC builds and String on macOS / GCC. Windows / MSVC and macOS / Clang win all of their tests in both runs, and Linux / Clang wins all 25 reused. Prettified documents are where the lead is widest: 1.34–10.23× over Glaze freshly allocated across the five builds, and 24–71× over the reflection writer on the two GCC builds.

These results come from the [Json-Performance](https://github.com/nihilai-collective/Json-Performance) sweep published October 11, 2026, with Glaze [e194d23](https://github.com/stephenberry/glaze/commit/e194d23), simdjson [7f6f8dc](https://github.com/simdjson/simdjson/commit/7f6f8dc) and BenchmarkSuite [125f8a7](https://github.com/nihilai-collective/benchmarksuite/commit/125f8a7) on every build. Jsonifier is [09747e9](https://github.com/nihilai-collective/jsonifier/commit/09747e9) on every build. Every test runs twice: once with a freshly allocated output string per iteration, and once with a reused one (see Method). Sampling and tie rules are listed under Method below and match those in *Two Stages, On Demand*. `simdjson (reflection)` is simdjson 5's C++26 `to_json`. It only runs on GCC, which has P2996 reflection. Its writer has no single-pass pretty mode, and its prettified results appear in the tables at a small fraction of the minified throughput. The Minify and Prettify tests reformat existing text instead of serializing objects, so they are left out of the counts.

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

**What is timed.** Every test runs twice. In the freshly allocated run, each iteration constructs a new output string, serializes into it, and destroys it, all inside the timed region, so allocation and deallocation are part of every measurement. In the reused run, labelled "(Reused)" in the sweep, the string is created once and held across iterations; it is cleared, keeping its capacity, outside the timed region before each iteration, so only the serialize work is measured. Both runs are tabulated below, freshly allocated first. The sweep's "Small" cut-down documents (at most 5 KiB minified) are not counted here. CPU caches are cleared before iterations. All libraries serialize the same test data.

**Throughput.** MB/s counts output bytes: each iteration is credited with the size of the JSON string that library produced, and throughput is total bytes over total time in the kept epoch window. MB here means 2²⁰ bytes. Output sizes can differ slightly between libraries for the same data, because each formats floats in its own way; on the Double test, for example, Glaze writes 1,798 bytes, Jsonifier 1,811 and simdjson's reflection writer 1,997. Each library is credited with its own output size.

**Sampling.**

1. Iterations start at 100 and double each epoch (100, 200, 400, …) up to 100,000.
2. Each epoch runs all its iterations and evaluates a trailing window of max(iterations / 10, 30) samples, capped at 100,000.
3. Sampling never stops early: epochs continue until 5 seconds have elapsed or the iteration cap is reached.
4. Every epoch after the first is scored by its relative standard error plus its epoch-over-epoch mean shift, and the lowest-scoring epoch is kept as the result.

**Convergence.** A kept epoch counts as converged only if its RSE is under 5% and its mean shift under 2.5% on the three x86 builds, or under 10% and 5% on the virtualized M1. A test is ranked only if every library in it converged, and tests that do not converge are left out of every ranking, which is why builds report different test counts.

**Ranking.** Variance is Bessel-corrected. Two libraries tie when Welch's t-test (two-sided, p < 0.05) cannot separate their kept epochs. Jsonifier is counted as a win when it ranks first and no other library is statistically tied with it, as a tie when it is statistically tied with another library at the top, and as a loss when another library ranks above it. Win, tie and loss counts use only converged results.

**Caveats.** Keeping the quietest epoch favours each library's least-disturbed stretch, which raises absolute throughput somewhat, but the rule applies identically to every library. The M1 numbers were taken in a virtual machine, so absolute MB/s may differ from bare metal; within a build, every library ran in the same environment.

| Platform / compiler | Write tests converged | Jsonifier fastest | Tied | Jsonifier lost |
| --- | --- | --- | --- | --- |
| Windows / MSVC 19.44 (i9-14900KF, AVX2) | 25 | 25 | 0 | 0 |
| Linux / Clang 24.0 (i9-14900KF, AVX2) | 25 | 24 | 0 | 1 |
| Linux / GCC 16.2 (i9-14900KF, AVX2) | 25 | 23 | 0 | 2 |
| macOS / GCC 16.2 (Apple M1, NEON) | 25 | 21 | 0 | 4 |
| macOS / Clang 23.1 (Apple M1, NEON) | 25 | 25 | 0 | 0 |
| **Total** | **125** | **118** | **0** | **7** |

Every converged write test per build follows, in MB/s, freshly allocated run first and reused run after it. Bool, Double, Int64, String and Uint64 serialize individual values one at a time in a loop. The rest serialize whole documents. *Lead* is Jsonifier's throughput divided by the fastest other library's, so a value under 1 is a loss.

### Windows / MSVC 19.44 (i9-14900KF, AVX2)

| Test | Jsonifier | Glaze | Lead |
| --- | --- | --- | --- |
| Bool | 1,081 | 89 | 12.15× |
| Double | 213 | 143 | 1.49× |
| Int64 | 523 | 377 | 1.39× |
| String | 1,197 | 932 | 1.28× |
| Uint64 | 524 | 403 | 1.30× |
| Canada (minified) | 1,184 | 807 | 1.47× |
| Canada (prettified) | 2,748 | 1,440 | 1.91× |
| CitmCatalog (minified) | 7,376 | 4,522 | 1.63× |
| CitmCatalog (prettified) | 4,416 | 1,930 | 2.29× |
| Discord (minified) | 7,001 | 4,442 | 1.58× |
| Discord (prettified) | 10,587 | 4,460 | 2.37× |
| Google Maps Response (minified) | 7,285 | 3,500 | 2.08× |
| Google Maps Response (prettified) | 13,995 | 5,409 | 2.59× |
| Instruments (minified) | 7,609 | 4,378 | 1.74× |
| Instruments (prettified) | 13,015 | 4,715 | 2.76× |
| Marine IK Reverse (minified) | 819 | 685 | 1.19× |
| Marine IK Reverse (prettified) | 2,763 | 1,812 | 1.52× |
| Marine IK (minified) | 822 | 599 | 1.37× |
| Marine IK (prettified) | 2,736 | 1,262 | 2.17× |
| Mesh (minified) | 1,283 | 917 | 1.40× |
| Mesh (prettified) | 2,061 | 1,535 | 1.34× |
| Random (minified) | 3,220 | 2,889 | 1.11× |
| Random (prettified) | 4,098 | 2,226 | 1.84× |
| Twitter (minified) | 8,033 | 4,479 | 1.79× |
| Twitter (prettified) | 10,746 | 3,600 | 2.98× |

### Linux / Clang 24.0 (i9-14900KF, AVX2)

| Test | Jsonifier | Glaze | Lead |
| --- | --- | --- | --- |
| Bool | 1,992 | 209 | 9.53× |
| Double | 334 | 301 | 1.11× |
| Int64 | 963 | 913 | 1.05× |
| String | 4,413 | 1,982 | 2.23× |
| Uint64 | 882 | 915 | 0.96× |
| Canada (minified) | 1,555 | 960 | 1.62× |
| Canada (prettified) | 4,411 | 3,248 | 1.36× |
| CitmCatalog (minified) | 10,325 | 5,671 | 1.82× |
| CitmCatalog (prettified) | 18,073 | 5,734 | 3.15× |
| Discord (minified) | 13,660 | 5,956 | 2.29× |
| Discord (prettified) | 17,620 | 5,094 | 3.46× |
| Google Maps Response (minified) | 11,350 | 4,070 | 2.79× |
| Google Maps Response (prettified) | 18,432 | 5,119 | 3.60× |
| Instruments (minified) | 18,132 | 5,375 | 3.37× |
| Instruments (prettified) | 22,674 | 5,690 | 3.98× |
| Marine IK Reverse (minified) | 883 | 617 | 1.43× |
| Marine IK Reverse (prettified) | 5,412 | 3,572 | 1.51× |
| Marine IK (minified) | 824 | 601 | 1.37× |
| Marine IK (prettified) | 5,970 | 3,053 | 1.96× |
| Mesh (minified) | 1,272 | 1,016 | 1.25× |
| Mesh (prettified) | 4,038 | 1,744 | 2.32× |
| Random (minified) | 14,503 | 3,574 | 4.06× |
| Random (prettified) | 18,303 | 4,060 | 4.51× |
| Twitter (minified) | 18,875 | 6,035 | 3.13× |
| Twitter (prettified) | 22,053 | 5,456 | 4.04× |

### Linux / GCC 16.2 (i9-14900KF, AVX2)

| Test | Jsonifier | Glaze | simdjson (reflection) | Lead |
| --- | --- | --- | --- | --- |
| Bool | 1,756 | 206 | 249 | 7.04× |
| Double | 332 | 309 | 348 | 0.96× |
| Int64 | 923 | 815 | 956 | 0.97× |
| String | 3,878 | 2,039 | 3,747 | 1.03× |
| Uint64 | 893 | 890 | 879 | 1.00× |
| Canada (minified) | 2,009 | 1,442 | 1,026 | 1.39× |
| Canada (prettified) | 5,904 | 3,092 | 176 | 1.91× |
| CitmCatalog (minified) | 12,401 | 6,260 | 11,273 | 1.10× |
| CitmCatalog (prettified) | 18,722 | 5,745 | 537 | 3.26× |
| Discord (minified) | 13,750 | 6,266 | 11,074 | 1.24× |
| Discord (prettified) | 18,744 | 6,160 | 531 | 3.04× |
| Google Maps Response (minified) | 11,151 | 3,922 | 10,115 | 1.10× |
| Google Maps Response (prettified) | 18,416 | 6,659 | 530 | 2.77× |
| Instruments (minified) | 19,295 | 5,638 | 16,024 | 1.20× |
| Instruments (prettified) | 24,714 | 6,232 | 490 | 3.97× |
| Marine IK Reverse (minified) | 1,243 | 983 | 878 | 1.27× |
| Marine IK Reverse (prettified) | 6,144 | 3,565 | 189 | 1.72× |
| Marine IK (minified) | 1,231 | 957 | 888 | 1.29× |
| Marine IK (prettified) | 6,124 | 2,907 | 191 | 2.11× |
| Mesh (minified) | 2,335 | 1,566 | 1,289 | 1.49× |
| Mesh (prettified) | 4,457 | 2,633 | 167 | 1.69× |
| Random (minified) | 14,312 | 3,675 | 11,518 | 1.24× |
| Random (prettified) | 18,573 | 4,826 | 539 | 3.85× |
| Twitter (minified) | 16,369 | 6,503 | 12,850 | 1.27× |
| Twitter (prettified) | 20,148 | 4,925 | 539 | 4.09× |

### macOS / GCC 16.2 (Apple M1, NEON)

| Test | Jsonifier | Glaze | simdjson (reflection) | Lead |
| --- | --- | --- | --- | --- |
| Bool | 1,587 | 110 | 197 | 8.07× |
| Double | 256 | 195 | 286 | 0.89× |
| Int64 | 686 | 497 | 733 | 0.94× |
| String | 1,194 | 1,023 | 1,226 | 0.97× |
| Uint64 | 705 | 510 | 766 | 0.92× |
| Canada (minified) | 1,874 | 1,365 | 668 | 1.37× |
| Canada (prettified) | 5,732 | 2,810 | 110 | 2.04× |
| CitmCatalog (minified) | 7,482 | 1,751 | 6,433 | 1.16× |
| CitmCatalog (prettified) | 16,482 | 3,130 | 233 | 5.27× |
| Discord (minified) | 10,081 | 2,540 | 7,535 | 1.34× |
| Discord (prettified) | 12,846 | 2,201 | 304 | 5.84× |
| Google Maps Response (minified) | 9,564 | 1,605 | 6,664 | 1.44× |
| Google Maps Response (prettified) | 14,789 | 2,688 | 286 | 5.50× |
| Instruments (minified) | 10,492 | 1,507 | 6,081 | 1.73× |
| Instruments (prettified) | 14,778 | 1,971 | 299 | 7.50× |
| Marine IK Reverse (minified) | 1,260 | 787 | 700 | 1.60× |
| Marine IK Reverse (prettified) | 5,981 | 2,673 | 130 | 2.24× |
| Marine IK (minified) | 1,201 | 756 | 684 | 1.59× |
| Marine IK (prettified) | 4,965 | 2,095 | 116 | 2.37× |
| Mesh (minified) | 2,200 | 973 | 879 | 2.26× |
| Mesh (prettified) | 3,555 | 1,440 | 84 | 2.47× |
| Random (minified) | 9,497 | 1,843 | 7,722 | 1.23× |
| Random (prettified) | 10,841 | 2,015 | 262 | 5.38× |
| Twitter (minified) | 12,665 | 2,969 | 9,058 | 1.40× |
| Twitter (prettified) | 9,198 | 1,347 | 391 | 6.83× |

### macOS / Clang 23.1 (Apple M1, NEON)

| Test | Jsonifier | Glaze | Lead |
| --- | --- | --- | --- |
| Bool | 1,337 | 109 | 12.31× |
| Double | 245 | 194 | 1.27× |
| Int64 | 2,838 | 435 | 6.52× |
| String | 885 | 662 | 1.34× |
| Uint64 | 2,287 | 405 | 5.65× |
| Canada (minified) | 2,050 | 1,315 | 1.56× |
| Canada (prettified) | 6,690 | 3,042 | 2.20× |
| CitmCatalog (minified) | 7,003 | 1,535 | 4.56× |
| CitmCatalog (prettified) | 14,718 | 2,416 | 6.09× |
| Discord (minified) | 8,877 | 1,977 | 4.49× |
| Discord (prettified) | 13,208 | 2,164 | 6.10× |
| Google Maps Response (minified) | 6,341 | 1,179 | 5.38× |
| Google Maps Response (prettified) | 16,376 | 2,024 | 8.09× |
| Instruments (minified) | 11,422 | 1,596 | 7.15× |
| Instruments (prettified) | 19,480 | 1,904 | 10.23× |
| Marine IK Reverse (minified) | 1,358 | 731 | 1.86× |
| Marine IK Reverse (prettified) | 6,317 | 2,605 | 2.42× |
| Marine IK (minified) | 1,284 | 699 | 1.84× |
| Marine IK (prettified) | 5,063 | 2,188 | 2.31× |
| Mesh (minified) | 2,104 | 1,070 | 1.97× |
| Mesh (prettified) | 4,979 | 1,781 | 2.80× |
| Random (minified) | 8,121 | 1,777 | 4.57× |
| Random (prettified) | 15,846 | 2,308 | 6.87× |
| Twitter (minified) | 10,983 | 3,175 | 3.46× |
| Twitter (prettified) | 16,827 | 3,534 | 4.76× |

### Reused output strings

With the string's capacity kept across iterations, Jsonifier is fastest in 122 of the 125 converged write tests, with no ties and three losses, all single-value tests against simdjson's reflection writer: Bool on Linux / GCC, and Bool and String on macOS / GCC. All five builds converge all 25 tests.

| Platform / compiler | Write tests converged | Jsonifier fastest | Tied | Jsonifier lost |
| --- | --- | --- | --- | --- |
| Windows / MSVC 19.44 (i9-14900KF, AVX2) | 25 | 25 | 0 | 0 |
| Linux / Clang 24.0 (i9-14900KF, AVX2) | 25 | 25 | 0 | 0 |
| Linux / GCC 16.2 (i9-14900KF, AVX2) | 25 | 24 | 0 | 1 |
| macOS / GCC 16.2 (Apple M1, NEON) | 25 | 23 | 0 | 2 |
| macOS / Clang 23.1 (Apple M1, NEON) | 25 | 25 | 0 | 0 |
| **Total** | **125** | **122** | **0** | **3** |

#### Windows / MSVC 19.44 (i9-14900KF, AVX2)

| Test | Jsonifier | Glaze | Lead |
| --- | --- | --- | --- |
| Bool | 1,573 | 270 | 5.83× |
| Double | 737 | 315 | 2.34× |
| Int64 | 4,132 | 1,051 | 3.93× |
| String | 7,461 | 2,449 | 3.05× |
| Uint64 | 4,890 | 1,053 | 4.64× |
| Canada (minified) | 1,516 | 1,085 | 1.40× |
| Canada (prettified) | 4,726 | 3,115 | 1.52× |
| CitmCatalog (minified) | 7,333 | 4,978 | 1.47× |
| CitmCatalog (prettified) | 14,840 | 7,508 | 1.98× |
| Discord (minified) | 7,444 | 5,319 | 1.40× |
| Discord (prettified) | 10,997 | 5,207 | 2.11× |
| Google Maps Response (minified) | 8,245 | 4,020 | 2.05× |
| Google Maps Response (prettified) | 15,706 | 6,590 | 2.38× |
| Instruments (minified) | 7,772 | 4,827 | 1.61× |
| Instruments (prettified) | 13,339 | 5,774 | 2.31× |
| Marine IK Reverse (minified) | 934 | 742 | 1.26× |
| Marine IK Reverse (prettified) | 4,655 | 3,385 | 1.37× |
| Marine IK (minified) | 936 | 757 | 1.24× |
| Marine IK (prettified) | 4,593 | 3,358 | 1.37× |
| Mesh (minified) | 1,605 | 1,281 | 1.25× |
| Mesh (prettified) | 3,031 | 2,168 | 1.40× |
| Random (minified) | 7,113 | 3,140 | 2.27× |
| Random (prettified) | 12,008 | 4,561 | 2.63× |
| Twitter (minified) | 8,392 | 5,394 | 1.56× |
| Twitter (prettified) | 10,906 | 4,326 | 2.52× |

#### Linux / Clang 24.0 (i9-14900KF, AVX2)

| Test | Jsonifier | Glaze | Lead |
| --- | --- | --- | --- |
| Bool | 3,125 | 897 | 3.48× |
| Double | 837 | 701 | 1.19× |
| Int64 | 4,865 | 2,408 | 2.02× |
| String | 11,790 | 5,804 | 2.03× |
| Uint64 | 5,208 | 2,533 | 2.06× |
| Canada (minified) | 1,556 | 997 | 1.56× |
| Canada (prettified) | 4,451 | 3,885 | 1.15× |
| CitmCatalog (minified) | 10,395 | 6,412 | 1.62× |
| CitmCatalog (prettified) | 18,125 | 7,714 | 2.35× |
| Discord (minified) | 13,989 | 6,153 | 2.27× |
| Discord (prettified) | 17,787 | 5,777 | 3.08× |
| Google Maps Response (minified) | 11,927 | 4,578 | 2.61× |
| Google Maps Response (prettified) | 19,202 | 5,613 | 3.42× |
| Instruments (minified) | 18,269 | 6,361 | 2.87× |
| Instruments (prettified) | 22,991 | 6,793 | 3.38× |
| Marine IK Reverse (minified) | 882 | 608 | 1.45× |
| Marine IK Reverse (prettified) | 5,385 | 3,994 | 1.35× |
| Marine IK (minified) | 827 | 623 | 1.33× |
| Marine IK (prettified) | 5,962 | 4,079 | 1.46× |
| Mesh (minified) | 1,272 | 1,057 | 1.20× |
| Mesh (prettified) | 4,038 | 1,829 | 2.21× |
| Random (minified) | 14,558 | 3,935 | 3.70× |
| Random (prettified) | 18,471 | 4,690 | 3.94× |
| Twitter (minified) | 19,023 | 7,139 | 2.66× |
| Twitter (prettified) | 22,137 | 6,194 | 3.57× |

#### Linux / GCC 16.2 (i9-14900KF, AVX2)

| Test | Jsonifier | Glaze | simdjson (reflection) | Lead |
| --- | --- | --- | --- | --- |
| Bool | 2,620 | 668 | 3,074 | 0.85× |
| Double | 825 | 733 | 652 | 1.12× |
| Int64 | 5,248 | 2,368 | 4,083 | 1.29× |
| String | 11,192 | 6,234 | 8,940 | 1.25× |
| Uint64 | 5,014 | 2,739 | 4,397 | 1.14× |
| Canada (minified) | 2,021 | 1,505 | 1,024 | 1.34× |
| Canada (prettified) | 5,809 | 3,788 | 177 | 1.53× |
| CitmCatalog (minified) | 12,567 | 7,435 | 11,264 | 1.12× |
| CitmCatalog (prettified) | 18,831 | 7,728 | 545 | 2.44× |
| Discord (minified) | 13,976 | 6,675 | 11,274 | 1.24× |
| Discord (prettified) | 19,097 | 7,341 | 529 | 2.60× |
| Google Maps Response (minified) | 11,581 | 4,556 | 10,520 | 1.10× |
| Google Maps Response (prettified) | 18,857 | 7,645 | 530 | 2.47× |
| Instruments (minified) | 19,377 | 6,471 | 15,438 | 1.26× |
| Instruments (prettified) | 24,894 | 7,516 | 493 | 3.31× |
| Marine IK Reverse (minified) | 1,234 | 998 | 879 | 1.24× |
| Marine IK Reverse (prettified) | 5,979 | 3,993 | 191 | 1.50× |
| Marine IK (minified) | 1,231 | 998 | 891 | 1.23× |
| Marine IK (prettified) | 6,140 | 4,024 | 202 | 1.53× |
| Mesh (minified) | 2,348 | 1,722 | 1,279 | 1.36× |
| Mesh (prettified) | 4,466 | 2,840 | 166 | 1.57× |
| Random (minified) | 14,143 | 4,041 | 11,542 | 1.23× |
| Random (prettified) | 18,175 | 5,759 | 524 | 3.16× |
| Twitter (minified) | 16,440 | 7,520 | 12,897 | 1.27× |
| Twitter (prettified) | 20,218 | 5,817 | 536 | 3.48× |

#### macOS / GCC 16.2 (Apple M1, NEON)

| Test | Jsonifier | Glaze | simdjson (reflection) | Lead |
| --- | --- | --- | --- | --- |
| Bool | 2,164 | 739 | 2,738 | 0.79× |
| Double | 817 | 385 | 669 | 1.22× |
| Int64 | 3,648 | 2,094 | 3,013 | 1.21× |
| String | 8,603 | 3,846 | 10,020 | 0.86× |
| Uint64 | 3,529 | 2,339 | 2,565 | 1.38× |
| Canada (minified) | 1,845 | 1,461 | 325 | 1.26× |
| Canada (prettified) | 5,536 | 2,171 | 107 | 2.55× |
| CitmCatalog (minified) | 7,584 | 1,750 | 6,404 | 1.18× |
| CitmCatalog (prettified) | 17,900 | 3,969 | 286 | 4.51× |
| Discord (minified) | 11,155 | 2,729 | 8,625 | 1.29× |
| Discord (prettified) | 12,374 | 2,673 | 326 | 4.63× |
| Google Maps Response (minified) | 9,923 | 1,731 | 7,426 | 1.34× |
| Google Maps Response (prettified) | 16,461 | 3,017 | 291 | 5.46× |
| Instruments (minified) | 11,345 | 1,630 | 6,335 | 1.79× |
| Instruments (prettified) | 14,889 | 2,201 | 298 | 6.77× |
| Marine IK Reverse (minified) | 1,240 | 781 | 695 | 1.59× |
| Marine IK Reverse (prettified) | 5,781 | 2,760 | 136 | 2.09× |
| Marine IK (minified) | 1,212 | 768 | 692 | 1.58× |
| Marine IK (prettified) | 5,152 | 2,381 | 114 | 2.16× |
| Mesh (minified) | 2,239 | 1,008 | 878 | 2.22× |
| Mesh (prettified) | 3,696 | 1,368 | 84 | 2.70× |
| Random (minified) | 10,358 | 1,983 | 7,774 | 1.33× |
| Random (prettified) | 10,794 | 2,156 | 258 | 5.01× |
| Twitter (minified) | 12,845 | 3,398 | 8,177 | 1.57× |
| Twitter (prettified) | 13,539 | 2,148 | 338 | 6.30× |

#### macOS / Clang 23.1 (Apple M1, NEON)

| Test | Jsonifier | Glaze | Lead |
| --- | --- | --- | --- |
| Bool | 1,830 | 620 | 2.95× |
| Double | 801 | 593 | 1.35× |
| Int64 | 2,841 | 2,110 | 1.35× |
| String | 5,331 | 3,981 | 1.34× |
| Uint64 | 3,390 | 1,905 | 1.78× |
| Canada (minified) | 2,049 | 1,406 | 1.46× |
| Canada (prettified) | 6,659 | 3,364 | 1.98× |
| CitmCatalog (minified) | 4,458 | 1,592 | 2.80× |
| CitmCatalog (prettified) | 15,127 | 2,789 | 5.42× |
| Discord (minified) | 10,506 | 2,272 | 4.63× |
| Discord (prettified) | 14,370 | 2,428 | 5.92× |
| Google Maps Response (minified) | 6,796 | 1,383 | 4.91× |
| Google Maps Response (prettified) | 18,346 | 2,784 | 6.59× |
| Instruments (minified) | 11,811 | 1,945 | 6.07× |
| Instruments (prettified) | 19,924 | 2,112 | 9.43× |
| Marine IK Reverse (minified) | 1,249 | 662 | 1.89× |
| Marine IK Reverse (prettified) | 5,890 | 2,690 | 2.19× |
| Marine IK (minified) | 1,343 | 741 | 1.81× |
| Marine IK (prettified) | 5,444 | 2,805 | 1.94× |
| Mesh (minified) | 1,899 | 1,017 | 1.87× |
| Mesh (prettified) | 4,244 | 1,643 | 2.58× |
| Random (minified) | 7,695 | 1,704 | 4.52× |
| Random (prettified) | 16,045 | 2,346 | 6.84× |
| Twitter (minified) | 12,459 | 3,685 | 3.38× |
| Twitter (prettified) | 17,456 | 4,102 | 4.26× |

### Losses

The prettified lead comes from the indent tables: a newline plus indent is a few 8-byte stores, not a loop. Windows / MSVC and macOS / Clang win every test in both runs, and so does Linux / Clang reused. The seven freshly allocated losses:

- **Uint64, Linux / Clang.** Glaze runs 915 vs Jsonifier's 882 MB/s (0.96×), a loss. Reused, Jsonifier is ahead, 5,208 vs 2,533 MB/s (2.06×).
- **Double, Linux / GCC.** Jsonifier runs 332 vs the reflection writer's 348 MB/s (0.96×), a loss. It is ahead of Glaze (309 MB/s, 1.07×), and ahead of the reflection writer reused (825 vs 652 MB/s, 1.27×).
- **Int64, Linux / GCC.** Jsonifier runs 923 vs 956 MB/s (0.97×), a loss. It is ahead of Glaze (815 MB/s, 1.13×), and ahead of the reflection writer reused (5,248 vs 4,083 MB/s, 1.29×).
- **Double, macOS / GCC.** Jsonifier runs 256 vs the reflection writer's 286 MB/s (0.89×), a loss. It is ahead of Glaze (195 MB/s, 1.31×), and ahead of the reflection writer reused (817 vs 669 MB/s, 1.22×).
- **Int64, macOS / GCC.** Jsonifier runs 686 vs 733 MB/s (0.94×), a loss. Reused it is ahead, 3,648 vs 3,013 MB/s (1.21×).
- **String, macOS / GCC.** Jsonifier runs 1,194 vs 1,226 MB/s (0.97×), a loss. It is ahead of Glaze (1,023 MB/s, 1.17×).
- **Uint64, macOS / GCC.** Jsonifier runs 705 vs 766 MB/s (0.92×), a loss. Reused it is ahead, 3,529 vs 2,565 MB/s (1.38×).

The three reused losses are single-value tests against the reflection writer: Bool on Linux / GCC, 2,620 vs 3,074 MB/s (0.85×); Bool on macOS / GCC, 2,164 vs 2,738 MB/s (0.79×); and String on macOS / GCC, 8,603 vs 10,020 MB/s (0.86×). Freshly allocated, Jsonifier wins Bool by 7.04× on Linux / GCC (1,756 vs 249 MB/s) and 8.07× on macOS / GCC (1,587 vs 197 MB/s).

Every document is ahead of the reflection writer on both GCC builds, freshly allocated and reused. String on Linux / GCC goes to Jsonifier in both runs, 3,878 vs 3,747 MB/s (1.03×) freshly allocated and 11,192 vs 8,940 (1.25×) reused. Minified documents lead by 1.10–2.80× freshly allocated (1.10× on CitmCatalog on Linux / GCC to 2.80× on Canada on macOS / GCC), and prettified documents by 24–71×. Not profiled yet: why the reflection writer edges ahead on single numeric values when the output string is freshly allocated, and on single Bool and String values when it is reused, is not known, and this paper does not guess at its algorithm.


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

The gap is a freshly allocated one. Freshly allocated, Jsonifier runs String at 1,194 MB/s on macOS / GCC (1.17× Glaze's 1,023, 0.97× the reflection writer's 1,226) and 885 MB/s on macOS / Clang (1.34× Glaze's 662). The AVX2 builds run the same test at 1,197–4,413 MB/s and lead Glaze by 1.28× (Windows / MSVC), 2.23× (Linux / Clang) and 1.90× (Linux / GCC). With the string reused the M1 lead over Glaze is 1.34× on macOS / Clang (5,331 vs 3,981 MB/s) and 2.24× on macOS / GCC (8,603 vs 3,846), against the 1.80–3.05× the AVX2 builds show reused. On macOS / GCC the reflection writer is still ahead reused, at 10,020 MB/s.

So most of the M1 shortfall sits in the part of the freshly allocated run that the reused run removes, the allocation and release of the output string, rather than in the copy loop. That is an inference from the two runs, not a profile. Two features of the general loop remain candidates for whatever is left:

- Each block stops at its first escapable byte, writes that one escape, and reloads from the next byte. A string dense in escapes therefore pays one vector load per escape.
- Finding the first escapable byte needs a byte-mask extraction (`opBitMaskRaw`). x86 has a single instruction for this; NEON has to emulate it.

Neither has been isolated.

### Does simdjson's writer win doubles on GCC?

On both GCC builds it wins the freshly allocated run: 348 vs Jsonifier's 332 MB/s on Linux / GCC (0.96×) and 286 vs 256 on macOS / GCC (0.89×). Reused, Jsonifier is ahead on both, 825 vs 652 MB/s (1.27×) on Linux / GCC and 817 vs 669 (1.22×) on macOS / GCC. Jsonifier beats Glaze on the Double test on all five builds freshly allocated, by 1.07–1.49×. Why the reflection writer is faster on the freshly allocated runs has not been measured, and this paper does not guess at its algorithm.

### What to check on MSVC

The Windows build has landed (table above), but its assembly has not been inspected yet. Three things are worth confirming there:

1. The 10^8 reciprocal uses `_umul128` and stays in registers, with no stack temporary.
2. `resize_and_overwrite` compiles without iterator-debug checks. Release builds default to `_ITERATOR_DEBUG_LEVEL=0`, so this should hold unless the project overrides it.
3. The fixed-size scalar stores stay scalar and are not merged into wider unaligned vector stores that split cache lines.

## Limitations

- **Over-allocation.** The size bound is worst case: every number reserves 32 bytes and, unless `prescanStrings` is set, every string `6n + 2`. A string-heavy document asks for up to about 6× its output size before shrinking. That is cheap with `resize_and_overwrite` but costs peak memory on large payloads. `prescanStrings` tightens the string part to the exact escaped length at the cost of an extra pass over each string; numbers still reserve 32 bytes each.
- **Excluded keys are a runtime lookup.** Types with `jsonifierExcludedKeys` do a set lookup per member in both passes.
- **Registration required.** Like the parser, the fast path needs `jsonifier::core<T>`; there is no reflection-free serializer for arbitrary types.
