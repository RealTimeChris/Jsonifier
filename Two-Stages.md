# Two Stages, On Demand: The Stage-1 + Stage-2 Architecture in Jsonifier

**Nihilai Collective Corp: Engineering Papers**  
*Nihilai Collective Corp*  
*Jsonifier, October 2026*  

---

## Abstract

Since Langdale and Lemire's 2019 paper *Parsing Gigabytes of JSON per Second*, the two-stage SIMD parsing model, a vectorized structural-indexing pass (stage 1) followed by a tape-driven materialization pass (stage 2), has been treated as the canonical architecture for high-performance JSON processing. simdjson, the reference implementation, routes every document through stage 1 unconditionally.

Jsonifier takes a different position: **two-stage parsing is a specialized tool, not a mandatory front door.** When parsing a full document into concrete, reflection-registered C++ types, every structural fact the tape would record is rediscovered anyway during value materialization, so building the tape means touching every byte twice for information used once. Jsonifier therefore parses full documents in a single fused pass, and reserves its stage-1 + stage-2 machinery for the workloads where a structural index genuinely pays for itself: partial/unordered reading, prettifying, and minifying. Measured with both paths compiled into one binary across 225 tests on five builds, skipping the tape wins on POD-type data on every build and on 98 of the 100 minified documents, the tape is faster on 56 of the 60 prettified documents on x86, and on the Apple M1 the fused path wins 31 of the 40 prettified documents.

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

For full-document parsing into known types, the tape is overhead in principle, and in practice whether skipping it is faster comes down to two questions: is the input indented, and is the machine x86? §2.1 through §2.4 measure both paths in one binary on five platform/compiler builds. In the freshly allocated run, the fused path wins all 25 POD-type tests and 98 of the 100 minified documents (full-size and Small) across all five builds; the tape takes Canada and Mesh on Linux/GCC. The two-stage path wins 56 of the 60 prettified documents on x86, and on the M1 the fused path wins 31 of the 40 prettified documents, the tape wins 5 and 4 are ties.

**The routing rule is simple: the two-stage machinery is engaged for partial reading, prettifying, and minifying: workloads where the caller does *not* want every value, or wants pure structural transformation. Full-document parsing takes the single-pass path.** §2.4 shows where the measurements say that rule should be refined.

One anticipated objection deserves preemption here: that Jsonifier's requirement of ahead-of-time registration (`jsonifier::core<T>`) concedes generality that simdjson retains, since simdjson parses arbitrary documents with no such declaration. For truly dynamic workloads (schemas unknown until runtime, exploratory traversal, structural transformation of unknown documents), this is correct, and simdjson's DOM and On Demand models are the appropriate tools; Jsonifier's registration model simply does not address that problem. But for the workload this paper concerns, parsing documents into concrete types the caller has defined, the objection dissolves on inspection, because the schema knowledge exists at compile time in both programs. A simdjson caller materializing a struct writes the schema into their source as a sequence of field accesses in a fixed order chosen at authoring time; that traversal code is a schema declaration in imperative clothing. The difference is not the presence of compile-time knowledge but its legibility to the library: expressed as hand-written traversal, the knowledge is opaque: simdjson cannot fuse key literals from it, cannot learn permuted orders through it, and cannot skip building the index it implies is unnecessary. Expressed as a reflection registration, the identical knowledge becomes architecture: fused member headers, adaptive order recovery, and the routing rule above. The comparison between the two libraries on known-type workloads is therefore not "declared schema versus no schema"; it is the same schema, declared once where the compiler can consume it versus restated per call site where it cannot.

### 2.1 Method: both paths in one binary

Every comparison in this section comes from the [Json-Performance](https://github.com/nihilai-collective/Json-Performance) sweep published October 22, 2026, with Jsonifier [da96147](https://github.com/nihilai-collective/jsonifier/commit/da96147), Glaze [628bec1](https://github.com/stephenberry/glaze/commit/628bec1), simdjson [70f83c9](https://github.com/simdjson/simdjson/commit/70f83c9) and BenchmarkSuite [ab75024](https://github.com/nihilai-collective/benchmarksuite/commit/ab75024) on every build. The harness registers both Jsonifier paths in the same binary. "jsonifier" is the default fused single-pass path, which the harness calls scalar structural iteration. "jsonifier (two-stage)" makes exactly the same `parseJson` call with `partialRead` set, which routes it through stage 1 and then stage 2. "simdjson (ondemand)" is the reference for §2.2 to §2.4. Glaze runs in the same sweep and appears here only in the validation test of §2.7, and `simdjson (reflection)`, simdjson 5's C++26 static-reflection reader, gets its own comparison in §2.5. Because the two Jsonifier paths are compiled together, run back to back on each test, and ranked against each other by the same statistics, the only thing that differs between them is the path.

All libraries parse fully into the target data structures and perform UTF-8 validation. Every test runs twice. In the freshly allocated run, every iteration constructs a new object to parse into and destroys it again inside the timed region, so allocation and deallocation are part of each measurement. In the reused run, labelled "(Reused)" in the sweep, the object is created once and held across iterations; it is cleared, keeping its capacity, outside the timed region before each iteration, so only the parse itself is measured. Parser instances are reused in both. §2.2, §2.3, §2.5 and §2.6 give both runs; the discussion in §2.4 quotes the freshly allocated run unless it says otherwise. Each build runs 45 full-parse tests: five POD-type tests (arrays of a single value type: Bool, Double, Int64, String, Uint64), ten corpus documents in minified and prettified form, and "Small" cut-down copies of the same ten documents (at most 5 KiB minified) in both forms, which weigh per-call overhead more heavily than the full-size documents. Two of the ten documents, "Marine IK Reverse" and its Small copy, request every key in the reverse of its document order; in every other test, all libraries receive keys in document order. The sweep also runs a partial-reading test on the Twitter document and its Small copy, which §2.6 covers, and a validation-only test, which §2.7 covers.

Sampling is adaptive. Iterations start at 100 and double each epoch, and sampling does not stop early: epochs continue until 5 seconds have elapsed or the iteration cap of 100,000 is reached. Every epoch after the first is scored by its relative standard error plus its epoch-over-epoch mean shift, and the lowest-scoring epoch is kept as the result. A result counts as converged only if that epoch has RSE below 5% and mean shift below 2.5% on the three x86 builds (10% and 5% on the virtualized M1), and non-converged results stay in the rankings and tallies, marked n/c in the tables. Fifteen full-parse test rows in this sweep have at least one of the three compared libraries not converged: six freshly allocated and nine reused. One is on Windows/MSVC (simdjson on minified Marine IK Small, reused) and the rest are on the two macOS builds, twelve of them on macOS/Clang. Each one is marked n/c where it appears. Ties are declared by Welch's t-test on the kept epoch (two-sided, p < 0.05), and every verdict in this paper is a pairwise test between the two libraries or paths named in its column. Two properties of this rule matter for reading the results. Keeping the quietest epoch favors the least-disturbed stretch of each run, which raises absolute throughput somewhat, but it is applied identically to every library. And because the kept epoch has the smallest variance available, small differences are more often resolved as wins or losses than they would be under a first-to-converge rule: this sweep produced five ties between the two Jsonifier paths across 225 freshly allocated tests and three across 225 reused ones.

### 2.2 Fused against two-stage

Ranked head to head, the fused path wins 157 tests, 5 are statistical ties, and the two-stage path wins 63. The split is a matter of platform and indentation:

| Platform / Compiler | Fused faster | Tie | Two-stage faster | Tests converged |
|---|---|---|---|---|
| Windows / MSVC 19.44 (i9-14900KF, AVX2) | 25 | 0 | 20 | 45 of 45 |
| Linux / Clang 24.0 (i9-14900KF, AVX2) | 28 | 0 | 17 | 45 of 45 |
| Linux / GCC 16.2 (i9-14900KF, AVX2) | 23 | 1 | 21 | 45 of 45 |
| macOS / GCC 16.2 (Apple M1, NEON) | 41 | 1 | 3 | 44 of 45 |
| macOS / Clang 23.1 (Apple M1, NEON) | 40 | 3 | 2 | 40 of 45 |
| **Aggregate** | **157** | **5** | **63** | **219 of 225** |

Split by input shape (fused / tie / two-stage):

| Input shape | Windows / MSVC | Linux (Clang + GCC) | macOS (GCC + Clang) |
|---|---|---|---|
| POD-type tests | 5 / 0 / 0 | 10 / 0 / 0 | 10 / 0 / 0 |
| Minified corpus documents | 10 / 0 / 0 | 18 / 0 / 2 | 20 / 0 / 0 |
| Prettified corpus documents | 0 / 0 / 10 | 3 / 1 / 16 | 17 / 2 / 1 |
| Small minified documents | 10 / 0 / 0 | 20 / 0 / 0 | 20 / 0 / 0 |
| Small prettified documents | 0 / 0 / 10 | 0 / 0 / 20 | 14 / 2 / 4 |

Against simdjson, counting a test for Jsonifier by the verdict of whichever of its two paths has the higher throughput against simdjson On Demand:

| Platform / Compiler | Best Jsonifier path (W / T / L) | Fused path alone (W / T / L) |
|---|---|---|
| Windows / MSVC 19.44 (i9-14900KF, AVX2) | 45 / 0 / 0 | 45 / 0 / 0 |
| Linux / Clang 24.0 (i9-14900KF, AVX2) | 44 / 0 / 1 | 36 / 0 / 9 |
| Linux / GCC 16.2 (i9-14900KF, AVX2) | 43 / 0 / 2 | 42 / 1 / 2 |
| macOS / GCC 16.2 (Apple M1, NEON) | 45 / 0 / 0 | 44 / 0 / 1 |
| macOS / Clang 23.1 (Apple M1, NEON) | 45 / 0 / 0 | 44 / 1 / 0 |
| **Aggregate** | **222 / 0 / 3** | **211 / 2 / 12** |

**Reused objects.** With the target object held across iterations:

| Platform / Compiler | Fused faster | Tie | Two-stage faster | Tests converged |
|---|---|---|---|---|
| Windows / MSVC 19.44 (i9-14900KF, AVX2) | 25 | 0 | 20 | 44 of 45 |
| Linux / Clang 24.0 (i9-14900KF, AVX2) | 28 | 0 | 17 | 45 of 45 |
| Linux / GCC 16.2 (i9-14900KF, AVX2) | 24 | 0 | 21 | 45 of 45 |
| macOS / GCC 16.2 (Apple M1, NEON) | 41 | 2 | 2 | 44 of 45 |
| macOS / Clang 23.1 (Apple M1, NEON) | 40 | 1 | 4 | 38 of 45 |
| **Aggregate** | **158** | **3** | **64** | **216 of 225** |

Split by input shape (fused / tie / two-stage):

| Input shape | Windows / MSVC | Linux (Clang + GCC) | macOS (GCC + Clang) |
|---|---|---|---|
| POD-type tests | 5 / 0 / 0 | 10 / 0 / 0 | 10 / 0 / 0 |
| Minified corpus documents | 10 / 0 / 0 | 18 / 0 / 2 | 20 / 0 / 0 |
| Prettified corpus documents | 0 / 0 / 10 | 4 / 0 / 16 | 16 / 1 / 3 |
| Small minified documents | 10 / 0 / 0 | 20 / 0 / 0 | 20 / 0 / 0 |
| Small prettified documents | 0 / 0 / 10 | 0 / 0 / 20 | 15 / 2 / 3 |

Against simdjson, counting a test for Jsonifier by the verdict of whichever of its two paths has the higher throughput against simdjson On Demand:

| Platform / Compiler | Best Jsonifier path (W / T / L) | Fused path alone (W / T / L) |
|---|---|---|
| Windows / MSVC 19.44 (i9-14900KF, AVX2) | 45 / 0 / 0 | 45 / 0 / 0 |
| Linux / Clang 24.0 (i9-14900KF, AVX2) | 44 / 0 / 1 | 36 / 0 / 9 |
| Linux / GCC 16.2 (i9-14900KF, AVX2) | 42 / 0 / 3 | 42 / 0 / 3 |
| macOS / GCC 16.2 (Apple M1, NEON) | 45 / 0 / 0 | 44 / 0 / 1 |
| macOS / Clang 23.1 (Apple M1, NEON) | 44 / 1 / 0 | 42 / 2 / 1 |
| **Aggregate** | **220 / 1 / 4** | **209 / 2 / 14** |

Across 225 tests the fused path wins 158, ties 3 and loses 64 to the two-stage path, and the x86 split holds: the tape wins 26 of the 30 full-size prettified documents and all 30 Small prettified ones, and the fused path wins all 30 Small minified documents and 28 of the 30 full-size minified ones (the tape takes Canada and Mesh on Linux/GCC). On the M1 the fused path wins all 40 minified documents and 31 of the 40 prettified ones, with 3 ties and 6 tape wins. Against simdjson, the faster Jsonifier path wins 220, ties 1 and loses 4, and the fused path alone wins 209, ties 2 and loses 14.

### 2.3 Per-test results

Throughput is in MB/s. "Faster Jsonifier path" is the Welch's t-test verdict between the two Jsonifier paths, "Best Jsonifier path vs simdjson" is the verdict for whichever of the two has the higher throughput.

#### Freshly allocated objects

**Windows / MSVC 19.44 (i9-14900KF, AVX2)**

| Test | Fused (MB/s) | Two-stage (MB/s) | simdjson (MB/s) | Fused ÷ two-stage | Faster Jsonifier path | Best Jsonifier path vs simdjson |
|---|---|---|---|---|---|---|
| Bool (POD) | 476 | 139 | 130 | 3.44× | Fused | Win |
| Double (POD) | 585 | 316 | 170 | 1.85× | Fused | Win |
| Int64 (POD) | 1,619 | 575 | 379 | 2.82× | Fused | Win |
| String (POD) | 1,001 | 719 | 651 | 1.39× | Fused | Win |
| Uint64 (POD) | 1,688 | 595 | 383 | 2.84× | Fused | Win |
| Canada (minified) | 580 | 548 | 367 | 1.06× | Fused | Win |
| Canada (prettified) | 1,600 | 1,653 | 1,096 | 0.97× | Two-stage | Win |
| CitmCatalog (minified) | 1,874 | 1,311 | 478 | 1.43× | Fused | Win |
| CitmCatalog (prettified) | 2,963 | 3,248 | 1,288 | 0.91× | Two-stage | Win |
| Discord (minified) | 1,152 | 1,062 | 600 | 1.08× | Fused | Win |
| Discord (prettified) | 1,491 | 1,734 | 991 | 0.86× | Two-stage | Win |
| Google Maps Response (minified) | 1,472 | 1,216 | 494 | 1.21× | Fused | Win |
| Google Maps Response (prettified) | 2,359 | 2,827 | 1,231 | 0.83× | Two-stage | Win |
| Instruments (minified) | 2,092 | 1,485 | 704 | 1.41× | Fused | Win |
| Instruments (prettified) | 2,263 | 2,676 | 1,281 | 0.85× | Two-stage | Win |
| Marine IK Reverse (minified) | 502 | 456 | 37 | 1.10× | Fused | Win |
| Marine IK Reverse (prettified) | 2,030 | 2,187 | 195 | 0.93× | Two-stage | Win |
| Marine IK (minified) | 505 | 461 | 259 | 1.09× | Fused | Win |
| Marine IK (prettified) | 2,003 | 2,168 | 1,271 | 0.92× | Two-stage | Win |
| Mesh (minified) | 941 | 807 | 438 | 1.17× | Fused | Win |
| Mesh (prettified) | 1,205 | 1,483 | 832 | 0.81× | Two-stage | Win |
| Random (minified) | 982 | 904 | 526 | 1.09× | Fused | Win |
| Random (prettified) | 1,317 | 1,560 | 949 | 0.84× | Two-stage | Win |
| Twitter (minified) | 1,382 | 1,205 | 789 | 1.15× | Fused | Win |
| Twitter (prettified) | 1,432 | 1,704 | 1,141 | 0.84× | Two-stage | Win |
| Canada Small (minified) | 595 | 563 | 329 | 1.06× | Fused | Win |
| Canada Small (prettified) | 1,528 | 1,657 | 1,012 | 0.92× | Two-stage | Win |
| CitmCatalog Small (minified) | 1,030 | 864 | 493 | 1.19× | Fused | Win |
| CitmCatalog Small (prettified) | 1,584 | 1,843 | 1,084 | 0.86× | Two-stage | Win |
| Discord Small (minified) | 1,111 | 1,002 | 637 | 1.11× | Fused | Win |
| Discord Small (prettified) | 1,293 | 1,412 | 912 | 0.92× | Two-stage | Win |
| Google Maps Response Small (minified) | 1,472 | 1,154 | 534 | 1.28× | Fused | Win |
| Google Maps Response Small (prettified) | 2,359 | 2,703 | 1,291 | 0.87× | Two-stage | Win |
| Instruments Small (minified) | 2,237 | 1,506 | 704 | 1.49× | Fused | Win |
| Instruments Small (prettified) | 2,149 | 2,695 | 1,328 | 0.80× | Two-stage | Win |
| Marine IK Reverse Small (minified) | 451 | 400 | 40 | 1.13× | Fused | Win |
| Marine IK Reverse Small (prettified) | 1,736 | 1,958 | 204 | 0.89× | Two-stage | Win |
| Marine IK Small (minified) | 474 | 412 | 237 | 1.15× | Fused | Win |
| Marine IK Small (prettified) | 1,774 | 2,066 | 1,255 | 0.86× | Two-stage | Win |
| Mesh Small (minified) | 728 | 632 | 289 | 1.15× | Fused | Win |
| Mesh Small (prettified) | 1,091 | 1,275 | 616 | 0.86× | Two-stage | Win |
| Random Small (minified) | 1,164 | 1,006 | 518 | 1.16× | Fused | Win |
| Random Small (prettified) | 1,562 | 1,871 | 986 | 0.83× | Two-stage | Win |
| Twitter Small (minified) | 1,838 | 1,485 | 1,014 | 1.24× | Fused | Win |
| Twitter Small (prettified) | 1,768 | 2,061 | 1,466 | 0.86× | Two-stage | Win |

**Linux / Clang 24.0 (i9-14900KF, AVX2)**

| Test | Fused (MB/s) | Two-stage (MB/s) | simdjson (MB/s) | Fused ÷ two-stage | Faster Jsonifier path | Best Jsonifier path vs simdjson |
|---|---|---|---|---|---|---|
| Bool (POD) | 1,918 | 334 | 232 | 5.74× | Fused | Win |
| Double (POD) | 1,301 | 420 | 267 | 3.10× | Fused | Win |
| Int64 (POD) | 3,179 | 938 | 597 | 3.39× | Fused | Win |
| String (POD) | 1,786 | 1,127 | 1,097 | 1.59× | Fused | Win |
| Uint64 (POD) | 3,366 | 954 | 553 | 3.53× | Fused | Win |
| Canada (minified) | 999 | 895 | 656 | 1.12× | Fused | Win |
| Canada (prettified) | 2,666 | 2,623 | 1,959 | 1.02× | Fused | Win |
| CitmCatalog (minified) | 2,480 | 1,825 | 1,223 | 1.36× | Fused | Win |
| CitmCatalog (prettified) | 3,846 | 4,290 | 3,097 | 0.90× | Two-stage | Win |
| Discord (minified) | 2,231 | 1,789 | 1,410 | 1.25× | Fused | Win |
| Discord (prettified) | 2,204 | 2,831 | 2,296 | 0.78× | Two-stage | Win |
| Google Maps Response (minified) | 2,621 | 1,971 | 1,380 | 1.33× | Fused | Win |
| Google Maps Response (prettified) | 3,727 | 4,363 | 3,247 | 0.85× | Two-stage | Win |
| Instruments (minified) | 2,591 | 1,981 | 1,716 | 1.31× | Fused | Win |
| Instruments (prettified) | 2,816 | 3,397 | 3,157 | 0.83× | Two-stage | Win |
| Marine IK Reverse (minified) | 850 | 748 | 622 | 1.14× | Fused | Win |
| Marine IK Reverse (prettified) | 3,490 | 3,324 | 2,920 | 1.05× | Fused | Win |
| Marine IK (minified) | 862 | 758 | 615 | 1.14× | Fused | Win |
| Marine IK (prettified) | 3,424 | 3,219 | 2,808 | 1.06× | Fused | Win |
| Mesh (minified) | 1,412 | 1,210 | 1,215 | 1.17× | Fused | Win |
| Mesh (prettified) | 1,990 | 2,098 | 2,223 | 0.95× | Two-stage | Loss |
| Random (minified) | 1,688 | 1,430 | 1,097 | 1.18× | Fused | Win |
| Random (prettified) | 2,192 | 2,409 | 1,954 | 0.91× | Two-stage | Win |
| Twitter (minified) | 2,027 | 1,865 | 1,722 | 1.09× | Fused | Win |
| Twitter (prettified) | 1,858 | 2,540 | 2,489 | 0.73× | Two-stage | Win |
| Canada Small (minified) | 1,006 | 935 | 715 | 1.08× | Fused | Win |
| Canada Small (prettified) | 2,499 | 2,689 | 2,041 | 0.93× | Two-stage | Win |
| CitmCatalog Small (minified) | 1,922 | 1,531 | 1,238 | 1.25× | Fused | Win |
| CitmCatalog Small (prettified) | 2,626 | 3,105 | 2,637 | 0.85× | Two-stage | Win |
| Discord Small (minified) | 2,090 | 1,724 | 1,437 | 1.21× | Fused | Win |
| Discord Small (prettified) | 1,953 | 2,493 | 2,133 | 0.78× | Two-stage | Win |
| Google Maps Response Small (minified) | 2,355 | 1,808 | 1,357 | 1.30× | Fused | Win |
| Google Maps Response Small (prettified) | 3,347 | 4,011 | 3,182 | 0.83× | Two-stage | Win |
| Instruments Small (minified) | 2,641 | 1,980 | 1,625 | 1.33× | Fused | Win |
| Instruments Small (prettified) | 2,749 | 3,382 | 2,962 | 0.81× | Two-stage | Win |
| Marine IK Reverse Small (minified) | 806 | 738 | 601 | 1.09× | Fused | Win |
| Marine IK Reverse Small (prettified) | 2,960 | 3,348 | 2,820 | 0.88× | Two-stage | Win |
| Marine IK Small (minified) | 902 | 803 | 597 | 1.12× | Fused | Win |
| Marine IK Small (prettified) | 3,134 | 3,630 | 2,767 | 0.86× | Two-stage | Win |
| Mesh Small (minified) | 991 | 888 | 774 | 1.12× | Fused | Win |
| Mesh Small (prettified) | 1,504 | 1,773 | 1,546 | 0.85× | Two-stage | Win |
| Random Small (minified) | 2,221 | 1,803 | 1,178 | 1.23× | Fused | Win |
| Random Small (prettified) | 2,535 | 3,133 | 2,131 | 0.81× | Two-stage | Win |
| Twitter Small (minified) | 2,628 | 2,315 | 2,188 | 1.14× | Fused | Win |
| Twitter Small (prettified) | 2,205 | 3,242 | 3,057 | 0.68× | Two-stage | Win |

**Linux / GCC 16.2 (i9-14900KF, AVX2)**

| Test | Fused (MB/s) | Two-stage (MB/s) | simdjson (MB/s) | Fused ÷ two-stage | Faster Jsonifier path | Best Jsonifier path vs simdjson |
|---|---|---|---|---|---|---|
| Bool (POD) | 2,196 | 286 | 196 | 7.69× | Fused | Win |
| Double (POD) | 1,213 | 386 | 232 | 3.15× | Fused | Win |
| Int64 (POD) | 3,368 | 835 | 542 | 4.04× | Fused | Win |
| String (POD) | 1,790 | 1,052 | 1,039 | 1.70× | Fused | Win |
| Uint64 (POD) | 3,470 | 867 | 509 | 4.00× | Fused | Win |
| Canada (minified) | 915 | 922 | 781 | 0.99× | Two-stage | Win |
| Canada (prettified) | 2,424 | 2,650 | 2,298 | 0.91× | Two-stage | Win |
| CitmCatalog (minified) | 2,446 | 1,896 | 1,377 | 1.29× | Fused | Win |
| CitmCatalog (prettified) | 4,024 | 4,354 | 3,421 | 0.92× | Two-stage | Win |
| Discord (minified) | 2,331 | 1,829 | 1,441 | 1.27× | Fused | Win |
| Discord (prettified) | 2,438 | 2,828 | 2,282 | 0.86× | Two-stage | Win |
| Google Maps Response (minified) | 2,686 | 1,945 | 1,111 | 1.38× | Fused | Win |
| Google Maps Response (prettified) | 4,263 | 4,268 | 2,638 | 1.00× | Tie | Win |
| Instruments (minified) | 3,358 | 2,342 | 1,385 | 1.43× | Fused | Win |
| Instruments (prettified) | 3,534 | 3,906 | 2,551 | 0.90× | Two-stage | Win |
| Marine IK Reverse (minified) | 768 | 726 | 613 | 1.06× | Fused | Win |
| Marine IK Reverse (prettified) | 3,297 | 3,335 | 2,844 | 0.99× | Two-stage | Win |
| Marine IK (minified) | 772 | 725 | 605 | 1.06× | Fused | Win |
| Marine IK (prettified) | 3,263 | 3,339 | 2,816 | 0.98× | Two-stage | Win |
| Mesh (minified) | 1,089 | 1,159 | 1,211 | 0.94× | Two-stage | Loss |
| Mesh (prettified) | 1,799 | 2,094 | 2,264 | 0.86× | Two-stage | Loss |
| Random (minified) | 1,476 | 1,414 | 855 | 1.04× | Fused | Win |
| Random (prettified) | 2,058 | 2,417 | 1,563 | 0.85× | Two-stage | Win |
| Twitter (minified) | 2,047 | 1,831 | 1,328 | 1.12× | Fused | Win |
| Twitter (prettified) | 2,116 | 2,597 | 1,938 | 0.81× | Two-stage | Win |
| Canada Small (minified) | 1,033 | 944 | 775 | 1.09× | Fused | Win |
| Canada Small (prettified) | 2,481 | 2,680 | 2,244 | 0.93× | Two-stage | Win |
| CitmCatalog Small (minified) | 1,959 | 1,654 | 1,215 | 1.18× | Fused | Win |
| CitmCatalog Small (prettified) | 2,807 | 3,303 | 2,538 | 0.85× | Two-stage | Win |
| Discord Small (minified) | 1,965 | 1,641 | 1,335 | 1.20× | Fused | Win |
| Discord Small (prettified) | 2,127 | 2,386 | 1,943 | 0.89× | Two-stage | Win |
| Google Maps Response Small (minified) | 2,392 | 1,808 | 1,116 | 1.32× | Fused | Win |
| Google Maps Response Small (prettified) | 3,801 | 4,016 | 2,692 | 0.95× | Two-stage | Win |
| Instruments Small (minified) | 3,361 | 2,251 | 1,265 | 1.49× | Fused | Win |
| Instruments Small (prettified) | 3,414 | 3,806 | 2,356 | 0.90× | Two-stage | Win |
| Marine IK Reverse Small (minified) | 833 | 729 | 569 | 1.14× | Fused | Win |
| Marine IK Reverse Small (prettified) | 3,074 | 3,314 | 2,691 | 0.93× | Two-stage | Win |
| Marine IK Small (minified) | 920 | 764 | 565 | 1.20× | Fused | Win |
| Marine IK Small (prettified) | 3,241 | 3,465 | 2,666 | 0.94× | Two-stage | Win |
| Mesh Small (minified) | 1,016 | 908 | 740 | 1.12× | Fused | Win |
| Mesh Small (prettified) | 1,534 | 1,737 | 1,529 | 0.88× | Two-stage | Win |
| Random Small (minified) | 2,182 | 1,745 | 913 | 1.25× | Fused | Win |
| Random Small (prettified) | 2,732 | 3,022 | 1,668 | 0.90× | Two-stage | Win |
| Twitter Small (minified) | 2,955 | 2,347 | 1,643 | 1.26× | Fused | Win |
| Twitter Small (prettified) | 2,690 | 3,206 | 2,283 | 0.84× | Two-stage | Win |

**macOS / GCC 16.2 (Apple M1, NEON)**

| Test | Fused (MB/s) | Two-stage (MB/s) | simdjson (MB/s) | Fused ÷ two-stage | Faster Jsonifier path | Best Jsonifier path vs simdjson |
|---|---|---|---|---|---|---|
| Bool (POD) | 1,599 | 228 | 193 | 7.01× | Fused | Win |
| Double (POD) | 883 | 295 | 141 | 2.99× | Fused | Win |
| Int64 (POD) | 2,300 | 636 | 469 | 3.62× | Fused | Win |
| String (POD) | 1,221 | 608 | 784 | 2.01× | Fused | Win |
| Uint64 (POD) | 2,214 | 623 | 450 | 3.55× | Fused | Win |
| Canada (minified) | 623 | 589 | 474 | 1.06× | Fused | Win |
| Canada (prettified) | 1,668 | 1,518 (n/c) | 1,169 | 1.10× | Fused | Win |
| CitmCatalog (minified) | 1,872 | 1,261 | 1,016 | 1.48× | Fused | Win |
| CitmCatalog (prettified) | 3,231 | 2,598 | 2,306 | 1.24× | Fused | Win |
| Discord (minified) | 1,684 | 1,101 | 908 | 1.53× | Fused | Win |
| Discord (prettified) | 1,959 | 1,706 | 1,381 | 1.15× | Fused | Win |
| Google Maps Response (minified) | 1,762 | 1,133 | 699 | 1.56× | Fused | Win |
| Google Maps Response (prettified) | 2,896 | 2,134 | 1,594 | 1.36× | Fused | Win |
| Instruments (minified) | 2,549 | 1,451 | 1,112 | 1.76× | Fused | Win |
| Instruments (prettified) | 2,999 | 2,353 | 1,930 | 1.27× | Fused | Win |
| Marine IK Reverse (minified) | 626 | 516 | 437 | 1.21× | Fused | Win |
| Marine IK Reverse (prettified) | 2,232 | 2,083 | 1,798 | 1.07× | Fused | Win |
| Marine IK (minified) | 618 | 495 | 441 | 1.25× | Fused | Win |
| Marine IK (prettified) | 2,459 | 2,185 | 1,887 | 1.13× | Fused | Win |
| Mesh (minified) | 1,002 | 875 | 782 | 1.15× | Fused | Win |
| Mesh (prettified) | 1,316 | 1,529 | 1,497 | 0.86× | Two-stage | Win |
| Random (minified) | 1,069 | 820 | 607 | 1.30× | Fused | Win |
| Random (prettified) | 1,534 | 1,286 | 1,114 | 1.19× | Fused | Win |
| Twitter (minified) | 1,755 | 1,556 | 1,082 | 1.13× | Fused | Win |
| Twitter (prettified) | 2,167 | 2,116 | 1,586 | 1.02× | Fused | Win |
| Canada Small (minified) | 736 | 630 | 400 | 1.17× | Fused | Win |
| Canada Small (prettified) | 1,584 | 1,682 | 1,227 | 0.94× | Two-stage | Win |
| CitmCatalog Small (minified) | 1,331 | 954 | 837 | 1.39× | Fused | Win |
| CitmCatalog Small (prettified) | 1,998 | 1,978 | 1,699 | 1.01× | Tie | Win |
| Discord Small (minified) | 1,509 | 1,077 | 846 | 1.40× | Fused | Win |
| Discord Small (prettified) | 1,653 | 1,619 | 1,183 | 1.02× | Fused | Win |
| Google Maps Response Small (minified) | 1,742 | 1,047 | 530 | 1.66× | Fused | Win |
| Google Maps Response Small (prettified) | 2,554 | 2,068 | 1,273 | 1.24× | Fused | Win |
| Instruments Small (minified) | 2,316 | 1,486 | 1,088 | 1.56× | Fused | Win |
| Instruments Small (prettified) | 2,947 | 2,294 | 1,842 | 1.28× | Fused | Win |
| Marine IK Reverse Small (minified) | 527 | 448 | 328 | 1.18× | Fused | Win |
| Marine IK Reverse Small (prettified) | 2,114 | 1,956 | 1,515 | 1.08× | Fused | Win |
| Marine IK Small (minified) | 621 | 517 | 358 | 1.20× | Fused | Win |
| Marine IK Small (prettified) | 2,226 | 2,110 | 1,515 | 1.06× | Fused | Win |
| Mesh Small (minified) | 773 | 636 | 457 | 1.22× | Fused | Win |
| Mesh Small (prettified) | 1,221 | 1,319 | 785 | 0.93× | Two-stage | Win |
| Random Small (minified) | 1,367 | 1,039 | 635 | 1.32× | Fused | Win |
| Random Small (prettified) | 1,902 | 1,410 | 1,128 | 1.35× | Fused | Win |
| Twitter Small (minified) | 2,173 | 1,713 | 1,216 | 1.27× | Fused | Win |
| Twitter Small (prettified) | 2,361 | 2,094 | 1,748 | 1.13× | Fused | Win |

**macOS / Clang 23.1 (Apple M1, NEON)**

| Test | Fused (MB/s) | Two-stage (MB/s) | simdjson (MB/s) | Fused ÷ two-stage | Faster Jsonifier path | Best Jsonifier path vs simdjson |
|---|---|---|---|---|---|---|
| Bool (POD) | 1,097 | 187 | 165 | 5.87× | Fused | Win |
| Double (POD) | 803 | 265 | 175 | 3.03× | Fused | Win |
| Int64 (POD) | 1,920 | 599 | 423 | 3.21× | Fused | Win |
| String (POD) | 1,032 | 601 | 707 | 1.72× | Fused | Win |
| Uint64 (POD) | 1,896 | 540 | 385 | 3.51× | Fused | Win |
| Canada (minified) | 544 | 497 | 290 (n/c) | 1.10× | Fused | Win |
| Canada (prettified) | 1,276 | 1,317 | 1,137 (n/c) | 0.97× | Tie | Win |
| CitmCatalog (minified) | 1,722 | 1,210 | 967 | 1.42× | Fused | Win |
| CitmCatalog (prettified) | 2,846 | 1,886 | 1,965 | 1.51× | Fused | Win |
| Discord (minified) | 2,193 | 1,210 | 1,117 | 1.81× | Fused | Win |
| Discord (prettified) | 2,562 | 1,685 | 1,866 | 1.52× | Fused | Win |
| Google Maps Response (minified) | 2,036 | 1,115 | 919 | 1.83× | Fused | Win |
| Google Maps Response (prettified) | 2,804 | 1,834 | 2,105 | 1.53× | Fused | Win |
| Instruments (minified) | 1,702 | 1,057 | 1,117 | 1.61× | Fused | Win |
| Instruments (prettified) | 2,948 | 2,279 | 2,218 | 1.29× | Fused | Win |
| Marine IK Reverse (minified) | 613 | 564 | 397 | 1.09× | Fused | Win |
| Marine IK Reverse (prettified) | 2,593 | 2,376 | 1,650 | 1.09× | Fused | Win |
| Marine IK (minified) | 548 | 483 | 274 (n/c) | 1.13× | Fused | Win |
| Marine IK (prettified) | 2,317 | 2,338 | 1,759 | 0.99× | Tie | Win |
| Mesh (minified) | 1,048 | 831 | 779 | 1.26× | Fused | Win |
| Mesh (prettified) | 1,479 | 1,324 | 1,244 (n/c) | 1.12× | Fused | Win |
| Random (minified) | 1,800 | 940 | 878 | 1.91× | Fused | Win |
| Random (prettified) | 2,033 | 956 | 1,278 (n/c) | 2.13× | Fused | Win |
| Twitter (minified) | 1,772 | 1,569 | 1,516 | 1.13× | Fused | Win |
| Twitter (prettified) | 2,296 | 2,111 | 1,778 | 1.09× | Fused | Win |
| Canada Small (minified) | 674 | 628 | 372 | 1.07× | Fused | Win |
| Canada Small (prettified) | 1,534 | 1,485 | 1,052 | 1.03× | Fused | Win |
| CitmCatalog Small (minified) | 1,283 | 918 | 773 | 1.40× | Fused | Win |
| CitmCatalog Small (prettified) | 1,760 | 1,419 | 1,445 | 1.24× | Fused | Win |
| Discord Small (minified) | 2,026 | 1,189 | 1,001 | 1.70× | Fused | Win |
| Discord Small (prettified) | 2,092 | 1,485 | 1,643 | 1.41× | Fused | Win |
| Google Maps Response Small (minified) | 2,013 | 1,095 | 921 | 1.84× | Fused | Win |
| Google Maps Response Small (prettified) | 2,765 | 1,826 | 1,888 | 1.51× | Fused | Win |
| Instruments Small (minified) | 2,343 | 1,254 | 1,042 | 1.87× | Fused | Win |
| Instruments Small (prettified) | 2,737 | 2,755 | 2,104 | 0.99× | Tie | Win |
| Marine IK Reverse Small (minified) | 614 | 517 | 343 | 1.19× | Fused | Win |
| Marine IK Reverse Small (prettified) | 2,187 | 2,032 | 1,604 | 1.08× | Fused | Win |
| Marine IK Small (minified) | 610 | 515 | 336 | 1.18× | Fused | Win |
| Marine IK Small (prettified) | 2,248 | 2,132 | 1,466 | 1.05× | Fused | Win |
| Mesh Small (minified) | 737 | 624 | 429 | 1.18× | Fused | Win |
| Mesh Small (prettified) | 1,171 | 1,203 | 873 | 0.97× | Two-stage | Win |
| Random Small (minified) | 1,972 | 888 | 892 | 2.22× | Fused | Win |
| Random Small (prettified) | 2,457 | 1,407 | 1,523 | 1.75× | Fused | Win |
| Twitter Small (minified) | 2,115 | 1,588 | 1,531 | 1.33× | Fused | Win |
| Twitter Small (prettified) | 2,113 | 2,244 | 2,103 | 0.94× | Two-stage | Win |

#### Reused objects

**Windows / MSVC 19.44 (i9-14900KF, AVX2)**

| Test | Fused (MB/s) | Two-stage (MB/s) | simdjson (MB/s) | Fused ÷ two-stage | Faster Jsonifier path | Best Jsonifier path vs simdjson |
|---|---|---|---|---|---|---|
| Bool (POD) | 501 | 141 | 130 | 3.55× | Fused | Win |
| Double (POD) | 694 | 345 | 180 | 2.01× | Fused | Win |
| Int64 (POD) | 1,955 | 626 | 409 | 3.12× | Fused | Win |
| String (POD) | 1,744 | 1,030 | 878 | 1.69× | Fused | Win |
| Uint64 (POD) | 2,044 | 632 | 404 | 3.23× | Fused | Win |
| Canada (minified) | 707 | 648 | 409 | 1.09× | Fused | Win |
| Canada (prettified) | 1,797 | 1,921 | 1,218 | 0.94× | Two-stage | Win |
| CitmCatalog (minified) | 2,314 | 1,533 | 565 | 1.51× | Fused | Win |
| CitmCatalog (prettified) | 3,285 | 3,651 | 1,480 | 0.90× | Two-stage | Win |
| Discord (minified) | 1,662 | 1,477 | 758 | 1.13× | Fused | Win |
| Discord (prettified) | 1,933 | 2,363 | 1,226 | 0.82× | Two-stage | Win |
| Google Maps Response (minified) | 1,675 | 1,388 | 542 | 1.21× | Fused | Win |
| Google Maps Response (prettified) | 2,560 | 3,029 | 1,314 | 0.85× | Two-stage | Win |
| Instruments (minified) | 2,648 | 1,763 | 788 | 1.50× | Fused | Win |
| Instruments (prettified) | 2,583 | 3,183 | 1,398 | 0.81× | Two-stage | Win |
| Marine IK Reverse (minified) | 547 | 495 | 37 | 1.11× | Fused | Win |
| Marine IK Reverse (prettified) | 2,150 | 2,341 | 196 | 0.92× | Two-stage | Win |
| Marine IK (minified) | 558 | 496 | 277 | 1.13× | Fused | Win |
| Marine IK (prettified) | 2,152 | 2,345 | 1,352 | 0.92× | Two-stage | Win |
| Mesh (minified) | 998 | 852 | 472 | 1.17× | Fused | Win |
| Mesh (prettified) | 1,259 | 1,540 | 894 | 0.82× | Two-stage | Win |
| Random (minified) | 1,435 | 1,293 | 631 | 1.11× | Fused | Win |
| Random (prettified) | 1,789 | 2,128 | 1,144 | 0.84× | Two-stage | Win |
| Twitter (minified) | 1,983 | 1,673 | 987 | 1.19× | Fused | Win |
| Twitter (prettified) | 1,842 | 2,323 | 1,438 | 0.79× | Two-stage | Win |
| Canada Small (minified) | 729 | 659 | 377 | 1.11× | Fused | Win |
| Canada Small (prettified) | 1,804 | 1,999 | 1,110 | 0.90× | Two-stage | Win |
| CitmCatalog Small (minified) | 1,406 | 1,054 | 542 | 1.33× | Fused | Win |
| CitmCatalog Small (prettified) | 1,762 | 2,274 | 1,095 | 0.77× | Two-stage | Win |
| Discord Small (minified) | 1,616 | 1,328 | 843 | 1.22× | Fused | Win |
| Discord Small (prettified) | 1,600 | 1,892 | 1,237 | 0.85× | Two-stage | Win |
| Google Maps Response Small (minified) | 1,722 | 1,358 | 566 | 1.27× | Fused | Win |
| Google Maps Response Small (prettified) | 2,602 | 3,071 | 1,364 | 0.85× | Two-stage | Win |
| Instruments Small (minified) | 2,583 | 1,655 | 758 | 1.56× | Fused | Win |
| Instruments Small (prettified) | 2,269 | 2,898 | 1,404 | 0.78× | Two-stage | Win |
| Marine IK Reverse Small (minified) | 554 | 483 | 40 | 1.15× | Fused | Win |
| Marine IK Reverse Small (prettified) | 2,105 | 2,348 | 205 | 0.90× | Two-stage | Win |
| Marine IK Small (minified) | 599 | 524 | 254 (n/c) | 1.14× | Fused | Win |
| Marine IK Small (prettified) | 2,068 | 2,492 | 1,335 | 0.83× | Two-stage | Win |
| Mesh Small (minified) | 931 | 755 | 405 | 1.23× | Fused | Win |
| Mesh Small (prettified) | 1,171 | 1,503 | 819 | 0.78× | Two-stage | Win |
| Random Small (minified) | 1,395 | 1,207 | 623 | 1.15× | Fused | Win |
| Random Small (prettified) | 1,833 | 2,175 | 1,158 | 0.84× | Two-stage | Win |
| Twitter Small (minified) | 2,187 | 1,783 | 1,116 | 1.23× | Fused | Win |
| Twitter Small (prettified) | 1,982 | 2,477 | 1,562 | 0.80× | Two-stage | Win |

**Linux / Clang 24.0 (i9-14900KF, AVX2)**

| Test | Fused (MB/s) | Two-stage (MB/s) | simdjson (MB/s) | Fused ÷ two-stage | Faster Jsonifier path | Best Jsonifier path vs simdjson |
|---|---|---|---|---|---|---|
| Bool (POD) | 2,131 | 342 | 230 | 6.24× | Fused | Win |
| Double (POD) | 1,432 | 427 | 272 | 3.36× | Fused | Win |
| Int64 (POD) | 3,526 | 978 | 604 | 3.60× | Fused | Win |
| String (POD) | 2,869 | 1,535 | 1,406 | 1.87× | Fused | Win |
| Uint64 (POD) | 3,804 | 983 | 556 | 3.87× | Fused | Win |
| Canada (minified) | 1,227 | 1,073 | 774 | 1.14× | Fused | Win |
| Canada (prettified) | 3,135 | 3,099 | 2,280 | 1.01× | Fused | Win |
| CitmCatalog (minified) | 3,091 | 2,163 | 1,426 | 1.43× | Fused | Win |
| CitmCatalog (prettified) | 4,326 | 4,967 | 3,552 | 0.87× | Two-stage | Win |
| Discord (minified) | 3,249 | 2,536 | 1,829 | 1.28× | Fused | Win |
| Discord (prettified) | 2,674 | 3,738 | 2,865 | 0.72× | Two-stage | Win |
| Google Maps Response (minified) | 3,052 | 2,206 | 1,480 | 1.38× | Fused | Win |
| Google Maps Response (prettified) | 3,972 | 4,753 | 3,498 | 0.84× | Two-stage | Win |
| Instruments (minified) | 3,103 | 2,255 | 1,919 | 1.38× | Fused | Win |
| Instruments (prettified) | 3,096 | 3,814 | 3,501 | 0.81× | Two-stage | Win |
| Marine IK Reverse (minified) | 938 | 825 | 680 | 1.14× | Fused | Win |
| Marine IK Reverse (prettified) | 3,797 | 3,558 | 3,177 | 1.07× | Fused | Win |
| Marine IK (minified) | 969 | 842 | 677 | 1.15× | Fused | Win |
| Marine IK (prettified) | 3,732 | 3,499 | 3,086 | 1.07× | Fused | Win |
| Mesh (minified) | 1,496 | 1,280 | 1,347 | 1.17× | Fused | Win |
| Mesh (prettified) | 2,089 | 2,200 | 2,462 | 0.95× | Two-stage | Loss |
| Random (minified) | 2,599 | 2,058 | 1,436 | 1.26× | Fused | Win |
| Random (prettified) | 2,868 | 3,329 | 2,555 | 0.86× | Two-stage | Win |
| Twitter (minified) | 2,576 | 2,540 | 2,132 | 1.01× | Fused | Win |
| Twitter (prettified) | 2,144 | 3,130 | 3,043 | 0.69× | Two-stage | Win |
| Canada Small (minified) | 1,249 | 1,170 | 819 | 1.07× | Fused | Win |
| Canada Small (prettified) | 2,976 | 3,281 | 2,387 | 0.91× | Two-stage | Win |
| CitmCatalog Small (minified) | 2,346 | 1,808 | 1,411 | 1.30× | Fused | Win |
| CitmCatalog Small (prettified) | 2,966 | 3,585 | 2,976 | 0.83× | Two-stage | Win |
| Discord Small (minified) | 2,820 | 2,257 | 1,871 | 1.25× | Fused | Win |
| Discord Small (prettified) | 2,396 | 3,186 | 2,726 | 0.75× | Two-stage | Win |
| Google Maps Response Small (minified) | 2,785 | 2,052 | 1,491 | 1.36× | Fused | Win |
| Google Maps Response Small (prettified) | 3,650 | 4,448 | 3,476 | 0.82× | Two-stage | Win |
| Instruments Small (minified) | 2,883 | 2,118 | 1,753 | 1.36× | Fused | Win |
| Instruments Small (prettified) | 2,909 | 3,610 | 3,196 | 0.81× | Two-stage | Win |
| Marine IK Reverse Small (minified) | 967 | 889 | 668 | 1.09× | Fused | Win |
| Marine IK Reverse Small (prettified) | 3,367 | 3,937 | 3,054 | 0.86× | Two-stage | Win |
| Marine IK Small (minified) | 1,125 | 972 | 649 | 1.16× | Fused | Win |
| Marine IK Small (prettified) | 3,591 | 4,305 | 3,059 | 0.83× | Two-stage | Win |
| Mesh Small (minified) | 1,210 | 1,065 | 1,040 | 1.14× | Fused | Win |
| Mesh Small (prettified) | 1,730 | 2,129 | 2,091 | 0.81× | Two-stage | Win |
| Random Small (minified) | 2,769 | 2,117 | 1,375 | 1.31× | Fused | Win |
| Random Small (prettified) | 2,880 | 3,659 | 2,405 | 0.79× | Two-stage | Win |
| Twitter Small (minified) | 2,970 | 2,654 | 2,354 | 1.12× | Fused | Win |
| Twitter Small (prettified) | 2,371 | 3,517 | 3,257 | 0.67× | Two-stage | Win |

**Linux / GCC 16.2 (i9-14900KF, AVX2)**

| Test | Fused (MB/s) | Two-stage (MB/s) | simdjson (MB/s) | Fused ÷ two-stage | Faster Jsonifier path | Best Jsonifier path vs simdjson |
|---|---|---|---|---|---|---|
| Bool (POD) | 2,407 | 288 | 195 | 8.37× | Fused | Win |
| Double (POD) | 1,304 | 396 | 236 | 3.29× | Fused | Win |
| Int64 (POD) | 3,739 | 871 | 544 | 4.30× | Fused | Win |
| String (POD) | 2,739 | 1,313 | 1,296 | 2.09× | Fused | Win |
| Uint64 (POD) | 3,797 | 894 | 511 | 4.25× | Fused | Win |
| Canada (minified) | 1,099 | 1,111 | 940 | 0.99× | Two-stage | Win |
| Canada (prettified) | 2,843 | 3,134 | 2,705 | 0.91× | Two-stage | Win |
| CitmCatalog (minified) | 3,062 | 2,250 | 1,649 | 1.36× | Fused | Win |
| CitmCatalog (prettified) | 4,555 | 5,075 | 4,012 | 0.90× | Two-stage | Win |
| Discord (minified) | 3,352 | 2,498 | 1,804 | 1.34× | Fused | Win |
| Discord (prettified) | 3,094 | 3,705 | 2,791 | 0.83× | Two-stage | Win |
| Google Maps Response (minified) | 3,133 | 2,103 | 1,182 | 1.49× | Fused | Win |
| Google Maps Response (prettified) | 4,611 | 4,588 | 2,790 | 1.00× | Fused | Win |
| Instruments (minified) | 4,014 | 2,664 | 1,478 | 1.51× | Fused | Win |
| Instruments (prettified) | 3,863 | 4,385 | 2,726 | 0.88× | Two-stage | Win |
| Marine IK Reverse (minified) | 844 | 794 | 674 | 1.06× | Fused | Win |
| Marine IK Reverse (prettified) | 3,539 | 3,588 | 3,099 | 0.99× | Two-stage | Win |
| Marine IK (minified) | 862 | 805 | 664 | 1.07× | Fused | Win |
| Marine IK (prettified) | 3,560 | 3,637 | 3,084 | 0.98× | Two-stage | Win |
| Mesh (minified) | 1,141 | 1,232 | 1,362 | 0.93× | Two-stage | Loss |
| Mesh (prettified) | 1,884 | 2,189 | 2,515 | 0.86× | Two-stage | Loss |
| Random (minified) | 2,078 | 1,984 | 1,056 | 1.05× | Fused | Win |
| Random (prettified) | 2,650 | 3,294 | 1,932 | 0.80× | Two-stage | Win |
| Twitter (minified) | 2,626 | 2,321 | 1,566 | 1.13× | Fused | Win |
| Twitter (prettified) | 2,507 | 3,472 | 2,278 | 0.72× | Two-stage | Win |
| Canada Small (minified) | 1,311 | 1,177 | 906 | 1.11× | Fused | Win |
| Canada Small (prettified) | 2,940 | 3,218 | 2,559 | 0.91× | Two-stage | Win |
| CitmCatalog Small (minified) | 2,350 | 1,873 | 1,410 | 1.26× | Fused | Win |
| CitmCatalog Small (prettified) | 3,179 | 3,782 | 2,908 | 0.84× | Two-stage | Win |
| Discord Small (minified) | 2,624 | 2,010 | 1,604 | 1.31× | Fused | Win |
| Discord Small (prettified) | 2,669 | 2,867 | 2,299 | 0.93× | Two-stage | Win |
| Google Maps Response Small (minified) | 2,845 | 2,025 | 1,221 | 1.40× | Fused | Win |
| Google Maps Response Small (prettified) | 4,130 | 4,402 | 2,886 | 0.94× | Two-stage | Win |
| Instruments Small (minified) | 3,760 | 2,401 | 1,352 | 1.57× | Fused | Win |
| Instruments Small (prettified) | 3,518 | 4,073 | 2,488 | 0.86× | Two-stage | Win |
| Marine IK Reverse Small (minified) | 1,032 | 887 | 634 | 1.16× | Fused | Win |
| Marine IK Reverse Small (prettified) | 3,522 | 3,955 | 2,961 | 0.89× | Two-stage | Win |
| Marine IK Small (minified) | 1,143 | 916 | 632 | 1.25× | Fused | Win |
| Marine IK Small (prettified) | 3,695 | 4,034 | 2,875 | 0.92× | Two-stage | Win |
| Mesh Small (minified) | 1,216 | 1,046 | 1,015 | 1.16× | Fused | Win |
| Mesh Small (prettified) | 1,745 | 2,017 | 2,061 | 0.86× | Two-stage | Loss |
| Random Small (minified) | 2,775 | 2,081 | 1,030 | 1.33× | Fused | Win |
| Random Small (prettified) | 3,141 | 3,476 | 1,906 | 0.90× | Two-stage | Win |
| Twitter Small (minified) | 3,338 | 2,606 | 1,744 | 1.28× | Fused | Win |
| Twitter Small (prettified) | 2,922 | 3,553 | 2,415 | 0.82× | Two-stage | Win |

**macOS / GCC 16.2 (Apple M1, NEON)**

| Test | Fused (MB/s) | Two-stage (MB/s) | simdjson (MB/s) | Fused ÷ two-stage | Faster Jsonifier path | Best Jsonifier path vs simdjson |
|---|---|---|---|---|---|---|
| Bool (POD) | 1,662 | 200 | 203 | 8.30× | Fused | Win |
| Double (POD) | 1,161 | 338 | 159 | 3.44× | Fused | Win |
| Int64 (POD) | 2,689 | 555 | 464 | 4.84× | Fused | Win |
| String (POD) | 1,913 | 932 | 1,003 | 2.05× | Fused | Win |
| Uint64 (POD) | 2,764 | 582 | 451 | 4.75× | Fused | Win |
| Canada (minified) | 878 | 796 | 511 | 1.10× | Fused | Win |
| Canada (prettified) | 2,005 | 1,964 | 1,541 | 1.02× | Fused | Win |
| CitmCatalog (minified) | 2,290 | 1,384 | 1,136 | 1.65× | Fused | Win |
| CitmCatalog (prettified) | 3,467 | 3,185 | 2,642 | 1.09× | Fused | Win |
| Discord (minified) | 2,279 | 1,404 | 1,054 | 1.62× | Fused | Win |
| Discord (prettified) | 2,387 | 2,095 | 1,658 | 1.14× | Fused | Win |
| Google Maps Response (minified) | 2,098 | 1,335 | 759 | 1.57× | Fused | Win |
| Google Maps Response (prettified) | 3,168 | 2,284 | 1,762 | 1.39× | Fused | Win |
| Instruments (minified) | 3,008 | 1,630 | 1,252 | 1.85× | Fused | Win |
| Instruments (prettified) | 3,186 | 2,597 | 2,020 | 1.23× | Fused | Win |
| Marine IK Reverse (minified) | 676 | 545 | 446 | 1.24× | Fused | Win |
| Marine IK Reverse (prettified) | 2,643 | 2,411 | 2,049 | 1.10× | Fused | Win |
| Marine IK (minified) | 765 | 601 | 477 | 1.27× | Fused | Win |
| Marine IK (prettified) | 2,773 | 2,439 | 2,036 (n/c) | 1.14× | Fused | Win |
| Mesh (minified) | 986 | 878 | 886 | 1.12× | Fused | Win |
| Mesh (prettified) | 1,296 | 1,582 | 1,465 | 0.82× | Two-stage | Win |
| Random (minified) | 1,730 | 1,197 | 721 | 1.45× | Fused | Win |
| Random (prettified) | 2,292 | 1,886 | 1,317 | 1.22× | Fused | Win |
| Twitter (minified) | 2,431 | 1,973 | 1,315 | 1.23× | Fused | Win |
| Twitter (prettified) | 2,825 | 2,435 | 1,716 | 1.16× | Fused | Win |
| Canada Small (minified) | 1,028 | 803 | 476 | 1.28× | Fused | Win |
| Canada Small (prettified) | 2,261 | 2,208 | 1,273 | 1.02× | Fused | Win |
| CitmCatalog Small (minified) | 1,670 | 1,186 | 971 | 1.41× | Fused | Win |
| CitmCatalog Small (prettified) | 2,545 | 2,398 | 1,886 | 1.06× | Fused | Win |
| Discord Small (minified) | 2,277 | 1,548 | 1,053 | 1.47× | Fused | Win |
| Discord Small (prettified) | 2,070 | 2,143 | 1,615 | 0.97× | Two-stage | Win |
| Google Maps Response Small (minified) | 2,161 | 1,316 | 594 | 1.64× | Fused | Win |
| Google Maps Response Small (prettified) | 3,106 | 2,541 | 1,449 | 1.22× | Fused | Win |
| Instruments Small (minified) | 2,960 | 1,528 | 1,148 | 1.94× | Fused | Win |
| Instruments Small (prettified) | 2,896 | 2,718 | 2,133 | 1.07× | Fused | Win |
| Marine IK Reverse Small (minified) | 736 | 562 | 426 | 1.31× | Fused | Win |
| Marine IK Reverse Small (prettified) | 2,462 | 2,490 | 1,721 | 0.99× | Tie | Win |
| Marine IK Small (minified) | 814 | 638 | 409 | 1.28× | Fused | Win |
| Marine IK Small (prettified) | 2,629 | 2,720 | 1,709 | 0.97× | Tie | Win |
| Mesh Small (minified) | 986 | 781 | 670 | 1.26× | Fused | Win |
| Mesh Small (prettified) | 1,563 | 1,503 | 1,291 | 1.04× | Fused | Win |
| Random Small (minified) | 1,896 | 1,269 | 770 | 1.49× | Fused | Win |
| Random Small (prettified) | 2,516 | 1,883 | 1,284 | 1.34× | Fused | Win |
| Twitter Small (minified) | 2,429 | 1,823 | 1,389 | 1.33× | Fused | Win |
| Twitter Small (prettified) | 2,962 | 2,706 | 1,736 | 1.09× | Fused | Win |

**macOS / Clang 23.1 (Apple M1, NEON)**

| Test | Fused (MB/s) | Two-stage (MB/s) | simdjson (MB/s) | Fused ÷ two-stage | Faster Jsonifier path | Best Jsonifier path vs simdjson |
|---|---|---|---|---|---|---|
| Bool (POD) | 1,203 | 168 | 156 | 7.17× | Fused | Win |
| Double (POD) | 900 | 280 | 177 | 3.22× | Fused | Win |
| Int64 (POD) | 2,338 | 655 | 437 | 3.57× | Fused | Win |
| String (POD) | 1,960 | 835 | 1,008 | 2.35× | Fused | Win |
| Uint64 (POD) | 2,315 | 608 | 403 (n/c) | 3.81× | Fused | Win |
| Canada (minified) | 757 (n/c) | 510 | 347 (n/c) | 1.48× | Fused | Win |
| Canada (prettified) | 1,803 | 1,656 (n/c) | 1,192 | 1.09× | Fused | Win |
| CitmCatalog (minified) | 2,286 | 1,363 | 920 | 1.68× | Fused | Win |
| CitmCatalog (prettified) | 2,300 | 2,900 | 2,370 | 0.79× | Two-stage | Win |
| Discord (minified) | 2,428 | 1,149 | 1,256 | 2.11× | Fused | Win |
| Discord (prettified) | 2,581 | 2,124 | 2,118 | 1.22× | Fused | Win |
| Google Maps Response (minified) | 2,261 | 1,276 | 911 | 1.77× | Fused | Win |
| Google Maps Response (prettified) | 3,226 | 2,469 | 2,353 | 1.31× | Fused | Win |
| Instruments (minified) | 2,380 | 1,559 | 1,213 | 1.53× | Fused | Win |
| Instruments (prettified) | 3,300 | 2,902 | 2,399 | 1.14× | Fused | Win |
| Marine IK Reverse (minified) | 751 | 583 (n/c) | 435 | 1.29× | Fused | Win |
| Marine IK Reverse (prettified) | 2,880 | 2,418 | 1,945 | 1.19× | Fused | Win |
| Marine IK (minified) | 710 | 517 (n/c) | 318 (n/c) | 1.37× | Fused | Win |
| Marine IK (prettified) | 2,561 | 2,390 | 1,750 | 1.07× | Fused | Win |
| Mesh (minified) | 1,072 | 872 | 818 | 1.23× | Fused | Win |
| Mesh (prettified) | 1,476 | 1,454 | 1,443 | 1.02× | Tie | Tie |
| Random (minified) | 2,147 | 1,167 | 982 | 1.84× | Fused | Win |
| Random (prettified) | 2,498 | 1,635 | 1,690 | 1.53× | Fused | Win |
| Twitter (minified) | 2,600 | 1,735 | 1,751 | 1.50× | Fused | Win |
| Twitter (prettified) | 2,471 | 2,635 | 2,377 | 0.94× | Two-stage | Win |
| Canada Small (minified) | 835 | 767 | 400 | 1.09× | Fused | Win |
| Canada Small (prettified) | 1,839 | 1,728 | 1,322 | 1.06× | Fused | Win |
| CitmCatalog Small (minified) | 1,512 | 1,268 | 898 | 1.19× | Fused | Win |
| CitmCatalog Small (prettified) | 2,425 (n/c) | 2,323 | 1,767 | 1.04× | Fused | Win |
| Discord Small (minified) | 2,279 | 1,248 | 1,350 | 1.83× | Fused | Win |
| Discord Small (prettified) | 2,584 | 1,748 | 2,169 | 1.48× | Fused | Win |
| Google Maps Response Small (minified) | 2,208 | 1,200 | 1,070 | 1.84× | Fused | Win |
| Google Maps Response Small (prettified) | 3,480 | 2,029 | 2,015 | 1.72× | Fused | Win |
| Instruments Small (minified) | 2,818 | 1,449 | 1,171 | 1.95× | Fused | Win |
| Instruments Small (prettified) | 3,256 | 2,878 | 2,100 | 1.13× | Fused | Win |
| Marine IK Reverse Small (minified) | 804 | 567 | 332 | 1.42× | Fused | Win |
| Marine IK Reverse Small (prettified) | 2,751 | 2,664 | 1,811 | 1.03× | Fused | Win |
| Marine IK Small (minified) | 817 | 646 | 331 | 1.27× | Fused | Win |
| Marine IK Small (prettified) | 2,760 | 2,530 | 839 (n/c) | 1.09× | Fused | Win |
| Mesh Small (minified) | 917 | 758 | 655 | 1.21× | Fused | Win |
| Mesh Small (prettified) | 1,324 | 1,427 | 1,150 | 0.93× | Two-stage | Win |
| Random Small (minified) | 2,175 | 1,204 | 1,011 | 1.81× | Fused | Win |
| Random Small (prettified) | 2,690 | 1,841 | 1,326 | 1.46× | Fused | Win |
| Twitter Small (minified) | 2,646 | 1,949 | 1,704 | 1.36× | Fused | Win |
| Twitter Small (prettified) | 2,105 | 2,572 | 2,312 | 0.82× | Two-stage | Win |

### 2.4 What the results show

Which Jsonifier path is faster depends on two things: whether the document is indented, and whether the machine is x86. On the three x86 builds the fused path wins all POD input, 28 of the 30 full-size minified documents and all 30 Small minified ones, while the tape wins 26 of the 30 full-size prettified documents and all 30 Small prettified ones. MSVC splits cleanly: all 20 minified documents to the fused path, all 20 prettified ones to the tape. On the M1 the fused path wins all POD input, all 40 minified documents and 31 of the 40 prettified documents (five to the tape, four ties).

**POD-type tests: the fused path wins all 25 freshly allocated and all 25 reused, on every build.** It runs them 1.39× (String, Windows/MSVC) to 7.69× (Bool, Linux/GCC) faster than the two-stage path, and 1.69× (String, Windows/MSVC) to 8.37× (Bool, Linux/GCC) faster with objects reused. These documents are the case §2 argues from: nothing to skip, every value materialized, so the tape is pure overhead. This is the one result that does not depend on the platform.

**Minified documents: the fused path's, at both sizes.** The fused path wins 98 of the 100 minified documents: all 20 under MSVC, on Linux/Clang, on macOS/GCC and on macOS/Clang, and 18 of 20 on Linux/GCC, where the two-stage path wins minified Canada by 1% (922 against 915 MB/s) and minified Mesh by 6% (1,159 against 1,089). The fused path's margins run from 4% (Random on Linux/GCC) to 91% (Random on macOS/Clang, 1,800 against 940 MB/s) on the full-size documents and up to 2.22× on the Small ones (Random Small on macOS/Clang, 1,972 against 888 MB/s). This is the workload the fused key literals of §3 were built for: machine-generated JSON in declared order.

**Prettified documents on x86: the two-stage path wins 56 of 60.** It wins all 10 under MSVC at each size, all 10 Small prettified documents on Linux/Clang and on Linux/GCC, 9 of the 10 full-size ones on Linux/GCC and 7 of 10 on Linux/Clang. The fused path wins prettified Canada, Marine IK and Marine IK Reverse on Linux/Clang, and prettified Google Maps Response on Linux/GCC is a tie. The tape's margins run from 1% (Marine IK Reverse on Linux/GCC, 3,335 against 3,297 MB/s) to 37% (Twitter on Linux/Clang, 2,540 against 1,858) on the full-size documents and from 6% to 47% on the Small ones, widest on Twitter Small on Linux/Clang. The two Marine IK documents, which are mostly numbers, are the closest on every x86 build, and Discord and Twitter are among the widest. The fused path already predicts the indentation of every line, including the lines that close an object or an array, matches the `": "` after each key as one two-byte constant, and verifies each predicted span with a 16-byte vector loop, a single 8-byte SWAR step and a scalar remainder switch (§3). That keeps the number-heavy documents close, but it does not win most prettified documents on x86. Stage 1 classifies every byte, whitespace included, in vector blocks at a cost that does not depend on layout, while the fused path still does a small amount of branchy work per line: the newline test, the depth multiplication, the span check and the branch on its result. On a heavily indented document that per-line cost appears to outweigh the second pass over the input that the tape costs. We have not isolated this, and state it as a hypothesis. With objects reused the tape wins 56 of the 60 prettified x86 documents again, and the fused path wins the same three on Linux/Clang plus Google Maps Response on Linux/GCC.

**Prettified documents on the M1: mostly the fused path.** Of the 40 prettified documents, the fused path wins 31, the tape wins 5 and 4 are ties. On the full-size documents the fused path wins 17 of 20: on macOS/GCC it wins 9, by 2% (Twitter) to 36% (Google Maps Response, 2,896 against 2,134 MB/s), and the tape wins Mesh by 16% (1,529 against 1,316); on macOS/Clang it wins 8, by 9% (Twitter) to 2.13× (Random, 2,033 against 956 MB/s), and Canada and Marine IK are ties. The Small documents lean the same way, less strongly. On macOS/GCC the fused path wins 7, by 2% (Discord Small) to 35% (Random Small, 1,902 against 1,410 MB/s), the tape wins Canada Small and Mesh Small, and CitmCatalog Small is a tie. On macOS/Clang the fused path wins 7, by 3% (Canada Small) to 75% (Random Small, 2,457 against 1,407 MB/s), the tape wins Mesh Small and Twitter Small, and Instruments Small is a tie. Reused, the fused path wins 31 of the 40, the tape wins 6 and 3 are ties, and the tape's full-size wins are Mesh on macOS/GCC and CitmCatalog and Twitter on macOS/Clang. NEON escapes most of the prettified penalty seen on x86, and we do not have an explanation for why. One contributing factor is that Jsonifier's stage 1 is comparatively more expensive there, since NEON has no `movemask` and the collectors emulate it with narrowing shifts, but we have not isolated it.

**Mesh is where Jsonifier is weakest.** The two-stage path wins prettified Mesh on all three x86 builds and on macOS/GCC in both runs, prettified Mesh Small on all three x86 builds in both runs, and minified Mesh on Linux/GCC in both runs. Mesh is also the only document on which the faster Jsonifier path loses to simdjson On Demand. Freshly allocated, that happens on prettified Mesh on Linux/Clang by 6% (2,223 against 2,098 MB/s) and on Linux/GCC in both forms, by 4% minified (1,211 against 1,159) and 8% prettified (2,264 against 2,094). With objects reused the same three lose by 11%, 10% and 13%, prettified Mesh Small on Linux/GCC loses by 2%, and prettified Mesh on macOS/Clang is a tie. The fused path alone also loses prettified Mesh to simdjson on macOS/GCC in both runs. We have not profiled what is particular about this document.

**MSVC slows simdjson more than either Jsonifier path.** All libraries run slower under MSVC than under Linux/Clang on the same i9-14900KF, but not by the same amount. Across the 9 full-size minified documents in declared key order, the fused path reaches a median 59% of its Linux/Clang throughput, the two-stage path 63% and simdjson 42%. Because the two Jsonifier paths lose a similar share, MSVC routes as Linux/Clang does: minified to the fused path, prettified to the tape. The spread between documents is wider for the fused path (52% on Discord to 81% on Instruments) than for the two-stage path (59% to 75%). That fits the fused path being branch-heavy code whose performance rests on the optimizer while stage 1 is branch-free intrinsic code whose shape is fixed by the source, but we have not isolated it.

**Against simdjson, Jsonifier wins 222 of 225 tests with whichever path is faster and loses three.** The losses are prettified Mesh on Linux/Clang (2,098 against 2,223 MB/s) and Mesh in both forms on Linux/GCC. The fused path alone wins 211, ties 2 and loses 12: those three, prettified Mesh on macOS/GCC, and prettified Discord, Instruments and Twitter on Linux/Clang with the Small copies of CitmCatalog, Discord, Instruments, Mesh and Twitter there; prettified Mesh Small on Linux/GCC and prettified Twitter Small on macOS/Clang are ties. Under MSVC both the faster path and the fused path alone win all 45. Several of these verdicts rest on non-converged results, marked n/c in §2.3.

**Reverse key order.** Requesting every key in reverse order is meant to force simdjson's On Demand API into the rescanning behavior described in §3, and under MSVC it does: simdjson parses minified Marine IK Reverse at 37 MB/s, against 259 MB/s for the same document in declared order, so the faster Jsonifier path is 13.7× faster than simdjson freshly allocated (14.8× reused). The prettified form shows the same gap, at 195 MB/s against 1,271, and the faster Jsonifier path is 11.2× faster (12.0× reused). The Small copies show it too, at 40 MB/s minified and 204 MB/s prettified under MSVC. On the other builds simdjson parses the reversed document about as fast as the declared one, and the faster Jsonifier path is 1.17× to 1.57× faster than simdjson on the full-size reversed document, freshly allocated, and 1.16× to 1.73× reused. Why the rescan cost shows up mostly under MSVC has not been profiled.

The routing implication is sharper than §2's rule of thumb. On x86 the fused path is right for POD and minified input, apart from Canada and Mesh on Linux/GCC, and prettified input belongs to the two-stage path on all three compilers, apart from Canada and the two Marine IK documents on Linux/Clang. On the M1 the fused path is right for all POD and minified input and for most prettified documents, but the tape takes prettified Mesh on macOS/GCC and two Small prettified documents on each build. Routing each test to its faster path would raise the record against simdjson from 211 wins, 2 ties and 12 losses to 222 wins and 3 losses. The router does not yet take the architecture or the indentation into account.

### 2.5 Stage 1 + reflection: two-stage Jsonifier against simdjson's reflection reader

simdjson 5 adds a C++26 static-reflection reader (`document.get<T>()`): stage 1 builds the structural index, then the reflected type drives materialization. That is the same shape as Jsonifier's two-stage path, a tape plus a compile-time schema, so ranking those two head to head compares the two stage-1 + reflection designs directly. The reader needs P2996 reflection, so it only runs on GCC, and the harness registers it for corpus documents and their Small copies, not the POD-type tests. Non-converged results are kept and marked n/c.

#### Freshly allocated objects

**Linux / GCC 16.2 (i9-14900KF, AVX2)**

| Test | Two-stage Jsonifier (MB/s) | simdjson reflection (MB/s) | Two-stage ÷ reflection | Verdict |
|---|---|---|---|---|
| Canada (minified) | 922 | 880 | 1.05× | Win |
| Canada (prettified) | 2,650 | 2,547 | 1.04× | Win |
| CitmCatalog (minified) | 1,896 | 1,437 | 1.32× | Win |
| CitmCatalog (prettified) | 4,354 | 3,551 | 1.23× | Win |
| Discord (minified) | 1,829 | 1,544 | 1.18× | Win |
| Discord (prettified) | 2,828 | 2,431 | 1.16× | Win |
| Google Maps Response (minified) | 1,945 | 1,320 | 1.47× | Win |
| Google Maps Response (prettified) | 4,268 | 3,107 | 1.37× | Win |
| Instruments (minified) | 2,342 | 1,859 | 1.26× | Win |
| Instruments (prettified) | 3,906 | 3,340 | 1.17× | Win |
| Marine IK Reverse (minified) | 726 | 736 | 0.99× | **Loss** |
| Marine IK Reverse (prettified) | 3,335 | 3,342 | 1.00× | Tie |
| Marine IK (minified) | 725 | 720 | 1.01× | Win |
| Marine IK (prettified) | 3,339 | 3,295 | 1.01× | Win |
| Mesh (minified) | 1,159 | 1,184 | 0.98× | **Loss** |
| Mesh (prettified) | 2,094 | 2,216 | 0.95× | **Loss** |
| Random (minified) | 1,414 | 1,192 | 1.19× | Win |
| Random (prettified) | 2,417 | 2,120 | 1.14× | Win |
| Twitter (minified) | 1,831 | 1,794 | 1.02× | Win |
| Twitter (prettified) | 2,597 | 2,586 | 1.00× | Tie |
| Canada Small (minified) | 944 | 937 | 1.01× | Win |
| Canada Small (prettified) | 2,680 | 2,679 | 1.00× | Tie |
| CitmCatalog Small (minified) | 1,654 | 1,276 | 1.30× | Win |
| CitmCatalog Small (prettified) | 3,303 | 2,720 | 1.21× | Win |
| Discord Small (minified) | 1,641 | 1,384 | 1.19× | Win |
| Discord Small (prettified) | 2,386 | 2,011 | 1.19× | Win |
| Google Maps Response Small (minified) | 1,808 | 1,314 | 1.38× | Win |
| Google Maps Response Small (prettified) | 4,016 | 3,084 | 1.30× | Win |
| Instruments Small (minified) | 2,251 | 1,718 | 1.31× | Win |
| Instruments Small (prettified) | 3,806 | 3,111 | 1.22× | Win |
| Marine IK Reverse Small (minified) | 729 | 747 | 0.98× | **Loss** |
| Marine IK Reverse Small (prettified) | 3,314 | 3,353 | 0.99× | **Loss** |
| Marine IK Small (minified) | 764 | 744 | 1.03× | Win |
| Marine IK Small (prettified) | 3,465 | 3,393 | 1.02× | Win |
| Mesh Small (minified) | 908 | 886 | 1.03× | Win |
| Mesh Small (prettified) | 1,737 | 1,810 | 0.96× | **Loss** |
| Random Small (minified) | 1,745 | 1,280 | 1.36× | Win |
| Random Small (prettified) | 3,022 | 2,287 | 1.32× | Win |
| Twitter Small (minified) | 2,347 | 2,139 | 1.10× | Win |
| Twitter Small (prettified) | 3,206 | 2,994 | 1.07× | Win |

**macOS / GCC 16.2 (Apple M1, NEON)**

| Test | Two-stage Jsonifier (MB/s) | simdjson reflection (MB/s) | Two-stage ÷ reflection | Verdict |
|---|---|---|---|---|
| Canada (minified) | 589 | 538 | 1.09× | Win |
| Canada (prettified) | 1,518 (n/c) | 1,517 (n/c) | 1.00× | Tie |
| CitmCatalog (minified) | 1,261 | 973 | 1.30× | Win |
| CitmCatalog (prettified) | 2,598 | 2,217 | 1.17× | Win |
| Discord (minified) | 1,101 | 932 | 1.18× | Win |
| Discord (prettified) | 1,706 | 1,550 | 1.10× | Win |
| Google Maps Response (minified) | 1,133 | 838 | 1.35× | Win |
| Google Maps Response (prettified) | 2,134 | 2,050 | 1.04× | Win |
| Instruments (minified) | 1,451 | 1,174 | 1.24× | Win |
| Instruments (prettified) | 2,353 | 1,807 | 1.30× | Win |
| Marine IK Reverse (minified) | 516 | 437 | 1.18× | Win |
| Marine IK Reverse (prettified) | 2,083 | 1,895 | 1.10× | Win |
| Marine IK (minified) | 495 | 414 | 1.20× | Win |
| Marine IK (prettified) | 2,185 | 1,868 | 1.17× | Win |
| Mesh (minified) | 875 | 635 | 1.38× | Win |
| Mesh (prettified) | 1,529 | 1,178 | 1.30× | Win |
| Random (minified) | 820 | 677 | 1.21× | Win |
| Random (prettified) | 1,286 | 1,243 | 1.03× | Win |
| Twitter (minified) | 1,556 | 1,198 | 1.30× | Win |
| Twitter (prettified) | 2,116 | 1,573 | 1.34× | Win |
| Canada Small (minified) | 630 | 543 | 1.16× | Win |
| Canada Small (prettified) | 1,682 | 1,486 | 1.13× | Win |
| CitmCatalog Small (minified) | 954 | 801 | 1.19× | Win |
| CitmCatalog Small (prettified) | 1,978 | 1,582 | 1.25× | Win |
| Discord Small (minified) | 1,077 | 999 | 1.08× | Win |
| Discord Small (prettified) | 1,619 | 1,261 | 1.28× | Win |
| Google Maps Response Small (minified) | 1,047 | 862 | 1.22× | Win |
| Google Maps Response Small (prettified) | 2,068 | 1,946 | 1.06× | Win |
| Instruments Small (minified) | 1,486 | 1,168 | 1.27× | Win |
| Instruments Small (prettified) | 2,294 | 2,019 | 1.14× | Win |
| Marine IK Reverse Small (minified) | 448 | 479 | 0.93× | **Loss** |
| Marine IK Reverse Small (prettified) | 1,956 | 1,850 | 1.06× | Win |
| Marine IK Small (minified) | 517 | 470 | 1.10× | Win |
| Marine IK Small (prettified) | 2,110 | 1,845 | 1.14× | Win |
| Mesh Small (minified) | 636 | 531 | 1.20× | Win |
| Mesh Small (prettified) | 1,319 | 990 | 1.33× | Win |
| Random Small (minified) | 1,039 | 793 | 1.31× | Win |
| Random Small (prettified) | 1,410 | 1,457 | 0.97× | **Loss** |
| Twitter Small (minified) | 1,713 | 1,255 | 1.37× | Win |
| Twitter Small (prettified) | 2,094 | 1,781 | 1.18× | Win |

#### Reused objects

**Linux / GCC 16.2 (i9-14900KF, AVX2)**

| Test | Two-stage Jsonifier (MB/s) | simdjson reflection (MB/s) | Two-stage ÷ reflection | Verdict |
|---|---|---|---|---|
| Canada (minified) | 1,111 | 1,054 | 1.05× | Win |
| Canada (prettified) | 3,134 | 3,030 | 1.03× | Win |
| CitmCatalog (minified) | 2,250 | 1,702 | 1.32× | Win |
| CitmCatalog (prettified) | 5,075 | 4,126 | 1.23× | Win |
| Discord (minified) | 2,498 | 1,791 | 1.39× | Win |
| Discord (prettified) | 3,705 | 2,778 | 1.33× | Win |
| Google Maps Response (minified) | 2,103 | 1,397 | 1.51× | Win |
| Google Maps Response (prettified) | 4,588 | 3,218 | 1.43× | Win |
| Instruments (minified) | 2,664 | 1,911 | 1.39× | Win |
| Instruments (prettified) | 4,385 | 3,425 | 1.28× | Win |
| Marine IK Reverse (minified) | 794 | 825 | 0.96× | **Loss** |
| Marine IK Reverse (prettified) | 3,588 | 3,668 | 0.98× | **Loss** |
| Marine IK (minified) | 805 | 817 | 0.99× | **Loss** |
| Marine IK (prettified) | 3,637 | 3,647 | 1.00× | Tie |
| Mesh (minified) | 1,232 | 1,263 | 0.98× | **Loss** |
| Mesh (prettified) | 2,189 | 2,345 | 0.93× | **Loss** |
| Random (minified) | 1,984 | 1,467 | 1.35× | Win |
| Random (prettified) | 3,294 | 2,592 | 1.27× | Win |
| Twitter (minified) | 2,321 | 2,091 | 1.11× | Win |
| Twitter (prettified) | 3,472 | 2,954 | 1.18× | Win |
| Canada Small (minified) | 1,177 | 1,149 | 1.02× | Win |
| Canada Small (prettified) | 3,218 | 3,177 | 1.01× | Win |
| CitmCatalog Small (minified) | 1,873 | 1,416 | 1.32× | Win |
| CitmCatalog Small (prettified) | 3,782 | 2,949 | 1.28× | Win |
| Discord Small (minified) | 2,010 | 1,581 | 1.27× | Win |
| Discord Small (prettified) | 2,867 | 2,300 | 1.25× | Win |
| Google Maps Response Small (minified) | 2,025 | 1,387 | 1.46× | Win |
| Google Maps Response Small (prettified) | 4,402 | 3,251 | 1.35× | Win |
| Instruments Small (minified) | 2,401 | 1,805 | 1.33× | Win |
| Instruments Small (prettified) | 4,073 | 3,267 | 1.25× | Win |
| Marine IK Reverse Small (minified) | 887 | 862 | 1.03× | Win |
| Marine IK Reverse Small (prettified) | 3,955 | 3,804 | 1.04× | Win |
| Marine IK Small (minified) | 916 | 866 | 1.06× | Win |
| Marine IK Small (prettified) | 4,034 | 3,820 | 1.06× | Win |
| Mesh Small (minified) | 1,046 | 1,058 | 0.99× | **Loss** |
| Mesh Small (prettified) | 2,017 | 2,134 | 0.95× | **Loss** |
| Random Small (minified) | 2,081 | 1,418 | 1.47× | Win |
| Random Small (prettified) | 3,476 | 2,593 | 1.34× | Win |
| Twitter Small (minified) | 2,606 | 2,342 | 1.11× | Win |
| Twitter Small (prettified) | 3,553 | 3,259 | 1.09× | Win |

**macOS / GCC 16.2 (Apple M1, NEON)**

| Test | Two-stage Jsonifier (MB/s) | simdjson reflection (MB/s) | Two-stage ÷ reflection | Verdict |
|---|---|---|---|---|
| Canada (minified) | 796 | 628 | 1.27× | Win |
| Canada (prettified) | 1,964 | 1,737 | 1.13× | Win |
| CitmCatalog (minified) | 1,384 | 1,154 | 1.20× | Win |
| CitmCatalog (prettified) | 3,185 | 2,398 | 1.33× | Win |
| Discord (minified) | 1,404 | 1,137 | 1.23× | Win |
| Discord (prettified) | 2,095 | 1,578 | 1.33× | Win |
| Google Maps Response (minified) | 1,335 | 951 | 1.40× | Win |
| Google Maps Response (prettified) | 2,284 | 2,173 | 1.05× | Win |
| Instruments (minified) | 1,630 | 1,173 | 1.39× | Win |
| Instruments (prettified) | 2,597 | 2,059 | 1.26× | Win |
| Marine IK Reverse (minified) | 545 | 525 | 1.04× | Win |
| Marine IK Reverse (prettified) | 2,411 | 2,094 | 1.15× | Win |
| Marine IK (minified) | 601 | 501 | 1.20× | Win |
| Marine IK (prettified) | 2,439 | 2,164 | 1.13× | Win |
| Mesh (minified) | 878 | 676 | 1.30× | Win |
| Mesh (prettified) | 1,582 | 1,132 | 1.40× | Win |
| Random (minified) | 1,197 | 772 | 1.55× | Win |
| Random (prettified) | 1,886 | 1,393 | 1.35× | Win |
| Twitter (minified) | 1,973 | 1,345 | 1.47× | Win |
| Twitter (prettified) | 2,435 | 1,786 | 1.36× | Win |
| Canada Small (minified) | 803 | 639 | 1.26× | Win |
| Canada Small (prettified) | 2,208 | 1,681 | 1.31× | Win |
| CitmCatalog Small (minified) | 1,186 | 916 | 1.30× | Win |
| CitmCatalog Small (prettified) | 2,398 | 1,879 | 1.28× | Win |
| Discord Small (minified) | 1,548 | 1,147 | 1.35× | Win |
| Discord Small (prettified) | 2,143 | 1,716 | 1.25× | Win |
| Google Maps Response Small (minified) | 1,316 | 1,050 | 1.25× | Win |
| Google Maps Response Small (prettified) | 2,541 | 2,242 | 1.13× | Win |
| Instruments Small (minified) | 1,528 | 1,099 | 1.39× | Win |
| Instruments Small (prettified) | 2,718 | 1,991 | 1.37× | Win |
| Marine IK Reverse Small (minified) | 562 | 540 | 1.04× | Win |
| Marine IK Reverse Small (prettified) | 2,490 | 2,391 | 1.04× | Win |
| Marine IK Small (minified) | 638 | 580 | 1.10× | Win |
| Marine IK Small (prettified) | 2,720 | 2,213 | 1.23× | Win |
| Mesh Small (minified) | 781 | 583 | 1.34× | Win |
| Mesh Small (prettified) | 1,503 | 1,236 | 1.22× | Win |
| Random Small (minified) | 1,269 | 967 | 1.31× | Win |
| Random Small (prettified) | 1,883 | 1,551 | 1.21× | Win |
| Twitter Small (minified) | 1,823 | 1,365 | 1.34× | Win |
| Twitter Small (prettified) | 2,706 | 1,830 | 1.48× | Win |

In the freshly allocated run, two-stage Jsonifier wins 68 of these 80 tests, ties 4 and loses 8. On the full-size documents it wins 34 of 40: on Linux/GCC it loses minified Marine IK Reverse and Mesh in both forms, by 1% to 5%, and ties prettified Marine IK Reverse and Twitter; on macOS/GCC it wins 19 and ties prettified Canada, where neither result converged. On the Small documents it wins 34 of 40, losing both Marine IK Reverse Small documents and prettified Mesh Small on Linux/GCC (by 1% to 4%), where it ties prettified Canada Small, and minified Marine IK Reverse Small (by 7%) and prettified Random Small (by 3%) on macOS/GCC. Mesh is also where Jsonifier is weakest against simdjson On Demand (§2.4); we have not profiled either result. With the fused path instead of the two-stage one, Jsonifier beats the reflection reader on 68 of the 80 and loses 12. With objects reused, two-stage Jsonifier wins 72 of 80, ties 1 and loses 7, all on Linux/GCC: minified Marine IK, Marine IK Reverse and Mesh Small, and Marine IK Reverse and Mesh in both forms with prettified Mesh Small, by 1% to 7%; prettified Marine IK is the tie. On macOS/GCC it wins all 40 reused.

### 2.6 Partial reading: where the tape earns its keep

The harness has one test built for the workload the tape exists for. "Twitter Partial" parses the Twitter document, and its Small copy, into a type that holds three fields per status (`text`, `user.screen_name` and `retweet_count`) and skips everything else. The harness reads the keys without assuming their order, and simdjson On Demand reads the same three fields. The harness requires stage 1 and stage 2 for this test, so both Jsonifier rows run the two-stage path; the table uses the "jsonifier (two-stage)" row. The two rows agree within 8% on 35 of the 40 results; all five wider gaps are on prettified Twitter or Twitter Small on the macOS builds, and the widest is reused prettified Twitter Small on macOS/Clang, where the two-stage row runs at 4,141 MB/s against the "jsonifier" row's 3,313. Results that did not converge are marked n/c.

#### Freshly allocated objects

| Platform / Compiler | Document | Jsonifier stage 1 + 2 (MB/s) | simdjson On Demand (MB/s) | Jsonifier ÷ simdjson | Verdict |
|---|---|---|---|---|---|
| Windows / MSVC 19.44 (i9-14900KF, AVX2) | Twitter (minified) | 5,899 | 1,943 | 3.04× | Win |
| Windows / MSVC 19.44 (i9-14900KF, AVX2) | Twitter (prettified) | 7,325 | 2,661 | 2.75× | Win |
| Windows / MSVC 19.44 (i9-14900KF, AVX2) | Twitter Small (minified) | 4,975 | 1,968 | 2.53× | Win |
| Windows / MSVC 19.44 (i9-14900KF, AVX2) | Twitter Small (prettified) | 6,369 | 2,742 | 2.32× | Win |
| Linux / Clang 24.0 (i9-14900KF, AVX2) | Twitter (minified) | 7,110 | 5,623 | 1.26× | Win |
| Linux / Clang 24.0 (i9-14900KF, AVX2) | Twitter (prettified) | 9,066 | 7,452 | 1.22× | Win |
| Linux / Clang 24.0 (i9-14900KF, AVX2) | Twitter Small (minified) | 5,243 | 5,185 | 1.01× | Win |
| Linux / Clang 24.0 (i9-14900KF, AVX2) | Twitter Small (prettified) | 6,593 | 6,798 | 0.97× | Loss |
| Linux / GCC 16.2 (i9-14900KF, AVX2) | Twitter (minified) | 7,330 | 5,403 | 1.36× | Win |
| Linux / GCC 16.2 (i9-14900KF, AVX2) | Twitter (prettified) | 8,875 | 7,062 | 1.26× | Win |
| Linux / GCC 16.2 (i9-14900KF, AVX2) | Twitter Small (minified) | 5,717 | 4,705 | 1.21× | Win |
| Linux / GCC 16.2 (i9-14900KF, AVX2) | Twitter Small (prettified) | 7,399 | 6,162 | 1.20× | Win |
| macOS / GCC 16.2 (Apple M1, NEON) | Twitter (minified) | 3,931 | 3,095 | 1.27× | Win |
| macOS / GCC 16.2 (Apple M1, NEON) | Twitter (prettified) | 4,626 | 3,896 | 1.19× | Win |
| macOS / GCC 16.2 (Apple M1, NEON) | Twitter Small (minified) | 3,115 | 2,967 | 1.05× | Win |
| macOS / GCC 16.2 (Apple M1, NEON) | Twitter Small (prettified) | 4,245 | 3,594 | 1.18× | Win |
| macOS / Clang 23.1 (Apple M1, NEON) | Twitter (minified) | 3,583 | 2,826 | 1.27× | Win |
| macOS / Clang 23.1 (Apple M1, NEON) | Twitter (prettified) | 4,208 | 3,766 | 1.12× | Win |
| macOS / Clang 23.1 (Apple M1, NEON) | Twitter Small (minified) | 3,020 | 2,845 | 1.06× | Win |
| macOS / Clang 23.1 (Apple M1, NEON) | Twitter Small (prettified) | 3,845 | 3,698 | 1.04× | Win |

#### Reused objects

| Platform / Compiler | Document | Jsonifier stage 1 + 2 (MB/s) | simdjson On Demand (MB/s) | Jsonifier ÷ simdjson | Verdict |
|---|---|---|---|---|---|
| Windows / MSVC 19.44 (i9-14900KF, AVX2) | Twitter (minified) | 6,255 | 1,980 | 3.16× | Win |
| Windows / MSVC 19.44 (i9-14900KF, AVX2) | Twitter (prettified) | 7,572 | 2,820 | 2.69× | Win |
| Windows / MSVC 19.44 (i9-14900KF, AVX2) | Twitter Small (minified) | 5,449 | 2,051 | 2.66× | Win |
| Windows / MSVC 19.44 (i9-14900KF, AVX2) | Twitter Small (prettified) | 6,925 | 2,887 | 2.40× | Win |
| Linux / Clang 24.0 (i9-14900KF, AVX2) | Twitter (minified) | 7,236 | 5,740 | 1.26× | Win |
| Linux / Clang 24.0 (i9-14900KF, AVX2) | Twitter (prettified) | 9,325 | 7,556 | 1.23× | Win |
| Linux / Clang 24.0 (i9-14900KF, AVX2) | Twitter Small (minified) | 5,503 | 5,333 | 1.03× | Win |
| Linux / Clang 24.0 (i9-14900KF, AVX2) | Twitter Small (prettified) | 7,038 | 6,983 | 1.01× | Win |
| Linux / GCC 16.2 (i9-14900KF, AVX2) | Twitter (minified) | 7,505 | 5,492 | 1.37× | Win |
| Linux / GCC 16.2 (i9-14900KF, AVX2) | Twitter (prettified) | 9,046 | 7,156 | 1.26× | Win |
| Linux / GCC 16.2 (i9-14900KF, AVX2) | Twitter Small (minified) | 6,143 | 4,887 | 1.26× | Win |
| Linux / GCC 16.2 (i9-14900KF, AVX2) | Twitter Small (prettified) | 7,603 | 6,324 | 1.20× | Win |
| macOS / GCC 16.2 (Apple M1, NEON) | Twitter (minified) | 4,260 | 3,178 | 1.34× | Win |
| macOS / GCC 16.2 (Apple M1, NEON) | Twitter (prettified) | 4,979 | 3,993 | 1.25× | Win |
| macOS / GCC 16.2 (Apple M1, NEON) | Twitter Small (minified) | 3,578 | 3,095 | 1.16× | Win |
| macOS / GCC 16.2 (Apple M1, NEON) | Twitter Small (prettified) | 4,498 | 3,586 | 1.25× | Win |
| macOS / Clang 23.1 (Apple M1, NEON) | Twitter (minified) | 3,518 | 3,068 | 1.15× | Win |
| macOS / Clang 23.1 (Apple M1, NEON) | Twitter (prettified) | 4,428 | 3,872 | 1.14× | Win |
| macOS / Clang 23.1 (Apple M1, NEON) | Twitter Small (minified) | 3,350 | 3,104 | 1.08× | Win |
| macOS / Clang 23.1 (Apple M1, NEON) | Twitter Small (prettified) | 4,141 | 3,911 | 1.06× | Win |

Jsonifier wins 39 of the 40 results and loses 1, by up to 3.16× (minified Twitter under MSVC, reused). All 20 full-size results are wins, and the reused run wins all 20. The one loss is freshly allocated prettified Twitter Small on Linux/Clang (0.97×). Throughput here is counted over the whole document, so these figures run well above the full-parse ones: on minified Twitter the fused path parses at 1,382 MB/s under MSVC and 2,027 MB/s on Linux/Clang when it materializes every value, against 5,899 and 7,110 MB/s here, because the bytes the caller does not ask for are skipped through the tape instead of being walked. The margin over simdjson is 2.32× to 3.16× under MSVC and 0.97× to 1.37× elsewhere, and it is narrower on the Small document than on the full-size one in 19 of the 20 comparable pairs; the exception is reused prettified Twitter on macOS/GCC. On minified Twitter in the freshly allocated run, simdjson On Demand under MSVC reaches 35% of its Linux/Clang throughput and Jsonifier's stage 1 + 2 path reaches 83%. On the full-size document this is the one test in the sweep where the index is unambiguously the right tool, and it is the reason the two-stage machinery stays in Jsonifier even though full-document parsing does not route through it.

### 2.7 Validation

The sweep's "Validate" test checks a 617 KiB document for well-formedness without materializing anything, against Glaze's validator; simdjson is not registered for it, and it runs freshly allocated only. Jsonifier wins on all five builds:

| Platform / Compiler | Jsonifier (MB/s) | Glaze (MB/s) | Jsonifier ÷ Glaze | Verdict |
|---|---|---|---|---|
| Windows / MSVC 19.44 (i9-14900KF, AVX2) | 3,004 | 2,468 | 1.22× | Win |
| Linux / Clang 24.0 (i9-14900KF, AVX2) | 3,391 | 2,576 | 1.32× | Win |
| Linux / GCC 16.2 (i9-14900KF, AVX2) | 3,025 | 2,446 | 1.24× | Win |
| macOS / GCC 16.2 (Apple M1, NEON) | 2,029 | 1,781 | 1.14× | Win |
| macOS / Clang 23.1 (Apple M1, NEON) | 1,875 | 1,779 | 1.05× | Win |

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
| Full-document parse | Stage 1 + On Demand traversal | Single fused pass, schema-directed; faster than its own two-stage path on POD data, on 98 of 100 minified documents and on 31 of 40 prettified documents on the M1; slower on 56 of 60 prettified x86 documents (see §2.2) |
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

The two-stage model is a genuinely great algorithm. Jsonifier's stage 1 is an unapologetic descendant of Langdale and Lemire's design, and credits it in source. The contribution here is architectural discipline about *when* to run it. A structural tape is an index, and indexes are worth building exactly when you will not read the whole book. Jsonifier builds it for partial reads and structural transforms, skips it for full parses, validates UTF-8 in the registers it was already holding, and lets the compiler specialize every remaining decision down to per-toolchain loop geometry. §2 shows what skipping the tape is worth, with both paths compiled into one binary and ranked head to head, on both freshly allocated and reused target objects. Across 225 freshly allocated tests on five platforms the fused path wins 157, ties 5 and loses 63, and where it loses is a matter of layout and architecture more than of principle. It wins all 25 POD-type tests, by up to 7.7× (Bool on Linux/GCC), 98 of the 100 minified documents, and 31 of the 40 prettified documents on the M1. The two-stage path wins 56 of the 60 prettified documents on x86, including all 20 under MSVC. The tape, in other words, is not only an index for partial reads: on indented input on x86 it is also the faster way to read the whole book, and the router should learn when. Taking whichever path is faster, Jsonifier beats simdjson On Demand on 222 of those 225 tests, its own stage-1 + reflection path beats simdjson's stage-1 + reflection reader on 68 of 80, and on the partial-reading test, where the index is the right tool, it beats simdjson On Demand on all 20 full-size results and on 39 of the 40 overall, with the one loss on the Small copy. The benchmarks are the receipts.

---

*Jsonifier is MIT-licensed and available at github.com/nihilai-collective/Jsonifier. Benchmark methodology and full sweep data: github.com/nihilai-collective/Json-Performance.*