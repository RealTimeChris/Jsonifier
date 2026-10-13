# Schema-Free, On Demand: Generic JSON Parsing in Jsonifier

**Nihilai Collective Corp: Engineering Papers**  
*Nihilai Collective Corp*  
*Jsonifier, October 2026*  

---

## Abstract

Jsonifier's two earlier papers were about parsing JSON into known types. If the parser knows the shape of the document at compile time, it can read in a single pass and skip building a structural tape. This paper covers the opposite case: **schema-free parsing**, where the caller walks a document of unknown shape and reads only the values it wants. That is the workload simdjson's On Demand API was built for, and the one where a structural index is actually worth building.

`jsonifier::generic` is a lazy, tape-driven parser. Its API is shaped like simdjson On Demand, but it relaxes one restriction: a value can be read more than once. This paper describes the design choices that let it match or beat On Demand on its own ground:

- a stage-1 reader tuned for small documents;
- a forward cursor that assumes reads happen in document order;
- zero-copy string extraction;
- a SIMD field index that is only built for objects that are actually being read out of order.

The benchmark suite reads every document four ways: every field in order, every field with each object's keys in reverse, a sparse subset of fields in order, and that subset in reverse. Each document is read both at full size and as a small (≤5 KiB) copy, and every single-document test runs twice: once filling freshly allocated output objects on every iteration, and once reusing the same cleared output objects, whose buffers keep their capacity. Across five platform/compiler builds, Jsonifier won 1,256 of 1,490 single-document results against simdjson On Demand, tied 69 and lost 165. On reverse-order reads it won 346 of 360. On Windows/MSVC it won 291 of 298 single-document results, tied 5 and lost 2.

The same parser also reads streams of many documents, such as NDJSON or comma-separated records, through `iterateMany`, the counterpart of simdjson's `iterate_many` (§8). Across five platform/compiler builds, on 26 streaming tests, Jsonifier won 110 of 130 results, tied 5 and lost 15, and won every streaming test on Windows/MSVC.

---

## 1. Background: why On Demand is hard to beat

simdjson's On Demand API does not build a DOM. Stage 1 produces a tape of structural positions, and stage 2 is a cursor over that tape driven by the caller. A value that is never requested is skipped over by structural depth, without being parsed. Reading in document order costs roughly one pass over the tape.

The price is a strictly forward-only API. Each value can be consumed once, and reading an object's fields out of order means rescanning the object for every key.

## 2. Design goals

1. **Keep On Demand's shape.** Existing On Demand code should port to `jsonifier::generic` mostly mechanically.
2. **Allow values to be read again.** A value handle remembers where it sits on the tape and can re-seek to it, instead of being invalidated.
3. **Make in-order reads free.** The common case must cost no more than a cursor increment and a key compare.
4. **Keep out-of-order reads close to in-order speed.** Their cost should stay close to linear in the size of the object, no matter what order the caller reads keys in.

## 3. Architecture

### 3.1 The tape

Stage 1 is Jsonifier's existing structural indexer (see *Two Stages, On Demand*, §4). It writes one `uint32_t` offset per structural character, and appends a sentinel entry `tape[count] = length`. Because of the sentinel, stage 2 never has to bounds-check the end of the tape on its hot paths.

Small documents are routed to a dedicated POD reader. When the input is shorter than one SIMD step, the fixed cost of setting up the full-width reader outweighs the actual work, and that reader's step sizes are chosen to fit the input.

### 3.2 A shared forward cursor

`document_state` holds one cursor into the tape and one depth counter. Every object, array and value handle refers to that shared state; a handle does not own its own iterator.

When a handle is used, it *settles* first:

- If the shared state is deeper than the handle, the state finishes the open containers up to the handle's depth.
- If the depth still doesn't match, the handle rewinds the cursor to its own opening position.

In-order reads never trigger either branch. Out-of-order reads pay only for the distance they actually move. On the hot paths, the cursor and depth are copied into locals, advanced there, and written back to the shared state once, so the compiler can keep them in registers.

### 3.3 Lazy errors

Accessors return `[[nodiscard]] error_code` and write their result to an out-parameter; they don't wrap results in a `result<T>`. A structural error is stored in the handle and passed along to every handle derived from it. `document::atEnd()` checks that the whole document was consumed. Errors therefore cost one well-predicted branch per access.

### 3.4 Finding fields

`findFieldUnordered(key)` runs in three tiers:

1. **The expected field.** In-order reads almost always find the requested key right at the cursor, possibly after skipping one comma. This tier needs one tape read and one key compare.
2. **Scan and wrap.** If the key isn't at the cursor, scan forward to the end of the object, then wrap around to the start of the object and scan up to where the cursor began.
3. **Escape-aware retry.** Only if both scans miss, rescan the object comparing keys with escape sequences decoded. Escaped keys are rare, so the common path never pays for decoding.

Once an object has shown that it is being read out of order, tier 2 is replaced by a probe of the field index described in §5.

### 3.5 Build-time CPU selection

Jsonifier compiles exactly one instruction-set tier per build, and the tier is chosen when CMake configures the project. At configure time, voided-hw-detection builds and runs a small detector program on the build machine. On x86 it reads CPUID leaves 1, 7 and 0x80000001 and checks XCR0 with `xgetbv`, so AVX, AVX2 and AVX-512 count only when the operating system saves their registers; AVX-512 additionally needs F, BW and VBMI2 together. On ARM it reads `HWCAP` on Linux and `sysctl` on macOS. The detector's output is cached in the build directory, so it runs once per configure, not once per build. CMake keeps the highest detected tier (AVX-512, then AVX2, then AVX on x86; SVE2 when its vector length is known, otherwise NEON on ARM), writes the selected feature bits into `simd/jsonifier_cpu_instructions.hpp`, and attaches the matching compiler flags to every translation unit that links Jsonifier. Setting `JSONIFIER_CPU_INSTRUCTIONS` at configure time replaces the detected bits, which is how cross-compiled builds pick their target; without it, a cross-compile skips detection and gets the scalar fallback. On x64, defining `JSONIFIER_MAX_AVX_TIER` caps the tier below what was detected, and `core/defines.hpp` refuses to compile if the translation unit's flags do not enable the configured tier. The whole library is then compiled once for that tier. `jsonifier_core` is a single class that inherits its parser, serializer, validator, minifier, prettifier and generic iterator through CRTP, so every call is a direct, inlinable call into code built for that ISA. There is no CPUID read, no cached backend value and no per-call check.

simdjson takes the opposite position for its core and DOM API. It compiles one kernel per ISA and dispatches through virtual functions: `implementation` exposes `create_dom_parser_implementation`, `minify` and `validate_utf8` as virtuals, and `dom_parser_implementation` declares `parse`, `stage1`, `stage2` and `parse_string` pure virtual. Its On Demand front end, the closer comparison for Jsonifier, is mostly fixed at compile time. Stage 1 and string unescaping are still dispatched at run time, but the On Demand code itself is compiled for the builtin implementation that the compiler flags select, and its documentation notes that the On Demand API "has limited runtime dispatch support" and recommends compiling for a specific x64 target.

The reason for one tier per build is inlining. Compiling several tiers into one binary needs per-function target attributes (`#pragma GCC target`, `#pragma clang attribute`), and those apply only to functions defined inside the region. A force-inlined, tier-specific callable passed to a standard-library template, such as the contexts Jsonifier hands to `std::basic_string::resize_and_overwrite`, is then called from code compiled at the baseline, and GCC and Clang reject that. Keeping `resize_and_overwrite` and the force-inlining is worth more than shipping several tiers in one binary, and a single tier per build keeps both. The cost is portability: a binary needs its configured tier on every machine that runs it, so a binary for mixed hardware is configured for the lowest tier it must support.

`jsonifier::generic` is compiled for the same tier as the rest of the library, so its stage 1 is the build's AVX-512, AVX2, AVX, NEON or SVE2 kernel, with no change to the calling code.

## 4. Zero-copy strings

`getString(std::string_view&)` returns a view straight into the input buffer whenever the string contains no escapes. It finds where the string ends with SIMD stop masks (a mask marking where a quote, backslash or control character appears):

- The first register is handled outside the loop ("peeled"), so short strings never enter the loop at all.
- The leftover bytes at the end are handled by a lookup table instead of a byte-by-byte loop.

Non-ASCII bytes go through UTF-8 validation on the view path. The `std::string` getter is split differently: pure-ASCII strings take the fast path, and everything else goes through the one-pass unescape-and-copy scanner. This keeps validation work off the path that already has to copy the bytes.

Strings that do contain escapes are decoded into an arena. Each string's tape offset maps to a fixed slot in the arena, so decoding the same value twice yields the same view. Earlier designs let the arena wrap around, which could overwrite views the caller still held; the fixed slots remove that failure.

**A GCC pitfall.** With `-funroll-loops`, GCC unrolls an un-peeled SIMD scan loop into a long prologue that runs even for short strings, which are most of the strings in the suite. The fix is to peel the first iteration by hand and use the table-driven tail described above.

## 5. The field index: making out-of-order reads cheap

Reading in reverse order is the worst case for tier 2 in §3.4. Each lookup scans past every key that comes after the target, so reading all *n* keys of an object in reverse costs O(n²).

### 5.1 The map

`field_index_map` is an insert-only, SwissTable-style hash map built specifically for this job:

- **Control bytes** come in 16-byte groups. An empty slot holds `-128`. An occupied slot holds `H2 = hash & 0x7F`, so every occupied control byte is non-negative.
- **Each entry** stores `{hash, object tape index, key tape index}`. It doesn't copy the key text; the key is compared against the tape directly.
- **The hash** mixes the key bytes with the object's tape index. The same key name in two different objects therefore hashes to different entries, and one map can serve the whole document. Short keys go through a single `mix64` mixing step; keys longer than 16 bytes are hashed a block at a time, murmur-style.
- **Probing** compares a whole control group against `H2` in one step, through the library's own SIMD wrappers (`gatherValuesU`, `opCmpEqRaw`, `opBitMaskRaw`), which lower to:
  - x86: `_mm_cmpeq_epi8` + `_mm_movemask_epi8`, one bit per slot;
  - NEON: `vceqq_u8` + a `vshrn`-narrowed nibble mask, four bits per slot, since NEON has no movemask instruction;
  - SVE2 (128-bit vectors): `svcmpeq_u8`, packed into the same four-bits-per-slot nibble mask;
  - any other target: the library's scalar SIMD emulation.
- **Reset** between documents clears the control bytes with `memset`, and only if the map was actually used for the previous document.

### 5.2 The lesson: whole-document indexing lost

The first version recorded every key the scanner skipped past, in every object. On Marine IK Reverse that made the parser **2.3x slower** than having no index at all, because Marine IK consists mostly of small objects with 3–6 fields each. For objects that small, re-scanning a few keys is cheaper than hashing, inserting and probing them.

### 5.3 Waiting until an object earns an index

The first fix was a counter on each object handle:

- The handle counts how many keys it has scanned past.
- Until that count reaches a threshold *T*, lookups use the plain scan-and-wrap path.
- After that, later scans record every key they pass into the map, and later lookups probe the map first.

Small objects never touch the map; large objects get O(1) lookups after one pass over their keys. The first A/B of that counter, run with *T* = 16:

| Test | Compiler | No index | Whole-doc index | Index after 16 skipped keys | vs simdjson |
|---|---|---|---|---|---|
| Marine IK Reverse (min) | GCC 16 | 7.49 ms | 17.1 ms | 6.24 ms | +79% |
| Marine IK Reverse (pretty) | GCC 16 | 8.77 ms | 18.0 ms | 7.42 ms | +61% |
| Marine IK Reverse (min) | Clang 24 | 8.03 ms | 19.2 ms | 6.48 ms | +42% |
| Marine IK Reverse (pretty) | Clang 24 | 8.86 ms | 20.0 ms | 7.25 ms | +40% |

*The "no index" and "index after 16 skipped keys" columns come from one A/B run and the "whole-doc index" column from another; each value is the median of 3 rounds, built with the CI flags. The "vs simdjson" column compares the "index after 16 skipped keys" build against simdjson in that same run.*

### 5.4 Choosing the threshold: a parameter sweep

*T* trades two costs. Set too high, objects with many fields rescan for longer before the map activates. Set too low, medium-sized objects pay for recording scans (hashing and inserting every key they pass) that a short rescan would have beaten. We swept *T* ∈ {2, 4, 8, 16} on four builds, running the full suite at each value, with the counter-only trigger above. The table shows Jsonifier's reverse-order margin over simdjson on the two tests that are sensitive to the threshold:

| Build | Test | *T* = 16 | *T* = 8 | *T* = 4 | *T* = 2 |
|---|---|---|---|---|---|
| Linux / GCC | Marine IK Reverse (min) | +74% | +82% | +11% | +39% |
| Linux / GCC | Random Reverse (min) | +29% | +37% | +42% | **−26%** |
| Linux / Clang | Marine IK Reverse (min) | +39% | +51% | **−14%** | +13% |
| Linux / Clang | Random Reverse (min) | +34% | +39% | +55% | **−25%** |
| macOS / Clang | Marine IK Reverse (min) | n/c | +55% | **−6%** | n/c |
| macOS / Clang | Random Reverse (min) | tie | n/c | +43% | **−27%** |
| macOS / GCC | Random Reverse (min) | +32% | +35% | n/c | **−38%** |

*"n/c" means the result did not converge in that run.*

Two failure modes repeat across builds:

- **At *T* = 2, Random Reverse loses on all four builds** where it converged. Random's objects are mid-sized, so almost every one crosses the threshold and pays for recording scans it didn't need.
- **At *T* = 4, Marine IK Reverse loses on two of the three builds** where it converged, for the same reason. On the third, Linux/GCC, it still wins, but its margin falls from +82% at *T* = 8 to +11%.

*T* = 8 is the lowest value with no reverse-order losses on any build. It also matched or beat *T* = 16 on most reverse tests: on Linux/Clang it lifted CitmCatalog Reverse from +85% to +106% and Discord Reverse from +307% to +365%, and in the sweep runs it turned Linux/Clang's in-order record from 21-0-2 into 22-1-0. (The sweep runs are separate from the runs reported in §7.)

The threshold is therefore chosen per OS/compiler, the same way Jsonifier chooses its stage-1 step geometry:

| Build | *T* | Basis |
|---|---|---|
| Linux / GCC | 8 | swept |
| Linux / Clang | 8 | swept |
| macOS / GCC | 8 | swept |
| macOS / Clang | 16 | swept; 8 and 16 had no reverse losses and could not be separated on this host |
| Windows / MSVC | 8 | not swept; uses the value that won on the other builds |

On macOS/Clang, 16 kept larger margins on the documents whose objects have many fields (Twitter, Discord, Mesh), while 8 won on Random, Google Maps and Instruments. Neither produced a reverse loss.

### 5.5 Requiring evidence of out-of-order access

A skipped-key count cannot tell a large object read out of order from a large object read sparsely in order. A sparse reader that asks for three fields of a 30-field object skips most of its keys on the way, so the count crosses *T* even though every lookup is found ahead of the cursor. Recording keys during those skips costs a hash and an insert per key for an index that no lookup needs.

The trigger in the measured code (§6) therefore asks for two kinds of evidence, both kept per object handle:

- **Size:** the skipped-key count has reached *T*, chosen per build as in §5.4. This is the gate that keeps small objects such as Marine IK's off the map.
- **Out-of-order access:** at least two lookups on this object have missed the expected field, scanned to the end of the object, and wrapped around to its start. A wrap is the event that only out-of-order reads produce. One wrap is not enough: a sparse reverse read of three fields often wraps once on its last lookup, whose key is first in the object, and an index built then would serve no further lookup.

Until both hold, lookups use scan-and-wrap and record nothing, so skips never pay for hashing. Once both hold, the next lookup that misses the expected field builds the object's index in one dedicated pass over the whole object, recording every key, and then probes it. Every later out-of-order lookup on that object is a single probe; a key the map does not hold falls through to the escape-aware retry of §3.4.

*T* was not re-swept after the wrap gate was added, and the wrap count of 2 is fixed on every build; it was not swept.

## 6. Methodology

- **Harness:** Json-Performance, branch `generic-parsing-main`, using the adaptive-sampling benchmark stage.
- **Tests:** 5 POD tests, plus 9 documents each run in minified and prettified form under four access patterns, at two sizes. Every test runs twice (see *Allocation* below), which gives 154 single-document results per build at full size and another 144 for the small documents.
  - **In order:** every field, keys requested in document order.
  - **Reverse:** every field, with every object's keys requested in reverse document order.
  - **Sparse:** a few fields from each record of the document's main arrays (for example a status's text, retweet count and user screen name in Twitter, or each performance's id, start and venue code in CitmCatalog); every other value is skipped by each library.
  - **Sparse reverse:** the same subset, requested in reverse document order.
- **Document sizes:** the full-size tests read the corpus documents as they are (from 11.8 KB for Google Maps up to 2.1 MB for Canada, minified). The *Small* tests read a truncated copy of each document: every array, and every object keyed by numbers, is cut to the same number of elements, and that number is the largest one that keeps the minified document at or under 5 KiB. The prettified small documents are the same values printed with indentation. Small documents put more weight on per-document setup, stage 1's fixed costs and the first lookups into each object, and less on steady-state throughput.
- **Allocation: fresh and reused.** Every single-document test runs twice for both libraries. In the **fresh** run, each iteration fills a newly constructed output struct, so every string, vector and nested object it holds is allocated again on every parse. In the **reused** run (`(Reused)` in the harness), one output struct lives across iterations and is cleared before each one, so its strings and vectors keep their capacity and the parse writes into buffers that are already allocated. In both runs each library keeps one parser for the whole test, so its tape, string buffer and other internal buffers are reused, as a long-running application would reuse them. The streaming tests (§8) run fresh only.
- **Readers:** the reverse and sparse readers are hand-written mirrors of the in-order readers. Both libraries fill the same structs through the same field-by-field traversal and use their unordered field lookup.
- **Correctness:** every test's output was serialized from both libraries and compared byte-for-byte, and all 515 pairs (103 tests, single-document and streaming, on each of the 5 builds) are identical. That comparison ran on Jsonifier `3e7bf6d`, before the small-document and reused tests were added, and it has not been re-run on the measured commit.
- **Convergence:** a result counts only if its retained epoch meets the RSE and mean-shift limits; non-converged results stay in the tallies and are marked (n/c) in the tables. Eighty-one of the 1,490 single-document results and 33 of the 130 streaming results did not converge (see §9). The x86 builds used RSE < 5% and shift < 2.5%. The NEON builds ran on virtualized Apple M1 hosts and used RSE < 10% and shift < 5%.
- **Integer parsing:** on every build except Windows/MSVC, Jsonifier reads integers with a SWAR parser ported from void-numerics, which consumes digits in 8-, 4-, 2- and 1-byte chunks instead of one at a time. The MSVC build uses an unrolled, bounds-checked digit parser instead.
- **Builds:**

| Build | Stage-1 kernel | Converged, full size | Converged, small | Converged, total |
|---|---|---|---|---|
| Windows 10.0.26200 / MSVC 19.44 | AVX2 | 150 / 154 | 144 / 144 | 294 / 298 |
| Linux 7.0.0 / Clang 24.0 | AVX2 | 154 / 154 | 144 / 144 | 298 / 298 |
| Linux 7.0.0 / GCC 16.2 | AVX2 | 151 / 154 | 144 / 144 | 295 / 298 |
| macOS 25.6 (M1, virtual) / GCC 16.2 | NEON | 118 / 154 | 137 / 144 | 255 / 298 |
| macOS 25.6 (M1, virtual) / Clang 23.1 | NEON | 128 / 154 | 139 / 144 | 267 / 298 |

All builds ran BenchmarkSuite `e19dd21` and Jsonifier `769b6b0`; simdjson is `45fecc2` on Windows/MSVC and `1f24df3` on the other four builds. Each build compiles one stage-1 tier, chosen at configure time (§3.5), and the table lists the kernel each build ran. The x86 builds ran on an Intel Core i9-14900KF. The results were published to `generic-parsing-main` on October 21, 2026.

## 7. Results

The percentages below are Jsonifier's throughput relative to simdjson On Demand. "tie" means Welch's t-test could not separate the two; "(n/c)" marks a value that did not converge, and it stays in the tallies. In the per-test tables, each cell gives the **fresh** result first and the **reused** result second.

### 7.1 Summary

**Fresh output objects:**

| Build | POD | In order | Reverse | Sparse | Sparse reverse | Small, in order | Small, reverse | Small, sparse | Small, sparse reverse | Total |
|---|---|---|---|---|---|---|---|---|---|---|
| Windows / MSVC | 5-0-0 | 16-2-0 | 18-0-0 | 18-0-0 | 18-0-0 | 17-0-1 | 18-0-0 | 18-0-0 | 18-0-0 | 146-2-1 |
| Linux / Clang | 5-0-0 | 17-0-1 | 18-0-0 | 16-0-2 | 17-0-1 | 11-1-6 | 18-0-0 | 13-0-5 | 16-0-2 | 131-1-17 |
| Linux / GCC | 5-0-0 | 10-0-8 | 16-1-1 | 16-1-1 | 17-0-1 | 8-0-10 | 16-0-2 | 9-0-9 | 18-0-0 | 115-2-32 |
| macOS / GCC | 5-0-0 | 11-4-3 | 17-1-0 | 11-4-3 | 16-1-1 | 14-2-2 | 18-0-0 | 17-1-0 | 17-1-0 | 126-14-9 |
| macOS / Clang | 4-0-1 | 14-2-2 | 16-2-0 | 11-2-5 | 13-0-5 | 15-0-3 | 17-1-0 | 11-0-7 | 12-2-4 | 113-9-27 |
| **All** | **24-0-1** | **68-8-14** | **85-4-1** | **72-7-11** | **81-1-8** | **65-3-22** | **87-1-2** | **68-1-21** | **81-3-6** | **631-28-86** |

**Reused output objects:**

| Build | POD | In order | Reverse | Sparse | Sparse reverse | Small, in order | Small, reverse | Small, sparse | Small, sparse reverse | Total |
|---|---|---|---|---|---|---|---|---|---|---|
| Windows / MSVC | 5-0-0 | 16-1-1 | 18-0-0 | 18-0-0 | 18-0-0 | 16-2-0 | 18-0-0 | 18-0-0 | 18-0-0 | 145-3-1 |
| Linux / Clang | 5-0-0 | 17-0-1 | 18-0-0 | 16-0-2 | 17-0-1 | 13-1-4 | 18-0-0 | 12-1-5 | 17-0-1 | 133-2-14 |
| Linux / GCC | 5-0-0 | 12-0-6 | 16-1-1 | 15-0-3 | 18-0-0 | 8-0-10 | 16-0-2 | 11-0-7 | 18-0-0 | 119-1-29 |
| macOS / GCC | 5-0-0 | 11-4-3 | 17-0-1 | 15-2-1 | 15-2-1 | 14-3-1 | 18-0-0 | 15-2-1 | 16-1-1 | 126-14-9 |
| macOS / Clang | 4-0-1 | 7-10-1 | 18-0-0 | 12-3-3 | 10-4-4 | 15-0-3 | 17-1-0 | 11-2-5 | 8-1-9 | 102-21-26 |
| **All** | **24-0-1** | **63-15-12** | **87-1-2** | **76-5-9** | **78-6-6** | **66-6-18** | **87-1-2** | **67-5-18** | **77-2-11** | **625-41-79** |


Each cell is wins-ties-losses.

Across all 1,490 single-document results (81 of them did not converge; see §9), Jsonifier won 1,256, tied 69 and lost 165: 631-28-86 with fresh output objects and 625-41-79 with reused ones.

- **Reverse order is almost all wins at both sizes.** Across fresh and reused runs, Jsonifier won 172 of 180 full-size reverse results (5 ties, 3 losses) and 174 of 180 small reverse results (2 ties, 4 losses). All 7 losses are on Canada: prettified Canada on Linux/GCC in both runs (−8.7% and −6.5%) and on macOS/GCC reused (−9.9%), and Canada Small on Linux/GCC in both forms and both runs (−4.7% to −6.8%). Four of the 7 ties are also Canada, and the other three are Google Maps on macOS/Clang.
- **Windows/MSVC lost 2 of 298 single-document results:** minified Canada read in order with reused objects (−2.7%) and minified Canada Small read in order with fresh objects (−1.5%). Its 5 ties are also Canada read in order: minified (fresh), prettified (fresh and reused), and minified and prettified Canada Small (reused). It won all 10 POD results and every reverse, sparse and sparse-reverse test, at both sizes and in both allocation modes.
- **Reusing the output objects helps both libraries and changes few verdicts.** Comparing each test's reused run with its fresh run, the median speedup from reuse is 6.1–10.3% for Jsonifier and 4.0–7.6% for simdjson, depending on the build. Jsonifier's margin over simdjson moves by a median +0.5 to +2.4 percentage points on the non-MSVC builds and by +8.8 points on MSVC, where allocation is most expensive: Discord Reverse (min) goes from +1532.0% to +1712.7%, and Mesh Small Sparse (min) from +120.5% to +187.0%.

### 7.2 POD tests

| Test | MSVC | Linux Clang | Linux GCC | macOS GCC | macOS Clang |
|---|---|---|---|---|---|
| Bool | +59.7% / +60.6% | +39.3% / +52.5% | +34.4% / +31.9% | +17.3% / +26.2% | +5.1% / +9.2% |
| Double | +29.9% / +41.3% | +69.8% / +70.1% | +51.7% / +52.6% | +62.3% / +85.9% | +49.3% / +60.0% |
| Int64 | +51.3% / +63.9% | +66.8% / +110.5% | +33.1% / +36.2% | +40.7% (n/c) / +60.4% | +12.7% / +30.4% |
| String | +2.0% / +17.4% | +3.2% / +8.8% | +3.0% / +6.1% | +6.1% / +21.9% | −11.8% / −14.9% |
| Uint64 | +59.2% / +60.7% | +62.8% / +63.8% | +51.6% / +53.6% | +70.3% / +42.9% | +36.0% / +23.1% |

### 7.3 Documents, in order

| Test | MSVC | Linux Clang | Linux GCC | macOS GCC | macOS Clang |
|---|---|---|---|---|---|
| Canada (min) | tie / −2.7% | +4.4% / +5.1% | −6.3% / −10.6% | +18.5% (n/c) / +22.5% | +18.9% (n/c) / +12.8% |
| Canada (pretty) | tie / tie | +5.2% / +5.7% | −4.6% / −5.7% | tie (n/c) / tie | +25.3% / tie |
| CitmCatalog (min) | +45.3% / +51.4% | +7.5% / +8.1% | +8.2% / +7.7% | +8.2% (n/c) / +10.0% | +5.6% / tie |
| CitmCatalog (pretty) | +41.6% / +47.4% | +4.8% / +6.2% | +5.6% / +7.5% | −2.4% / +7.8% | +11.9% / +4.9% |
| Discord (min) | +57.1% / +66.6% | +7.6% / +18.0% | +3.5% / +10.5% | +22.5% / +27.5% | +9.1% / +26.4% |
| Discord (pretty) | +47.6% / +72.7% | +9.6% / +12.8% | +5.0% / +9.1% | +9.8% / +29.0% | +1.7% / +5.8% |
| Google Maps (min) | +37.8% / +41.3% | +17.8% / +21.2% | +14.3% / +14.4% | +28.2% / +23.1% | −4.7% / +4.3% |
| Google Maps (pretty) | +29.4% / +36.0% | +16.3% / +16.2% | +12.1% / +14.8% | +15.5% / +28.4% | +6.6% / tie |
| Instruments (min) | +58.7% / +69.9% | +7.7% / +7.9% | −30.5% / +16.8% (n/c) | +18.8% / +12.7% | +18.2% / tie |
| Instruments (pretty) | +60.5% / +70.5% | +5.0% / +4.8% | +15.6% / +1.7% | −6.7% / −8.4% | +8.4% (n/c) / tie |
| Marine IK (min) | +13.2% / +13.9% | +2.9% / +2.5% | −3.6% / −3.5% | +39.9% / tie (n/c) | tie (n/c) / −4.8% |
| Marine IK (pretty) | +14.6% / +18.6% | +2.9% / +3.0% | −2.1% / −5.0% | tie (n/c) / tie (n/c) | −6.6% / tie (n/c) |
| Mesh (min) | +49.0% / +57.8% | +0.3% / +0.5% | −5.5% / −3.1% | +4.4% / −3.0% | tie (n/c) / tie (n/c) |
| Mesh (pretty) | +46.0% / +54.6% | −0.6% / −0.7% | −6.5% / −6.6% | −26.0% / −11.7% | +11.0% / +6.6% |
| Random (min) | +51.4% / +67.1% | +26.3% / +32.2% | +23.4% / +28.2% | +24.2% / +57.2% | +16.5% / tie |
| Random (pretty) | +52.7% / +63.2% | +25.2% / +28.6% | +21.3% / +27.4% | +45.3% / +64.5% | +18.8% / +22.4% |
| Twitter (min) | +40.1% / +48.9% | +11.3% / +20.1% | +2.2% / +11.0% | tie (n/c) / +44.0% | +45.5% / tie |
| Twitter (pretty) | +32.4% / +42.3% | +8.6% / +16.9% | −0.4% / +3.3% | tie (n/c) / tie | +18.7% (n/c) / tie |

### 7.4 Documents, keys in reverse order

| Test | MSVC | Linux Clang | Linux GCC | macOS GCC | macOS Clang |
|---|---|---|---|---|---|
| Canada (min) | +18.2% / +19.2% | +2.5% / +4.1% | tie / tie | +23.3% (n/c) / +6.9% (n/c) | tie (n/c) / +13.9% (n/c) |
| Canada (pretty) | +16.6% / +16.3% | +4.1% / +7.3% | −8.7% / −6.5% | tie (n/c) / −9.9% | +4.3% / +3.8% |
| CitmCatalog (min) | +304.1% / +305.3% | +73.0% / +76.2% | +76.2% / +80.3% | +57.3% / +38.5% (n/c) | +74.7% (n/c) / +75.2% |
| CitmCatalog (pretty) | +299.1% / +312.6% | +67.5% / +71.0% | +71.5% / +77.0% | +50.6% (n/c) / +63.7% | +65.7% / +51.1% |
| Discord (min) | +1532.0% / +1712.7% | +252.1% / +282.3% | +312.9% / +347.5% | +351.2% / +355.9% | +302.8% / +300.3% |
| Discord (pretty) | +1527.2% / +1672.5% | +247.6% / +276.7% | +315.0% / +338.2% | +329.0% / +326.2% | +238.1% / +246.2% |
| Google Maps (min) | +105.1% / +113.7% | +8.7% / +8.4% | +21.6% / +23.1% | +49.7% / +47.7% | +4.8% / +12.1% |
| Google Maps (pretty) | +104.8% / +115.3% | +6.8% / +8.3% | +17.6% / +19.0% | +24.3% / +43.4% (n/c) | tie / +6.4% |
| Instruments (min) | +683.3% / +683.4% | +132.1% / +135.3% | +30.5% / +84.8% (n/c) | +130.5% / +127.0% | +131.5% (n/c) / +184.4% (n/c) |
| Instruments (pretty) | +639.7% / +722.5% | +119.8% / +123.7% | +94.4% / +130.5% | +159.3% / +202.9% (n/c) | +121.9% / +138.1% |
| Marine IK (min) | +317.0% (n/c) / +335.4% (n/c) | +37.2% / +37.3% | +68.4% / +70.4% | +60.6% / +58.3% (n/c) | +30.7% (n/c) / +46.2% (n/c) |
| Marine IK (pretty) | +299.9% (n/c) / +300.8% (n/c) | +28.9% / +28.8% | +47.7% / +49.4% | +55.2% (n/c) / +25.7% (n/c) | +38.5% (n/c) / +58.3% (n/c) |
| Mesh (min) | +357.9% / +397.5% | +32.8% / +36.4% | +46.5% / +51.7% | +113.5% / +63.4% | +55.5% / +64.6% |
| Mesh (pretty) | +340.8% / +383.5% | +30.7% / +33.4% | +40.6% / +44.1% | +41.8% (n/c) / +56.3% | +42.2% / +47.0% |
| Random (min) | +228.8% / +261.9% | +30.8% / +34.9% | +27.5% / +30.0% | +20.7% / +32.0% (n/c) | +35.2% / +36.1% |
| Random (pretty) | +226.7% / +252.9% | +28.9% / +31.7% | +24.9% / +28.8% | +51.9% (n/c) / +34.3% | +22.0% (n/c) / +24.9% |
| Twitter (min) | +679.6% / +764.4% | +208.4% / +220.4% | +131.5% / +140.4% | +86.7% (n/c) / +230.6% (n/c) | +159.1% (n/c) / +155.5% (n/c) |
| Twitter (pretty) | +669.1% / +741.7% | +189.2% / +199.0% | +120.5% / +129.0% | +179.9% / +178.5% | +155.6% / +156.0% |

### 7.5 Documents, sparse fields in order

| Test | MSVC | Linux Clang | Linux GCC | macOS GCC | macOS Clang |
|---|---|---|---|---|---|
| Canada (min) | +62.6% / +61.3% | +8.5% / +7.8% | +16.8% / +17.4% | +34.4% / +32.9% (n/c) | +22.3% / +16.3% |
| Canada (pretty) | +49.1% / +51.9% | +14.3% / +15.1% | +9.8% / +9.5% | +6.7% (n/c) / +20.8% | −9.5% / +6.3% |
| CitmCatalog (min) | +292.7% / +284.5% | +23.1% / +22.8% | +12.9% / +13.8% | +6.6% / +60.9% | +12.6% / +14.9% |
| CitmCatalog (pretty) | +199.5% / +208.5% | +19.4% / +19.5% | +12.9% / +11.7% | +19.1% / +11.4% | +19.5% / +18.4% |
| Discord (min) | +29.0% / +47.5% | −4.6% / −5.0% | +6.3% / +9.2% | +23.8% / +19.6% | −12.2% / +6.9% |
| Discord (pretty) | +29.9% / +40.3% | −3.0% / −3.3% | +5.3% / +7.7% | tie / +18.9% | −11.4% / tie |
| Google Maps (min) | +55.7% / +65.3% | +3.4% / +5.8% | −3.6% / −1.6% | −20.8% / tie (n/c) | −4.5% / −3.3% |
| Google Maps (pretty) | +53.8% / +59.5% | +5.7% / +6.1% | tie / −1.7% | tie / +13.2% | −2.0% / −4.4% |
| Instruments (min) | +40.0% / +41.0% | +3.0% / +4.3% | +2.1% / −6.0% (n/c) | −40.2% / +4.8% | tie / tie |
| Instruments (pretty) | +43.1% / +47.2% | +3.3% / +2.4% | +16.3% / +4.8% | tie / tie | +8.8% / tie |
| Marine IK (min) | +31.9% / +31.1% | +19.6% / +21.0% | +25.7% / +27.1% | +17.6% / +52.1% | +28.7% / +47.8% (n/c) |
| Marine IK (pretty) | +36.4% / +37.5% | +17.9% / +17.8% | +14.0% / +15.2% | +6.3% (n/c) / +16.3% | +19.1% (n/c) / +38.6% |
| Mesh (min) | +291.2% / +326.4% | +9.8% / +10.7% | +19.1% / +20.2% | +44.4% / +18.2% (n/c) | +19.5% / +18.9% (n/c) |
| Mesh (pretty) | +244.5% / +260.8% | +7.8% / +8.8% | +11.3% / +12.8% | −38.5% (n/c) / −41.1% | +11.6% / +9.2% |
| Random (min) | +58.2% / +67.0% | +11.3% / +13.3% | +5.6% / +3.5% | +47.4% / +65.6% (n/c) | +12.5% / +14.4% |
| Random (pretty) | +57.3% / +64.0% | +9.6% / +12.8% | +4.7% / +5.0% | tie (n/c) / +51.1% | +12.0% / +8.2% (n/c) |
| Twitter (min) | +138.0% / +146.3% | +6.9% / +6.9% | +9.9% / +11.2% | +19.3% / +28.7% | +2.9% / +2.4% |
| Twitter (pretty) | +118.4% / +127.8% | +4.9% / +6.1% | +7.0% / +6.4% | +9.6% / +19.1% | tie / −5.9% |

### 7.6 Documents, sparse fields in reverse order

| Test | MSVC | Linux Clang | Linux GCC | macOS GCC | macOS Clang |
|---|---|---|---|---|---|
| Canada (min) | +269.9% / +258.2% | −1.1% / −1.1% | +22.6% / +22.7% | +6.7% (n/c) / tie | +7.2% / +13.9% |
| Canada (pretty) | +167.1% / +164.2% | +6.6% / +6.9% | +8.5% / +9.1% | +22.3% / +16.6% | −1.9% / −8.2% |
| CitmCatalog (min) | +227.5% / +231.7% | +31.7% / +31.6% | +32.6% / +34.8% | +5.8% / +8.3% | +5.6% / +17.8% |
| CitmCatalog (pretty) | +194.4% / +195.8% | +27.4% / +27.2% | +27.8% / +27.4% | +20.3% / +32.8% | +16.0% / +15.2% |
| Discord (min) | +126.7% / +147.2% | +3.4% / +3.6% | +8.5% / +10.6% | +29.3% / +27.3% | +45.3% / tie |
| Discord (pretty) | +119.5% / +143.8% | +1.8% / +4.6% | +6.6% / +8.9% | +24.0% / +32.9% | −16.8% / −6.4% |
| Google Maps (min) | +72.8% / +72.4% | +5.6% / +7.7% | +14.2% / +15.2% | −58.3% / +6.6% | −9.1% / −47.6% |
| Google Maps (pretty) | +73.4% / +80.3% | +5.6% / +5.6% | +69.8% / +58.5% | +6.1% / +6.7% | +11.6% / tie |
| Instruments (min) | +192.2% / +209.0% | +4.4% / +6.5% | +12.7% / +18.4% | +75.0% / +37.1% | −2.5% / −3.8% |
| Instruments (pretty) | +172.2% / +199.9% | +5.0% / +5.3% | −32.1% / +17.8% | +21.8% / +12.7% | +17.3% / tie |
| Marine IK (min) | +204.4% / +205.2% | +12.9% / +13.1% | +61.6% / +61.3% | +30.4% / +29.1% | +39.3% / +39.2% |
| Marine IK (pretty) | +156.7% / +158.1% | +11.8% / +12.4% | +30.0% / +29.4% | +19.2% (n/c) / +24.7% | +27.2% (n/c) / +38.3% |
| Mesh (min) | +290.2% / +304.6% | +9.3% / +9.2% | +19.7% / +20.2% | +5.7% / −9.0% | +28.5% / +18.2% |
| Mesh (pretty) | +244.6% / +262.4% | +6.1% / +7.1% | +12.1% / +13.0% | +7.3% (n/c) / +21.3% | +29.6% / +19.7% (n/c) |
| Random (min) | +82.3% / +89.6% | +5.0% / +6.8% | +7.6% / +8.0% | +43.7% / +41.5% | +21.3% / tie |
| Random (pretty) | +82.1% / +87.9% | +6.1% / +6.7% | +10.0% / +8.0% | +13.1% / tie (n/c) | +26.3% / +13.3% |
| Twitter (min) | +201.3% / +208.9% | +15.6% / +14.9% | +17.8% / +17.9% | +10.4% / +10.7% | +2.7% / +5.9% |
| Twitter (pretty) | +189.9% / +186.9% | +12.9% / +13.8% | +15.7% / +16.1% | tie / +22.7% | −6.9% (n/c) / +7.2% |

### 7.7 Small documents, in order

| Test | MSVC | Linux Clang | Linux GCC | macOS GCC | macOS Clang |
|---|---|---|---|---|---|
| Canada (min) | −1.5% / tie | +8.5% / +1.2% | −13.6% / −13.4% | −3.4% / +10.8% | +7.5% / +7.6% |
| Canada (pretty) | +2.7% / tie | +4.7% / +2.4% | −11.5% / −13.6% | +1.5% / tie | +5.0% / +2.8% |
| CitmCatalog (min) | +18.1% / +30.1% | −6.0% / −5.5% | −4.3% / −2.8% | +16.4% / +7.2% | +2.0% / +2.0% |
| CitmCatalog (pretty) | +14.9% / +25.2% | −6.8% / −6.2% | −5.7% / −2.5% | +14.1% / +11.6% | −3.1% / −2.4% |
| Discord (min) | +23.9% / +64.7% | +0.7% / +6.6% | +1.2% / +8.1% | +22.2% / +37.4% | +4.7% / +14.1% |
| Discord (pretty) | +21.6% / +52.9% | −1.1% / +8.4% | +0.5% / +4.3% | +16.2% / +27.1% | +11.2% / −14.7% |
| Google Maps (min) | +28.7% / +42.5% | +18.0% / +18.2% | +14.0% / +14.8% | +36.0% / +24.8% | −23.3% / +4.1% |
| Google Maps (pretty) | +32.4% / +39.3% | +14.8% / +14.9% | +10.4% / +11.6% | +30.7% / +15.6% | +12.5% / +19.6% |
| Instruments (min) | +55.1% / +58.6% | +3.4% / +3.6% | −7.0% / −4.7% | +5.2% / +7.6% | +5.2% / +5.1% |
| Instruments (pretty) | +51.9% / +57.3% | +3.5% / +3.6% | −6.8% / −6.2% | tie (n/c) / tie | −1.8% / +1.7% |
| Marine IK (min) | +4.5% / +8.6% | −0.6% / tie | −5.5% / −5.1% | −14.2% / +35.0% | +10.2% / +13.3% |
| Marine IK (pretty) | +6.2% / +9.6% | tie / +1.0% | −5.7% / −5.5% | +4.0% / +5.9% | +1.5% / +32.1% |
| Mesh (min) | +6.4% / +31.9% | −7.0% / −5.1% | −7.8% / −9.5% | +25.5% / +6.8% | +24.1% / +6.5% |
| Mesh (pretty) | +7.7% / +30.5% | −8.2% / −6.3% | −11.3% / −15.1% | tie / tie | +4.3% / −2.0% |
| Random (min) | +40.0% / +55.1% | +15.5% / +19.2% | +18.3% / +21.9% | +35.2% / +57.9% | +7.7% / +13.5% |
| Random (pretty) | +37.8% / +52.7% | +15.5% / +17.3% | +12.9% / +18.4% | +71.3% / +41.1% | +10.0% / +15.2% |
| Twitter (min) | +33.4% / +41.2% | +1.2% / +3.1% | +0.2% / +2.6% | +6.0% / −30.2% | +1.3% / +4.4% |
| Twitter (pretty) | +32.5% / +48.2% | +0.6% / +2.9% | +0.3% / +0.4% | +7.9% / +14.9% | +1.3% / +3.9% |

### 7.8 Small documents, keys in reverse order

| Test | MSVC | Linux Clang | Linux GCC | macOS GCC | macOS Clang |
|---|---|---|---|---|---|
| Canada (min) | +11.8% / +13.0% | +4.6% / +5.7% | −5.5% / −4.7% | +5.0% / +10.1% | +6.4% / +6.9% |
| Canada (pretty) | +14.3% / +16.3% | +7.4% / +5.1% | −6.0% / −6.8% | +2.5% / +14.3% | +3.1% / +4.1% |
| CitmCatalog (min) | +212.6% / +225.8% | +53.4% / +56.3% | +68.3% / +73.9% | +63.1% / +62.2% | +57.6% / +58.5% |
| CitmCatalog (pretty) | +212.9% / +226.4% | +49.7% / +54.6% | +63.7% / +69.3% | +56.5% / +64.0% | +51.1% / +54.5% |
| Discord (min) | +1189.0% / +1408.4% | +261.3% / +304.7% | +268.6% / +306.7% | +316.2% / +362.6% | +273.0% / +298.1% |
| Discord (pretty) | +1192.6% / +1417.6% | +258.1% / +291.1% | +261.7% / +293.8% | +302.3% / +347.4% | +251.3% / +256.2% |
| Google Maps (min) | +107.5% / +114.6% | +8.4% / +9.2% | +19.4% / +22.0% | +28.0% / +24.8% | tie / +31.7% |
| Google Maps (pretty) | +96.8% / +115.2% | +5.4% / +6.7% | +17.7% / +17.6% | +3.8% / +27.1% | +20.0% / tie |
| Instruments (min) | +690.4% / +737.9% | +132.4% / +136.1% | +140.0% / +142.3% | +190.4% / +190.6% | +128.7% / +142.9% |
| Instruments (pretty) | +679.9% / +714.4% | +124.6% / +129.1% | +126.5% / +131.0% | +94.9% / +167.4% | +117.7% / +127.8% |
| Marine IK (min) | +194.9% / +194.8% | +30.4% / +32.1% | +53.5% / +55.4% | +50.2% / +68.1% | +23.1% / +36.8% |
| Marine IK (pretty) | +184.7% / +194.4% | +29.6% / +31.0% | +50.6% / +51.5% | +43.0% / +62.6% | +31.6% / +20.7% |
| Mesh (min) | +121.7% / +196.3% | +21.7% / +26.9% | +28.4% / +35.4% | +27.4% / +31.1% | +24.0% / +43.7% |
| Mesh (pretty) | +118.6% / +187.4% | +20.1% / +23.2% | +23.7% / +29.4% | +40.1% / +29.0% | +17.8% / +24.7% |
| Random (min) | +248.3% / +282.9% | +51.6% / +55.1% | +43.1% / +47.6% | +62.3% / +72.9% | +38.9% / +42.6% |
| Random (pretty) | +238.5% / +274.7% | +50.0% / +54.4% | +40.4% / +45.5% | +42.2% / +57.1% | +35.6% / +37.0% |
| Twitter (min) | +697.6% / +726.5% | +166.6% / +175.0% | +146.4% / +149.6% | +267.7% / +247.1% | +161.7% / +157.4% |
| Twitter (pretty) | +700.8% / +710.0% | +162.0% / +168.8% | +137.8% / +145.2% | +166.0% / +202.4% | +154.4% / +158.7% |

### 7.9 Small documents, sparse fields in order

| Test | MSVC | Linux Clang | Linux GCC | macOS GCC | macOS Clang |
|---|---|---|---|---|---|
| Canada (min) | +42.1% / +38.2% | +27.0% / +27.7% | +14.6% / +13.3% | +38.9% / +36.8% | −3.9% / −5.9% |
| Canada (pretty) | +40.3% / +40.9% | +26.9% / +25.5% | +14.6% / +13.6% | +24.1% / +24.5% | +2.2% / −1.5% |
| CitmCatalog (min) | +177.6% / +203.6% | +8.4% / +8.6% | +12.6% / +13.5% | +17.4% / +20.4% | +15.0% / +14.4% |
| CitmCatalog (pretty) | +143.0% / +150.4% | +11.0% / +8.9% | +11.7% / +11.6% | +16.9% / +16.4% | +21.7% / +12.8% |
| Discord (min) | +31.0% / +43.5% | +0.7% / −0.8% | −0.7% / +0.3% | +21.5% / +34.7% | +2.1% / +1.7% |
| Discord (pretty) | +33.6% / +45.2% | −0.5% / −1.7% | −2.1% / −2.4% | +22.6% / +32.2% | −9.6% / tie |
| Google Maps (min) | +56.6% / +64.9% | +1.6% / +3.4% | −1.9% / −4.0% | +13.8% / +13.7% | −6.0% / +6.8% |
| Google Maps (pretty) | +57.1% / +67.6% | +2.0% / +1.4% | −1.3% / −3.6% | +18.7% / tie | +71.4% / −11.1% |
| Instruments (min) | +21.7% / +48.7% | −1.6% / −1.9% | −1.5% / −1.8% | +18.0% / tie | −3.6% / −1.7% |
| Instruments (pretty) | +23.4% / +37.0% | −0.9% / tie | −0.8% / −0.4% | +4.2% / +24.6% | −5.9% / −6.7% |
| Marine IK (min) | +17.3% / +18.6% | +5.8% / +6.0% | +16.4% / +17.6% | +18.6% / +4.0% | +18.5% / +16.5% |
| Marine IK (pretty) | +30.5% / +24.4% | +9.6% / +10.0% | +15.3% / +15.8% | +26.3% / +25.5% | +5.5% / +32.0% |
| Mesh (min) | +120.5% / +187.0% | +11.5% / +11.0% | +12.4% / +17.4% | +15.7% / −10.5% | +11.3% / +26.9% |
| Mesh (pretty) | +104.4% / +162.9% | +9.1% / +8.4% | +10.4% / +12.2% | tie / +19.5% | +10.4% / +1.9% |
| Random (min) | +56.5% / +51.3% | +5.2% / +5.5% | +1.8% / +1.8% | +17.6% / +10.4% (n/c) | +10.7% / +2.2% |
| Random (pretty) | +42.3% / +50.8% | +6.6% / +6.6% | −0.6% / +1.7% | +17.1% / +20.8% | +16.2% / +5.0% |
| Twitter (min) | +101.9% / +110.1% | −6.9% / −6.8% | −1.9% / −2.8% | +14.5% / +19.4% | −4.5% / tie |
| Twitter (pretty) | +93.6% / +100.5% | −9.5% / −9.1% | −3.3% / −5.6% | +25.3% / +14.7% | −3.5% / +1.8% |

### 7.10 Small documents, sparse fields in reverse order

| Test | MSVC | Linux Clang | Linux GCC | macOS GCC | macOS Clang |
|---|---|---|---|---|---|
| Canada (min) | +157.4% / +152.0% | +16.5% / +16.7% | +23.1% / +21.3% | +32.9% / +20.9% | −10.9% / −9.8% |
| Canada (pretty) | +136.3% / +135.1% | +19.2% / +19.8% | +16.7% / +18.8% | +18.3% / +27.3% | −5.0% / −1.2% |
| CitmCatalog (min) | +145.4% / +137.0% | +20.2% / +20.4% | +23.8% / +24.6% | +23.4% / +19.9% | +9.1% / +9.6% |
| CitmCatalog (pretty) | +120.8% / +128.2% | +17.9% / +18.0% | +20.9% / +21.7% | +32.8% / +27.7% | +7.9% / +2.3% |
| Discord (min) | +151.0% / +177.2% | +6.1% / +5.2% | +2.2% / +4.8% | +40.2% / +48.3% | tie / +2.7% |
| Discord (pretty) | +140.3% / +168.2% | +4.6% / +3.8% | +3.1% / +2.9% | +53.4% (n/c) / +10.2% (n/c) | +11.0% / −4.1% |
| Google Maps (min) | +84.3% / +84.2% | +0.3% / +1.9% | +9.8% / +12.3% | +12.7% / +11.5% | −6.7% / −6.6% |
| Google Maps (pretty) | +71.3% / +77.3% | +1.7% / +1.9% | +7.8% / +7.0% | +14.4% / +15.8% | −4.5% / −7.8% |
| Instruments (min) | +175.4% / +200.5% | +4.7% / +4.8% | +17.8% / +18.2% | +19.8% / +20.0% | +5.2% / −15.2% |
| Instruments (pretty) | +163.2% / +182.2% | +1.2% / +2.2% | +16.3% / +17.6% | +60.4% / +19.9% | +4.0% / tie |
| Marine IK (min) | +91.6% / +96.8% | +17.9% / +18.2% | +36.6% / +37.5% | +161.9% (n/c) / +47.9% | +32.4% / +231.6% (n/c) |
| Marine IK (pretty) | +92.0% / +89.4% | +18.9% / +17.9% | +30.8% / +33.1% | +15.3% / +70.6% | +17.1% / −23.9% (n/c) |
| Mesh (min) | +119.5% / +183.4% | +3.0% / +2.4% | +15.7% / +14.4% | +10.8% / tie | +6.0% / +15.2% |
| Mesh (pretty) | +122.7% / +179.2% | +3.5% / +0.6% | +11.1% / +8.3% | tie / +17.6% | tie / −6.5% (n/c) |
| Random (min) | +86.4% / +83.9% | +2.3% / +2.2% | +8.8% / +7.6% | +20.5% / +23.4% | +21.5% (n/c) / +18.8% |
| Random (pretty) | +77.2% / +80.8% | +3.9% / +3.0% | +9.8% / +7.7% | +19.0% (n/c) / −19.5% (n/c) | +3.4% / +2.3% |
| Twitter (min) | +162.8% / +179.6% | −0.7% / +1.7% | +4.6% / +5.9% | +23.3% / +19.1% | +13.4% (n/c) / −10.9% |
| Twitter (pretty) | +148.8% / +149.4% | −3.6% / −2.3% | +4.7% / +3.5% | +20.1% / +47.3% | +9.0% / +2.1% |

### 7.11 What the results show

- **Reverse order is where the design pays off.** On documents whose objects have many fields (Discord, Twitter, Instruments), simdjson has to rescan for every out-of-order key, and Jsonifier is 1.3x to 18.1x faster at full size: +30.5% (Instruments min, Linux/GCC, fresh) up to +1712.7% (Discord min, MSVC, reused), and up to +355.9% outside MSVC (Discord min, macOS/GCC). The small documents show the same pattern, from +94.9% to +1417.6%, because a 5 KiB Discord or Instruments document still holds objects with dozens of keys. On documents made of small objects (Canada, Google Maps) there is little to rescan, and outside MSVC the full-size margin runs from −9.9% to +49.7%. That matches the cost model in §5: the map only pays off when there are many skipped keys to index.
- **Sparse reverse reads are a strength.** Jsonifier won 159 of 180 full-size sparse-reverse results across both allocation modes (7 ties, 14 losses), and 158 of 180 small ones (5 ties, 17 losses). MSVC won every one of its 72, and Linux/GCC lost only one, prettified Instruments at full size (fresh, −32.1%). Of the 14 full-size losses, 9 are on macOS/Clang, 2 on macOS/GCC (minified Google Maps, fresh, −58.3%, and minified Mesh, reused, −9.0%), 2 on Linux/Clang (minified Canada, −1.1% in both runs) and 1 on Linux/GCC; 4 of the 14 are Google Maps.
- **MSVC shows the largest gaps.** On MSVC, simdjson's rescans are far slower than on the other builds: Discord Reverse (pretty, fresh) runs at 32.8 MB/s on MSVC against 222.1 MB/s on Linux/Clang, on the same CPU. Jsonifier reads the same test at 533.7 and 772.1 MB/s. Every MSVC full-size sparse result is a win, by +29.0% to +326.4%, and every full-size sparse-reverse result by +72.4% to +304.6%.
- **Sparse reads in order are mostly wins.** Across both allocation modes, Jsonifier won 148 of 180 full-size sparse results (12 ties, 20 losses) and 135 of 180 small ones (6 ties, 39 losses). MSVC won all 36 full-size results and all 36 small ones. On full-size documents, 8 of the 20 losses are on macOS/Clang, 4 on Linux/Clang (Discord, −3.0% to −5.0%), 4 on Linux/GCC (Google Maps, −1.6% to −3.6%, and minified Instruments, reused, −6.0%) and 4 on macOS/GCC, which include the largest (minified Instruments, fresh, −40.2%, and prettified Mesh, −38.5% fresh and −41.1% reused).
- **In-order full reads are mostly wins at full size.** Across 180 full-size in-order results, Jsonifier won 131, tied 23 and lost 26. Linux/Clang went 34-0-2 and MSVC 32-3-1. Linux/GCC split its in-order tests 22-0-14 across the two modes, macOS/GCC 22-8-6 and macOS/Clang 21-12-3.
- **Small documents read in order are the weakest in-order category.** Across 180 results, Jsonifier won 131, tied 9 and lost 40. MSVC (33-2-1), macOS/Clang (30-0-6) and macOS/GCC (28-5-3) won most of theirs, but Linux/Clang went 24-2-10 and Linux/GCC 16-0-20. These documents are at most 5 KiB, so a parse is short, and the fixed per-parse costs of Jsonifier's stage 1 and document setup count for more. The Linux/Clang losses there are modest (−0.6% to −8.2%) and Linux/GCC's reach −15.1% on Mesh, while the in-order small-document results on Google Maps and Random on those two builds are all wins, from +10.4% to +21.9%.
- **In-order losses at full size cluster on Linux/GCC.** It lost Canada, Marine IK and Mesh in both forms and both runs, minified Instruments (fresh, −30.5%) and prettified Twitter (fresh, −0.4%); apart from that Instruments result, the losses run from −0.4% to −10.6%. macOS/GCC lost six: CitmCatalog (pretty, fresh, −2.4%), Instruments (pretty, −6.7% fresh and −8.4% reused) and Mesh (min, reused, −3.0%; pretty, −26.0% fresh and −11.7% reused). macOS/Clang lost three: Google Maps (min, fresh, −4.7%) and Marine IK (min, reused, −4.8%; pretty, fresh, −6.6%). Canada, Marine IK and Mesh are dominated by float arrays, where the time goes into number parsing and stage 1, not into the generic layer's field lookup. Linux/Clang lost prettified Mesh (−0.6% fresh, −0.7% reused), and MSVC lost minified Canada (reused, −2.7%).
- **The POD tests win on most builds.** Jsonifier went 48-0-2 on them, and every build except macOS/Clang won all 10. Bool won all 10 results (+5.1% to +60.6%), Double all 10 (+29.9% to +85.9%), Int64 all 10 (+12.7% to +110.5%) and Uint64 all 10 (+23.1% to +70.3%). String won 8 of 10 (+2.0% to +21.9%), losing on macOS/Clang fresh (−11.8%) and reused (−14.9%).

## 8. Streams: many documents in one buffer

Many real inputs are not one document but a sequence of them: NDJSON logs, JSON Lines exports, or documents joined by commas. simdjson handles these with `iterate_many`, which runs stage 1 over a window of the input and hands out one On Demand document at a time. `jsonifier::generic` does the same through `parser::iterateMany`, and every document it hands out is an ordinary `generic::document`, with re-readable values and the per-object field index from §5.

### 8.1 Design

```cpp
for (jsonifier::generic::document doc : parser.iterateMany(buffer)) { ... }
auto stream = parser.iterateMany<parse_options{ .newLineDelimited = false }>(buffer, batchSize);
```

- **The separator is part of the type.** `parse_options::newLineDelimited` selects it: `true` (NDJSON and other whitespace-separated streams, the `iterateMany` default) or `false` for comma-delimited streams. The two modes find document boundaries in different ways (§8.2), and the choice is made at compile time.
- **Windowed stage 1.** Stage 1 runs over one window of `batchSize` bytes (1 MiB by default), using the same indexer, and the same small-input routing, as `iterate()`. Tape offsets are relative to the window, so no single window can exceed 4 GiB, but the stream as a whole can.
- **One document per tape slice.** Each document is bound to the parser's shared cursor (§3.2), starting at its first structural. The accessors, the lazy errors and the field index work unchanged; the field index is reset per document.
- **Documents cut by the window edge.** No document is ever parsed from two windows. The next window starts at the first byte of the first document that the current window could not complete, and a document larger than a whole window returns `error_code::capacity`, as in simdjson.
- **Comma-separated streams.** In comma mode, a comma between top-level documents is accepted as a separator. A leading or doubled comma is a `tape_error`, and a trailing comma is `trailing_content`. A comma that falls at the start of a new window is still consumed as a separator.
- **Reporting.** The stream exposes `truncatedBytes()` for an unfinished final document, and each iterator exposes `currentIndex()` and `source()` for the current document's offset and raw text. After an error, the stream yields that error once and then ends.
- **Lifetime.** As with simdjson, advancing the stream invalidates the previous document; the only thing that survives is a zero-copy view of an unescaped string, which points into the caller's buffer.

### 8.2 Finding where documents end

The simplest way to find each document's end is to walk its structurals and count depth before handing the document out. That bounds every later tape scan to the document, but a reader that consumes the document in order walks the same structurals again, so every in-order document pays for two passes over its part of the tape.

The whitespace-delimited mode finds boundaries without that pass:

- **A local rule marks document starts.** Inside any container, every value is preceded on the tape by `[`, `{`, `,` or `:`. So in a whitespace-delimited stream, a structural that can start a value and directly follows a closer or a scalar begins a new top-level document. No depth needs to be tracked to recognize it.
- **One short backward scan per window.** Scanning backward from the end of the window's tape, the first position that satisfies the rule is the start of the window's last document. Everything before it is complete. That last document is deferred to the next window unless this is the final window; only when a window holds a single document, or for the final document of the final window, is a depth walk used to decide completeness. The scan usually stops within a few structurals of the window's end.
- **The reader tells the stream where the document ended.** When the caller advances, the parser's state already records how far the reads went. If the cursor is still inside the document, the stream finishes the open containers from the cursor. For an in-order reader, whose cursor rests just before the closing brace, that is about one step. If the cursor is past a closed root value, it is the end. Only a document the caller never touched is skipped from its start.

Comma mode cannot use the local rule, because a top-level comma looks exactly like a comma inside a container. It keeps the up-front depth walk, and that difference is visible in the results: on the in-order corpus streams on x86, Jsonifier's comma-separated throughput is 6.3% to 28.2% below its own NDJSON throughput on the same records.

### 8.3 Method

The streaming benchmarks run in the same Json-Performance suite and the same run as §6 (branch `generic-parsing-main`), using the same adaptive-sampling stage. Both libraries use a 1 MiB batch, simdjson runs unthreaded (`parser.threaded = false`), and both fill the same structs through the same field-by-field readers. Each stream is read from a saved file and repeated to its target size, so every build streams the same bytes. There are 26 tests per build, in two groups:

- **Amazon Cellphones and Stream Formats.** These follow simdjson's streaming benchmarks. *Amazon Cellphones* is the `amazon_cellphones.ndjson` file from the simdjson repository (793 lines, each a 9-element array), with the same per-brand rating aggregation and the same skipped header line; *Large Amazon Cellphones* repeats it to 10 MiB, as simdjson's `large_amazon_cellphones` benchmark does. *Stream Formats* uses the `{"id", "name", "payload", "flag"}` documents with 16-byte (Small) and 4096-byte (Large) payloads, in NDJSON and comma-separated form, and sums `"id"` over the stream. simdjson generates 128 MB of these; the port repeats the first 64 documents to 16 MB so that a run fits the sampling window, so the ids cycle through 0–63. Every record here is an array or a four-key object, so this group has no reverse variant.
- **Record streams from the paper's corpus.** The main record array of five of the documents from §6 is split into one document per record, each record is re-serialized minified, and the stream is repeated to 4 MiB: CitmCatalog `performances`, Google Maps `rows`, Instruments `patterns`, Random `result` and Twitter `statuses`. Each stream runs in NDJSON and comma-separated form, both in order and with every object's keys read in reverse, using the reverse readers from §6. The other four documents have no record array to split and are not streamed.
- **Correctness:** for every streaming test on every build, Jsonifier's serialized output was compared with simdjson's, and all 130 pairs are byte-identical. They are part of the 515 pairs in §6, with the same scope: the comparison ran on `3e7bf6d`.

| Build | Stage-1 kernel | Converged |
|---|---|---|
| Windows 10.0.26200 / MSVC 19.44 | AVX2 | 21 / 26 |
| Linux 7.0.0 / Clang 24.0 | AVX2 | 26 / 26 |
| Linux 7.0.0 / GCC 16.2 | AVX2 | 26 / 26 |
| macOS 25.6 (M1, virtual) / GCC 16.2 | NEON | 13 / 26 |
| macOS 25.6 (M1, virtual) / Clang 23.1 | NEON | 11 / 26 |

The commits, hosts and convergence limits are the same as in §6. Unlike the single-document tests, the streaming tests have no reused variant: each document is read into a freshly constructed record.

### 8.4 Results

As in §7, the percentages are Jsonifier's throughput relative to simdjson On Demand.

| Build | simdjson's benchmarks W-T-L | Corpus, in order W-T-L | Corpus, reverse W-T-L | Total W-T-L |
|---|---|---|---|---|
| Windows / MSVC | 6-0-0 | 10-0-0 | 10-0-0 | 26-0-0 |
| Linux / Clang | 5-0-1 | 10-0-0 | 9-0-1 | 24-0-2 |
| Linux / GCC | 4-1-1 | 8-0-2 | 8-0-2 | 20-1-5 |
| macOS / GCC | 6-0-0 | 10-0-0 | 7-1-2 | 23-1-2 |
| macOS / Clang | 2-1-3 | 6-2-2 | 9-0-1 | 17-3-6 |
| **All** | **23-2-5** | **44-2-4** | **43-1-6** | **110-5-15** |

**Amazon Cellphones and Stream Formats:**

| Test | MSVC | Linux Clang | Linux GCC | macOS GCC | macOS Clang |
|---|---|---|---|---|---|
| Amazon Cellphones (NDJSON) | +48.4% | +16.0% | +4.5% | +30.8% | +8.3% |
| Large Amazon Cellphones (NDJSON) | +40.9% | +13.9% | +4.4% | +13.1% (n/c) | tie (n/c) |
| Stream Formats Small (NDJSON) | +44.2% | −4.9% | +2.9% | +42.4% (n/c) | −9.6% |
| Stream Formats Small (comma) | +95.0% | +66.1% | +92.4% | +95.1% (n/c) | +52.9% (n/c) |
| Stream Formats Large (NDJSON) | +23.7% | +2.9% | −7.7% | +49.3% | −8.9% |
| Stream Formats Large (comma) | +28.7% | +9.2% | tie | +11.3% | −3.8% |

**Corpus record streams, in order:**

| Test | MSVC | Linux Clang | Linux GCC | macOS GCC | macOS Clang |
|---|---|---|---|---|---|
| CitmCatalog (NDJSON) | +36.3% | +5.3% | +7.2% | +6.6% | tie (n/c) |
| CitmCatalog (comma) | +29.6% | +17.6% | +22.5% | +22.2% (n/c) | +7.9% (n/c) |
| Google Maps (NDJSON) | +45.0% | +27.5% | +24.7% | +13.6% | −2.5% |
| Google Maps (comma) | +25.6% | +35.0% | +41.1% | +26.8% (n/c) | +30.8% (n/c) |
| Instruments (NDJSON) | +96.6% | +19.3% | −1.5% | +9.4% (n/c) | −4.7% (n/c) |
| Instruments (comma) | +62.5% | +32.5% | +34.2% | +20.2% | +19.1% |
| Random (NDJSON) | +62.7% | +29.7% | +25.6% | +23.3% | +26.4% |
| Random (comma) | +53.6% | +32.5% | +43.9% | +72.8% (n/c) | +43.5% |
| Twitter (NDJSON) | +45.4% | +12.4% | −0.5% | +5.8% | +10.2% (n/c) |
| Twitter (comma) | +39.5% | +15.5% | +13.1% | +21.7% | tie (n/c) |

**Corpus record streams, keys in reverse order:**

| Test | MSVC | Linux Clang | Linux GCC | macOS GCC | macOS Clang |
|---|---|---|---|---|---|
| CitmCatalog (NDJSON) | +225.4% (n/c) | +43.1% | +40.4% | +25.9% (n/c) | +78.8% (n/c) |
| CitmCatalog (comma) | +225.0% (n/c) | +57.9% | +62.7% | +68.6% (n/c) | +57.5% |
| Google Maps (NDJSON) | +63.7% | +11.1% | +19.5% | +26.6% | −7.2% (n/c) |
| Google Maps (comma) | +54.4% (n/c) | +13.3% | +37.0% | tie | +6.3% (n/c) |
| Instruments (NDJSON) | +58.8% | −12.1% | −21.9% | −14.5% | +34.5% (n/c) |
| Instruments (comma) | +78.0% | +2.9% | −2.5% | −10.2% (n/c) | +18.3% |
| Random (NDJSON) | +274.7% | +60.8% | +40.2% | +33.4% | +62.4% |
| Random (comma) | +282.8% | +77.8% | +60.6% | +75.5% (n/c) | +56.0% (n/c) |
| Twitter (NDJSON) | +771.9% (n/c) | +229.3% | +141.3% | +173.9% (n/c) | +264.7% (n/c) |
| Twitter (comma) | +755.1% (n/c) | +239.6% | +171.7% | +186.7% (n/c) | +162.7% (n/c) |

### 8.5 What the results show

- **Amazon Cellphones and Stream Formats.** Jsonifier took 23 of the 30 results in that group, tied 2 and lost 5. It won Amazon Cellphones (+4.5% to +48.4%) and Stream Formats Small (comma) (+52.9% to +95.1%) on all five builds, and Large Amazon Cellphones (+4.4% to +40.9%) on four, tying on macOS/Clang. The five losses are all Stream Formats: Small (NDJSON) on Linux/Clang (−4.9%) and macOS/Clang (−9.6%), Large (NDJSON) on Linux/GCC (−7.7%) and macOS/Clang (−8.9%), and Large (comma) on macOS/Clang (−3.8%); Large (comma) on Linux/GCC is a tie. Stream Formats Small has the smallest documents in the suite, so it puts the most weight on per-document overhead.
- **Windows/MSVC won every streaming test,** 26-0-0; Linux/Clang lost two of 26, macOS/GCC two, Linux/GCC five and macOS/Clang six.
- **In-order corpus streams are mostly wins.** Of the 50 in-order corpus results, Jsonifier won 44, tied 2 and lost 4: Instruments in NDJSON form on Linux/GCC (−1.5%) and macOS/Clang (−4.7%), Twitter in NDJSON form on Linux/GCC (−0.5%), and Google Maps in NDJSON form on macOS/Clang (−2.5%). The ties are CitmCatalog (NDJSON) and Twitter (comma) on macOS/Clang.
- **Reverse order carries over from single documents to streams.** Jsonifier won 43 of 50 reverse stream results and tied 1. The margins follow object size, as in §7.11: Twitter statuses, which have many fields, give +141.3% to +771.9%, while Google Maps rows, which have few, give −7.2% to +63.7%. Five of the six losses are Instruments: NDJSON on Linux/Clang (−12.1%), Linux/GCC (−21.9%) and macOS/GCC (−14.5%), and comma-separated on Linux/GCC (−2.5%) and macOS/GCC (−10.2%); the sixth is Google Maps NDJSON on macOS/Clang (−7.2%). The tie is Google Maps in comma form on macOS/GCC.
- **MSVC again shows the largest gaps.** On the same CPU, simdjson reads Random Reverse (comma) at 122.4 MB/s under MSVC and 413.6 MB/s under Linux/Clang, while Jsonifier reads it at 468.4 and 735.4 MB/s.

## 9. Limitations

- **Linux/GCC and macOS/Clang hold most of the single-document losses:** 61 and 53 of the 165, against 31 on Linux/Clang, 18 on macOS/GCC and 2 on MSVC. macOS/Clang's are spread across access patterns, led by 13 small sparse reverse, 12 small sparse, 9 full-size sparse reverse and 8 full-size sparse. The largest individual losses are on the macOS builds: Google Maps Sparse Reverse (min) at −58.3% on macOS/GCC (fresh) and −47.6% on macOS/Clang (reused), and Mesh Sparse (pretty, reused) at −41.1% on macOS/GCC. Both macOS builds ran on virtualized hosts, so part of these results likely reflects host noise.
- **Linux/GCC loses mostly on reads in document order.** 34 of its 61 losses are in-order reads: 20 of its 36 small in-order results and 14 of its 36 full-size ones. At full size they are on Canada, Instruments, Marine IK, Mesh and Twitter, by −0.4% to −30.5%; at small size they reach −15.1% (Mesh Small, pretty, reused). It also loses reverse reads of Canada at both sizes (−4.7% to −8.7%), 16 small sparse and 4 full-size sparse results, and one full-size sparse-reverse result (Instruments, pretty, fresh, −32.1%), while it won every small sparse-reverse result.
- **Small documents lose on Linux.** Linux/Clang lost 10 of its 36 small in-order results and Linux/GCC 20 of 36, while Linux/Clang lost two of its 36 full-size in-order tests. Linux/Clang also lost 10 of its 36 small sparse results and 3 of its 36 small sparse-reverse ones, and Linux/GCC 16 small sparse. At 5 KiB per document, Jsonifier's fixed per-parse cost is the next target.
- **Number-heavy documents in order** remain a common loss. Canada, Marine IK and Mesh lose in order on Linux/GCC in both forms and both runs, Mesh loses on macOS/GCC, and MSVC loses minified Canada (reused, −2.7%). The cause is float-array throughput rather than out-of-order access, so it is out of scope for the field index.
- **Instruments streams** lose on four builds: read in order in NDJSON form on Linux/GCC (−1.5%) and macOS/Clang (−4.7%), and in reverse in NDJSON form on Linux/Clang (−12.1%), Linux/GCC (−21.9%) and macOS/GCC (−14.5%) and in comma form on Linux/GCC (−2.5%) and macOS/GCC (−10.2%), although the single-document Instruments Reverse tests win on every build at both sizes.
- **Some results did not converge.** 81 of the 1,490 single-document results did not converge: 43 on macOS/GCC, 31 on macOS/Clang, 4 on Windows/MSVC and 3 on Linux/GCC. 33 of the 130 streaming results did not converge: 15 on macOS/Clang, 13 on macOS/GCC and 5 on Windows/MSVC. Linux/Clang converged on every test, and both Linux builds on every streaming test. These results stay in the tallies and are marked (n/c) in §7 and §8. Most of them come from the two NEON builds, whose limits are looser (RSE < 10%, shift < 5%), so read those cells with that in mind.
- **The thresholds were tuned on this suite.** *T* was selected by sweeping {2, 4, 8, 16} on the full-size benchmarks, before the wrap gate and the small documents were added, and MSVC was not swept. The wrap count of 2 was not swept.
- **Comma-separated streams still walk each document up front.** The local boundary rule in §8.2 only holds for whitespace-delimited streams, so comma mode finds each document's end with a depth walk before handing it out, and the caller then walks the same structurals again. On the x86 in-order corpus streams this leaves comma mode 6.3% to 28.2% slower than NDJSON mode on the same records. The rule also assumes the input is valid: inside a malformed container, two adjacent values with no separator would be split into separate documents, and the reader would then report the errors. Reverse reads still pay a walk from the first key to the end of the object when the stream advances, the same walk an up-front boundary pass would pay when handing the document out.
- **Streams were compared unthreaded and fresh only.** simdjson can run stage 1 for the next window on a second thread; Jsonifier has no threaded mode, so §8 compares both single-threaded. The streaming tests also have no reused-output variant.
- **A known inefficiency remains in the miss path.** A lookup for a key that doesn't exist still ends with a full escape-aware rescan of the object, even after the object is indexed. It does not affect correctness and is a candidate for follow-up work.
- **The output comparison predates the measured commits.** The byte-for-byte comparison in §6 ran on `3e7bf6d`, not on the commits measured here, and it does not cover the small-document or reused tests.

## 10. Conclusion

A schema-free, tape-driven parser can match simdjson On Demand on its home ground and beat it outright once the access pattern stops being strictly forward. `jsonifier::generic` keeps On Demand's shape and relaxes its single-use restriction. Four pieces carry that result:

- a stage-1 reader tuned for small documents;
- a shared forward cursor, so reads in document order stay cheap;
- zero-copy strings with a fixed-slot arena;
- a SIMD field index that is only built once an object has shown it needs one.

Across 1,490 single-document results on five builds, covering full-size and small documents, each read into both freshly allocated and reused output objects, Jsonifier won 1,256, tied 69 and lost 165. On reverse reads it won 346 of 360 at both sizes, and on Windows/MSVC it lost 2 single-document results of 298, by 1.5% and 2.7%. Reusing the output objects speeds up both libraries by similar amounts and leaves the verdicts largely unchanged (631-28-86 fresh, 625-41-79 reused). The main remaining targets are Linux/GCC, with 61 of the 165 losses, macOS/Clang, with 53 on a virtualized host, and small documents on Linux.

The design also carries over to streams. Because each streamed document is an ordinary `generic::document` bound to the parser's shared cursor, `iterateMany` needed no new reading machinery, only windowing and document boundaries. The one lesson it added repeats §5's: finding every document's end up front makes an in-order reader pay for two passes over the tape, and letting the reader's own cursor report where each document ended removes that pass for whitespace-delimited streams. Across 130 streaming results, Jsonifier won 110, tied 5 and lost 15, and it won every streaming test on Windows/MSVC. Instruments streams and the Stream Formats tests on macOS/Clang are the main remaining streaming targets.

The main engineering lesson is in §5. An index over the whole document made the target workload 2.3x *slower*, because most objects are small. The version that works builds the index lazily, per object, and only once that object has shown both that it is large and that it is being read out of order: enough skipped keys to clear *T*, and two lookups that had to wrap around. In short: make the in-order read cost nothing, and only pay for an index on objects whose access pattern shows it is needed.

## 11. Dedication

This paper is dedicated to the author's Little Buddy.