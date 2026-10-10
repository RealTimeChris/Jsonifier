# Schema-Free, On Demand: Generic JSON Parsing in Jsonifier

**Nihilai Collective Corp — Engineering Papers**  
*Nihilai Collective Corp*  
*October 2026 — Jsonifier*  

---

## Abstract

Jsonifier's two earlier papers were about parsing JSON into known types. If the parser knows the shape of the document at compile time, it can read in a single pass and skip building a structural tape. This paper covers the opposite case: **schema-free parsing**, where the caller walks a document of unknown shape and reads only the values it wants. That is the workload simdjson's On Demand API was built for, and the one where a structural index is actually worth building.

`jsonifier::generic` is a lazy, tape-driven parser. Its API is shaped like simdjson On Demand, but it relaxes one restriction: a value can be read more than once. This paper describes the design choices that let it match or beat On Demand on its own ground:

- a stage-1 reader tuned for small documents;
- a forward cursor that assumes reads happen in document order;
- zero-copy string extraction;
- a SIMD field index that is only built for objects that are actually being read out of order.

The benchmark suite reads every document four ways: every field in order, every field with each object's keys in reverse, a sparse subset of fields in order, and that subset in reverse. Each document is read both at full size and as a small (≤5 KiB) copy, and every single-document test runs twice: once filling freshly allocated output objects on every iteration, and once reusing the same cleared output objects, whose buffers keep their capacity. Across five platform/compiler builds, Jsonifier won 1,208 of 1,490 single-document results against simdjson On Demand, tied 69 and lost 213. On reverse-order reads it won 339 of 360. On Windows/MSVC it won 291 of 298 single-document results, tied 3 and lost 4.

The same parser also reads streams of many documents, such as NDJSON or comma-separated records, through `iterateMany`, the counterpart of simdjson's `iterate_many` (§8). Across five platform/compiler builds, on 26 streaming tests, Jsonifier won 112 of 130 results, tied 5 and lost 13, and won every streaming test on Windows/MSVC.

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

### 3.5 Runtime CPU selection

`jsonifier::generic` takes part in the same runtime tier selection as the rest of the library. On x86, one Jsonifier binary carries every instruction-set tier it was configured for and picks one at run time. The headers are compiled once per tier inside a target-attribute region (`backend_passes.hpp`: `#pragma GCC target` on GCC, `#pragma clang attribute` on Clang), which yields one complete `jsonifier_core_internal<backend>` per tier: AVX-512 (`avx512f`, `avx512bw`, `avx512vbmi2`), AVX2 and AVX. Each of those inherits its tier's parser, serializer, validator, minifier, prettifier and generic iterator through CRTP, so every call inside a tier is a direct, inlinable call into code built for that tier. On first use, `selectSupportedBackend` reads CPUID (leaves 0, 1 and 7, plus the extended leaves for `lzcnt`) and XCR0, so a tier counts only if both the CPU and the operating system support it, and caches the widest supported tier in a function-local `static`. Every public entry point then dispatches with a fold expression over the compiled tiers:

```cpp
static const jsonifier_backend backend{ selectedBackend() };
static_cast<void>(((backend == cores::backendType && (result = cores::template parseJson<options>(internal::forward<value_type>(object), in), true)) || ...));
```

That is a load of a cached enum and at most three compares, once per call. There is no function pointer and no vtable. simdjson's runtime dispatch, by contrast, goes through virtual functions: `implementation` exposes `create_dom_parser_implementation`, `minify` and `validate_utf8` as virtuals, and `dom_parser_implementation` declares `parse`, `stage1`, `stage2` and `parse_string` pure virtual. Its own On Demand documentation notes that the On Demand API "has limited runtime dispatch support" and recommends compiling for a specific x64 target. In Jsonifier the same dispatch covers every API, including typed parsing, serialization and the generic On Demand-style iterator, and inside a tier nothing is virtual, so the per-tier code keeps all of its compile-time specialization.

The scope is x86. ARM builds compile one backend (NEON, or SVE2 when configured), as simdjson's ARM builds do. The set of x86 tiers compiled in is bounded by the configured tier (`JSONIFIER_CONFIGURED_AVX_TIER`), so a build configured for AVX2 carries AVX2 and AVX but not AVX-512. We have not separately measured the cost of the per-call check. This is the case simdjson's On Demand documentation leaves to the caller: a generic Jsonifier reader compiled once for x86 runs its AVX-512, AVX2 or AVX stage 1 depending on the machine, with no change to the calling code.

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
| macOS / Clang | Marine IK Reverse (min) | — | +55% | **−6%** | — |
| macOS / Clang | Random Reverse (min) | tie | — | +43% | **−27%** |
| macOS / GCC | Random Reverse (min) | +32% | +35% | — | **−38%** |

*"—" means the result did not converge in that run.*

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
- **Convergence:** a result counts only if its retained epoch meets the RSE and mean-shift limits; non-converged results are dropped from all tallies. The x86 builds used RSE < 5% and shift < 2.5%. The NEON builds ran on a virtualized M1 and used RSE < 10% and shift < 5%.
- **Integer parsing:** Jsonifier reads integers with a SWAR parser ported from void-numerics, which consumes digits in 8-, 4-, 2- and 1-byte chunks instead of one at a time.
- **Builds:**

| Build | Stage-1 kernel | Converged, full size | Converged, small | Converged, total |
|---|---|---|---|---|
| Windows 10.0.26200 / MSVC 19.44 | AVX2 | 154 / 154 | 144 / 144 | 298 / 298 |
| Linux 7.0.0 / Clang 24.0 | AVX2 | 154 / 154 | 144 / 144 | 298 / 298 |
| Linux 7.0.0 / GCC 16.2 | AVX2 | 154 / 154 | 144 / 144 | 298 / 298 |
| macOS 25.6 (M1, virtual) / GCC 16.2 | NEON | 154 / 154 | 144 / 144 | 298 / 298 |
| macOS 25.6 (M1, virtual) / Clang 23.1 | NEON | 154 / 154 | 144 / 144 | 298 / 298 |

All builds ran BenchmarkSuite `125f8a7`. Jsonifier was `6bd7669` and simdjson `5147800` on Windows/MSVC; the other four builds ran Jsonifier `09747e9` and simdjson `7f6f8dc`. Jsonifier selects its generic stage-1 kernel at run time from the detected CPU; the table lists the kernel each build ran. The x86 builds ran on an Intel Core i9-14900KF. The results were published to `generic-parsing-main` on October 11, 2026.

## 7. Results

The percentages below are Jsonifier's throughput relative to simdjson On Demand. "tie" means Welch's t-test could not separate the two; "—" means the result did not converge. In the per-test tables, each cell gives the **fresh** result first and the **reused** result second.

### 7.1 Summary

**Fresh output objects:**

| Build | POD | In order | Reverse | Sparse | Sparse reverse | Small, in order | Small, reverse | Small, sparse | Small, sparse reverse | Total |
|---|---|---|---|---|---|---|---|---|---|---|
| Windows / MSVC | 4-0-1 | 17-1-0 | 18-0-0 | 18-0-0 | 18-0-0 | 16-1-1 | 18-0-0 | 18-0-0 | 18-0-0 | 145-2-2 |
| Linux / Clang | 5-0-0 | 17-0-1 | 18-0-0 | 16-0-2 | 18-0-0 | 11-1-6 | 18-0-0 | 10-0-8 | 16-2-0 | 129-3-17 |
| Linux / GCC | 5-0-0 | 5-1-12 | 14-0-4 | 16-0-2 | 16-0-2 | 4-0-14 | 16-0-2 | 11-1-6 | 18-0-0 | 105-2-42 |
| macOS / GCC | 4-0-1 | 11-3-4 | 17-0-1 | 14-2-2 | 17-0-1 | 13-1-4 | 18-0-0 | 17-1-0 | 17-0-1 | 128-7-14 |
| macOS / Clang | 3-0-2 | 8-4-6 | 18-0-0 | 9-5-4 | 8-5-5 | 10-3-5 | 16-1-1 | 8-1-9 | 10-2-6 | 90-21-38 |
| **All** | **21-0-4** | **58-9-23** | **85-0-5** | **73-7-10** | **77-5-8** | **54-6-30** | **86-1-3** | **64-3-23** | **79-4-7** | **597-35-113** |

**Reused output objects:**

| Build | POD | In order | Reverse | Sparse | Sparse reverse | Small, in order | Small, reverse | Small, sparse | Small, sparse reverse | Total |
|---|---|---|---|---|---|---|---|---|---|---|
| Windows / MSVC | 4-0-1 | 17-1-0 | 18-0-0 | 18-0-0 | 18-0-0 | 17-0-1 | 18-0-0 | 18-0-0 | 18-0-0 | 146-1-2 |
| Linux / Clang | 5-0-0 | 16-2-0 | 18-0-0 | 15-1-2 | 18-0-0 | 13-0-5 | 18-0-0 | 10-1-7 | 14-2-2 | 127-6-16 |
| Linux / GCC | 5-0-0 | 8-0-10 | 14-0-4 | 15-2-1 | 16-0-2 | 8-0-10 | 16-0-2 | 13-0-5 | 18-0-0 | 113-2-34 |
| macOS / GCC | 5-0-0 | 13-1-4 | 18-0-0 | 16-0-2 | 16-1-1 | 15-1-2 | 17-1-0 | 15-3-0 | 18-0-0 | 133-7-9 |
| macOS / Clang | 3-0-2 | 9-4-5 | 15-1-2 | 4-5-9 | 11-5-2 | 10-2-6 | 16-0-2 | 9-1-8 | 15-0-3 | 92-18-39 |
| **All** | **22-0-3** | **63-8-19** | **83-1-6** | **68-8-14** | **79-6-5** | **63-3-24** | **85-1-4** | **65-5-20** | **83-2-5** | **611-34-100** |


Each cell is wins-ties-losses.

Across all 1,490 single-document results, every one of which converged, Jsonifier won 1,208, tied 69 and lost 213: 597-35-113 with fresh output objects and 611-34-100 with reused ones.

- **Reverse order is almost all wins at both sizes.** Across fresh and reused runs, Jsonifier won 168 of 180 full-size reverse results (1 tie, 11 losses) and 171 of 180 small reverse results (2 ties, 7 losses). All 18 of those losses are on Canada or Google Maps: twelve on Linux/GCC (Canada at both sizes, −3.1% to −5.8%, and Google Maps, −4.3% to −29.2%), one on macOS/GCC (minified Canada, fresh, −3.7%) and five on macOS/Clang (Canada, −4.4% to −37.8%, and Google Maps, −7.1% and −7.9%).
- **Windows/MSVC lost 4 of 298 single-document results:** Canada Small read in order (pretty, fresh, −1.0%; min, reused, −1.2%) and the String POD test (fresh, −2.5%; reused, −6.6%). Its 3 ties are Canada read in order (full size, pretty, fresh and reused) and Canada Small (min, fresh) read in order. It won every reverse, sparse and sparse-reverse test, at both sizes and in both allocation modes.
- **Reusing the output objects helps both libraries and changes few verdicts.** Comparing each test's reused run with its fresh run, the median speedup from reuse is 6.0–10.8% for Jsonifier and 4.8–8.3% for simdjson, depending on the build. Jsonifier's median margin over simdjson moves by −0.7 to +2.9 percentage points on the non-MSVC builds and by +9.1 points on MSVC, where allocation is most expensive: Discord Reverse (min) goes from +1494.0% to +1680.3%, and Mesh Small Sparse (min) from +112.2% to +194.2%.

### 7.2 POD tests

| Test | MSVC | Linux Clang | Linux GCC | macOS GCC | macOS Clang |
|---|---|---|---|---|---|
| Bool | +1.4% / +4.8% | +34.7% / +34.8% | +49.3% / +40.8% | +15.6% / +30.6% | −16.3% / +11.9% |
| Double | +19.9% / +22.3% | +57.1% / +56.7% | +52.3% / +53.7% | +76.5% / +98.2% | +84.9% / +66.2% |
| Int64 | +10.4% / +5.7% | +68.6% / +65.5% | +37.7% / +39.2% | +26.8% / +19.6% | +19.6% / +14.7% |
| String | −2.5% / −6.6% | +9.5% / +13.7% | +3.4% / +7.8% | −12.8% / +5.1% | −11.9% / −23.0% |
| Uint64 | +10.3% / +11.7% | +58.5% / +57.1% | +52.2% / +58.0% | +63.5% / +26.3% | +6.2% / −15.9% |

### 7.3 Documents, in order

| Test | MSVC | Linux Clang | Linux GCC | macOS GCC | macOS Clang |
|---|---|---|---|---|---|
| Canada (min) | +2.2% / +2.5% | +5.3% / +8.1% | −6.9% / −9.4% | +9.3% / +12.2% | +20.0% / +13.4% |
| Canada (pretty) | tie / tie | +5.3% / +6.2% | −7.4% / −9.6% | −7.8% / −6.7% | −11.3% / +26.1% |
| CitmCatalog (min) | +31.2% / +33.8% | +4.8% / +4.9% | +2.3% / +3.1% | tie / +26.3% | tie / +21.9% |
| CitmCatalog (pretty) | +29.6% / +32.0% | +3.0% / +4.4% | +0.5% / +2.4% | +12.0% / +23.0% | +39.1% / tie |
| Discord (min) | +48.9% / +72.3% | +10.3% / +16.2% | −5.5% / +1.2% | +16.1% / +25.2% | +4.7% / +6.2% |
| Discord (pretty) | +51.5% / +71.5% | +9.6% / +15.3% | −4.8% / −1.0% | +15.3% / +30.0% | −21.3% / +1.5% |
| Google Maps (min) | +34.7% / +39.0% | +19.2% / +20.4% | −4.1% / −22.2% | +22.6% / +30.5% | tie / +26.7% |
| Google Maps (pretty) | +31.9% / +31.5% | +15.6% / +14.9% | +7.6% / +78.6% | +23.1% / +17.8% | −10.6% / −8.7% |
| Instruments (min) | +53.2% / +65.2% | +9.4% / +10.0% | tie / −29.5% | +12.1% / +11.6% | +22.7% / tie |
| Instruments (pretty) | +55.2% / +62.9% | +7.3% / +7.3% | −5.2% / −7.4% | tie / +5.8% | tie / tie |
| Marine IK (min) | +18.4% / +16.1% | +3.1% / +2.2% | −5.1% / −5.8% | −3.5% / −7.2% | +5.5% / tie |
| Marine IK (pretty) | +17.2% / +18.3% | +4.9% / tie | −4.5% / −4.9% | tie / tie | −4.7% / −4.9% |
| Mesh (min) | +54.2% / +62.9% | +1.1% / tie | −4.0% / −1.8% | −14.6% / −8.0% | tie / +22.7% |
| Mesh (pretty) | +50.0% / +58.3% | −1.8% / +1.9% | −6.7% / −5.2% | −29.6% / −9.7% | −24.4% / −22.8% |
| Random (min) | +53.7% / +64.5% | +18.5% / +24.1% | +15.2% / +17.8% | +34.7% / +42.4% | +19.7% / +24.6% |
| Random (pretty) | +45.3% / +63.5% | +18.0% / +20.6% | +12.8% / +17.0% | +36.0% / +27.6% | +18.8% / +36.1% |
| Twitter (min) | +35.6% / +49.7% | +5.3% / +18.0% | −0.7% / +9.6% | +10.7% / +2.3% | −5.0% / −9.8% |
| Twitter (pretty) | +34.4% / +41.9% | +4.0% / +6.6% | −2.6% / +6.9% | +12.5% / +16.7% | +15.7% / −22.6% |

### 7.4 Documents, keys in reverse order

| Test | MSVC | Linux Clang | Linux GCC | macOS GCC | macOS Clang |
|---|---|---|---|---|---|
| Canada (min) | +18.4% / +20.5% | +5.4% / +4.5% | −3.1% / −3.5% | −3.7% / +21.2% | +16.8% / −4.4% |
| Canada (pretty) | +16.0% / +19.5% | +5.0% / +2.7% | −5.5% / −5.2% | +20.0% / +5.4% | +13.0% / tie |
| CitmCatalog (min) | +305.0% / +314.0% | +71.6% / +73.9% | +75.7% / +81.8% | +50.5% / +88.5% | +79.0% / +56.5% |
| CitmCatalog (pretty) | +295.1% / +311.2% | +65.6% / +68.4% | +73.0% / +77.1% | +51.3% / +42.0% | +60.2% / +63.8% |
| Discord (min) | +1494.0% / +1680.3% | +251.9% / +280.4% | +321.0% / +321.7% | +320.9% / +369.4% | +229.6% / +325.7% |
| Discord (pretty) | +1502.0% / +1676.8% | +250.3% / +273.4% | +257.3% / +316.3% | +313.0% / +355.3% | +251.9% / +365.8% |
| Google Maps (min) | +116.9% / +121.2% | +8.7% / +10.1% | −29.2% / −7.6% | +19.7% / +21.2% | +25.2% / +12.7% |
| Google Maps (pretty) | +104.9% / +114.9% | +8.5% / +9.4% | −5.6% / −4.3% | +16.4% / +18.6% | +26.2% / −7.9% |
| Instruments (min) | +664.4% / +691.8% | +124.4% / +127.4% | +137.5% / +140.9% | +138.9% / +135.8% | +73.5% / +94.4% |
| Instruments (pretty) | +660.3% / +683.0% | +116.4% / +116.2% | +126.4% / +131.3% | +121.1% / +139.1% | +115.6% / +121.1% |
| Marine IK (min) | +313.7% / +317.4% | +33.2% / +35.8% | +68.1% / +72.1% | +87.5% / +52.8% | +42.8% / +54.4% |
| Marine IK (pretty) | +308.5% / +316.1% | +30.7% / +33.2% | +47.3% / +51.9% | +58.6% / +30.9% | +28.3% / +31.5% |
| Mesh (min) | +365.8% / +407.4% | +35.6% / +38.0% | +48.4% / +51.3% | +59.6% / +32.0% | +35.5% / +38.8% |
| Mesh (pretty) | +350.6% / +387.3% | +28.3% / +36.9% | +41.8% / +46.3% | +39.8% / +52.2% | +28.5% / +50.0% |
| Random (min) | +227.9% / +253.5% | +25.7% / +27.9% | +19.4% / +21.1% | +54.8% / +31.2% | +43.2% / +23.4% |
| Random (pretty) | +224.2% / +250.9% | +24.9% / +27.5% | +19.9% / +20.5% | +39.9% / +49.2% | +8.5% / +9.5% |
| Twitter (min) | +689.0% / +775.1% | +199.3% / +213.8% | +125.6% / +135.6% | +171.9% / +165.0% | +163.5% / +209.6% |
| Twitter (pretty) | +678.5% / +739.2% | +183.7% / +196.1% | +113.7% / +123.0% | +142.9% / +168.8% | +178.2% / +150.7% |

### 7.5 Documents, sparse fields in order

| Test | MSVC | Linux Clang | Linux GCC | macOS GCC | macOS Clang |
|---|---|---|---|---|---|
| Canada (min) | +63.4% / +63.3% | +5.5% / +4.8% | +20.2% / +14.8% | tie / +3.4% | +34.7% / +10.0% |
| Canada (pretty) | +53.4% / +50.4% | +15.7% / +16.1% | +9.9% / +11.3% | +3.6% / +16.5% | −9.6% / −12.3% |
| CitmCatalog (min) | +235.2% / +238.5% | +21.3% / +21.1% | +17.9% / +17.2% | −7.8% / +9.2% | +20.1% / tie |
| CitmCatalog (pretty) | +177.4% / +181.2% | +17.6% / +18.2% | +12.7% / +14.1% | −6.5% / +13.2% | +13.9% / +6.3% |
| Discord (min) | +30.3% / +39.2% | −4.7% / −6.6% | +1.7% / tie | +18.0% / +18.7% | tie / −1.2% |
| Discord (pretty) | +30.6% / +39.3% | −2.9% / −7.2% | +9.0% / −9.6% | +15.2% / +17.3% | +4.7% / −14.4% |
| Google Maps (min) | +59.6% / +60.6% | +5.1% / +4.4% | −8.7% / tie | +8.2% / +15.0% | tie / −5.9% |
| Google Maps (pretty) | +51.6% / +56.3% | +3.7% / +4.5% | −17.8% / +5.9% | +9.1% / +11.7% | +7.2% / −10.7% |
| Instruments (min) | +40.4% / +42.6% | +2.4% / +2.0% | +4.1% / +5.0% | +10.6% / +7.1% | −11.0% / −5.3% |
| Instruments (pretty) | +44.2% / +46.9% | +1.4% / +2.5% | +1.4% / +1.7% | +19.0% / +14.7% | tie / −15.7% |
| Marine IK (min) | +49.2% / +53.2% | +12.4% / +12.3% | +32.3% / +31.9% | +22.0% / +10.0% | +23.7% / +29.5% |
| Marine IK (pretty) | +47.6% / +49.3% | +12.2% / +14.4% | +14.9% / +19.1% | +16.3% / +13.7% | tie / +13.0% |
| Mesh (min) | +293.2% / +306.4% | +8.1% / +7.4% | +20.5% / +21.0% | tie / −1.3% | +36.7% / tie |
| Mesh (pretty) | +250.5% / +262.8% | +5.3% / +7.1% | +11.9% / +13.4% | +8.3% / −3.7% | +8.9% / −3.9% |
| Random (min) | +55.9% / +66.9% | +10.7% / +11.6% | +3.0% / +4.6% | +39.6% / +34.3% | tie / tie |
| Random (pretty) | +52.9% / +62.8% | +11.0% / +13.0% | +1.3% / +3.5% | +57.1% / +43.5% | +15.3% / tie |
| Twitter (min) | +140.9% / +161.0% | +2.9% / +2.5% | +4.7% / +6.1% | +21.1% / +21.6% | −11.5% / −11.6% |
| Twitter (pretty) | +126.3% / +131.3% | +0.4% / tie | +3.0% / +2.9% | +7.2% / +20.1% | −1.4% / tie |

### 7.6 Documents, sparse fields in reverse order

| Test | MSVC | Linux Clang | Linux GCC | macOS GCC | macOS Clang |
|---|---|---|---|---|---|
| Canada (min) | +267.6% / +265.4% | +3.5% / +1.6% | +25.8% / +20.6% | +31.5% / +30.1% | +8.5% / +23.9% |
| Canada (pretty) | +175.5% / +176.3% | +10.2% / +11.8% | +9.5% / +8.9% | +32.8% / tie | −4.4% / +23.2% |
| CitmCatalog (min) | +231.8% / +234.9% | +32.5% / +32.5% | +75.7% / +32.6% | +13.0% / +18.8% | tie / +65.8% |
| CitmCatalog (pretty) | +193.8% / +209.9% | +27.0% / +27.3% | +48.6% / +40.5% | +19.3% / +18.8% | tie / tie |
| Discord (min) | +132.0% / +153.7% | +4.1% / +5.8% | +13.0% / −7.7% | +24.3% / +28.4% | +6.5% / +29.4% |
| Discord (pretty) | +129.8% / +152.3% | +3.5% / +4.9% | −37.4% / +3.8% | +30.5% / +28.6% | tie / +1.8% |
| Google Maps (min) | +71.1% / +87.0% | +5.8% / +6.7% | +10.7% / +5.5% | +9.8% / +7.8% | −15.8% / −5.5% |
| Google Maps (pretty) | +68.2% / +80.6% | +7.0% / +5.4% | −1.4% / −37.3% | +13.1% / +15.3% | −6.9% / −19.3% |
| Instruments (min) | +184.5% / +195.0% | +2.4% / +1.3% | +24.4% / +24.0% | +23.0% / +39.3% | +3.6% / tie |
| Instruments (pretty) | +174.2% / +180.9% | +2.4% / +1.9% | +19.1% / +19.9% | +22.2% / +26.0% | tie / +4.3% |
| Marine IK (min) | +199.7% / +192.1% | +18.3% / +20.3% | +54.9% / +56.3% | +43.8% / +30.0% | +36.7% / tie |
| Marine IK (pretty) | +160.7% / +159.9% | +14.5% / +13.5% | +27.2% / +29.7% | +25.3% / +15.3% | tie / +19.8% |
| Mesh (min) | +284.7% / +308.7% | +8.8% / +6.3% | +22.5% / +20.4% | +5.2% / +19.6% | +40.1% / +16.6% |
| Mesh (pretty) | +247.8% / +266.8% | +4.5% / +8.0% | +13.2% / +14.3% | −1.5% / −6.8% | −15.2% / tie |
| Random (min) | +79.9% / +92.0% | +5.3% / +7.1% | +9.6% / +9.3% | +27.3% / +13.0% | −5.4% / +51.8% |
| Random (pretty) | +77.8% / +87.4% | +5.1% / +5.9% | +10.6% / +9.5% | +26.5% / +25.3% | +22.6% / +11.1% |
| Twitter (min) | +196.7% / +194.4% | +11.7% / +11.8% | +18.2% / +15.8% | +30.2% / +31.1% | +13.0% / +23.0% |
| Twitter (pretty) | +182.0% / +188.6% | +10.6% / +9.0% | +12.5% / +12.1% | +18.7% / +48.8% | +12.9% / tie |

### 7.7 Small documents, in order

| Test | MSVC | Linux Clang | Linux GCC | macOS GCC | macOS Clang |
|---|---|---|---|---|---|
| Canada (min) | tie / −1.2% | +3.5% / +6.6% | −12.2% / −13.9% | +6.8% / +5.8% | +18.5% / −3.7% |
| Canada (pretty) | −1.0% / +2.7% | +3.4% / +5.5% | −11.2% / −15.5% | +3.2% / +18.1% | +28.4% / +19.2% |
| CitmCatalog (min) | +22.1% / +23.3% | −5.6% / −3.6% | −6.1% / −3.3% | +9.6% / +13.8% | −8.0% / −17.9% |
| CitmCatalog (pretty) | +16.9% / +23.2% | −5.9% / −4.2% | −6.0% / −4.4% | +17.6% / +28.3% | +14.8% / −18.1% |
| Discord (min) | +39.5% / +61.7% | −1.9% / +5.0% | −2.0% / +4.5% | +17.6% / +29.2% | +2.9% / +11.5% |
| Discord (pretty) | +40.0% / +55.3% | tie / +3.5% | −1.8% / +4.3% | +12.9% / +33.2% | +3.0% / tie |
| Google Maps (min) | +27.2% / +39.9% | +18.2% / +18.9% | +13.1% / +14.7% | +24.0% / +34.3% | tie / +6.1% |
| Google Maps (pretty) | +28.3% / +38.2% | +14.7% / +15.9% | +12.6% / +13.2% | +20.7% / +25.0% | −11.3% / +1.0% |
| Instruments (min) | +49.8% / +58.1% | +2.9% / +3.3% | −4.6% / −6.3% | −58.2% / +11.1% | +19.4% / −54.1% |
| Instruments (pretty) | +47.4% / +50.8% | +1.2% / +0.4% | −7.2% / −8.6% | tie / +5.6% | +162.9% / −27.5% |
| Marine IK (min) | +4.3% / +7.2% | +1.8% / +2.4% | −3.8% / −4.3% | +5.4% / +10.5% | tie / tie |
| Marine IK (pretty) | +7.3% / +10.0% | +0.9% / −0.2% | −3.9% / −6.0% | −2.8% / tie | −2.2% / −6.7% |
| Mesh (min) | +8.4% / +33.4% | −4.4% / −7.8% | −9.3% / −9.8% | −15.7% / −10.8% | tie / +4.8% |
| Mesh (pretty) | +12.7% / +35.3% | −6.2% / −6.3% | −10.3% / −13.7% | −9.8% / −10.6% | −3.1% / +4.3% |
| Random (min) | +42.0% / +49.5% | +15.1% / +17.0% | +16.0% / +23.6% | +51.1% / +57.9% | +17.7% / +16.9% |
| Random (pretty) | +46.0% / +55.7% | +13.3% / +17.8% | +17.3% / +21.2% | +20.9% / +44.9% | +13.5% / +16.5% |
| Twitter (min) | +38.3% / +51.5% | +0.8% / +2.1% | −2.0% / +1.5% | +14.6% / +20.1% | −1.3% / +6.9% |
| Twitter (pretty) | +37.4% / +45.9% | −1.7% / +3.0% | −1.6% / +0.6% | +11.4% / +20.8% | +1.8% / +0.9% |

### 7.8 Small documents, keys in reverse order

| Test | MSVC | Linux Clang | Linux GCC | macOS GCC | macOS Clang |
|---|---|---|---|---|---|
| Canada (min) | +13.9% / +14.7% | +9.4% / +11.7% | −5.5% / −5.5% | +8.9% / tie | −4.6% / +17.3% |
| Canada (pretty) | +16.5% / +16.8% | +12.0% / +8.6% | −5.4% / −5.8% | +3.9% / +12.4% | +51.4% / −37.8% |
| CitmCatalog (min) | +215.1% / +237.7% | +51.5% / +54.9% | +68.8% / +77.6% | +54.7% / +66.5% | +44.9% / +57.0% |
| CitmCatalog (pretty) | +218.2% / +240.2% | +48.6% / +54.7% | +64.4% / +73.0% | +49.5% / +59.0% | +31.2% / +50.7% |
| Discord (min) | +1239.4% / +1485.2% | +262.2% / +290.9% | +275.5% / +311.6% | +341.0% / +386.6% | +300.5% / +180.4% |
| Discord (pretty) | +1211.5% / +1450.1% | +251.9% / +287.3% | +260.7% / +298.3% | +325.8% / +381.8% | +299.5% / +289.4% |
| Google Maps (min) | +109.9% / +122.2% | +9.2% / +9.2% | +21.7% / +21.5% | +24.3% / +32.6% | tie / +3.5% |
| Google Maps (pretty) | +102.1% / +118.3% | +5.6% / +7.5% | +19.1% / +18.4% | +20.4% / +15.7% | +9.5% / −7.1% |
| Instruments (min) | +664.9% / +713.4% | +126.6% / +130.1% | +139.5% / +137.9% | +139.4% / +154.9% | +132.0% / +117.2% |
| Instruments (pretty) | +667.9% / +670.4% | +118.2% / +125.2% | +130.2% / +132.0% | +135.7% / +152.9% | +91.3% / +67.5% |
| Marine IK (min) | +189.7% / +186.4% | +34.2% / +33.5% | +55.1% / +58.5% | +48.7% / +29.6% | +69.1% / +25.4% |
| Marine IK (pretty) | +187.5% / +187.9% | +29.2% / +32.1% | +51.2% / +53.9% | +45.9% / +46.7% | +10.0% / +41.0% |
| Mesh (min) | +121.1% / +204.3% | +20.0% / +24.2% | +26.4% / +33.1% | +27.5% / +11.9% | +29.9% / +32.3% |
| Mesh (pretty) | +120.6% / +203.5% | +20.0% / +20.6% | +22.1% / +26.7% | +13.1% / +32.0% | +18.3% / +33.6% |
| Random (min) | +247.2% / +279.3% | +45.0% / +49.3% | +44.7% / +47.4% | +52.9% / +61.1% | +42.2% / +36.9% |
| Random (pretty) | +240.3% / +275.6% | +45.3% / +48.6% | +40.6% / +44.9% | +62.5% / +56.7% | +48.6% / +42.7% |
| Twitter (min) | +705.7% / +761.8% | +162.6% / +174.3% | +140.7% / +151.4% | +174.3% / +154.2% | +161.0% / +167.7% |
| Twitter (pretty) | +729.8% / +740.3% | +151.8% / +162.7% | +135.0% / +144.0% | +168.5% / +164.9% | +139.7% / +129.1% |

### 7.9 Small documents, sparse fields in order

| Test | MSVC | Linux Clang | Linux GCC | macOS GCC | macOS Clang |
|---|---|---|---|---|---|
| Canada (min) | +28.3% / +20.3% | +21.9% / +20.8% | +14.4% / +13.1% | +34.1% / +38.1% | −6.9% / −18.0% |
| Canada (pretty) | +26.6% / +32.5% | +21.6% / +24.4% | +13.4% / +15.1% | +24.6% / +23.5% | −7.4% / tie |
| CitmCatalog (min) | +161.3% / +171.8% | +11.7% / +12.1% | +16.4% / +17.6% | +9.5% / +13.8% | +4.1% / +14.4% |
| CitmCatalog (pretty) | +122.8% / +136.5% | +8.0% / +9.5% | +12.6% / +15.6% | +13.9% / +16.3% | +25.1% / +10.3% |
| Discord (min) | +30.3% / +39.2% | −1.6% / −3.3% | −2.2% / −1.0% | +19.6% / tie | −2.4% / −8.7% |
| Discord (pretty) | +27.0% / +35.3% | −1.1% / −8.5% | −5.0% / −1.3% | +16.4% / +18.2% | −8.7% / −21.7% |
| Google Maps (min) | +54.5% / +63.4% | −1.1% / tie | −6.5% / −5.9% | +14.4% / +6.5% | −2.1% / −6.0% |
| Google Maps (pretty) | +54.3% / +59.4% | −1.5% / −3.7% | −2.6% / −4.3% | +13.9% / +20.5% | −4.3% / +12.3% |
| Instruments (min) | +30.1% / +48.5% | −2.8% / −3.1% | +1.4% / +0.9% | +12.0% / +6.6% | tie / −2.5% |
| Instruments (pretty) | +26.0% / +31.8% | −3.2% / −0.9% | tie / +0.3% | tie / +2.5% | −5.6% / −7.3% |
| Marine IK (min) | +30.4% / +25.6% | +3.8% / +3.5% | +29.6% / +27.3% | +7.9% / +8.0% | −5.3% / +36.3% |
| Marine IK (pretty) | +31.2% / +33.0% | +10.5% / +6.7% | +23.3% / +22.0% | +17.8% / +20.8% | +3.4% / +17.6% |
| Mesh (min) | +112.2% / +194.2% | +5.5% / +4.2% | +15.0% / +17.0% | +15.6% / +6.7% | +19.9% / +10.9% |
| Mesh (pretty) | +93.4% / +132.5% | +2.3% / +0.8% | +10.1% / +12.6% | +5.7% / tie | +1.6% / −2.0% |
| Random (min) | +43.5% / +56.5% | +2.6% / +4.0% | +1.6% / +4.4% | +22.7% / +19.4% | +2.6% / +4.5% |
| Random (pretty) | +41.5% / +51.3% | +1.2% / +4.2% | +4.5% / +4.0% | +27.4% / +20.1% | +1.7% / +4.6% |
| Twitter (min) | +99.6% / +119.5% | −11.8% / −12.4% | −2.5% / +0.9% | +14.7% / tie | +4.5% / −15.9% |
| Twitter (pretty) | +96.7% / +102.4% | −14.5% / −12.4% | −2.7% / −1.8% | +9.6% / +17.5% | −9.1% / +4.3% |

### 7.10 Small documents, sparse fields in reverse order

| Test | MSVC | Linux Clang | Linux GCC | macOS GCC | macOS Clang |
|---|---|---|---|---|---|
| Canada (min) | +146.6% / +131.1% | +27.1% / +26.7% | +22.9% / +21.8% | +40.2% / +35.0% | −27.1% / +14.9% |
| Canada (pretty) | +129.4% / +138.5% | +24.2% / +24.3% | +20.3% / +21.4% | +20.5% / +23.2% | −4.7% / −10.3% |
| CitmCatalog (min) | +146.5% / +142.3% | +19.7% / +18.4% | +26.5% / +27.6% | +20.2% / +22.4% | −5.3% / +7.9% |
| CitmCatalog (pretty) | +146.7% / +134.8% | +17.6% / +18.8% | +24.7% / +22.5% | +19.0% / +22.9% | +8.6% / +8.6% |
| Discord (min) | +152.4% / +179.3% | +3.6% / +4.6% | +4.3% / +6.2% | +26.2% / +29.8% | tie / +5.4% |
| Discord (pretty) | +149.8% / +175.4% | +0.8% / +3.5% | +5.3% / +4.8% | +22.1% / +36.1% | tie / +15.2% |
| Google Maps (min) | +77.5% / +88.0% | +6.0% / +6.2% | +7.2% / +5.7% | +6.5% / +10.4% | −6.4% / −17.8% |
| Google Maps (pretty) | +71.0% / +71.4% | +4.8% / +4.5% | +5.6% / +6.4% | +10.3% / +17.1% | +26.2% / +5.8% |
| Instruments (min) | +180.8% / +192.2% | tie / +0.5% | +17.5% / +18.2% | +49.3% / +28.6% | +17.7% / +17.0% |
| Instruments (pretty) | +160.2% / +182.2% | +0.2% / +0.4% | +15.2% / +14.4% | +24.8% / +29.5% | +5.2% / +3.2% |
| Marine IK (min) | +111.4% / +113.8% | +14.2% / +14.1% | +29.6% / +34.0% | +24.4% / +32.2% | +34.7% / +13.8% |
| Marine IK (pretty) | +99.1% / +94.7% | +13.0% / +11.3% | +27.4% / +27.3% | +29.8% / +29.4% | −19.8% / +7.9% |
| Mesh (min) | +121.8% / +169.6% | +1.4% / −1.1% | +12.6% / +13.9% | +30.8% / +21.7% | +10.2% / +19.2% |
| Mesh (pretty) | +103.4% / +149.1% | tie / tie | +5.6% / +9.5% | −6.8% / +23.7% | +7.5% / −0.7% |
| Random (min) | +71.0% / +84.7% | +5.6% / +5.1% | +9.3% / +9.0% | +18.4% / +32.1% | +3.9% / +2.8% |
| Random (pretty) | +66.9% / +76.3% | +2.0% / +2.4% | +7.7% / +8.3% | +9.6% / +23.2% | +3.1% / +4.8% |
| Twitter (min) | +155.4% / +172.1% | +0.9% / tie | +5.4% / +5.3% | +34.8% / +37.6% | −3.3% / +0.9% |
| Twitter (pretty) | +154.8% / +169.4% | +0.5% / −1.5% | +5.4% / +3.9% | +20.4% / +20.6% | +5.4% / +2.1% |

### 7.11 What the results show

- **Reverse order is where the design pays off.** On documents whose objects have many fields (Discord, Twitter, Instruments), simdjson has to rescan for every out-of-order key, and Jsonifier is 1.7x to 17.8x faster at full size: +73.5% up to +1680.3% (Discord min, MSVC, reused). The small documents show the same pattern, from +67.5% to +1485.2%, because a 5 KiB Discord or Instruments document still holds objects with dozens of keys. On documents made of small objects (Canada, Google Maps) there is little to rescan, and outside MSVC the full-size margin runs from −29.2% to +26.2%. That matches the cost model in §5: the map only pays off when there are many skipped keys to index.
- **Sparse reverse reads are a strength.** Jsonifier won 156 of 180 full-size sparse-reverse results across both allocation modes (11 ties, 13 losses), and 162 of 180 small ones (6 ties, 12 losses). MSVC won every one of its 72, and Linux/GCC every one of its 36 small results. Of the 13 full-size losses, 7 are on macOS/Clang, 4 on Linux/GCC and 2 on macOS/GCC, and 6 of the 13 are Google Maps.
- **MSVC shows the largest gaps.** On MSVC, simdjson's rescans are far slower than on the other builds: Discord Reverse (pretty, fresh) runs at 33.6 MB/s on MSVC against 224.5 MB/s on Linux/Clang, on the same CPU. Jsonifier reads the same test at 537.8 and 786.3 MB/s. Every MSVC reverse result is a win, by +104.9% or more outside Canada (+16.0% to +20.5%); every MSVC full-size sparse result is a win, by +30.3% to +306.4%, and every sparse-reverse result by +68.2% to +308.7%.
- **Sparse reads in order are mostly wins.** Across both allocation modes, Jsonifier won 141 of 180 full-size sparse results (15 ties, 24 losses) and 129 of 180 small ones (8 ties, 43 losses). MSVC won all 36 results at each size. On full-size documents, 13 of the 24 losses are on macOS/Clang (Twitter, Google Maps, Instruments and others), 4 are Discord on Linux/Clang, 4 are on macOS/GCC (CitmCatalog fresh and Mesh reused) and 3 are on Linux/GCC (Google Maps fresh and Discord pretty, reused).
- **In-order full reads are mostly wins at full size.** Across 180 full-size in-order results, Jsonifier won 121, tied 17 and lost 42. MSVC went 34-2-0 and Linux/Clang 33-2-1. Linux/GCC split its in-order tests 13-1-22 across the two modes, macOS/GCC 24-4-8 and macOS/Clang 17-8-11.
- **Small documents read in order are the weakest category.** Across 180 results, Jsonifier won 117, tied 9 and lost 54. MSVC (33-1-2), macOS/GCC (28-2-6) and Linux/Clang (24-1-11) won most of theirs, but macOS/Clang went 20-5-11 and Linux/GCC 12-0-24. These documents are at most 5 KiB, so a parse is short, and the fixed per-parse costs of Jsonifier's stage 1 and document setup count for more. The Linux/Clang losses there are modest (−0.2% to −7.8%) and Linux/GCC's reach −15.5% on Canada, while the in-order small-document wins on Google Maps and Random on those two builds run from +12.6% to +23.6%.
- **In-order losses at full size cluster on Linux/GCC.** It lost Canada, Discord, Google Maps, Instruments, Marine IK, Mesh and Twitter in order, mostly by −0.7% to −9.6%, but minified Google Maps by −22.2% and minified Instruments by −29.5% with reused objects. macOS/GCC lost eight: Canada (pretty, −7.8% fresh and −6.7% reused), Marine IK (min, −3.5% and −7.2%) and Mesh (min, −14.6% and −8.0%; pretty, −29.6% and −9.7%). Canada, Marine IK and Mesh are dominated by float arrays, where the time goes into number parsing and stage 1, not into the generic layer's field lookup. Linux/Clang lost one full-size in-order test, prettified Mesh (fresh, −1.8%).
- **The POD tests win on most builds.** Jsonifier went 43-0-7 on them. Double won all 10 results (+19.9% to +98.2%) and Int64 all 10 (+5.7% to +68.6%). Uint64 won 9 of 10 (+6.2% to +63.5%), losing on macOS/Clang reused (−15.9%), and Bool won 9 of 10 (+1.4% to +49.3%), losing on macOS/Clang fresh (−16.3%). String is the weak POD test: it won 5 of 10 (+3.4% to +13.7%) and lost 5 (MSVC fresh, −2.5%, and reused, −6.6%; macOS/GCC fresh, −12.8%; macOS/Clang fresh, −11.9%, and reused, −23.0%).

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

Comma mode cannot use the local rule, because a top-level comma looks exactly like a comma inside a container. It keeps the up-front depth walk, and that difference is visible in the results: on the in-order corpus streams on x86, Jsonifier's comma-separated throughput is 6.4% to 23.3% below its own NDJSON throughput on the same records.

### 8.3 Method

The streaming benchmarks run in the same Json-Performance suite and the same run as §6 (branch `generic-parsing-main`), using the same adaptive-sampling stage. Both libraries use a 1 MiB batch, simdjson runs unthreaded (`parser.threaded = false`), and both fill the same structs through the same field-by-field readers. Each stream is read from a saved file and repeated to its target size, so every build streams the same bytes. There are 26 tests per build, in two groups:

- **Amazon Cellphones and Stream Formats.** These follow simdjson's streaming benchmarks. *Amazon Cellphones* is the `amazon_cellphones.ndjson` file from the simdjson repository (793 lines, each a 9-element array), with the same per-brand rating aggregation and the same skipped header line; *Large Amazon Cellphones* repeats it to 10 MiB, as simdjson's `large_amazon_cellphones` benchmark does. *Stream Formats* uses the `{"id", "name", "payload", "flag"}` documents with 16-byte (Small) and 4096-byte (Large) payloads, in NDJSON and comma-separated form, and sums `"id"` over the stream. simdjson generates 128 MB of these; the port repeats the first 64 documents to 16 MB so that a run fits the sampling window, so the ids cycle through 0–63. Every record here is an array or a four-key object, so this group has no reverse variant.
- **Record streams from the paper's corpus.** The main record array of five of the documents from §6 is split into one document per record, each record is re-serialized minified, and the stream is repeated to 4 MiB: CitmCatalog `performances`, Google Maps `rows`, Instruments `patterns`, Random `result` and Twitter `statuses`. Each stream runs in NDJSON and comma-separated form, both in order and with every object's keys read in reverse, using the reverse readers from §6. The other four documents have no record array to split and are not streamed.
- **Correctness:** for every streaming test on every build, Jsonifier's serialized output was compared with simdjson's, and all 130 pairs are byte-identical. They are part of the 515 pairs in §6, with the same scope: the comparison ran on `3e7bf6d`.

| Build | Stage-1 kernel | Converged |
|---|---|---|
| Windows 10.0.26200 / MSVC 19.44 | AVX2 | 26 / 26 |
| Linux 7.0.0 / Clang 24.0 | AVX2 | 26 / 26 |
| Linux 7.0.0 / GCC 16.2 | AVX2 | 26 / 26 |
| macOS 25.6 (M1, virtual) / GCC 16.2 | NEON | 26 / 26 |
| macOS 25.6 (M1, virtual) / Clang 23.1 | NEON | 26 / 26 |

The commits, hosts and convergence limits are the same as in §6. Unlike the single-document tests, the streaming tests have no reused variant: each document is read into a freshly constructed record.

### 8.4 Results

As in §7, the percentages are Jsonifier's throughput relative to simdjson On Demand.

| Build | simdjson's benchmarks W-T-L | Corpus, in order W-T-L | Corpus, reverse W-T-L | Total W-T-L |
|---|---|---|---|---|
| Windows / MSVC | 6-0-0 | 10-0-0 | 10-0-0 | 26-0-0 |
| Linux / Clang | 5-0-1 | 10-0-0 | 9-0-1 | 24-0-2 |
| Linux / GCC | 4-2-0 | 8-0-2 | 8-0-2 | 20-2-4 |
| macOS / GCC | 6-0-0 | 10-0-0 | 8-1-1 | 24-1-1 |
| macOS / Clang | 2-0-4 | 7-2-1 | 9-0-1 | 18-2-6 |
| **All** | **23-2-5** | **45-2-3** | **44-1-5** | **112-5-13** |

**Amazon Cellphones and Stream Formats:**

| Test | MSVC | Linux Clang | Linux GCC | macOS GCC | macOS Clang |
|---|---|---|---|---|---|
| Amazon Cellphones (NDJSON) | +44.4% | +20.5% | +2.9% | +42.7% | +64.0% |
| Large Amazon Cellphones (NDJSON) | +37.5% | +11.7% | tie | +12.1% | −9.8% |
| Stream Formats Small (NDJSON) | +31.6% | −4.4% | +5.6% | +9.7% | −16.2% |
| Stream Formats Small (comma) | +23.5% | +71.6% | +97.7% | +92.6% | +45.8% |
| Stream Formats Large (NDJSON) | +14.3% | +1.4% | tie | +4.8% | −9.2% |
| Stream Formats Large (comma) | +19.1% | +9.1% | +10.4% | +14.1% | −6.6% |

**Corpus record streams, in order:**

| Test | MSVC | Linux Clang | Linux GCC | macOS GCC | macOS Clang |
|---|---|---|---|---|---|
| CitmCatalog (NDJSON) | +32.7% | +3.8% | +4.6% | +4.6% | tie |
| CitmCatalog (comma) | +35.1% | +15.2% | +23.5% | +11.7% | tie |
| Google Maps (NDJSON) | +42.2% | +23.6% | +14.7% | +20.7% | +5.5% |
| Google Maps (comma) | +31.8% | +25.6% | +36.8% | +42.8% | +7.7% |
| Instruments (NDJSON) | +65.4% | +14.2% | −10.0% | +7.4% | −7.8% |
| Instruments (comma) | +54.4% | +28.3% | +28.1% | +13.0% | +14.4% |
| Random (NDJSON) | +58.8% | +19.9% | +10.0% | +61.0% | +44.4% |
| Random (comma) | +49.6% | +23.6% | +31.9% | +55.5% | +42.6% |
| Twitter (NDJSON) | +38.9% | +8.6% | −3.7% | +59.5% | +7.7% |
| Twitter (comma) | +37.2% | +12.3% | +11.9% | +28.5% | +16.6% |

**Corpus record streams, keys in reverse order:**

| Test | MSVC | Linux Clang | Linux GCC | macOS GCC | macOS Clang |
|---|---|---|---|---|---|
| CitmCatalog (NDJSON) | +221.7% | +41.2% | +41.6% | +30.5% | +49.0% |
| CitmCatalog (comma) | +232.2% | +51.5% | +67.1% | +77.9% | +53.1% |
| Google Maps (NDJSON) | +67.3% | +9.2% | +13.1% | +35.9% | −4.3% |
| Google Maps (comma) | +50.8% | +11.9% | +30.6% | +27.6% | +26.6% |
| Instruments (NDJSON) | +70.7% | −11.0% | −25.5% | −40.1% | +10.7% |
| Instruments (comma) | +76.2% | +6.1% | −1.8% | tie | +20.1% |
| Random (NDJSON) | +272.0% | +54.5% | +32.9% | +58.9% | +55.5% |
| Random (comma) | +281.9% | +70.8% | +48.5% | +57.7% | +80.0% |
| Twitter (NDJSON) | +766.4% | +224.8% | +134.5% | +173.7% | +179.7% |
| Twitter (comma) | +774.2% | +235.9% | +163.2% | +173.5% | +200.3% |

### 8.5 What the results show

- **Amazon Cellphones and Stream Formats.** Jsonifier took 23 of the 30 results in that group, tied 2 and lost 5. It won Amazon Cellphones on all five builds (+2.9% to +64.0%), won Large Amazon Cellphones on three builds (+11.7% to +37.5%), tying on Linux/GCC and losing on macOS/Clang (−9.8%), and won Stream Formats Small (comma) on all five builds (+23.5% to +97.7%). The other four losses are Stream Formats: Small (NDJSON) on Linux/Clang (−4.4%) and macOS/Clang (−16.2%), and Large on macOS/Clang in NDJSON (−9.2%) and comma (−6.6%) form. The two ties are Stream Formats Large (NDJSON) on Linux/GCC (−1.3%) and Large Amazon Cellphones there (−0.8%). Stream Formats Small has the smallest documents in the suite, so it puts the most weight on per-document overhead.
- **Windows/MSVC won every streaming test,** 26-0-0; Linux/Clang lost two of 26, Linux/GCC four, macOS/GCC one and macOS/Clang six.
- **In-order corpus streams are mostly wins.** Of the 50 in-order corpus results, Jsonifier won 45, tied 2 and lost 3, all NDJSON: Instruments on Linux/GCC (−10.0%) and macOS/Clang (−7.8%), and Twitter on Linux/GCC (−3.7%). The ties are CitmCatalog on macOS/Clang, in both forms.
- **Reverse order carries over from single documents to streams.** Jsonifier won 44 of 50 reverse stream results and tied 1. The margins follow object size, as in §7.11: Twitter statuses, which have many fields, give +134.5% to +774.2%, while Google Maps rows, which have few, give +9.2% to +67.3%. Four of the five losses are Instruments: NDJSON on Linux/Clang (−11.0%), Linux/GCC (−25.5%) and macOS/GCC (−40.1%), and comma-separated on Linux/GCC (−1.8%); the fifth is Google Maps NDJSON on macOS/Clang (−4.3%). The tie is Instruments in comma form on macOS/GCC.
- **MSVC again shows the largest gaps.** On the same CPU, simdjson reads Random Reverse (comma) at 120.4 MB/s under MSVC and 421.4 MB/s under Linux/Clang, while Jsonifier reads it at 459.9 and 719.5 MB/s.

## 9. Limitations

- **Linux/GCC and macOS/Clang hold most of the single-document losses:** 76 and 77 of the 213, against 33 on Linux/Clang, 23 on macOS/GCC and 4 on MSVC. macOS/Clang's are spread across access patterns, led by 17 small sparse, 13 full-size sparse, 11 at each size read in order and 9 small sparse reverse. The largest individual losses are on the small documents read in order: Instruments Small (min) at −58.2% on macOS/GCC (fresh) and −54.1% on macOS/Clang (reused). Both macOS builds ran on a virtualized M1, so part of these results likely reflects host noise.
- **Linux/GCC loses mostly on reads in document order.** 46 of its 76 losses are in-order reads: 24 of its 36 small in-order results and 22 of its 36 full-size ones. At full size they are on Canada, Discord, Google Maps, Instruments, Marine IK, Mesh and Twitter, mostly −0.7% to −9.6%, with minified Google Maps at −22.2% and minified Instruments at −29.5% (both reused); at small size they reach −15.5% (Canada Small, pretty, reused). It also loses reverse reads of Canada at both sizes (−3.1% to −5.8%) and of Google Maps at full size (−4.3% to −29.2%), while it won every small sparse-reverse result.
- **Small documents read in order lose on Linux.** Linux/Clang lost 11 of its 36 small in-order results and Linux/GCC 24 of 36, while Linux/Clang lost one of its 36 full-size in-order tests. Linux/Clang also lost 15 of its 36 small sparse in-order results and 2 of its 36 small sparse-reverse ones. At 5 KiB per document, Jsonifier's fixed per-parse cost is the next target.
- **Number-heavy documents in order** remain a common loss. Canada, Marine IK and Mesh lose in order on both GCC builds. The cause is float-array throughput rather than out-of-order access, so it is out of scope for the field index.
- **Instruments streams** lose on three builds: read in order in NDJSON form on Linux/GCC (−10.0%) and macOS/Clang (−7.8%), and in reverse in NDJSON form on Linux/Clang (−11.0%), Linux/GCC (−25.5%) and macOS/GCC (−40.1%) and in comma form on Linux/GCC (−1.8%), although the single-document Instruments Reverse tests win on every build.
- **Convergence was complete in this sweep.** Every build converged on all 298 single-document results and all 26 streaming tests, so no result was dropped from the tallies; the NEON builds did so under the looser limits (RSE < 10%, shift < 5%).
- **The thresholds were tuned on this suite.** *T* was selected by sweeping {2, 4, 8, 16} on the full-size benchmarks, before the wrap gate and the small documents were added, and MSVC was not swept. The wrap count of 2 was not swept.
- **Comma-separated streams still walk each document up front.** The local boundary rule in §8.2 only holds for whitespace-delimited streams, so comma mode finds each document's end with a depth walk before handing it out, and the caller then walks the same structurals again. On the x86 in-order corpus streams this leaves comma mode 5.1% to 20.2% slower than NDJSON mode on the same records. The rule also assumes the input is valid: inside a malformed container, two adjacent values with no separator would be split into separate documents, and the reader would then report the errors. Reverse reads still pay a walk from the first key to the end of the object when the stream advances, the same walk an up-front boundary pass would pay when handing the document out.
- **Streams were compared unthreaded and fresh only.** simdjson can run stage 1 for the next window on a second thread; Jsonifier has no threaded mode, so §8 compares both single-threaded. The streaming tests also have no reused-output variant.
- **A known inefficiency remains in the miss path.** A lookup for a key that doesn't exist still ends with a full escape-aware rescan of the object, even after the object is indexed. It does not affect correctness and is a candidate for follow-up work.
- **The output comparison predates the measured commits.** The byte-for-byte comparison in §6 ran on `3e7bf6d`, not on the commits measured here, and it does not cover the small-document or reused tests.

## 10. Conclusion

A schema-free, tape-driven parser can match simdjson On Demand on its home ground and beat it outright once the access pattern stops being strictly forward. `jsonifier::generic` keeps On Demand's shape and relaxes its single-use restriction. Four pieces carry that result:

- a stage-1 reader tuned for small documents;
- a shared forward cursor, so reads in document order stay cheap;
- zero-copy strings with a fixed-slot arena;
- a SIMD field index that is only built once an object has shown it needs one.

Across 1,490 single-document results on five builds, covering full-size and small documents, each read into both freshly allocated and reused output objects, Jsonifier won 1,208, tied 69 and lost 213. On reverse reads it won 339 of 360 at both sizes, and on Windows/MSVC it lost 4 single-document results of 298, by 1.0% to 6.6%. Reusing the output objects speeds up both libraries by similar amounts and leaves the verdicts largely unchanged (597-35-113 fresh, 611-34-100 reused). The main remaining targets are Linux/GCC, with 76 of the 213 losses, macOS/Clang, with 77 on a virtualized host, and small documents read in order on Linux.

The design also carries over to streams. Because each streamed document is an ordinary `generic::document` bound to the parser's shared cursor, `iterateMany` needed no new reading machinery, only windowing and document boundaries. The one lesson it added repeats §5's: finding every document's end up front makes an in-order reader pay for two passes over the tape, and letting the reader's own cursor report where each document ended removes that pass for whitespace-delimited streams. Across 130 streaming results, Jsonifier won 112, tied 5 and lost 13, and it won every streaming test on Windows/MSVC. Instruments streams and the Stream Formats tests on macOS/Clang are the main remaining streaming targets.

The main engineering lesson is in §5. An index over the whole document made the target workload 2.3x *slower*, because most objects are small. The version that works builds the index lazily, per object, and only once that object has shown both that it is large and that it is being read out of order: enough skipped keys to clear *T*, and two lookups that had to wrap around. In short: make the in-order read cost nothing, and only pay for an index on objects whose access pattern shows it is needed.

## 11. Dedication

This paper is dedicated to the author's Little Buddy.