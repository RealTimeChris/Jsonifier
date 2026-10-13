# Two Stages, On Demand: The Stage-1 + Stage-2 Architecture in Jsonifier

**Nihilai Collective Corp: Engineering Papers**  
*Nihilai Collective Corp*  
*Jsonifier, October 2026*  

---

## Abstract

Since Langdale and Lemire's 2019 paper *Parsing Gigabytes of JSON per Second*, the two-stage SIMD parsing model, a vectorized structural-indexing pass (stage 1) followed by a tape-driven materialization pass (stage 2), has been treated as the canonical architecture for high-performance JSON processing. simdjson, the reference implementation, routes every document through stage 1 unconditionally.

Jsonifier takes a different position: **two-stage parsing is a specialized tool, not a mandatory front door.** When parsing a full document into concrete, reflection-registered C++ types, every structural fact the tape would record is rediscovered anyway during value materialization, so building the tape means touching every byte twice for information used once. Jsonifier therefore parses full documents in a single fused pass, and reserves its stage-1 + stage-2 machinery for the workloads where a structural index genuinely pays for itself: partial/unordered reading, prettifying, and minifying. Measured with both paths compiled into one binary across 225 tests on five builds, skipping the tape wins on POD-type data on every build and on 95 of the 100 minified documents, the tape is faster on 56 of the 60 prettified documents on x86, and on the Apple M1 the fused path wins 30 of the 40 prettified documents.

This paper describes both halves of that architecture: the single-pass primary path, and Jsonifier's stage-1 implementation, including its per-compiler-tuned step geometry, its fold-expression drain architecture for bitmask-to-index extraction, and its distributed UTF-8 validation strategy. Throughout, it compares both directly with simdjson's current design.

---

## 1. Background: the canonical two-stage model

simdjson's pipeline is well documented. Stage 1 sweeps the input in 64-byte blocks, computing per-block bitmasks: backslashes, odd-length escape terminations, quotes, an in-string range mask derived via carry-less multiplication (prefix XOR), structural characters (`{ } [ ] : ,`), whitespace, and pseudo-structural scalar starts. The surviving bits are converted to byte offsets, the *structural tape*, via a tzcnt/blsr extraction loop. Stage 2 then walks the tape to build a DOM or, in the On Demand API, lazily materializes only the values the caller touches.

The model's strength is that stage 1 is branch-free and data-parallel; its cost is that it is unconditional. Every byte of every document is classified and indexed before a single value is parsed, regardless of whether the caller's access pattern needs the index at all.

## 2. The Jsonifier position: pay for the tape only when the tape pays for you

Jsonifier's primary API parses JSON directly into user-defined structs registered through compile-time reflection (`jsonifier::core<T>`). In that setting the parser already knows, at compile time, the expected shape of the document: which keys exist, their serialized order, their types. The parse loop is a schema-directed walk, not a blind tree construction.

Consider what the structural tape provides: the positions of every brace, bracket, colon, comma, quote, and scalar start. Now consider what a schema-directed single-pass parser does at each of those same positions: it is *already there*, with its iterator parked on that exact byte, consuming it as part of matching a known key literal or delimiting a value. The tape's information is a strict subset of what the fused pass discovers for free in program order. Building it first means that, for a full-document parse in which every value is materialized:

- every byte of the input is loaded twice (once in stage 1, once in stage 2),
- the tape itself is written and re-read (cache traffic proportional to structural density), and
- stage 2's control flow is driven by indirect loads through the index array rather than by a pointer walking linearly through memory the prefetcher already understands.

(We scope the double-load claim deliberately: a two-stage consumer that *skips* content, such as simdjson's On Demand API skipping unrequested fields, does not reload the skipped bytes in its second pass. That is exactly the workload where the tape earns its keep, and exactly why Jsonifier retains the two-stage machinery for partial reading. The accounting above describes the full-materialization case, where nothing is skipped and the tape's information is fully redundant with the walk.)

For full-document parsing into known types, the tape is overhead in principle, and in practice whether skipping it is faster comes down to two questions: is the input indented, and is the machine x86? §2.1 through §2.4 measure both paths in one binary on five platform/compiler builds. In the freshly allocated run, the fused path wins all 25 POD-type tests and 95 of the 100 minified documents (full-size and Small) across all five builds; the tape takes Canada and Mesh on Linux/GCC, Twitter and Instruments Small on macOS/GCC, and Canada on macOS/Clang. The two-stage path wins 56 of the 60 prettified documents on x86, and on the M1 the fused path wins 30 of the 40 prettified documents, the tape wins 9 and one is a tie.

**The routing rule is simple: the two-stage machinery is engaged for partial reading, prettifying, and minifying: workloads where the caller does *not* want every value, or wants pure structural transformation. Full-document parsing takes the single-pass path.** §2.4 shows where the measurements say that rule should be refined.

One anticipated objection deserves preemption here: that Jsonifier's requirement of ahead-of-time registration (`jsonifier::core<T>`) concedes generality that simdjson retains, since simdjson parses arbitrary documents with no such declaration. For truly dynamic workloads (schemas unknown until runtime, exploratory traversal, structural transformation of unknown documents), this is correct, and simdjson's DOM and On Demand models are the appropriate tools; Jsonifier's registration model simply does not address that problem. But for the workload this paper concerns, parsing documents into concrete types the caller has defined, the objection dissolves on inspection, because the schema knowledge exists at compile time in both programs. A simdjson caller materializing a struct writes the schema into their source as a sequence of field accesses in a fixed order chosen at authoring time; that traversal code is a schema declaration in imperative clothing. The difference is not the presence of compile-time knowledge but its legibility to the library: expressed as hand-written traversal, the knowledge is opaque: simdjson cannot fuse key literals from it, cannot learn permuted orders through it, and cannot skip building the index it implies is unnecessary. Expressed as a reflection registration, the identical knowledge becomes architecture: fused member headers, adaptive order recovery, and the routing rule above. The comparison between the two libraries on known-type workloads is therefore not "declared schema versus no schema"; it is the same schema, declared once where the compiler can consume it versus restated per call site where it cannot.

### 2.1 Method: both paths in one binary

Every comparison in this section comes from the [Json-Performance](https://github.com/nihilai-collective/Json-Performance) sweep published October 21, 2026, with Jsonifier [769b6b0](https://github.com/nihilai-collective/jsonifier/commit/769b6b0), Glaze [628bec1](https://github.com/stephenberry/glaze/commit/628bec1) and BenchmarkSuite [e19dd21](https://github.com/nihilai-collective/benchmarksuite/commit/e19dd21) on every build. simdjson is [1f24df3](https://github.com/simdjson/simdjson/commit/1f24df3) on Linux/Clang, Linux/GCC and macOS/Clang, [45fecc2](https://github.com/simdjson/simdjson/commit/45fecc2) on Windows/MSVC, and [683e686](https://github.com/simdjson/simdjson/commit/683e686) on macOS/GCC. The harness registers both Jsonifier paths in the same binary. "jsonifier" is the default fused single-pass path, which the harness calls scalar structural iteration. "jsonifier (two-stage)" makes exactly the same `parseJson` call with `partialRead` set, which routes it through stage 1 and then stage 2. "simdjson (ondemand)" is the reference for §2.2 to §2.4. Glaze runs in the same sweep and appears here only in the validation test of §2.7, and `simdjson (reflection)`, simdjson 5's C++26 static-reflection reader, gets its own comparison in §2.5. Because the two Jsonifier paths are compiled together, run back to back on each test, and ranked against each other by the same statistics, the only thing that differs between them is the path.

All libraries parse fully into the target data structures and perform UTF-8 validation. Every test runs twice. In the freshly allocated run, every iteration constructs a new object to parse into and destroys it again inside the timed region, so allocation and deallocation are part of each measurement. In the reused run, labelled "(Reused)" in the sweep, the object is created once and held across iterations; it is cleared, keeping its capacity, outside the timed region before each iteration, so only the parse itself is measured. Parser instances are reused in both. §2.2, §2.3, §2.5 and §2.6 give both runs; the discussion in §2.4 quotes the freshly allocated run unless it says otherwise. Each build runs 45 full-parse tests: five POD-type tests (arrays of a single value type: Bool, Double, Int64, String, Uint64), ten corpus documents in minified and prettified form, and "Small" cut-down copies of the same ten documents (at most 5 KiB minified) in both forms, which weigh per-call overhead more heavily than the full-size documents. Two of the ten documents, "Marine IK Reverse" and its Small copy, request every key in the reverse of its document order; in every other test, all libraries receive keys in document order. The sweep also runs a partial-reading test on the Twitter document and its Small copy, which §2.6 covers, and a validation-only test, which §2.7 covers.

Sampling is adaptive. Iterations start at 100 and double each epoch, and sampling does not stop early: epochs continue until 5 seconds have elapsed or the iteration cap of 100,000 is reached. Every epoch after the first is scored by its relative standard error plus its epoch-over-epoch mean shift, and the lowest-scoring epoch is kept as the result. A result counts as converged only if that epoch has RSE below 5% and mean shift below 2.5% on the three x86 builds (10% and 5% on the virtualized M1), and non-converged results stay in the rankings and tallies, marked n/c in the tables. Fifty full-parse test rows in this sweep have at least one of the three compared libraries not converged: twenty-six freshly allocated and twenty-four reused. One in each run is on Windows/MSVC (simdjson on prettified Marine IK Reverse) and the rest are on the two macOS builds. Each one is marked n/c where it appears. Ties are declared by Welch's t-test on the kept epoch (two-sided, p < 0.05), and every verdict in this paper is a pairwise test between the two libraries or paths named in its column. Two properties of this rule matter for reading the results. Keeping the quietest epoch favors the least-disturbed stretch of each run, which raises absolute throughput somewhat, but it is applied identically to every library. And because the kept epoch has the smallest variance available, small differences are more often resolved as wins or losses than they would be under a first-to-converge rule: this sweep produced two ties between the two Jsonifier paths across 225 freshly allocated tests and four across 225 reused ones.

### 2.2 Fused against two-stage

Ranked head to head, the fused path wins 153 tests, 2 are statistical ties, and the two-stage path wins 70. The split is a matter of platform and indentation:

| Platform / Compiler | Fused faster | Tie | Two-stage faster | Tests converged |
|---|---|---|---|---|
| Windows / MSVC 19.51 (i9-14900KF, AVX2) | 26 | 1 | 18 | 44 of 45 |
| Linux / Clang 24.0 (i9-14900KF, AVX2) | 27 | 0 | 18 | 45 of 45 |
| Linux / GCC 16.2 (i9-14900KF, AVX2) | 23 | 0 | 22 | 45 of 45 |
| macOS / GCC 16.2 (Apple M1, NEON) | 37 | 1 | 7 | 26 of 45 |
| macOS / Clang 23.1 (Apple M1, NEON) | 40 | 0 | 5 | 39 of 45 |
| **Aggregate** | **153** | **2** | **70** | **199 of 225** |

Split by input shape (fused / tie / two-stage):

| Input shape | Windows / MSVC | Linux (Clang + GCC) | macOS (GCC + Clang) |
|---|---|---|---|
| POD-type tests | 5 / 0 / 0 | 10 / 0 / 0 | 10 / 0 / 0 |
| Minified corpus documents | 10 / 0 / 0 | 18 / 0 / 2 | 18 / 0 / 2 |
| Prettified corpus documents | 1 / 0 / 9 | 2 / 0 / 18 | 17 / 1 / 2 |
| Small minified documents | 10 / 0 / 0 | 20 / 0 / 0 | 19 / 0 / 1 |
| Small prettified documents | 0 / 1 / 9 | 0 / 0 / 20 | 13 / 0 / 7 |

Against simdjson, counting a test for Jsonifier by the verdict of whichever of its two paths has the higher throughput against simdjson On Demand:

| Platform / Compiler | Best Jsonifier path (W / T / L) | Fused path alone (W / T / L) |
|---|---|---|
| Windows / MSVC 19.51 (i9-14900KF, AVX2) | 45 / 0 / 0 | 44 / 0 / 1 |
| Linux / Clang 24.0 (i9-14900KF, AVX2) | 45 / 0 / 0 | 37 / 1 / 7 |
| Linux / GCC 16.2 (i9-14900KF, AVX2) | 43 / 0 / 2 | 43 / 0 / 2 |
| macOS / GCC 16.2 (Apple M1, NEON) | 45 / 0 / 0 | 44 / 1 / 0 |
| macOS / Clang 23.1 (Apple M1, NEON) | 44 / 0 / 1 | 43 / 0 / 2 |
| **Aggregate** | **222 / 0 / 3** | **211 / 2 / 12** |

**Reused objects.** With the target object held across iterations:

| Platform / Compiler | Fused faster | Tie | Two-stage faster | Tests converged |
|---|---|---|---|---|
| Windows / MSVC 19.51 (i9-14900KF, AVX2) | 26 | 0 | 19 | 44 of 45 |
| Linux / Clang 24.0 (i9-14900KF, AVX2) | 26 | 0 | 19 | 45 of 45 |
| Linux / GCC 16.2 (i9-14900KF, AVX2) | 23 | 0 | 22 | 45 of 45 |
| macOS / GCC 16.2 (Apple M1, NEON) | 40 | 1 | 4 | 29 of 45 |
| macOS / Clang 23.1 (Apple M1, NEON) | 39 | 3 | 3 | 38 of 45 |
| **Aggregate** | **154** | **4** | **67** | **201 of 225** |

Split by input shape (fused / tie / two-stage):

| Input shape | Windows / MSVC | Linux (Clang + GCC) | macOS (GCC + Clang) |
|---|---|---|---|
| POD-type tests | 5 / 0 / 0 | 10 / 0 / 0 | 10 / 0 / 0 |
| Minified corpus documents | 10 / 0 / 0 | 17 / 0 / 3 | 19 / 1 / 0 |
| Prettified corpus documents | 1 / 0 / 9 | 2 / 0 / 18 | 16 / 1 / 3 |
| Small minified documents | 10 / 0 / 0 | 20 / 0 / 0 | 19 / 1 / 0 |
| Small prettified documents | 0 / 0 / 10 | 0 / 0 / 20 | 15 / 1 / 4 |

Against simdjson, counting a test for Jsonifier by the verdict of whichever of its two paths has the higher throughput against simdjson On Demand:

| Platform / Compiler | Best Jsonifier path (W / T / L) | Fused path alone (W / T / L) |
|---|---|---|
| Windows / MSVC 19.51 (i9-14900KF, AVX2) | 45 / 0 / 0 | 44 / 0 / 1 |
| Linux / Clang 24.0 (i9-14900KF, AVX2) | 43 / 0 / 2 | 36 / 1 / 8 |
| Linux / GCC 16.2 (i9-14900KF, AVX2) | 42 / 0 / 3 | 42 / 0 / 3 |
| macOS / GCC 16.2 (Apple M1, NEON) | 43 / 1 / 1 | 42 / 2 / 1 |
| macOS / Clang 23.1 (Apple M1, NEON) | 43 / 2 / 0 | 42 / 2 / 1 |
| **Aggregate** | **216 / 3 / 6** | **206 / 5 / 14** |

Across 225 tests the fused path wins 154, ties 4 and loses 67 to the two-stage path, and the x86 split holds: the tape wins 27 of the 30 full-size prettified documents and all 30 Small prettified ones, and the fused path wins all 30 Small minified documents and 27 of the 30 full-size minified ones (the tape takes Canada and Mesh on Linux/GCC and Twitter on Linux/Clang). On the M1 the fused path wins 19 of the 20 Small minified documents (Canada Small on macOS/Clang is a tie) and 19 of the 20 full-size minified ones (Mesh on macOS/Clang is a tie), and 31 of the 40 prettified documents, with 2 ties and 7 tape wins. Against simdjson, the faster Jsonifier path wins 216, ties 3 and loses 6, and the fused path alone wins 206, ties 5 and loses 14.

### 2.3 Per-test results

Throughput is in MB/s. "Faster Jsonifier path" is the Welch's t-test verdict between the two Jsonifier paths, "Best Jsonifier path vs simdjson" is the verdict for whichever of the two has the higher throughput.

#### Freshly allocated objects

**Windows / MSVC 19.51 (i9-14900KF, AVX2)**

| Test | Fused (MB/s) | Two-stage (MB/s) | simdjson (MB/s) | Fused ÷ two-stage | Faster Jsonifier path | Best Jsonifier path vs simdjson |
|---|---|---|---|---|---|---|
| Bool (POD) | 521 | 215 | 113 | 2.42× | Fused | Win |
| Double (POD) | 689 | 341 | 170 | 2.02× | Fused | Win |
| Int64 (POD) | 1,733 | 604 | 376 | 2.87× | Fused | Win |
| String (POD) | 1,040 | 721 | 602 | 1.44× | Fused | Win |
| Uint64 (POD) | 1,536 | 602 | 367 | 2.55× | Fused | Win |
| Canada (minified) | 650 | 622 | 407 | 1.04× | Fused | Win |
| Canada (prettified) | 1,748 | 1,879 | 1,162 | 0.93× | Two-stage | Win |
| CitmCatalog (minified) | 1,788 | 1,291 | 433 | 1.39× | Fused | Win |
| CitmCatalog (prettified) | 2,995 | 3,180 | 1,124 | 0.94× | Two-stage | Win |
| Discord (minified) | 1,134 | 1,026 | 529 | 1.11× | Fused | Win |
| Discord (prettified) | 1,426 | 1,640 | 849 | 0.87× | Two-stage | Win |
| Google Maps Response (minified) | 1,629 | 1,180 | 441 | 1.38× | Fused | Win |
| Google Maps Response (prettified) | 2,811 | 2,774 | 1,080 | 1.01× | Fused | Win |
| Instruments (minified) | 2,027 | 1,464 | 588 | 1.38× | Fused | Win |
| Instruments (prettified) | 2,226 | 2,626 | 1,085 | 0.85× | Two-stage | Win |
| Marine IK Reverse (minified) | 589 | 530 | 107 | 1.11× | Fused | Win |
| Marine IK Reverse (prettified) | 2,325 | 2,507 | 522 (n/c) | 0.93× | Two-stage | Win |
| Marine IK (minified) | 591 | 536 | 329 | 1.10× | Fused | Win |
| Marine IK (prettified) | 2,355 | 2,560 | 1,437 | 0.92× | Two-stage | Win |
| Mesh (minified) | 1,124 | 963 | 829 | 1.17× | Fused | Win |
| Mesh (prettified) | 1,385 | 1,759 | 1,435 | 0.79× | Two-stage | Win |
| Random (minified) | 989 | 903 | 459 | 1.10× | Fused | Win |
| Random (prettified) | 1,410 | 1,599 | 832 | 0.88× | Two-stage | Win |
| Twitter (minified) | 1,307 | 1,125 | 705 | 1.16× | Fused | Win |
| Twitter (prettified) | 1,405 | 1,655 | 1,023 | 0.85× | Two-stage | Win |
| Canada Small (minified) | 638 | 599 | 348 | 1.07× | Fused | Win |
| Canada Small (prettified) | 1,648 | 1,784 | 1,018 | 0.92× | Two-stage | Win |
| CitmCatalog Small (minified) | 984 | 816 | 434 | 1.20× | Fused | Win |
| CitmCatalog Small (prettified) | 1,610 | 1,745 | 932 | 0.92× | Two-stage | Win |
| Discord Small (minified) | 1,083 | 871 | 547 | 1.24× | Fused | Win |
| Discord Small (prettified) | 1,221 | 1,272 | 801 | 0.96× | Two-stage | Win |
| Google Maps Response Small (minified) | 1,643 | 1,158 | 440 | 1.42× | Fused | Win |
| Google Maps Response Small (prettified) | 2,659 | 2,668 | 1,080 | 1.00× | Tie | Win |
| Instruments Small (minified) | 2,017 | 1,460 | 602 | 1.38× | Fused | Win |
| Instruments Small (prettified) | 2,018 | 2,645 | 1,118 | 0.76× | Two-stage | Win |
| Marine IK Reverse Small (minified) | 477 | 415 | 77 | 1.15× | Fused | Win |
| Marine IK Reverse Small (prettified) | 1,878 | 2,025 | 394 | 0.93× | Two-stage | Win |
| Marine IK Small (minified) | 498 | 437 | 246 | 1.14× | Fused | Win |
| Marine IK Small (prettified) | 1,969 | 2,142 | 1,138 | 0.92× | Two-stage | Win |
| Mesh Small (minified) | 806 | 676 | 346 | 1.19× | Fused | Win |
| Mesh Small (prettified) | 1,198 | 1,369 | 715 | 0.88× | Two-stage | Win |
| Random Small (minified) | 1,151 | 1,008 | 449 | 1.14× | Fused | Win |
| Random Small (prettified) | 1,569 | 1,801 | 827 | 0.87× | Two-stage | Win |
| Twitter Small (minified) | 1,590 | 1,352 | 885 | 1.18× | Fused | Win |
| Twitter Small (prettified) | 1,573 | 1,899 | 1,242 | 0.83× | Two-stage | Win |

**Linux / Clang 24.0 (i9-14900KF, AVX2)**

| Test | Fused (MB/s) | Two-stage (MB/s) | simdjson (MB/s) | Fused ÷ two-stage | Faster Jsonifier path | Best Jsonifier path vs simdjson |
|---|---|---|---|---|---|---|
| Bool (POD) | 2,012 | 339 | 232 | 5.94× | Fused | Win |
| Double (POD) | 1,299 | 417 | 266 | 3.11× | Fused | Win |
| Int64 (POD) | 3,193 | 927 | 588 | 3.45× | Fused | Win |
| String (POD) | 1,760 | 1,174 | 1,048 | 1.50× | Fused | Win |
| Uint64 (POD) | 3,353 | 952 | 549 | 3.52× | Fused | Win |
| Canada (minified) | 977 | 951 | 653 | 1.03× | Fused | Win |
| Canada (prettified) | 2,625 | 2,667 | 1,959 | 0.98× | Two-stage | Win |
| CitmCatalog (minified) | 2,462 | 1,824 | 1,235 | 1.35× | Fused | Win |
| CitmCatalog (prettified) | 3,802 | 4,341 | 3,120 | 0.88× | Two-stage | Win |
| Discord (minified) | 2,287 | 1,816 | 1,488 | 1.26× | Fused | Win |
| Discord (prettified) | 2,084 | 2,846 | 2,322 | 0.73× | Two-stage | Win |
| Google Maps Response (minified) | 2,728 | 1,990 | 1,382 | 1.37× | Fused | Win |
| Google Maps Response (prettified) | 3,702 | 4,385 | 3,239 | 0.84× | Two-stage | Win |
| Instruments (minified) | 2,706 | 2,018 | 1,743 | 1.34× | Fused | Win |
| Instruments (prettified) | 2,778 | 3,508 | 3,172 | 0.79× | Two-stage | Win |
| Marine IK Reverse (minified) | 857 | 766 | 607 | 1.12× | Fused | Win |
| Marine IK Reverse (prettified) | 3,453 | 3,144 | 2,770 | 1.10× | Fused | Win |
| Marine IK (minified) | 861 | 774 | 606 | 1.11× | Fused | Win |
| Marine IK (prettified) | 3,501 | 3,286 | 2,792 | 1.07× | Fused | Win |
| Mesh (minified) | 1,371 | 1,230 | 1,208 | 1.11× | Fused | Win |
| Mesh (prettified) | 2,049 | 2,232 | 2,215 | 0.92× | Two-stage | Win |
| Random (minified) | 1,713 | 1,432 | 1,101 | 1.20× | Fused | Win |
| Random (prettified) | 2,089 | 2,426 | 1,974 | 0.86× | Two-stage | Win |
| Twitter (minified) | 2,048 | 1,901 | 1,723 | 1.08× | Fused | Win |
| Twitter (prettified) | 1,950 | 2,756 | 2,484 | 0.71× | Two-stage | Win |
| Canada Small (minified) | 1,020 | 978 | 711 | 1.04× | Fused | Win |
| Canada Small (prettified) | 2,535 | 2,651 | 2,051 | 0.96× | Two-stage | Win |
| CitmCatalog Small (minified) | 1,964 | 1,564 | 1,241 | 1.26× | Fused | Win |
| CitmCatalog Small (prettified) | 2,630 | 3,166 | 2,621 | 0.83× | Two-stage | Win |
| Discord Small (minified) | 2,096 | 1,780 | 1,472 | 1.18× | Fused | Win |
| Discord Small (prettified) | 1,965 | 2,595 | 2,143 | 0.76× | Two-stage | Win |
| Google Maps Response Small (minified) | 2,396 | 1,837 | 1,354 | 1.30× | Fused | Win |
| Google Maps Response Small (prettified) | 3,324 | 4,031 | 3,186 | 0.82× | Two-stage | Win |
| Instruments Small (minified) | 2,761 | 1,986 | 1,628 | 1.39× | Fused | Win |
| Instruments Small (prettified) | 2,782 | 3,484 | 3,003 | 0.80× | Two-stage | Win |
| Marine IK Reverse Small (minified) | 842 | 754 | 590 | 1.12× | Fused | Win |
| Marine IK Reverse Small (prettified) | 2,965 | 3,389 | 2,760 | 0.88× | Two-stage | Win |
| Marine IK Small (minified) | 934 | 820 | 596 | 1.14× | Fused | Win |
| Marine IK Small (prettified) | 3,141 | 3,607 | 2,797 | 0.87× | Two-stage | Win |
| Mesh Small (minified) | 973 | 927 | 776 | 1.05× | Fused | Win |
| Mesh Small (prettified) | 1,533 | 1,728 | 1,531 | 0.89× | Two-stage | Win |
| Random Small (minified) | 2,339 | 1,791 | 1,190 | 1.31× | Fused | Win |
| Random Small (prettified) | 2,459 | 3,221 | 2,134 | 0.76× | Two-stage | Win |
| Twitter Small (minified) | 2,745 | 2,353 | 2,221 | 1.17× | Fused | Win |
| Twitter Small (prettified) | 2,333 | 3,256 | 3,074 | 0.72× | Two-stage | Win |

**Linux / GCC 16.2 (i9-14900KF, AVX2)**

| Test | Fused (MB/s) | Two-stage (MB/s) | simdjson (MB/s) | Fused ÷ two-stage | Faster Jsonifier path | Best Jsonifier path vs simdjson |
|---|---|---|---|---|---|---|
| Bool (POD) | 2,168 | 280 | 192 | 7.74× | Fused | Win |
| Double (POD) | 1,192 | 380 | 233 | 3.14× | Fused | Win |
| Int64 (POD) | 3,317 | 823 | 503 | 4.03× | Fused | Win |
| String (POD) | 1,751 | 1,014 | 1,033 | 1.73× | Fused | Win |
| Uint64 (POD) | 3,379 | 857 | 494 | 3.94× | Fused | Win |
| Canada (minified) | 903 | 912 | 770 | 0.99× | Two-stage | Win |
| Canada (prettified) | 2,402 | 2,611 | 2,248 | 0.92× | Two-stage | Win |
| CitmCatalog (minified) | 2,420 | 1,865 | 1,362 | 1.30× | Fused | Win |
| CitmCatalog (prettified) | 3,896 | 4,293 | 3,366 | 0.91× | Two-stage | Win |
| Discord (minified) | 2,320 | 1,820 | 1,418 | 1.27× | Fused | Win |
| Discord (prettified) | 2,471 | 2,741 | 2,231 | 0.90× | Two-stage | Win |
| Google Maps Response (minified) | 2,620 | 1,924 | 1,071 | 1.36× | Fused | Win |
| Google Maps Response (prettified) | 4,164 | 4,221 | 2,590 | 0.99× | Two-stage | Win |
| Instruments (minified) | 3,331 | 2,311 | 1,342 | 1.44× | Fused | Win |
| Instruments (prettified) | 3,414 | 3,793 | 2,538 | 0.90× | Two-stage | Win |
| Marine IK Reverse (minified) | 758 | 719 | 602 | 1.05× | Fused | Win |
| Marine IK Reverse (prettified) | 3,242 | 3,285 | 2,751 | 0.99× | Two-stage | Win |
| Marine IK (minified) | 770 | 725 | 601 | 1.06× | Fused | Win |
| Marine IK (prettified) | 3,265 | 3,355 | 2,798 | 0.97× | Two-stage | Win |
| Mesh (minified) | 1,087 | 1,159 | 1,228 | 0.94× | Two-stage | Loss |
| Mesh (prettified) | 1,771 | 2,095 | 2,257 | 0.85× | Two-stage | Loss |
| Random (minified) | 1,466 | 1,384 | 840 | 1.06× | Fused | Win |
| Random (prettified) | 2,038 | 2,368 | 1,525 | 0.86× | Two-stage | Win |
| Twitter (minified) | 2,022 | 1,821 | 1,316 | 1.11× | Fused | Win |
| Twitter (prettified) | 2,009 | 2,552 | 1,908 | 0.79× | Two-stage | Win |
| Canada Small (minified) | 1,043 | 967 | 775 | 1.08× | Fused | Win |
| Canada Small (prettified) | 2,554 | 2,673 | 2,246 | 0.96× | Two-stage | Win |
| CitmCatalog Small (minified) | 1,948 | 1,586 | 1,207 | 1.23× | Fused | Win |
| CitmCatalog Small (prettified) | 2,725 | 3,308 | 2,525 | 0.82× | Two-stage | Win |
| Discord Small (minified) | 1,968 | 1,639 | 1,266 | 1.20× | Fused | Win |
| Discord Small (prettified) | 2,009 | 2,353 | 1,847 | 0.85× | Two-stage | Win |
| Google Maps Response Small (minified) | 2,372 | 1,798 | 1,117 | 1.32× | Fused | Win |
| Google Maps Response Small (prettified) | 3,684 | 3,893 | 2,615 | 0.95× | Two-stage | Win |
| Instruments Small (minified) | 3,303 | 2,237 | 1,239 | 1.48× | Fused | Win |
| Instruments Small (prettified) | 3,272 | 3,746 | 2,309 | 0.87× | Two-stage | Win |
| Marine IK Reverse Small (minified) | 823 | 725 | 574 | 1.13× | Fused | Win |
| Marine IK Reverse Small (prettified) | 2,941 | 3,298 | 2,728 | 0.89× | Two-stage | Win |
| Marine IK Small (minified) | 923 | 760 | 567 | 1.21× | Fused | Win |
| Marine IK Small (prettified) | 3,083 | 3,457 | 2,659 | 0.89× | Two-stage | Win |
| Mesh Small (minified) | 1,007 | 897 | 742 | 1.12× | Fused | Win |
| Mesh Small (prettified) | 1,518 | 1,715 | 1,508 | 0.89× | Two-stage | Win |
| Random Small (minified) | 2,174 | 1,732 | 879 | 1.26× | Fused | Win |
| Random Small (prettified) | 2,686 | 3,050 | 1,648 | 0.88× | Two-stage | Win |
| Twitter Small (minified) | 2,995 | 2,259 | 1,577 | 1.33× | Fused | Win |
| Twitter Small (prettified) | 2,525 | 3,042 | 2,229 | 0.83× | Two-stage | Win |

**macOS / GCC 16.2 (Apple M1, NEON)**

| Test | Fused (MB/s) | Two-stage (MB/s) | simdjson (MB/s) | Fused ÷ two-stage | Faster Jsonifier path | Best Jsonifier path vs simdjson |
|---|---|---|---|---|---|---|
| Bool (POD) | 1,369 | 203 | 179 | 6.74× | Fused | Win |
| Double (POD) | 790 | 254 | 125 | 3.11× | Fused | Win |
| Int64 (POD) | 1,946 | 497 | 411 | 3.91× | Fused | Win |
| String (POD) | 750 | 590 | 644 | 1.27× | Fused | Win |
| Uint64 (POD) | 2,106 | 573 | 414 | 3.67× | Fused | Win |
| Canada (minified) | 543 (n/c) | 517 | 373 (n/c) | 1.05× | Fused | Win |
| Canada (prettified) | 1,633 | 1,574 | 1,044 (n/c) | 1.04× | Fused | Win |
| CitmCatalog (minified) | 1,181 | 1,121 (n/c) | 829 | 1.05× | Fused | Win |
| CitmCatalog (prettified) | 2,454 | 2,096 | 2,151 | 1.17× | Fused | Win |
| Discord (minified) | 1,552 | 934 | 873 | 1.66× | Fused | Win |
| Discord (prettified) | 1,756 | 1,528 | 1,335 | 1.15× | Fused | Win |
| Google Maps Response (minified) | 990 | 768 | 537 | 1.29× | Fused | Win |
| Google Maps Response (prettified) | 2,586 | 1,718 (n/c) | 1,413 | 1.50× | Fused | Win |
| Instruments (minified) | 2,296 | 1,292 | 960 (n/c) | 1.78× | Fused | Win |
| Instruments (prettified) | 2,269 | 1,868 | 1,593 | 1.21× | Fused | Win |
| Marine IK Reverse (minified) | 481 | 335 (n/c) | 315 (n/c) | 1.43× | Fused | Win |
| Marine IK Reverse (prettified) | 2,153 (n/c) | 1,979 | 1,791 | 1.09× | Fused | Win |
| Marine IK (minified) | 554 | 319 (n/c) | 337 (n/c) | 1.74× | Fused | Win |
| Marine IK (prettified) | 1,667 (n/c) | 1,843 | 1,482 (n/c) | 0.90× | Two-stage | Win |
| Mesh (minified) | 833 | 732 | 803 | 1.14× | Fused | Win |
| Mesh (prettified) | 1,273 | 1,372 | 1,152 | 0.93× | Two-stage | Win |
| Random (minified) | 783 (n/c) | 652 | 585 | 1.20× | Fused | Win |
| Random (prettified) | 1,238 | 1,171 | 768 (n/c) | 1.06× | Fused | Win |
| Twitter (minified) | 1,130 (n/c) | 1,392 | 1,040 | 0.81× | Two-stage | Win |
| Twitter (prettified) | 1,957 | 1,948 | 1,460 (n/c) | 1.00× | Tie | Win |
| Canada Small (minified) | 759 | 636 | 342 | 1.19× | Fused | Win |
| Canada Small (prettified) | 1,767 | 1,616 | 1,089 | 1.09× | Fused | Win |
| CitmCatalog Small (minified) | 1,220 | 871 | 772 | 1.40× | Fused | Win |
| CitmCatalog Small (prettified) | 1,978 | 1,582 (n/c) | 1,533 | 1.25× | Fused | Win |
| Discord Small (minified) | 1,287 | 910 | 803 | 1.41× | Fused | Win |
| Discord Small (prettified) | 1,660 | 1,481 | 1,131 | 1.12× | Fused | Win |
| Google Maps Response Small (minified) | 1,492 | 1,012 | 522 | 1.47× | Fused | Win |
| Google Maps Response Small (prettified) | 2,463 | 1,862 | 1,265 | 1.32× | Fused | Win |
| Instruments Small (minified) | 868 | 1,315 | 835 (n/c) | 0.66× | Two-stage | Win |
| Instruments Small (prettified) | 2,885 | 2,365 | 1,505 | 1.22× | Fused | Win |
| Marine IK Reverse Small (minified) | 538 | 447 | 328 | 1.20× | Fused | Win |
| Marine IK Reverse Small (prettified) | 1,385 | 1,740 (n/c) | 1,212 | 0.80× | Two-stage | Win |
| Marine IK Small (minified) | 619 | 504 | 309 | 1.23× | Fused | Win |
| Marine IK Small (prettified) | 1,527 (n/c) | 1,759 | 1,420 | 0.87× | Two-stage | Win |
| Mesh Small (minified) | 532 | 304 | 395 | 1.75× | Fused | Win |
| Mesh Small (prettified) | 1,190 | 1,216 | 874 (n/c) | 0.98× | Two-stage | Win |
| Random Small (minified) | 1,417 | 961 | 480 | 1.47× | Fused | Win |
| Random Small (prettified) | 1,943 | 1,315 | 1,101 | 1.48× | Fused | Win |
| Twitter Small (minified) | 2,007 | 1,617 | 1,162 | 1.24× | Fused | Win |
| Twitter Small (prettified) | 1,841 | 1,296 (n/c) | 1,385 | 1.42× | Fused | Win |

**macOS / Clang 23.1 (Apple M1, NEON)**

| Test | Fused (MB/s) | Two-stage (MB/s) | simdjson (MB/s) | Fused ÷ two-stage | Faster Jsonifier path | Best Jsonifier path vs simdjson |
|---|---|---|---|---|---|---|
| Bool (POD) | 1,111 | 213 | 172 | 5.22× | Fused | Win |
| Double (POD) | 878 | 265 | 177 | 3.31× | Fused | Win |
| Int64 (POD) | 1,884 | 600 | 422 | 3.14× | Fused | Win |
| String (POD) | 1,088 | 613 | 745 | 1.78× | Fused | Win |
| Uint64 (POD) | 1,927 | 548 | 415 | 3.52× | Fused | Win |
| Canada (minified) | 519 | 594 | 392 | 0.87× | Two-stage | Win |
| Canada (prettified) | 1,698 | 1,534 | 1,196 | 1.11× | Fused | Win |
| CitmCatalog (minified) | 1,890 | 1,085 (n/c) | 874 | 1.74× | Fused | Win |
| CitmCatalog (prettified) | 3,304 | 2,863 | 2,119 | 1.15× | Fused | Win |
| Discord (minified) | 2,180 | 1,104 | 1,179 | 1.97× | Fused | Win |
| Discord (prettified) | 1,939 | 1,304 | 1,654 | 1.49× | Fused | Win |
| Google Maps Response (minified) | 2,057 | 1,158 | 880 | 1.78× | Fused | Win |
| Google Maps Response (prettified) | 2,938 | 1,918 | 1,827 | 1.53× | Fused | Win |
| Instruments (minified) | 2,580 | 1,613 | 1,261 | 1.60× | Fused | Win |
| Instruments (prettified) | 3,008 | 2,704 | 2,248 | 1.11× | Fused | Win |
| Marine IK Reverse (minified) | 652 | 466 (n/c) | 392 | 1.40× | Fused | Win |
| Marine IK Reverse (prettified) | 2,595 | 2,352 | 1,792 | 1.10× | Fused | Win |
| Marine IK (minified) | 663 | 572 | 401 | 1.16× | Fused | Win |
| Marine IK (prettified) | 2,644 | 1,792 | 1,138 (n/c) | 1.48× | Fused | Win |
| Mesh (minified) | 921 | 776 | 707 | 1.19× | Fused | Win |
| Mesh (prettified) | 1,429 | 1,220 | 1,336 | 1.17× | Fused | Win |
| Random (minified) | 1,556 | 813 | 759 | 1.91× | Fused | Win |
| Random (prettified) | 2,220 | 1,193 | 1,387 | 1.86× | Fused | Win |
| Twitter (minified) | 2,104 | 1,571 | 1,347 | 1.34× | Fused | Win |
| Twitter (prettified) | 2,305 | 2,139 | 2,152 | 1.08× | Fused | Win |
| Canada Small (minified) | 649 | 607 | 355 | 1.07× | Fused | Win |
| Canada Small (prettified) | 1,592 | 1,647 | 1,086 | 0.97× | Two-stage | Win |
| CitmCatalog Small (minified) | 1,225 | 910 | 807 | 1.35× | Fused | Win |
| CitmCatalog Small (prettified) | 678 (n/c) | 1,899 | 1,467 (n/c) | 0.36× | Two-stage | Win |
| Discord Small (minified) | 1,840 | 1,137 | 1,148 | 1.62× | Fused | Win |
| Discord Small (prettified) | 1,764 | 1,548 | 1,674 | 1.14× | Fused | Win |
| Google Maps Response Small (minified) | 2,045 | 1,048 | 889 | 1.95× | Fused | Win |
| Google Maps Response Small (prettified) | 2,970 | 1,808 | 2,033 | 1.64× | Fused | Win |
| Instruments Small (minified) | 2,524 | 1,487 | 1,016 (n/c) | 1.70× | Fused | Win |
| Instruments Small (prettified) | 2,811 | 2,552 | 1,863 | 1.10× | Fused | Win |
| Marine IK Reverse Small (minified) | 558 | 487 | 314 | 1.14× | Fused | Win |
| Marine IK Reverse Small (prettified) | 2,118 | 1,870 | 1,244 | 1.13× | Fused | Win |
| Marine IK Small (minified) | 527 | 463 (n/c) | 313 | 1.14× | Fused | Win |
| Marine IK Small (prettified) | 1,857 | 1,752 | 1,498 | 1.06× | Fused | Win |
| Mesh Small (minified) | 705 | 663 | 395 | 1.06× | Fused | Win |
| Mesh Small (prettified) | 1,144 | 1,172 | 796 | 0.98× | Two-stage | Win |
| Random Small (minified) | 2,030 | 1,029 | 882 | 1.97× | Fused | Win |
| Random Small (prettified) | 2,475 | 1,183 | 1,192 | 2.09× | Fused | Win |
| Twitter Small (minified) | 2,301 | 1,609 | 1,520 | 1.43× | Fused | Win |
| Twitter Small (prettified) | 1,828 | 2,165 | 2,177 | 0.84× | Two-stage | Loss |

#### Reused objects

**Windows / MSVC 19.51 (i9-14900KF, AVX2)**

| Test | Fused (MB/s) | Two-stage (MB/s) | simdjson (MB/s) | Fused ÷ two-stage | Faster Jsonifier path | Best Jsonifier path vs simdjson |
|---|---|---|---|---|---|---|
| Bool (POD) | 569 | 221 | 116 | 2.57× | Fused | Win |
| Double (POD) | 832 | 376 | 179 | 2.21× | Fused | Win |
| Int64 (POD) | 2,054 | 651 | 388 | 3.15× | Fused | Win |
| String (POD) | 1,738 | 1,020 | 796 | 1.70× | Fused | Win |
| Uint64 (POD) | 1,821 | 668 | 396 | 2.73× | Fused | Win |
| Canada (minified) | 799 | 748 | 459 | 1.07× | Fused | Win |
| Canada (prettified) | 2,081 | 2,212 | 1,290 | 0.94× | Two-stage | Win |
| CitmCatalog (minified) | 2,187 | 1,490 | 497 | 1.47× | Fused | Win |
| CitmCatalog (prettified) | 3,361 | 3,589 | 1,265 | 0.94× | Two-stage | Win |
| Discord (minified) | 1,617 | 1,402 | 636 | 1.15× | Fused | Win |
| Discord (prettified) | 1,818 | 2,188 | 1,013 | 0.83× | Two-stage | Win |
| Google Maps Response (minified) | 1,952 | 1,325 | 476 | 1.47× | Fused | Win |
| Google Maps Response (prettified) | 3,225 | 3,040 | 1,141 | 1.06× | Fused | Win |
| Instruments (minified) | 2,582 | 1,731 | 645 | 1.49× | Fused | Win |
| Instruments (prettified) | 2,525 | 3,081 | 1,179 | 0.82× | Two-stage | Win |
| Marine IK Reverse (minified) | 651 | 575 | 109 | 1.13× | Fused | Win |
| Marine IK Reverse (prettified) | 2,515 | 2,727 | 524 (n/c) | 0.92× | Two-stage | Win |
| Marine IK (minified) | 666 | 594 | 350 | 1.12× | Fused | Win |
| Marine IK (prettified) | 2,574 | 2,763 | 1,542 | 0.93× | Two-stage | Win |
| Mesh (minified) | 1,210 | 1,015 | 964 | 1.19× | Fused | Win |
| Mesh (prettified) | 1,442 | 1,848 | 1,685 | 0.78× | Two-stage | Win |
| Random (minified) | 1,465 | 1,302 | 543 | 1.13× | Fused | Win |
| Random (prettified) | 1,889 | 2,237 | 977 | 0.84× | Two-stage | Win |
| Twitter (minified) | 1,881 | 1,566 | 867 | 1.20× | Fused | Win |
| Twitter (prettified) | 1,807 | 2,255 | 1,249 | 0.80× | Two-stage | Win |
| Canada Small (minified) | 802 | 732 | 381 | 1.10× | Fused | Win |
| Canada Small (prettified) | 2,036 | 2,218 | 1,099 | 0.92× | Two-stage | Win |
| CitmCatalog Small (minified) | 1,341 | 1,044 | 512 | 1.28× | Fused | Win |
| CitmCatalog Small (prettified) | 1,942 | 2,167 | 1,085 | 0.90× | Two-stage | Win |
| Discord Small (minified) | 1,413 | 1,217 | 695 | 1.16× | Fused | Win |
| Discord Small (prettified) | 1,481 | 1,702 | 1,021 | 0.87× | Two-stage | Win |
| Google Maps Response Small (minified) | 1,968 | 1,318 | 468 | 1.49× | Fused | Win |
| Google Maps Response Small (prettified) | 2,901 | 3,116 | 1,128 | 0.93× | Two-stage | Win |
| Instruments Small (minified) | 2,356 | 1,582 | 652 | 1.49× | Fused | Win |
| Instruments Small (prettified) | 2,282 | 2,892 | 1,227 | 0.79× | Two-stage | Win |
| Marine IK Reverse Small (minified) | 573 | 508 | 79 | 1.13× | Fused | Win |
| Marine IK Reverse Small (prettified) | 2,179 | 2,432 | 402 | 0.90× | Two-stage | Win |
| Marine IK Small (minified) | 619 | 548 | 273 | 1.13× | Fused | Win |
| Marine IK Small (prettified) | 2,307 | 2,547 | 1,270 | 0.91× | Two-stage | Win |
| Mesh Small (minified) | 1,057 | 873 | 635 | 1.21× | Fused | Win |
| Mesh Small (prettified) | 1,449 | 1,730 | 1,218 | 0.84× | Two-stage | Win |
| Random Small (minified) | 1,430 | 1,238 | 536 | 1.15× | Fused | Win |
| Random Small (prettified) | 1,851 | 2,126 | 961 | 0.87× | Two-stage | Win |
| Twitter Small (minified) | 1,913 | 1,575 | 942 | 1.21× | Fused | Win |
| Twitter Small (prettified) | 1,809 | 2,185 | 1,356 | 0.83× | Two-stage | Win |

**Linux / Clang 24.0 (i9-14900KF, AVX2)**

| Test | Fused (MB/s) | Two-stage (MB/s) | simdjson (MB/s) | Fused ÷ two-stage | Faster Jsonifier path | Best Jsonifier path vs simdjson |
|---|---|---|---|---|---|---|
| Bool (POD) | 2,211 | 336 | 236 | 6.58× | Fused | Win |
| Double (POD) | 1,436 | 429 | 269 | 3.35× | Fused | Win |
| Int64 (POD) | 3,477 | 977 | 591 | 3.56× | Fused | Win |
| String (POD) | 2,854 | 1,539 | 1,379 | 1.85× | Fused | Win |
| Uint64 (POD) | 3,710 | 974 | 555 | 3.81× | Fused | Win |
| Canada (minified) | 1,189 | 1,152 | 765 | 1.03× | Fused | Win |
| Canada (prettified) | 3,092 | 3,166 | 2,255 | 0.98× | Two-stage | Win |
| CitmCatalog (minified) | 3,050 | 2,176 | 1,431 | 1.40× | Fused | Win |
| CitmCatalog (prettified) | 4,283 | 4,995 | 3,537 | 0.86× | Two-stage | Win |
| Discord (minified) | 3,409 | 2,542 | 1,905 | 1.34× | Fused | Win |
| Discord (prettified) | 2,581 | 3,860 | 3,031 | 0.67× | Two-stage | Win |
| Google Maps Response (minified) | 3,158 | 2,195 | 1,502 | 1.44× | Fused | Win |
| Google Maps Response (prettified) | 3,937 | 4,709 | 3,483 | 0.84× | Two-stage | Win |
| Instruments (minified) | 3,205 | 2,309 | 1,913 | 1.39× | Fused | Win |
| Instruments (prettified) | 3,016 | 3,932 | 3,477 | 0.77× | Two-stage | Win |
| Marine IK Reverse (minified) | 941 | 842 | 670 | 1.12× | Fused | Win |
| Marine IK Reverse (prettified) | 3,750 | 3,404 | 3,055 | 1.10× | Fused | Win |
| Marine IK (minified) | 972 | 865 | 668 | 1.12× | Fused | Win |
| Marine IK (prettified) | 3,809 | 3,560 | 3,085 | 1.07× | Fused | Win |
| Mesh (minified) | 1,458 | 1,304 | 1,346 | 1.12× | Fused | Win |
| Mesh (prettified) | 2,153 | 2,373 | 2,439 | 0.91× | Two-stage | Loss |
| Random (minified) | 2,627 | 2,051 | 1,441 | 1.28× | Fused | Win |
| Random (prettified) | 2,674 | 3,307 | 2,550 | 0.81× | Two-stage | Win |
| Twitter (minified) | 2,618 | 2,633 | 2,132 | 0.99× | Two-stage | Win |
| Twitter (prettified) | 2,242 | 3,544 | 3,027 | 0.63× | Two-stage | Win |
| Canada Small (minified) | 1,279 | 1,213 | 821 | 1.05× | Fused | Win |
| Canada Small (prettified) | 3,029 | 3,204 | 2,320 | 0.95× | Two-stage | Win |
| CitmCatalog Small (minified) | 2,410 | 1,858 | 1,452 | 1.30× | Fused | Win |
| CitmCatalog Small (prettified) | 2,977 | 3,644 | 2,971 | 0.82× | Two-stage | Win |
| Discord Small (minified) | 2,850 | 2,257 | 1,863 | 1.26× | Fused | Win |
| Discord Small (prettified) | 2,405 | 3,178 | 2,708 | 0.76× | Two-stage | Win |
| Google Maps Response Small (minified) | 2,847 | 2,023 | 1,524 | 1.41× | Fused | Win |
| Google Maps Response Small (prettified) | 3,611 | 4,524 | 3,518 | 0.80× | Two-stage | Win |
| Instruments Small (minified) | 3,009 | 2,107 | 1,769 | 1.43× | Fused | Win |
| Instruments Small (prettified) | 2,909 | 3,664 | 3,260 | 0.79× | Two-stage | Win |
| Marine IK Reverse Small (minified) | 1,028 | 896 | 664 | 1.15× | Fused | Win |
| Marine IK Reverse Small (prettified) | 3,403 | 3,941 | 3,093 | 0.86× | Two-stage | Win |
| Marine IK Small (minified) | 1,170 | 996 | 650 | 1.17× | Fused | Win |
| Marine IK Small (prettified) | 3,608 | 4,256 | 3,079 | 0.85× | Two-stage | Win |
| Mesh Small (minified) | 1,172 | 1,091 | 1,004 | 1.07× | Fused | Win |
| Mesh Small (prettified) | 1,755 | 2,024 | 2,048 | 0.87× | Two-stage | Loss |
| Random Small (minified) | 2,956 | 2,141 | 1,413 | 1.38× | Fused | Win |
| Random Small (prettified) | 2,794 | 3,777 | 2,570 | 0.74× | Two-stage | Win |
| Twitter Small (minified) | 3,044 | 2,639 | 2,381 | 1.15× | Fused | Win |
| Twitter Small (prettified) | 2,525 | 3,615 | 3,336 | 0.70× | Two-stage | Win |

**Linux / GCC 16.2 (i9-14900KF, AVX2)**

| Test | Fused (MB/s) | Two-stage (MB/s) | simdjson (MB/s) | Fused ÷ two-stage | Faster Jsonifier path | Best Jsonifier path vs simdjson |
|---|---|---|---|---|---|---|
| Bool (POD) | 2,370 | 281 | 192 | 8.44× | Fused | Win |
| Double (POD) | 1,274 | 389 | 237 | 3.28× | Fused | Win |
| Int64 (POD) | 3,664 | 857 | 507 | 4.28× | Fused | Win |
| String (POD) | 2,693 | 1,255 | 1,274 | 2.15× | Fused | Win |
| Uint64 (POD) | 3,781 | 868 | 503 | 4.35× | Fused | Win |
| Canada (minified) | 1,086 | 1,099 | 916 | 0.99× | Two-stage | Win |
| Canada (prettified) | 2,815 | 3,081 | 2,651 | 0.91× | Two-stage | Win |
| CitmCatalog (minified) | 3,029 | 2,208 | 1,644 | 1.37× | Fused | Win |
| CitmCatalog (prettified) | 4,429 | 4,995 | 3,936 | 0.89× | Two-stage | Win |
| Discord (minified) | 3,296 | 2,390 | 1,759 | 1.38× | Fused | Win |
| Discord (prettified) | 3,138 | 3,557 | 2,788 | 0.88× | Two-stage | Win |
| Google Maps Response (minified) | 3,066 | 2,156 | 1,155 | 1.42× | Fused | Win |
| Google Maps Response (prettified) | 4,436 | 4,532 | 2,726 | 0.98× | Two-stage | Win |
| Instruments (minified) | 4,036 | 2,613 | 1,425 | 1.54× | Fused | Win |
| Instruments (prettified) | 3,738 | 4,211 | 2,707 | 0.89× | Two-stage | Win |
| Marine IK Reverse (minified) | 834 | 788 | 662 | 1.06× | Fused | Win |
| Marine IK Reverse (prettified) | 3,491 | 3,512 | 3,047 | 0.99× | Two-stage | Win |
| Marine IK (minified) | 855 | 794 | 661 | 1.08× | Fused | Win |
| Marine IK (prettified) | 3,565 | 3,617 | 3,064 | 0.99× | Two-stage | Win |
| Mesh (minified) | 1,152 | 1,213 | 1,355 | 0.95× | Two-stage | Loss |
| Mesh (prettified) | 1,875 | 2,179 | 2,511 | 0.86× | Two-stage | Loss |
| Random (minified) | 2,073 | 1,954 | 1,043 | 1.06× | Fused | Win |
| Random (prettified) | 2,629 | 3,194 | 1,878 | 0.82× | Two-stage | Win |
| Twitter (minified) | 2,597 | 2,504 | 1,563 | 1.04× | Fused | Win |
| Twitter (prettified) | 2,362 | 3,138 | 2,257 | 0.75× | Two-stage | Win |
| Canada Small (minified) | 1,310 | 1,180 | 888 | 1.11× | Fused | Win |
| Canada Small (prettified) | 3,054 | 3,234 | 2,588 | 0.94× | Two-stage | Win |
| CitmCatalog Small (minified) | 2,287 | 1,844 | 1,359 | 1.24× | Fused | Win |
| CitmCatalog Small (prettified) | 3,047 | 3,701 | 2,856 | 0.82× | Two-stage | Win |
| Discord Small (minified) | 2,602 | 2,054 | 1,548 | 1.27× | Fused | Win |
| Discord Small (prettified) | 2,476 | 2,895 | 2,247 | 0.86× | Two-stage | Win |
| Google Maps Response Small (minified) | 2,792 | 2,004 | 1,243 | 1.39× | Fused | Win |
| Google Maps Response Small (prettified) | 4,088 | 4,336 | 2,897 | 0.94× | Two-stage | Win |
| Instruments Small (minified) | 3,675 | 2,410 | 1,325 | 1.53× | Fused | Win |
| Instruments Small (prettified) | 3,542 | 3,979 | 2,439 | 0.89× | Two-stage | Win |
| Marine IK Reverse Small (minified) | 1,015 | 879 | 638 | 1.16× | Fused | Win |
| Marine IK Reverse Small (prettified) | 3,361 | 3,927 | 2,969 | 0.86× | Two-stage | Win |
| Marine IK Small (minified) | 1,142 | 912 | 623 | 1.25× | Fused | Win |
| Marine IK Small (prettified) | 3,585 | 4,065 | 2,885 | 0.88× | Two-stage | Win |
| Mesh Small (minified) | 1,237 | 1,048 | 1,014 | 1.18× | Fused | Win |
| Mesh Small (prettified) | 1,679 | 1,989 | 2,046 | 0.84× | Two-stage | Loss |
| Random Small (minified) | 2,659 | 2,043 | 1,027 | 1.30× | Fused | Win |
| Random Small (prettified) | 3,053 | 3,487 | 1,882 | 0.88× | Two-stage | Win |
| Twitter Small (minified) | 3,340 | 2,574 | 1,732 | 1.30× | Fused | Win |
| Twitter Small (prettified) | 2,674 | 3,364 | 2,391 | 0.80× | Two-stage | Win |

**macOS / GCC 16.2 (Apple M1, NEON)**

| Test | Fused (MB/s) | Two-stage (MB/s) | simdjson (MB/s) | Fused ÷ two-stage | Faster Jsonifier path | Best Jsonifier path vs simdjson |
|---|---|---|---|---|---|---|
| Bool (POD) | 1,442 | 221 | 169 | 6.52× | Fused | Win |
| Double (POD) | 1,060 | 312 | 148 | 3.39× | Fused | Win |
| Int64 (POD) | 2,288 | 607 | 442 | 3.77× | Fused | Win |
| String (POD) | 1,885 | 858 | 872 | 2.20× | Fused | Win |
| Uint64 (POD) | 2,366 | 609 | 387 | 3.88× | Fused | Win |
| Canada (minified) | 799 (n/c) | 620 | 434 (n/c) | 1.29× | Fused | Win |
| Canada (prettified) | 1,713 | 1,473 | 1,017 (n/c) | 1.16× | Fused | Win |
| CitmCatalog (minified) | 1,528 | 1,289 | 1,029 | 1.18× | Fused | Win |
| CitmCatalog (prettified) | 3,254 | 2,595 | 2,310 | 1.25× | Fused | Win |
| Discord (minified) | 1,846 | 1,268 | 1,042 | 1.46× | Fused | Win |
| Discord (prettified) | 2,229 | 1,955 | 1,527 | 1.14× | Fused | Win |
| Google Maps Response (minified) | 1,824 | 1,216 | 571 | 1.50× | Fused | Win |
| Google Maps Response (prettified) | 2,663 | 1,789 (n/c) | 1,425 (n/c) | 1.49× | Fused | Win |
| Instruments (minified) | 2,313 | 1,491 | 1,160 | 1.55× | Fused | Win |
| Instruments (prettified) | 2,989 | 2,447 | 1,684 | 1.22× | Fused | Win |
| Marine IK Reverse (minified) | 537 | 477 (n/c) | 321 | 1.13× | Fused | Win |
| Marine IK Reverse (prettified) | 2,409 | 2,180 | 1,922 | 1.10× | Fused | Win |
| Marine IK (minified) | 545 | 395 | 383 | 1.38× | Fused | Win |
| Marine IK (prettified) | 1,799 | 2,145 | 1,654 (n/c) | 0.84× | Two-stage | Win |
| Mesh (minified) | 882 | 855 | 842 | 1.03× | Fused | Win |
| Mesh (prettified) | 1,285 (n/c) | 1,294 | 1,525 | 0.99× | Tie | Loss |
| Random (minified) | 1,531 | 928 (n/c) | 693 | 1.65× | Fused | Win |
| Random (prettified) | 2,027 | 1,286 (n/c) | 787 | 1.58× | Fused | Win |
| Twitter (minified) | 2,057 (n/c) | 1,479 (n/c) | 978 | 1.39× | Fused | Win |
| Twitter (prettified) | 2,147 | 2,378 | 1,712 | 0.90× | Two-stage | Win |
| Canada Small (minified) | 892 | 779 | 372 | 1.15× | Fused | Win |
| Canada Small (prettified) | 1,988 (n/c) | 1,875 (n/c) | 1,158 (n/c) | 1.06× | Fused | Win |
| CitmCatalog Small (minified) | 1,657 | 1,189 | 812 | 1.39× | Fused | Win |
| CitmCatalog Small (prettified) | 1,994 | 2,177 | 1,740 | 0.92× | Two-stage | Win |
| Discord Small (minified) | 2,009 | 1,428 | 871 | 1.41× | Fused | Win |
| Discord Small (prettified) | 2,113 | 1,937 | 1,505 | 1.09× | Fused | Win |
| Google Maps Response Small (minified) | 2,097 | 1,191 | 548 | 1.76× | Fused | Win |
| Google Maps Response Small (prettified) | 2,931 | 2,383 | 1,375 | 1.23× | Fused | Win |
| Instruments Small (minified) | 2,502 (n/c) | 521 | 797 (n/c) | 4.80× | Fused | Win |
| Instruments Small (prettified) | 2,784 | 2,151 | 1,997 | 1.29× | Fused | Win |
| Marine IK Reverse Small (minified) | 675 | 540 | 397 | 1.25× | Fused | Win |
| Marine IK Reverse Small (prettified) | 1,973 | 1,350 | 1,577 | 1.46× | Fused | Win |
| Marine IK Small (minified) | 733 | 572 (n/c) | 349 | 1.28× | Fused | Win |
| Marine IK Small (prettified) | 2,244 | 2,461 | 1,565 | 0.91× | Two-stage | Win |
| Mesh Small (minified) | 960 | 674 (n/c) | 555 | 1.42× | Fused | Win |
| Mesh Small (prettified) | 1,449 | 1,275 | 1,378 | 1.14× | Fused | Tie |
| Random Small (minified) | 1,698 | 1,169 | 773 | 1.45× | Fused | Win |
| Random Small (prettified) | 2,323 | 1,384 (n/c) | 778 | 1.68× | Fused | Win |
| Twitter Small (minified) | 2,361 | 1,928 (n/c) | 1,132 | 1.22× | Fused | Win |
| Twitter Small (prettified) | 2,437 (n/c) | 2,296 | 1,617 | 1.06× | Fused | Win |

**macOS / Clang 23.1 (Apple M1, NEON)**

| Test | Fused (MB/s) | Two-stage (MB/s) | simdjson (MB/s) | Fused ÷ two-stage | Faster Jsonifier path | Best Jsonifier path vs simdjson |
|---|---|---|---|---|---|---|
| Bool (POD) | 1,199 | 220 | 183 | 5.46× | Fused | Win |
| Double (POD) | 1,004 | 332 | 178 | 3.03× | Fused | Win |
| Int64 (POD) | 2,338 | 272 | 405 | 8.59× | Fused | Win |
| String (POD) | 1,956 | 811 | 1,067 | 2.41× | Fused | Win |
| Uint64 (POD) | 2,321 | 604 | 425 | 3.84× | Fused | Win |
| Canada (minified) | 809 | 680 | 427 | 1.19× | Fused | Win |
| Canada (prettified) | 2,035 | 1,862 | 1,325 (n/c) | 1.09× | Fused | Win |
| CitmCatalog (minified) | 2,439 | 1,468 | 987 | 1.66× | Fused | Win |
| CitmCatalog (prettified) | 3,793 | 3,283 | 2,315 | 1.16× | Fused | Win |
| Discord (minified) | 2,782 | 1,432 | 1,461 | 1.94× | Fused | Win |
| Discord (prettified) | 2,348 | 1,980 | 1,839 | 1.19× | Fused | Win |
| Google Maps Response (minified) | 2,007 | 1,172 | 1,022 | 1.71× | Fused | Win |
| Google Maps Response (prettified) | 3,161 | 2,300 | 1,871 | 1.37× | Fused | Win |
| Instruments (minified) | 2,809 | 1,640 | 1,287 | 1.71× | Fused | Win |
| Instruments (prettified) | 3,330 | 2,930 | 2,326 | 1.14× | Fused | Win |
| Marine IK Reverse (minified) | 752 | 594 | 437 | 1.27× | Fused | Win |
| Marine IK Reverse (prettified) | 2,876 | 2,273 | 1,944 | 1.27× | Fused | Win |
| Marine IK (minified) | 768 | 647 | 437 | 1.19× | Fused | Win |
| Marine IK (prettified) | 2,841 | 2,186 (n/c) | 1,641 (n/c) | 1.30× | Fused | Win |
| Mesh (minified) | 789 (n/c) | 805 | 790 | 0.98× | Tie | Tie |
| Mesh (prettified) | 1,415 | 1,539 | 983 (n/c) | 0.92× | Two-stage | Win |
| Random (minified) | 2,125 | 980 | 749 | 2.17× | Fused | Win |
| Random (prettified) | 2,644 | 1,447 | 1,551 | 1.83× | Fused | Win |
| Twitter (minified) | 2,643 | 1,856 | 1,800 | 1.42× | Fused | Win |
| Twitter (prettified) | 2,775 | 2,663 | 2,473 | 1.04× | Fused | Win |
| Canada Small (minified) | 700 | 708 | 417 | 0.99× | Tie | Win |
| Canada Small (prettified) | 1,834 | 1,573 | 1,245 | 1.17× | Fused | Win |
| CitmCatalog Small (minified) | 1,815 | 1,231 | 1,003 | 1.47× | Fused | Win |
| CitmCatalog Small (prettified) | 2,661 | 2,127 | 1,727 | 1.25× | Fused | Win |
| Discord Small (minified) | 2,096 (n/c) | 1,553 | 1,554 | 1.35× | Fused | Win |
| Discord Small (prettified) | 2,208 | 1,893 | 2,176 | 1.17× | Fused | Tie |
| Google Maps Response Small (minified) | 2,333 | 1,202 | 983 | 1.94× | Fused | Win |
| Google Maps Response Small (prettified) | 3,221 | 2,290 | 2,223 | 1.41× | Fused | Win |
| Instruments Small (minified) | 2,888 | 1,655 | 1,300 | 1.74× | Fused | Win |
| Instruments Small (prettified) | 3,059 | 2,281 (n/c) | 2,234 | 1.34× | Fused | Win |
| Marine IK Reverse Small (minified) | 742 | 604 | 364 | 1.23× | Fused | Win |
| Marine IK Reverse Small (prettified) | 2,696 | 2,202 | 1,561 | 1.22× | Fused | Win |
| Marine IK Small (minified) | 714 | 436 | 344 | 1.64× | Fused | Win |
| Marine IK Small (prettified) | 2,250 | 2,308 | 1,591 | 0.98× | Tie | Win |
| Mesh Small (minified) | 862 | 781 | 649 | 1.10× | Fused | Win |
| Mesh Small (prettified) | 1,243 | 1,389 | 1,309 | 0.89× | Two-stage | Win |
| Random Small (minified) | 1,996 | 1,315 | 942 | 1.52× | Fused | Win |
| Random Small (prettified) | 2,613 | 1,724 (n/c) | 1,544 | 1.52× | Fused | Win |
| Twitter Small (minified) | 2,854 | 1,839 | 1,740 | 1.55× | Fused | Win |
| Twitter Small (prettified) | 2,338 | 2,565 | 2,285 | 0.91× | Two-stage | Win |

### 2.4 What the results show

Which Jsonifier path is faster depends on two things: whether the document is indented, and whether the machine is x86. On the three x86 builds the fused path wins all POD input, 28 of the 30 full-size minified documents and all 30 Small minified ones, while the tape wins 27 of the 30 full-size prettified documents and 29 of the 30 Small prettified ones (the other is a tie). MSVC nearly splits cleanly: minified to the fused path, prettified to the tape, apart from prettified Google Maps Response, which the fused path wins, and its Small copy, which is a tie. On the M1 the fused path wins all POD input, 37 of the 40 minified documents (three tape wins) and 30 of the 40 prettified documents (nine to the tape, one tie).

**POD-type tests: the fused path wins all 25 freshly allocated and all 25 reused, on every build.** It runs them 1.27× (String, macOS/GCC) to 7.74× (Bool, Linux/GCC) faster than the two-stage path, and 1.70× (String, Windows/MSVC) to 8.59× (Int64, macOS/Clang) faster with objects reused. These documents are the case §2 argues from: nothing to skip, every value materialized, so the tape is pure overhead. This is the one result that does not depend on the platform.

**Minified documents: the fused path's, at both sizes.** The fused path wins 95 of the 100 minified documents: all 20 under MSVC, all 20 on Linux/Clang, 18 of 20 on Linux/GCC, 18 of 20 on macOS/GCC and 19 of 20 on macOS/Clang. On Linux/GCC the two-stage path wins minified Canada by 1% (912 against 903 MB/s) and minified Mesh by 7% (1,159 against 1,087), on macOS/GCC it wins minified Twitter by 23% (1,392 against 1,130, with the fused result not converged) and minified Instruments Small by 51% (1,315 against 868), and on macOS/Clang it wins minified Canada by 14% (594 against 519). The fused path's margins run from 3% (Canada on Linux/Clang) to 98% (Discord on macOS/Clang, 2,180 against 1,104 MB/s) on the full-size documents and up to 1.97× on the Small ones (Random Small on macOS/Clang, 2,030 against 1,029 MB/s). This is the workload the fused key literals of §3 were built for: machine-generated JSON in declared order.

**Prettified documents on x86: the two-stage path wins 56 of 60.** It wins all 20 on Linux/GCC, all 10 Small prettified documents on Linux/Clang, 9 of the 10 Small ones under MSVC, and 9 of the 10 full-size prettified documents under MSVC and 8 of 10 on Linux/Clang. The fused path wins prettified Google Maps Response under MSVC by 1% (2,811 against 2,774 MB/s) and prettified Marine IK and Marine IK Reverse on Linux/Clang, and Google Maps Response Small under MSVC is a tie. The tape's margins run from 1% (Marine IK Reverse on Linux/GCC, 3,285 against 3,242 MB/s) to 41% (Twitter on Linux/Clang, 2,756 against 1,950) on the full-size documents and from 4% to 40% on the Small ones, widest on Twitter Small on Linux/Clang. The two Marine IK documents, which are mostly numbers, are the closest on every x86 build, and Discord and Twitter are among the widest. The fused path already predicts the indentation of every line, including the lines that close an object or an array, matches the `": "` after each key as one two-byte constant, and verifies each predicted span with a 16-byte vector loop, a single 8-byte SWAR step and a scalar remainder switch (§3). That keeps the number-heavy documents close, but it does not win most prettified documents on x86. Stage 1 classifies every byte, whitespace included, in vector blocks at a cost that does not depend on layout, while the fused path still does a small amount of branchy work per line: the newline test, the depth multiplication, the span check and the branch on its result. On a heavily indented document that per-line cost appears to outweigh the second pass over the input that the tape costs. We have not isolated this, and state it as a hypothesis. With objects reused the tape wins 57 of the 60 prettified x86 documents, and the fused path wins the same three as in the freshly allocated run.

**Prettified documents on the M1: mostly the fused path.** Of the 40 prettified documents, the fused path wins 30, the tape wins 9 and one is a tie. On the full-size documents the fused path wins 17 of 20: on macOS/GCC it wins 7, by 4% (Canada) to 51% (Google Maps Response, 2,586 against 1,718 MB/s), the tape wins Marine IK by 11% (1,843 against 1,667) and Mesh by 8% (1,372 against 1,273), and Twitter is a tie; on macOS/Clang it wins all 10, by 8% (Twitter) to 86% (Random, 2,220 against 1,193 MB/s). The Small documents lean the same way, less strongly. On macOS/GCC the fused path wins 7, by 9% (Canada Small) to 48% (Random Small, 1,943 against 1,315 MB/s), and the tape wins Marine IK Reverse Small, Marine IK Small and Mesh Small. On macOS/Clang the fused path wins 6 and the tape wins Canada, CitmCatalog, Mesh and Twitter Small; the CitmCatalog Small result (678 against 1,899 MB/s) did not converge for the fused path and is far outside the pattern of the other Small documents, so we read it as an outlier. Reused, the fused path wins 31 of the 40, the tape wins 7 and 2 are ties, and the tape's full-size wins are Marine IK and Twitter on macOS/GCC and Mesh on macOS/Clang. NEON escapes most of the prettified penalty seen on x86, and we do not have an explanation for why. One contributing factor is that Jsonifier's stage 1 is comparatively more expensive there, since NEON has no `movemask` and the collectors emulate it with narrowing shifts, but we have not isolated it.

**Mesh is where Jsonifier is weakest.** The two-stage path wins prettified Mesh on all three x86 builds, on macOS/GCC freshly allocated and on macOS/Clang with objects reused, and minified Mesh on Linux/GCC in both runs. Mesh is also the document on which both Jsonifier paths lose to simdjson On Demand most often. Freshly allocated, that happens on Linux/GCC in both forms, by 6% minified (1,228 against 1,159 MB/s) and 7% prettified (2,257 against 2,095). With objects reused it loses minified and prettified Mesh on Linux/GCC by 10% and 13%, prettified Mesh Small there by 3%, prettified Mesh and Mesh Small on Linux/Clang by 3% and 1%, and prettified Mesh on macOS/GCC by 15% (1,525 against 1,294), while prettified Mesh Small on macOS/GCC and minified Mesh on macOS/Clang are ties. The fused path alone also loses prettified Mesh to simdjson under MSVC in both runs. We have not profiled what is particular about this document.

**MSVC slows simdjson more than either Jsonifier path.** All libraries run slower under MSVC than under Linux/Clang on the same i9-14900KF, but not by the same amount. Across the 9 full-size minified documents in declared key order, the fused path reaches a median 66% of its Linux/Clang throughput, the two-stage path 65% and simdjson 41%. Because the two Jsonifier paths lose about the same share, MSVC routes as Linux/Clang does: minified to the fused path, prettified to the tape. The spread between documents is wider for the fused path (50% on Discord to 82% on Mesh) than for the two-stage path (56% to 78%). That fits the fused path being branch-heavy code whose performance rests on the optimizer while stage 1 is branch-free intrinsic code whose shape is fixed by the source, but we have not isolated it.

**Against simdjson, Jsonifier wins 222 of 225 tests with whichever path is faster and loses three.** The losses are Mesh in both forms on Linux/GCC and prettified Twitter Small on macOS/Clang (2,165 against 2,177 MB/s). The fused path alone wins 211, ties 2 and loses 12: those three, prettified Mesh under MSVC, prettified CitmCatalog Small on macOS/Clang, and prettified Discord, Instruments, Mesh and Twitter on Linux/Clang with the Small copies of Discord, Instruments and Twitter there; prettified Mesh Small on Linux/Clang and minified Instruments Small on macOS/GCC are ties. Under MSVC the faster path wins all 45. Several of these verdicts rest on non-converged results, marked n/c in §2.3.

**Reverse key order.** Requesting every key in reverse order is meant to force simdjson's On Demand API into the rescanning behavior described in §3, and under MSVC it does: simdjson parses minified Marine IK Reverse at 107 MB/s, against 329 MB/s for the same document in declared order, so the faster Jsonifier path is 5.5× faster than simdjson freshly allocated (6.0× reused). The prettified form shows the same gap, at 522 MB/s against 1,437, and the faster Jsonifier path is 4.8× faster (5.2× reused). The Small copies show it too, at 394 MB/s prettified under MSVC. On the other builds the faster Jsonifier path is 1.19× to 1.66× faster than simdjson on the full-size reversed document, freshly allocated, and 1.15× to 1.72× reused. Why the rescan cost shows up mostly under MSVC has not been profiled.

The routing implication is sharper than §2's rule of thumb. On x86 the fused path is right for POD and minified input, apart from Canada and Mesh on Linux/GCC, and prettified input belongs to the two-stage path on all three compilers, apart from Google Maps Response under MSVC and the two Marine IK documents on Linux/Clang. On the M1 the fused path is right for POD and minified input, apart from Twitter and Instruments Small on macOS/GCC and Canada on macOS/Clang, and for most prettified documents, but the tape takes prettified Marine IK and Mesh on macOS/GCC and three or four Small prettified documents on each build. Routing each test to its faster path would raise the record against simdjson from 211 wins, 2 ties and 12 losses to 222 wins and 3 losses. The router does not yet take the architecture or the indentation into account.

### 2.5 Stage 1 + reflection: two-stage Jsonifier against simdjson's reflection reader

simdjson 5 adds a C++26 static-reflection reader (`document.get<T>()`): stage 1 builds the structural index, then the reflected type drives materialization. That is the same shape as Jsonifier's two-stage path, a tape plus a compile-time schema, so ranking those two head to head compares the two stage-1 + reflection designs directly. The reader needs P2996 reflection, so it only runs on GCC, and the harness registers it for corpus documents and their Small copies, not the POD-type tests. Non-converged results are kept and marked n/c.

#### Freshly allocated objects

**Linux / GCC 16.2 (i9-14900KF, AVX2)**

| Test | Two-stage Jsonifier (MB/s) | simdjson reflection (MB/s) | Two-stage ÷ reflection | Verdict |
|---|---|---|---|---|
| Canada (minified) | 912 | 859 | 1.06× | Win |
| Canada (prettified) | 2,611 | 2,506 | 1.04× | Win |
| CitmCatalog (minified) | 1,865 | 1,430 | 1.30× | Win |
| CitmCatalog (prettified) | 4,293 | 3,530 | 1.22× | Win |
| Discord (minified) | 1,820 | 1,524 | 1.19× | Win |
| Discord (prettified) | 2,741 | 2,427 | 1.13× | Win |
| Google Maps Response (minified) | 1,924 | 1,272 | 1.51× | Win |
| Google Maps Response (prettified) | 4,221 | 2,973 | 1.42× | Win |
| Instruments (minified) | 2,311 | 1,828 | 1.26× | Win |
| Instruments (prettified) | 3,793 | 3,326 | 1.14× | Win |
| Marine IK Reverse (minified) | 719 | 728 | 0.99× | **Loss** |
| Marine IK Reverse (prettified) | 3,285 | 3,272 | 1.00× | Win |
| Marine IK (minified) | 725 | 719 | 1.01× | Win |
| Marine IK (prettified) | 3,355 | 3,268 | 1.03× | Win |
| Mesh (minified) | 1,159 | 1,190 | 0.97× | **Loss** |
| Mesh (prettified) | 2,095 | 2,197 | 0.95× | **Loss** |
| Random (minified) | 1,384 | 1,173 | 1.18× | Win |
| Random (prettified) | 2,368 | 2,085 | 1.14× | Win |
| Twitter (minified) | 1,821 | 1,799 | 1.01× | Win |
| Twitter (prettified) | 2,552 | 2,577 | 0.99× | **Loss** |
| Canada Small (minified) | 967 | 939 | 1.03× | Win |
| Canada Small (prettified) | 2,673 | 2,736 | 0.98× | **Loss** |
| CitmCatalog Small (minified) | 1,586 | 1,275 | 1.24× | Win |
| CitmCatalog Small (prettified) | 3,308 | 2,662 | 1.24× | Win |
| Discord Small (minified) | 1,639 | 1,344 | 1.22× | Win |
| Discord Small (prettified) | 2,353 | 1,969 | 1.20× | Win |
| Google Maps Response Small (minified) | 1,798 | 1,273 | 1.41× | Win |
| Google Maps Response Small (prettified) | 3,893 | 2,990 | 1.30× | Win |
| Instruments Small (minified) | 2,237 | 1,727 | 1.30× | Win |
| Instruments Small (prettified) | 3,746 | 3,167 | 1.18× | Win |
| Marine IK Reverse Small (minified) | 725 | 741 | 0.98× | **Loss** |
| Marine IK Reverse Small (prettified) | 3,298 | 3,420 | 0.96× | **Loss** |
| Marine IK Small (minified) | 760 | 751 | 1.01× | Win |
| Marine IK Small (prettified) | 3,457 | 3,406 | 1.01× | Win |
| Mesh Small (minified) | 897 | 866 | 1.04× | Win |
| Mesh Small (prettified) | 1,715 | 1,783 | 0.96× | **Loss** |
| Random Small (minified) | 1,732 | 1,279 | 1.35× | Win |
| Random Small (prettified) | 3,050 | 2,303 | 1.32× | Win |
| Twitter Small (minified) | 2,259 | 2,146 | 1.05× | Win |
| Twitter Small (prettified) | 3,042 | 2,969 | 1.02× | Win |

**macOS / GCC 16.2 (Apple M1, NEON)**

| Test | Two-stage Jsonifier (MB/s) | simdjson reflection (MB/s) | Two-stage ÷ reflection | Verdict |
|---|---|---|---|---|
| Canada (minified) | 517 | 435 | 1.19× | Win |
| Canada (prettified) | 1,574 | 1,205 | 1.31× | Win |
| CitmCatalog (minified) | 1,121 (n/c) | 771 | 1.45× | Win |
| CitmCatalog (prettified) | 2,096 | 2,062 | 1.02× | Tie |
| Discord (minified) | 934 | 975 | 0.96× | **Loss** |
| Discord (prettified) | 1,528 | 1,117 (n/c) | 1.37× | Win |
| Google Maps Response (minified) | 768 | 775 | 0.99× | Tie |
| Google Maps Response (prettified) | 1,718 (n/c) | 1,909 | 0.90× | **Loss** |
| Instruments (minified) | 1,292 | 1,088 | 1.19× | Win |
| Instruments (prettified) | 1,868 | 1,793 | 1.04× | Win |
| Marine IK Reverse (minified) | 335 (n/c) | 373 | 0.90× | **Loss** |
| Marine IK Reverse (prettified) | 1,979 | 1,783 | 1.11× | Win |
| Marine IK (minified) | 319 (n/c) | 364 | 0.88× | **Loss** |
| Marine IK (prettified) | 1,843 | 1,659 (n/c) | 1.11× | Win |
| Mesh (minified) | 732 | 461 (n/c) | 1.59× | Win |
| Mesh (prettified) | 1,372 | 969 | 1.42× | Win |
| Random (minified) | 652 | 639 | 1.02× | Tie |
| Random (prettified) | 1,171 | 961 | 1.22× | Win |
| Twitter (minified) | 1,392 | 1,062 | 1.31× | Win |
| Twitter (prettified) | 1,948 | 1,468 | 1.33× | Win |
| Canada Small (minified) | 636 | 435 | 1.46× | Win |
| Canada Small (prettified) | 1,616 | 1,419 | 1.14× | Win |
| CitmCatalog Small (minified) | 871 | 751 | 1.16× | Win |
| CitmCatalog Small (prettified) | 1,582 (n/c) | 1,293 | 1.22× | Win |
| Discord Small (minified) | 910 | 857 | 1.06× | Win |
| Discord Small (prettified) | 1,481 | 1,294 | 1.14× | Win |
| Google Maps Response Small (minified) | 1,012 | 855 | 1.18× | Win |
| Google Maps Response Small (prettified) | 1,862 | 1,895 | 0.98× | Tie |
| Instruments Small (minified) | 1,315 | 1,065 | 1.24× | Win |
| Instruments Small (prettified) | 2,365 | 1,672 | 1.41× | Win |
| Marine IK Reverse Small (minified) | 447 | 437 | 1.02× | Win |
| Marine IK Reverse Small (prettified) | 1,740 (n/c) | 1,562 | 1.11× | Win |
| Marine IK Small (minified) | 504 | 352 | 1.43× | Win |
| Marine IK Small (prettified) | 1,759 | 1,850 | 0.95× | **Loss** |
| Mesh Small (minified) | 304 | 398 (n/c) | 0.76× | **Loss** |
| Mesh Small (prettified) | 1,216 | 979 | 1.24× | Win |
| Random Small (minified) | 961 | 765 | 1.26× | Win |
| Random Small (prettified) | 1,315 | 831 (n/c) | 1.58× | Win |
| Twitter Small (minified) | 1,617 | 1,168 | 1.38× | Win |
| Twitter Small (prettified) | 1,296 (n/c) | 1,366 | 0.95× | **Loss** |

#### Reused objects

**Linux / GCC 16.2 (i9-14900KF, AVX2)**

| Test | Two-stage Jsonifier (MB/s) | simdjson reflection (MB/s) | Two-stage ÷ reflection | Verdict |
|---|---|---|---|---|
| Canada (minified) | 1,099 | 1,040 | 1.06× | Win |
| Canada (prettified) | 3,081 | 2,974 | 1.04× | Win |
| CitmCatalog (minified) | 2,208 | 1,690 | 1.31× | Win |
| CitmCatalog (prettified) | 4,995 | 4,068 | 1.23× | Win |
| Discord (minified) | 2,390 | 1,776 | 1.35× | Win |
| Discord (prettified) | 3,557 | 2,782 | 1.28× | Win |
| Google Maps Response (minified) | 2,156 | 1,324 | 1.63× | Win |
| Google Maps Response (prettified) | 4,532 | 3,089 | 1.47× | Win |
| Instruments (minified) | 2,613 | 1,875 | 1.39× | Win |
| Instruments (prettified) | 4,211 | 3,409 | 1.24× | Win |
| Marine IK Reverse (minified) | 788 | 815 | 0.97× | **Loss** |
| Marine IK Reverse (prettified) | 3,512 | 3,624 | 0.97× | **Loss** |
| Marine IK (minified) | 794 | 814 | 0.98× | **Loss** |
| Marine IK (prettified) | 3,617 | 3,657 | 0.99× | **Loss** |
| Mesh (minified) | 1,213 | 1,262 | 0.96× | **Loss** |
| Mesh (prettified) | 2,179 | 2,323 | 0.94× | **Loss** |
| Random (minified) | 1,954 | 1,437 | 1.36× | Win |
| Random (prettified) | 3,194 | 2,535 | 1.26× | Win |
| Twitter (minified) | 2,504 | 2,082 | 1.20× | Win |
| Twitter (prettified) | 3,138 | 2,953 | 1.06× | Win |
| Canada Small (minified) | 1,180 | 1,109 | 1.06× | Win |
| Canada Small (prettified) | 3,234 | 3,149 | 1.03× | Win |
| CitmCatalog Small (minified) | 1,844 | 1,405 | 1.31× | Win |
| CitmCatalog Small (prettified) | 3,701 | 2,889 | 1.28× | Win |
| Discord Small (minified) | 2,054 | 1,552 | 1.32× | Win |
| Discord Small (prettified) | 2,895 | 2,276 | 1.27× | Win |
| Google Maps Response Small (minified) | 2,004 | 1,336 | 1.50× | Win |
| Google Maps Response Small (prettified) | 4,336 | 3,143 | 1.38× | Win |
| Instruments Small (minified) | 2,410 | 1,776 | 1.36× | Win |
| Instruments Small (prettified) | 3,979 | 3,231 | 1.23× | Win |
| Marine IK Reverse Small (minified) | 879 | 847 | 1.04× | Win |
| Marine IK Reverse Small (prettified) | 3,927 | 3,832 | 1.02× | Win |
| Marine IK Small (minified) | 912 | 845 | 1.08× | Win |
| Marine IK Small (prettified) | 4,065 | 3,868 | 1.05× | Win |
| Mesh Small (minified) | 1,048 | 1,042 | 1.01× | Win |
| Mesh Small (prettified) | 1,989 | 2,124 | 0.94× | **Loss** |
| Random Small (minified) | 2,043 | 1,418 | 1.44× | Win |
| Random Small (prettified) | 3,487 | 2,537 | 1.37× | Win |
| Twitter Small (minified) | 2,574 | 2,329 | 1.11× | Win |
| Twitter Small (prettified) | 3,364 | 3,205 | 1.05× | Win |

**macOS / GCC 16.2 (Apple M1, NEON)**

| Test | Two-stage Jsonifier (MB/s) | simdjson reflection (MB/s) | Two-stage ÷ reflection | Verdict |
|---|---|---|---|---|
| Canada (minified) | 620 | 557 (n/c) | 1.11× | Win |
| Canada (prettified) | 1,473 | 1,346 | 1.09× | Win |
| CitmCatalog (minified) | 1,289 | 1,059 | 1.22× | Win |
| CitmCatalog (prettified) | 2,595 | 2,314 | 1.12× | Win |
| Discord (minified) | 1,268 | 1,093 | 1.16× | Win |
| Discord (prettified) | 1,955 | 1,466 | 1.33× | Win |
| Google Maps Response (minified) | 1,216 | 902 | 1.35× | Win |
| Google Maps Response (prettified) | 1,789 (n/c) | 1,984 | 0.90× | **Loss** |
| Instruments (minified) | 1,491 | 1,101 | 1.35× | Win |
| Instruments (prettified) | 2,447 | 1,315 | 1.86× | Win |
| Marine IK Reverse (minified) | 477 (n/c) | 456 | 1.05× | Win |
| Marine IK Reverse (prettified) | 2,180 | 2,091 | 1.04× | Win |
| Marine IK (minified) | 395 | 406 | 0.97× | Tie |
| Marine IK (prettified) | 2,145 | 1,584 (n/c) | 1.35× | Win |
| Mesh (minified) | 855 | 450 (n/c) | 1.90× | Win |
| Mesh (prettified) | 1,294 | 1,081 | 1.20× | Win |
| Random (minified) | 928 (n/c) | 705 | 1.32× | Win |
| Random (prettified) | 1,286 (n/c) | 1,231 | 1.05× | Tie |
| Twitter (minified) | 1,479 (n/c) | 1,013 | 1.46× | Win |
| Twitter (prettified) | 2,378 | 1,658 | 1.43× | Win |
| Canada Small (minified) | 779 | 492 | 1.58× | Win |
| Canada Small (prettified) | 1,875 (n/c) | 1,610 | 1.16× | Win |
| CitmCatalog Small (minified) | 1,189 | 917 | 1.30× | Win |
| CitmCatalog Small (prettified) | 2,177 | 1,534 | 1.42× | Win |
| Discord Small (minified) | 1,428 | 963 | 1.48× | Win |
| Discord Small (prettified) | 1,937 | 1,528 | 1.27× | Win |
| Google Maps Response Small (minified) | 1,191 | 937 | 1.27× | Win |
| Google Maps Response Small (prettified) | 2,383 | 2,100 | 1.13× | Win |
| Instruments Small (minified) | 521 | 654 | 0.80× | **Loss** |
| Instruments Small (prettified) | 2,151 | 1,755 | 1.23× | Win |
| Marine IK Reverse Small (minified) | 540 | 508 | 1.06× | Win |
| Marine IK Reverse Small (prettified) | 1,350 | 1,904 | 0.71× | **Loss** |
| Marine IK Small (minified) | 572 (n/c) | 457 | 1.25× | Win |
| Marine IK Small (prettified) | 2,461 | 1,797 | 1.37× | Win |
| Mesh Small (minified) | 674 (n/c) | 360 | 1.87× | Win |
| Mesh Small (prettified) | 1,275 | 1,085 | 1.18× | Win |
| Random Small (minified) | 1,169 | 758 | 1.54× | Win |
| Random Small (prettified) | 1,384 (n/c) | 1,301 | 1.06× | Tie |
| Twitter Small (minified) | 1,928 (n/c) | 1,247 | 1.55× | Win |
| Twitter Small (prettified) | 2,296 | 1,759 | 1.31× | Win |

In the freshly allocated run, two-stage Jsonifier wins 61 of these 80 tests, ties 4 and loses 15. On the full-size documents it wins 29 of 40: on Linux/GCC it is slower on minified Marine IK Reverse, Mesh in both forms and prettified Twitter, all by 5% or less; on macOS/GCC it loses minified Discord, Marine IK Reverse and Marine IK and prettified Google Maps Response (by 4% to 12%) and ties minified Google Maps Response and Random and prettified CitmCatalog. On the Small documents it wins 32 of 40, losing prettified Canada Small, both Marine IK Reverse Small documents and prettified Mesh Small on Linux/GCC (by 2% to 4%), and minified Mesh Small (by 24%) and prettified Marine IK Small and Twitter Small (by 5%) on macOS/GCC, where it ties prettified Google Maps Response Small. Mesh is also where Jsonifier is weakest against simdjson On Demand (§2.4); we have not profiled either result. With the fused path instead of the two-stage one, Jsonifier beats the reflection reader on 64 of the 80, with 2 ties. With objects reused, two-stage Jsonifier wins 67 of 80, ties 3 and loses 10. Seven of the losses are on Linux/GCC: Marine IK Reverse, Marine IK and Mesh in both forms and prettified Mesh Small, all by 6% or less. On macOS/GCC it wins 34 of 40 reused, loses prettified Google Maps Response by 10%, minified Instruments Small by 20% and prettified Marine IK Reverse Small by 29%, and ties minified Marine IK and prettified Random and Random Small.

### 2.6 Partial reading: where the tape earns its keep

The harness has one test built for the workload the tape exists for. "Twitter Partial" parses the Twitter document, and its Small copy, into a type that holds three fields per status (`text`, `user.screen_name` and `retweet_count`) and skips everything else. The harness reads the keys without assuming their order, and simdjson On Demand reads the same three fields. The harness requires stage 1 and stage 2 for this test, so both Jsonifier rows run the two-stage path; the table uses the "jsonifier (two-stage)" row. The two rows agree within 8% on 31 of the 40 results; all nine wider gaps are on the macOS builds, and the widest is freshly allocated minified Twitter Small on macOS/GCC, where the two-stage row runs at 1,122 MB/s against the "jsonifier" row's 2,826. Results that did not converge are marked n/c.

#### Freshly allocated objects

| Platform / Compiler | Document | Jsonifier stage 1 + 2 (MB/s) | simdjson On Demand (MB/s) | Jsonifier ÷ simdjson | Verdict |
|---|---|---|---|---|---|
| Windows / MSVC 19.51 (i9-14900KF, AVX2) | Twitter (minified) | 5,445 | 2,723 | 2.00× | Win |
| Windows / MSVC 19.51 (i9-14900KF, AVX2) | Twitter (prettified) | 6,952 | 3,514 | 1.98× | Win |
| Windows / MSVC 19.51 (i9-14900KF, AVX2) | Twitter Small (minified) | 4,155 | 2,770 | 1.50× | Win |
| Windows / MSVC 19.51 (i9-14900KF, AVX2) | Twitter Small (prettified) | 5,321 | 3,549 | 1.50× | Win |
| Linux / Clang 24.0 (i9-14900KF, AVX2) | Twitter (minified) | 6,923 | 5,649 | 1.23× | Win |
| Linux / Clang 24.0 (i9-14900KF, AVX2) | Twitter (prettified) | 9,189 | 7,470 | 1.23× | Win |
| Linux / Clang 24.0 (i9-14900KF, AVX2) | Twitter Small (minified) | 5,234 | 5,420 | 0.97× | Loss |
| Linux / Clang 24.0 (i9-14900KF, AVX2) | Twitter Small (prettified) | 6,842 | 6,848 | 1.00× | Tie |
| Linux / GCC 16.2 (i9-14900KF, AVX2) | Twitter (minified) | 7,356 | 5,473 | 1.34× | Win |
| Linux / GCC 16.2 (i9-14900KF, AVX2) | Twitter (prettified) | 8,842 | 7,164 | 1.23× | Win |
| Linux / GCC 16.2 (i9-14900KF, AVX2) | Twitter Small (minified) | 5,647 | 4,888 | 1.16× | Win |
| Linux / GCC 16.2 (i9-14900KF, AVX2) | Twitter Small (prettified) | 7,101 | 6,351 | 1.12× | Win |
| macOS / GCC 16.2 (Apple M1, NEON) | Twitter (minified) | 3,806 | 1,597 | 2.38× | Win |
| macOS / GCC 16.2 (Apple M1, NEON) | Twitter (prettified) | 4,440 | 3,368 | 1.32× | Win |
| macOS / GCC 16.2 (Apple M1, NEON) | Twitter Small (minified) | 1,122 | 2,695 | 0.42× | Loss |
| macOS / GCC 16.2 (Apple M1, NEON) | Twitter Small (prettified) | 3,961 | 3,287 | 1.21× | Win |
| macOS / Clang 23.1 (Apple M1, NEON) | Twitter (minified) | 3,982 | 2,233 | 1.78× | Win |
| macOS / Clang 23.1 (Apple M1, NEON) | Twitter (prettified) | 4,089 | 3,915 | 1.04× | Win |
| macOS / Clang 23.1 (Apple M1, NEON) | Twitter Small (minified) | 3,367 | 3,096 | 1.09× | Win |
| macOS / Clang 23.1 (Apple M1, NEON) | Twitter Small (prettified) | 4,039 | 3,820 | 1.06× | Win |

#### Reused objects

| Platform / Compiler | Document | Jsonifier stage 1 + 2 (MB/s) | simdjson On Demand (MB/s) | Jsonifier ÷ simdjson | Verdict |
|---|---|---|---|---|---|
| Windows / MSVC 19.51 (i9-14900KF, AVX2) | Twitter (minified) | 5,708 | 2,836 | 2.01× | Win |
| Windows / MSVC 19.51 (i9-14900KF, AVX2) | Twitter (prettified) | 7,277 | 3,628 | 2.01× | Win |
| Windows / MSVC 19.51 (i9-14900KF, AVX2) | Twitter Small (minified) | 4,394 | 2,917 | 1.51× | Win |
| Windows / MSVC 19.51 (i9-14900KF, AVX2) | Twitter Small (prettified) | 5,829 | 3,765 | 1.55× | Win |
| Linux / Clang 24.0 (i9-14900KF, AVX2) | Twitter (minified) | 7,098 | 5,811 | 1.22× | Win |
| Linux / Clang 24.0 (i9-14900KF, AVX2) | Twitter (prettified) | 9,376 | 7,594 | 1.23× | Win |
| Linux / Clang 24.0 (i9-14900KF, AVX2) | Twitter Small (minified) | 5,534 | 5,454 | 1.01× | Win |
| Linux / Clang 24.0 (i9-14900KF, AVX2) | Twitter Small (prettified) | 7,113 | 6,995 | 1.02× | Win |
| Linux / GCC 16.2 (i9-14900KF, AVX2) | Twitter (minified) | 7,563 | 5,541 | 1.36× | Win |
| Linux / GCC 16.2 (i9-14900KF, AVX2) | Twitter (prettified) | 9,029 | 7,266 | 1.24× | Win |
| Linux / GCC 16.2 (i9-14900KF, AVX2) | Twitter Small (minified) | 5,933 | 5,007 | 1.18× | Win |
| Linux / GCC 16.2 (i9-14900KF, AVX2) | Twitter Small (prettified) | 7,463 | 6,447 | 1.16× | Win |
| macOS / GCC 16.2 (Apple M1, NEON) | Twitter (minified) | 3,801 | 2,158 | 1.76× | Win |
| macOS / GCC 16.2 (Apple M1, NEON) | Twitter (prettified) | 4,417 | 3,574 | 1.24× | Win |
| macOS / GCC 16.2 (Apple M1, NEON) | Twitter Small (minified) | 2,116 | 2,437 | 0.87× | Loss |
| macOS / GCC 16.2 (Apple M1, NEON) | Twitter Small (prettified) | 3,866 | 1,297 | 2.98× | Win |
| macOS / Clang 23.1 (Apple M1, NEON) | Twitter (minified) | 3,367 | 1,955 | 1.72× | Win |
| macOS / Clang 23.1 (Apple M1, NEON) | Twitter (prettified) | 4,473 | 3,759 | 1.19× | Win |
| macOS / Clang 23.1 (Apple M1, NEON) | Twitter Small (minified) | 3,641 | 2,681 | 1.36× | Win |
| macOS / Clang 23.1 (Apple M1, NEON) | Twitter Small (prettified) | 4,152 | 4,183 | 0.99× | Loss |

Jsonifier wins 35 of the 40 results, ties 1 and loses 4, by up to 2.98× (prettified Twitter Small on macOS/GCC, reused). All 20 full-size results are wins. The tie and losses are all on Twitter Small: freshly allocated, Jsonifier loses minified Twitter Small on Linux/Clang (0.97×) and macOS/GCC (0.42×) and ties prettified Twitter Small on Linux/Clang; reused, it loses minified Twitter Small on macOS/GCC (0.87×) and prettified Twitter Small on macOS/Clang (0.99×). Throughput here is counted over the whole document, so these figures run well above the full-parse ones: on minified Twitter the fused path parses at 1,307 MB/s under MSVC and 2,048 MB/s on Linux/Clang when it materializes every value, against 5,445 and 6,923 MB/s here, because the bytes the caller does not ask for are skipped through the tape instead of being walked. The margin over simdjson is 1.50× to 2.01× under MSVC and 0.42× to 2.98× elsewhere, and it is narrower on the Small document than on the full-size one in 18 of the 20 comparable pairs; the exceptions are freshly allocated prettified Twitter on macOS/Clang and reused prettified Twitter on macOS/GCC. On minified Twitter in the freshly allocated run, simdjson On Demand under MSVC reaches 48% of its Linux/Clang throughput and Jsonifier's stage 1 + 2 path reaches 79%. On the full-size document this is the one test in the sweep where the index is unambiguously the right tool, and it is the reason the two-stage machinery stays in Jsonifier even though full-document parsing does not route through it.

### 2.7 Validation

The sweep's "Validate" test checks a 617 KiB document for well-formedness without materializing anything, against Glaze's validator; simdjson is not registered for it, and it runs freshly allocated only. Jsonifier wins on all five builds:

| Platform / Compiler | Jsonifier (MB/s) | Glaze (MB/s) | Jsonifier ÷ Glaze | Verdict |
|---|---|---|---|---|
| Windows / MSVC 19.51 (i9-14900KF, AVX2) | 2,804 | 2,657 | 1.06× | Win |
| Linux / Clang 24.0 (i9-14900KF, AVX2) | 3,557 | 2,595 | 1.37× | Win |
| Linux / GCC 16.2 (i9-14900KF, AVX2) | 3,031 | 2,456 | 1.23× | Win |
| macOS / GCC 16.2 (Apple M1, NEON) | 1,525 | 1,328 | 1.15× | Win |
| macOS / Clang 23.1 (Apple M1, NEON) | 2,127 | 1,707 | 1.25× | Win |

## 3. The single-pass path: `json_cursor` over raw text

The fused path is implemented as `json_cursor<parseOpts, read_buffer_ptr>`: a set of static navigation primitives over a raw `read_buffer_ptr` cursor, with the end pointer, the detected indentation and the string buffer held in a `parse_context` and the nesting depth passed down the call chain, walking the document exactly once. Several design decisions distinguish it:

**Compile-time key fusion.** For each reflected member, the expected token is baked into the binary at compile time as a fused literal. In minified known-order mode, an entire member header (leading comma, quoted key, and colon) collapses into a single constant:

```cpp
template<uint64_t index, typename literal_type> JSONIFIER_INLINE static constexpr auto makeMemberLiteralNew(const literal_type& keyLiteral) noexcept {
	if constexpr (index > 0) {
		return string_literal{ "," } + string_literal{ "\"" } + keyLiteral + string_literal{ "\"" } + string_literal{ ":" };
	} else {
		return string_literal{ "\"" } + keyLiteral + string_literal{ "\"" } + string_literal{ ":" };
	}
}
```

When the document arrives in the expected order, the overwhelmingly common case for machine-generated JSON, matching `,"name":` is one constant-length comparison, and the cursor jumps the entire header in a single add. No tokenizer, no tape, no per-character state machine.

**Adaptive out-of-order recovery.** When the known-order guess misses, the parser falls back to a compile-time hash map over the type's keys (`hash_map<value_type>::findIndex`), dispatches through a generated table, and, crucially, *learns*: a thread-local `antiHashStatesNew` array records which index actually matched at each slot, so a document stream with a consistently permuted key order pays the hash cost once and then rides the corrected fast path.

This is worth dwelling on, because key order is where iterative traversal models pay their hidden tax. simdjson's On Demand API resolves a field lookup by scanning forward through the object from the current cursor position; the project's own documentation (doc/basics.md, field-access guidance) instructs callers to request fields in the order they appear in the document for best performance. When a requested key is *not* next, the cursor scans and skips every intervening key/value pair to find it, and a subsequent request for a key that lies *behind* the cursor forces a wrap-around rescan of the object. The degenerate case is a caller requesting fields in an order that consistently mismatches the document: each lookup can traverse a large fraction of the object's raw bytes, turning an O(members) parse into an O(members²) crawl over structural positions, per object, per document, forever. The model has no memory; the millionth permuted document costs exactly what the first one did.

Jsonifier's failure mode for the same situation is: one hash lookup, one dispatch-table indirection, and a learned correction. The mismatch cost is paid once per key slot per thread, not once per object instance. A feed of a billion documents with keys in reversed order parses at effectively the same throughput as a feed in declared order, because after the first document the "expected order" *is* the observed order. The schema-directed model converts key order from a per-document runtime tax into a per-stream calibration.

**Depth-predicted indentation.** The non-minified specialization exploits the fact that pretty-printed JSON indents each line by a fixed unit times its nesting depth. At the root, `collectIndentSize` measures that unit once: the indent character (`wsChar`) and how many of it make one level (`indentSize`). After every `{`, `[` and `,`, `skipWhitespacePredicted` steps over the newline and predicts the next line's indentation as `context.indentSize * depth`. It then verifies the whole predicted span in one call to `spanIsIndent`, which works in three tiers on every architecture: a 16-byte vector loop compares the span against the broadcast indent character while more than 16 bytes remain, a single 8-byte SWAR step (one XOR against the broadcast character in a `uint64_t`) handles 9 to 16 remaining bytes, and a size-class switch with overlapping scalar loads resolves the last 8 or fewer, so no span length needs a byte loop. The tiers were chosen by A/B testing across compilers: a wider 32-byte loop and a remainder case that assembled a 128-bit vector from two 8-byte copies both cost MSVC and GCC measurably, while a pure 8-byte SWAR loop halved prettified Canada's throughput under MSVC because its long spans need the vector tier. Before a closing `}` or `]`, `skipWhitespacePredictedClose` makes the same prediction one level shallower, and after each key `collectObjectColon` matches `": "` as a single two-byte constant. If the span matches and the next byte is not whitespace, the cursor jumps the entire indentation at once. If the prediction misses, the parser falls back to `skipWhitespaceScalar`, a `whitespaceTable` lookup loop that advances one byte per iteration.

The result is a parser whose inner loop is dominated by wide constant comparisons and direct value materialization, with SIMD engaged surgically where it wins (string unescaping, discussed in §6) rather than as a mandatory preprocessing pass.

## 4. Stage 1: structural indexing, Jsonifier style

When the workload *does* justify a structural index, Jsonifier's stage 1 (`simd_string_reader`) implements the Langdale–Lemire bitmask algebra with several architectural departures.

### 4.1 Step geometry as a per-compiler compile-time constant

simdjson classifies input per 64-byte block, and its generic indexer steps through the buffer 128 bytes per iteration: two 64-byte blocks, software-pipelined one block deep, a geometry it retains even on its widest kernel (the Icelake implementation invokes `index<128>`). Jsonifier generalizes the unit of work to a *step* of `simdBlocksPerStep` 64-byte blocks, with the constant chosen per ISA **and per compiler**:

```cpp
template<> struct backend_traits<jsonifier_backend::avx2> : compiler_step_tuning<4, 4, 4, 8, 4, 8> {
	static constexpr const char* name{ "AVX2" };
	using simd_int_t = jsonifier_simd_int_256;
};
```

`compiler_step_tuning` takes (tape step, blocks per step) pairs for Clang, GCC and MSVC in that order and resolves to the current compiler's pair, so the AVX2 traits above carry 4 blocks per step on Clang and 8 on GCC and MSVC, with a tape step of 4 on all three. `simdBlocksPerStep` and `simdTapeStep` are read from the configured backend's traits.

These values are not guesses. They were selected by Cartesian parameter sweeps across five platform/compiler CI targets (Windows/MSVC, Linux/GCC, Linux/Clang, macOS/Clang, plus ARM), validated against popcount histograms of structural density on the benchmark corpus. The finding that Clang and GCC want *different* geometry on the identical ISA reflects real differences in how each compiler schedules the unrolled block bodies: on AVX2 Clang takes four blocks (256 bytes) per step where GCC and MSVC take eight (512 bytes), and on NEON both compilers take four blocks per step with a drain burst of 8. It is only expressible because the entire pipeline is specialized at compile time. A runtime-dispatched kernel gets one shape per ISA; a Cathedral-Architecture kernel gets one shape per (ISA × compiler) cell.

Processing up to 8 blocks (512 bytes) per step before draining amortizes the loop-carried state updates and gives the out-of-order core a deep window of independent block computations, a 2-4x wider scan window than simdjson's fixed 128-byte step.

### 4.2 The classification core: `rope_detector` and the collectors

The per-block bitmask algebra lives in small, composable functor structs. `cmp_eq_op` fans a broadcast comparison across the registers of a block and fuses the per-register movemasks into one `uint64_t` with compile-time shift amounts. The escape/quote/in-string state machine is `rope_detector`, a CRTP mixin over a plain `rope_block` of three masks:

```cpp
JSONIFIER_INLINE void next(const simd_array_t in01, const jsonifier_simd_int_t bsRegister, const jsonifier_simd_int_t quoteRegister) noexcept {
	const uint64_t escaped = nextEscapeAndTerminalCode(simd::cmp_eq_op::impl(in01, bsRegister));
	const uint64_t quotes  = (simd::cmp_eq_op::impl(in01, quoteRegister) & ~escaped);
	rope_block::escaped	   = escaped;
	rope_block::quotes	   = quotes;
	return quotes ? finishNext() : finishNextNoInString();
}
```

The escape logic is the classic odd-length-backslash-run computation with the standard fast exit: a block containing zero backslashes skips the arithmetic and just consumes the carried `nextIsEscaped` bit, the same short-circuit simdjson's `json_escape_scanner` ships by default. Jsonifier extends the principle one level up: the prefix-XOR (`clmul` on x86, a shift-XOR ladder on NEON) that turns the quote mask into an in-string range mask is itself conditional: `next` branches on the quote mask, and a quoteless block bypasses the multiply entirely, inheriting `prevInString` directly via `finishNextNoInString` (the NEON variant names the multiplying branch `finishNextInString`; on AVX it is `finishNext`). Since long stretches of numeric or minified structural data contain no quotes at all, entire regions of such documents never touch the carry-less multiplier.

Whitespace and operator classification use the same `shuffle`-against-lookup-table trick simdjson pioneered, expressed as `ws_collector` and `op_collector` over the block's register array. On NEON, where `movemask` doesn't exist, the collectors use the `vshrn_n_u16`-based 4-bit-per-lane narrowing (with `tzcnt >> 2` index correction in `postCmpTzcnt`) and, in the tuned NEON `op_collector`, a `vqtbl1q_u8` nibble-shuffle keyed on `(byte + 3) >> 4`; the reverse-bits/RBIT strategy is available as a compile-time switch where it profiles faster.

### 4.3 Pseudo-structural promotion

Scalar starts are promoted to structurals exactly as in the original algorithm (`followsNonquoteScalar` carries the cross-block bit), so the tape marks the first byte of every number, `true`/`false`/`null`, and string, giving stage 2 direct seek points to every value:

```cpp
JSONIFIER_INLINE uint64_t getStructurals(const simd_array_t in01, const jsonifier_simd_int_t opTable, const jsonifier_simd_int_t spaceMask,
	const jsonifier_simd_int_t whitespaceTableLocal) noexcept {
	const uint64_t whitespace  = simd::ws_collector::impl(in01, whitespaceTableLocal);
	const uint64_t op		   = simd::op_collector::impl(in01, opTable, spaceMask);
	const uint64_t scalar	   = ~(op | whitespace | simd::rope_detector<rope_block>::quotes);
	const uint64_t follows	   = simd::rope_detector<rope_block>::followsNonquoteScalar(scalar);
	const uint64_t scalarStart = scalar & ~follows;
	return op | simd::rope_detector<rope_block>::quotes | scalarStart;
}
```

Note the overload pair: the minified variant omits the whitespace collector entirely: an entire classification lane deleted at compile time when the caller declares the input minified. This is the same philosophy as the compiler-specific step constants: every fact known before runtime is burned into the instruction stream.

### 4.4 The drain architecture: bits → indices

Extraction, converting each 64-bit structural mask into byte offsets on the tape, is where naive implementations serialize hard, because the classic loop (`tzcnt`, store, `blsr`, repeat) is a loop-carried dependency chain of length popcount.

Jsonifier's answer differs by ISA.

**AVX-512: fully vectorized drain.** With `VBMI2` available, the bitmask never enters a scalar loop at all. `_mm512_maskz_compress_epi8` compresses a constant 0..63 byte ramp under the structural mask, producing the set-bit positions as packed bytes; four `cvtepu8_epi32` widenings plus a broadcast base-add stream up to 64 indices to the tape in a handful of instructions:

```cpp
const __m512i indexes			= _mm512_maskz_compress_epi8(bits,
			  _mm512_set_epi32(0x3f3e3d3c, 0x3b3a3938, 0x37363534, 0x33323130, 0x2f2e2d2c, 0x2b2a2928, 0x27262524, 0x23222120, 0x1f1e1d1c, 0x1b1a1918, 0x17161514, 0x13121110,
				  0x0f0e0d0c, 0x0b0a0908, 0x07060504, 0x03020100));
const __m512i startIndexLocal = _mm512_set1_epi32(base);
__m512i t0					  = _mm512_cvtepu8_epi32(_mm512_castsi512_si128(indexes));
_mm512_storeu_si512(tape, _mm512_add_epi32(t0, startIndexLocal));
```

The widening cascade is guarded by the lane's precomputed popcount (`count > 16`, `> 32`, `> 48`), so sparse blocks pay for one store, not four. simdjson employs the same family of trick: its generic indexer exposes a `CUSTOM_BIT_INDEXER` hook, and the Icelake kernel fills it with its own `VBMI2` compress-based extractor. On this ISA the two libraries again share the inner mechanism, and the performance separation comes from the surrounding step architecture (§4.1, §4.4 drain scheduling below) rather than the extraction primitive.

**AVX2/AVX/NEON: the folded stepped drain.** Without byte-compress, extraction must use the tzcnt chain, but the chain's *structure* is still a compile-time decision. `write_indices_functor` emits one extract-advance pair per index; `write_indices_stepped_functor` groups them into unconditional bursts of `simdTapeStep` writes:

```cpp
template<auto...> struct write_indices_functor {
	using size_type = uint64_t;

	template<uint64_t index> JSONIFIER_INLINE static void impl(size_type base, size_type& bits, write_structural_index_ptr tape) noexcept {
		tape[static_cast<uint64_t>(tag<index>{})] = simd::tape_writer_op::extractIndex(base, bits);
		bits									  = simd::tape_writer_op::advance(bits);
	}
};

template<uint64_t step> struct write_indices_stepped_functor {
	using size_type = uint64_t;
	template<uint64_t index> JSONIFIER_INLINE static bool impl(size_type base, size_type& bits, write_structural_index_ptr tape, uint64_t cnt) noexcept {
		if constexpr (index > 0) {
			if ((index < cnt)) [[unlikely]] {
				functor_runner<write_indices_functor, make_integer_sequence<step>>::impl(base, bits, tape + index);
				return true;
			} else {
				return false;
			}
		} else {
			functor_runner<write_indices_functor, make_integer_sequence<step>>::impl(base, bits, tape + index);
			return true;
		}
	}
};
```

The `functor_runner`'s `implAnd` expands a stepped range sequence `<0, 64, simdTapeStep>` through an `&&`-fold: each group writes `simdTapeStep` indices *unconditionally* (over-writing garbage past the true count is harmless, since the tape cursor only advances by the real popcount), and the fold short-circuits the moment a group's start index reaches the count. One predictable branch per `simdTapeStep` extractions instead of one per extraction.

Credit where due: simdjson's current generic `bit_indexer` implements the same stepped-burst pattern (`write_indexes_stepped<START, END, STEP>` via recursive template expansion, unconditional bursts, `simdjson_unlikely` short-circuit checks at each group boundary), with the burst size exposed as a build macro (`SIMDJSON_STRUCTURAL_INDEXER_STEP`, default 4). The two libraries have converged on the extraction inner loop itself. The divergences are in everything around it:

- **Coverage.** simdjson's stepped expansion runs to a fixed `STEP_UNTIL` of 24 set bits and falls back to a plain scalar loop for denser blocks; Jsonifier's fold covers the full 0..64 range at the same stride.
- **Tuning axis.** simdjson's STEP is one global knob; Jsonifier's `simdTapeStep` is swept and pinned per (ISA × compiler) cell, jointly with `simdBlocksPerStep`, because the two constants interact. AVX2 uses a burst of 4 on all three compilers, with 4 blocks per step on Clang and 8 on GCC and MSVC; NEON uses a burst of 8 on both Clang and GCC, at 4 blocks per step (§4.1).
- **Drain scheduling.** This is the structural difference. simdjson pipelines at a depth of one block: each `next()` call drains the *previous* block's structurals while the current block is being classified. Jsonifier defers draining for an entire step: up to eight masks and their popcounts are materialized in `bitsArr`/`cntsArr` before `add_tape_values` drains them back-to-back, each lane's tape destination precomputed from the popcount prefix. Eight independent tzcnt chains with no interleaved classification dependencies, handed to the out-of-order core as one batch.

Above the per-lane drain sits the same fold pattern at block scope. `add_tape_values` drains all blocks of a step, threading the running tape offset through a fold over the lane indices:

```cpp
JSONIFIER_INLINE static void impl(const array<uint64_t, blocksPerStep>& bitsArr, const array<uint64_t, blocksPerStep>& cnts, write_structural_index_ptr tape,
	size_type strIdx) noexcept {
	uint64_t offset = 0;
	(((drainLane<indices>(bitsArr, cnts, tape + offset, strIdx)), offset += cnts[tag<indices>{}]), ...);
}
```

Because per-block popcounts were captured during classification (`cntsArr[I]`), the drains of successive blocks have no data dependence on each other's tzcnt chains; each lane knows its destination offset up front. The classification of blocks *N+1..7* and the drain of block *N* are independent instruction streams the scheduler is free to interleave.

## 5. Stage 2: the tape-driven iterator

Stage 2 is not a separate parser; it is a *specialization* of the same `json_cursor` interface over `write_structural_index_ptr` instead of `read_buffer_ptr`. The reflection-driven parse machinery (`parse_impl`, the dispatch tables, the anti-hash learning) is written once against the cursor interface: `cursor_t<options, context_type>` picks the specialization from the context's `iterator_type`, and the `json_cursor<parseOpts, write_structural_index_ptr>` specialization supplies the token-navigation primitives:

```cpp
template<typename context_type> JSONIFIER_INLINE static bool skipValue(write_structural_index_ptr& iter, context_type& context) noexcept {
	if (iter >= context.endIter) [[unlikely]] {
		return reject<parse_statuses::unexpected_end_of_input>(iter, context);
	}
	const char first = static_cast<char>(*valuePtr(iter, context));
	if (first == '{' || first == '[') {
		int64_t depth{};
		while (iter < context.endIter) {
			depth += nestingDeltaTable[static_cast<uint64_t>(context.stringRoot[*iter])];
			++iter;
			if (depth == 0) {
				return true;
			}
		}
		return reject<parse_statuses::unexpected_string_end>(iter, context);
	}
	++iter;
	return true;
}
```

`nestingDeltaTable` maps `{` and `[` to +1, `}` and `]` to −1 and every other byte to 0, so the loop needs one table load and one add per tape entry and a single branch on the running depth.

This is the payoff that justifies the tape for partial reading: skipping an unwanted value is `++iter`. Skipping an entire unwanted subtree touches only its structural characters, one indexed byte load per structural entry, and never the bytes in between. `skipString` is a single increment, because stage 1 already resolved every escape sequence's effect on string extent. In the raw-pointer cursor, by contrast, skipping a string means re-scanning it for an unescaped closing quote (`skipStringImpl`'s memchr-and-check-backslash-parity loop), and skipping a container means walking every byte.

The asymmetry defines the routing rule. When the caller wants *every* value (full-document parse into a reflected type), skips are rare and the tape is overhead. When the caller wants a *few* values from a large document (partial reading), or wants only the structure itself (prettify/minify, where the transformation is literally "copy bytes, adjusting whitespace at structural positions"), skips dominate and the tape converts O(bytes) navigation into O(structurals).

The same routing logic appears in stage 1's own entry point: `reset<minified>` selects between the whitespace-aware and whitespace-free classification pipelines, and the tail-block handling pads with `0x20`, indexes the pad, then retroactively pops any tape entries pointing past the true document length: branchless main loop, exact tape.

## 6. Distributed UTF-8 validation

Let's be precise about what simdjson does, because it is also an in-register scheme: `json_structural_indexer::next` feeds every 64-byte block into `utf8_checker::check_next_input` using the very registers the classifier just loaded. Validation is fused into stage 1, costs no extra pass over memory, and covers the entire input. (The standalone `generic_validate_utf8` exists too, for the buffer-validation API.) On Demand then additionally defers some string-level checks to traversal.

So the distinction is not "in-register versus dedicated pass"; both libraries validate in registers already in flight. The distinction is **scope and location**:

- **simdjson** validates *all input bytes*, in *stage 1*. Elegant and total, but it means the range checker runs over structural characters, whitespace, numbers, and literals, bytes that a JSON parser's own grammar already constrains to ASCII. For a document that is 70% non-string content, 70% of the validation work confirms what token matching would prove for free.
- **Jsonifier** validates *string bytes only*, in the *string parse loop*. Everything outside strings is implicitly ASCII-constrained: structural characters are matched literally, numbers pass through a digit-table parser, `true`/`false`/`null` are compared as packed integer constants; any non-ASCII byte in those positions is a parse error by construction, no range checker required. The only place arbitrary bytes can legally appear is inside strings, and that is exactly, and only, where `utf8_register_validator` runs.

One accounting qualifier belongs in the main text rather than a footnote, because it bounds the size of the claim: simdjson's checker leads with an ASCII fast path: a block whose registers OR to an ASCII-only result skips the multibyte carry chain entirely and pays roughly one reduction and one test. For ASCII-heavy input, then, "validation work proportional to input bytes" is proportional with a small constant, not with the full cost of the multibyte machinery. The scope distinction survives the qualifier (Jsonifier runs *no* validation instructions of any kind over non-string bytes, fast-path or otherwise, and the two approaches still separate cleanly on documents dense in non-ASCII string content), but the honest comparison states the fast path up front rather than burying it.

Jsonifier ships the same range-based algorithm simdjson uses (the `byte1High`/`byte1Low`/`byte2High` nibble-lookup classifier with the `carry`/`tooShort`/`tooLong`/`surrogate` error-bit algebra) in two deployments. The first, the standalone `validateUtf8` built on `utf8_checker`, is the conventional block validator, with the same ASCII fast path (`orAll` the block's registers, one test, skip the multibyte machinery). The second is the distributed one:

```cpp
if (delimiters == static_cast<integer_type>(0)) {
	validator.checkRegister(simdValue);
	string1Start += bytesProcessed;
	string2 += bytesProcessed;
	continue;
}
```

The validator carries `prevInput`/`incompleteRegister` across registers exactly as the block checker carries them across blocks, and `checkPartial` handles the sub-register tail at each quote or backslash boundary by padding with `0x20` (an innocuous ASCII byte), so multi-byte sequences spanning register boundaries are still caught, and a string ending mid-sequence trips the incomplete carry. Because the string parse loop cascades through progressively narrower SIMD widths, the carry additionally survives width transitions through a compact three-byte `utf8_validation_state` flushed at each width's loop exit and reseeded into the next width's validator. The position-dependent thresholds (last byte ≥ 0xC0, second-to-last ≥ 0xE0, third-to-last ≥ 0xF0) reconstruct exactly the incomplete carry the register-resident validator would have held. The stage-1 companion paper covers this mechanism in full [§6.1 there].

The consequence differs by path. On simdjson's architecture, validation work is proportional to *input bytes*: every block, string or not, at minimum passes through the checker's ASCII test, and any block containing non-ASCII runs the full multibyte carry chain. On Jsonifier's, validation work is proportional to *string bytes*, and on the single-pass full-document path it rides registers the unescaper already loaded, so it never adds a memory pass and never touches non-string content at all. A document that is mostly numbers, structure, and whitespace validates nearly for free; the grammar itself is the validator for everything the range checker skips. There is also a timing difference: simdjson's stage-1 fusion produces its verdict at `check_eof`, after the whole input is scanned; Jsonifier's distributed scheme delivers a strict per-string verdict at each string's closing quote, so an invalid byte fails the parse at the value that contains it. It is always on: there is no option to disable it, because its cost is already confined to string bytes the parser was loading anyway.

## 7. Where the two libraries stand

| Dimension | simdjson | Jsonifier |
|---|---|---|
| Stage-1 usage | Unconditional, all documents | Partial reading, prettify, minify only (benchmarks may force it for comparability; see §2.1) |
| Full-document parse | Stage 1 + On Demand traversal | Single fused pass, schema-directed; faster than its own two-stage path on POD data, on 95 of 100 minified documents and on 30 of 40 prettified documents on the M1; slower on 56 of 60 prettified x86 documents (see §2.2) |
| Target of stage 2 | DOM / lazy generic values | Reflected concrete types via shared cursor interface |
| Out-of-order keys | Forward scan + wrap-around rescan per lookup, per object; no memory across documents | One hash fallback, then learned per-slot order correction (thread-local, per stream) |
| Schema knowledge (known-type workloads) | Exists in caller's traversal code, invisible to the library | Declared once via reflection, consumed by the architecture |
| Step size | 128 bytes (two 64-byte blocks), pipelined one block deep | 256–512 bytes, per-(ISA × compiler) constant |
| Bit extraction | Stepped tzcnt bursts (global STEP macro, scalar tail past 24 bits); VBMI2 compress on Icelake | Same primitives, but full-range folded bursts with per-toolchain-swept stride |
| Drain scheduling | Previous block drained during current block's classification | Whole-step deferred batch drain, lane offsets precomputed from popcounts |
| Escape short-circuit | Yes (zero-backslash fast exit) | Yes (same) |
| In-string prefix-XOR | Computed every block | Skipped for quoteless blocks |
| UTF-8 validation | In-register, fused into stage 1, over all input bytes (ASCII fast path per block); verdict at EOF | In-register, fused into string parsing, over string bytes only, carried across the width cascade; verdict per string |
| Specialization axis | Runtime CPU dispatch through virtual functions (core/DOM API); On Demand mostly fixed at compile time | One ISA tier per build, chosen at configure time; no virtual calls and no runtime dispatch, so everything is compile-time (Cathedral Architecture) |

The last row is the root of every other difference. simdjson's core API can ship one x86 binary that picks its instruction set at run time; Jsonifier deliberately does not. Its founding constraint is that apart from the data, nothing is decided at run time. The schema, the compiler, the parse options and the instruction-set tier are all known at build time, so every routing decision in this paper (tape or no tape, whitespace lane or not, validation or not, 4 blocks or 8, burst of 1 or 4) is still resolved before the first byte is read.

Jsonifier compiles exactly one instruction-set tier per build, and the tier is chosen when CMake configures the project. At configure time, voided-hw-detection builds and runs a small detector program on the build machine. On x86 it reads CPUID leaves 1, 7 and 0x80000001 and checks XCR0 with `xgetbv`, so AVX, AVX2 and AVX-512 count only when the operating system saves their registers; AVX-512 additionally needs F, BW and VBMI2 together. On ARM it reads `HWCAP` on Linux and `sysctl` on macOS. The detector's output is cached in the build directory, so it runs once per configure, not once per build. CMake keeps the highest detected tier (AVX-512, then AVX2, then AVX on x86; SVE2 when its vector length is known, otherwise NEON on ARM), writes the selected feature bits into `simd/jsonifier_cpu_instructions.hpp`, and attaches the matching compiler flags to every translation unit that links Jsonifier. Setting `JSONIFIER_CPU_INSTRUCTIONS` at configure time replaces the detected bits, which is how cross-compiled builds pick their target; without it, a cross-compile skips detection and gets the scalar fallback. On x64, defining `JSONIFIER_MAX_AVX_TIER` caps the tier below what was detected, and `core/defines.hpp` refuses to compile if the translation unit's flags do not enable the configured tier. The whole library is then compiled once for that tier. `jsonifier_core` is a single class that inherits its parser, serializer, validator, minifier, prettifier and generic iterator through CRTP, so every call is a direct, inlinable call into code built for that ISA. There is no CPUID read, no cached backend value and no per-call check.

simdjson takes the opposite position for its core and DOM API. It compiles one kernel per ISA and dispatches through virtual functions: `implementation` exposes `create_dom_parser_implementation`, `minify` and `validate_utf8` as virtuals, and `dom_parser_implementation` declares `parse`, `stage1`, `stage2` and `parse_string` pure virtual. Its On Demand front end, the closer comparison for Jsonifier, is mostly fixed at compile time. Stage 1 and string unescaping are still dispatched at run time, but the On Demand code itself is compiled for the builtin implementation that the compiler flags select, and its documentation notes that the On Demand API "has limited runtime dispatch support" and recommends compiling for a specific x64 target.

The reason for one tier per build is inlining. Compiling several tiers into one binary needs per-function target attributes (`#pragma GCC target`, `#pragma clang attribute`), and those apply only to functions defined inside the region. A force-inlined, tier-specific callable passed to a standard-library template, such as the contexts Jsonifier hands to `std::basic_string::resize_and_overwrite`, is then called from code compiled at the baseline, and GCC and Clang reject that. Keeping `resize_and_overwrite` and the force-inlining is worth more than shipping several tiers in one binary, and a single tier per build keeps both. The cost is portability: a binary needs its configured tier on every machine that runs it, so a binary for mixed hardware is configured for the lowest tier it must support.

ARM builds compile one backend as well (NEON, or SVE2 when configured), as simdjson's ARM builds do.

## 8. Conclusion

The two-stage model is a genuinely great algorithm. Jsonifier's stage 1 is an unapologetic descendant of Langdale and Lemire's design, and credits it in source. The contribution here is architectural discipline about *when* to run it. A structural tape is an index, and indexes are worth building exactly when you will not read the whole book. Jsonifier builds it for partial reads and structural transforms, skips it for full parses, validates UTF-8 in the registers it was already holding, and lets the compiler specialize every remaining decision down to per-toolchain loop geometry. §2 shows what skipping the tape is worth, with both paths compiled into one binary and ranked head to head, on both freshly allocated and reused target objects. Across 225 freshly allocated tests on five platforms the fused path wins 153, ties 2 and loses 70, and where it loses is a matter of layout and architecture more than of principle. It wins all 25 POD-type tests, by up to 7.7× (Bool on Linux/GCC), 95 of the 100 minified documents, and 30 of the 40 prettified documents on the M1. The two-stage path wins 56 of the 60 prettified documents on x86, including all 20 on Linux/GCC. The tape, in other words, is not only an index for partial reads: on indented input on x86 it is also the faster way to read the whole book, and the router should learn when. Taking whichever path is faster, Jsonifier beats simdjson On Demand on 222 of those 225 tests, its own stage-1 + reflection path beats simdjson's stage-1 + reflection reader on 61 of 80, and on the partial-reading test, where the index is the right tool, it beats simdjson On Demand on all 20 full-size results and on 35 of the 40 overall, with every tie and loss on the Small copy. The benchmarks are the receipts.

---

*Jsonifier is MIT-licensed and available at github.com/nihilai-collective/Jsonifier. Benchmark methodology and full sweep data: github.com/nihilai-collective/Json-Performance.*