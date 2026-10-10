# Two Stages, On Demand: The Stage-1 + Stage-2 Architecture in Jsonifier

**Nihilai Collective Corp — Engineering Papers**  
*Nihilai Collective Corp*  
*October 2026 — Jsonifier*  

---

## Abstract

Since Langdale and Lemire's 2019 paper *Parsing Gigabytes of JSON per Second*, the two-stage SIMD parsing model — a vectorized structural-indexing pass (stage 1) followed by a tape-driven materialization pass (stage 2) — has been treated as the canonical architecture for high-performance JSON processing. simdjson, the reference implementation, routes every document through stage 1 unconditionally.

Jsonifier takes a different position: **two-stage parsing is a specialized tool, not a mandatory front door.** When parsing a full document into concrete, reflection-registered C++ types, every structural fact the tape would record is rediscovered anyway during value materialization — so building the tape means touching every byte twice for information used once. Jsonifier therefore parses full documents in a single fused pass, and reserves its stage-1 + stage-2 machinery for the workloads where a structural index genuinely pays for itself: partial/unordered reading, prettifying, and minifying. Measured with both paths compiled into one binary, skipping the tape wins on POD-type data on every build and on 46 of the 50 minified documents, the tape is faster on 23 of the 30 prettified documents on x86, and on the Apple M1 the fused path wins 16 of the 20 prettified documents.

This paper describes both halves of that architecture: the single-pass primary path, and Jsonifier's stage-1 implementation — including its per-compiler-tuned step geometry, its fold-expression drain architecture for bitmask-to-index extraction, and its distributed UTF-8 validation strategy — with direct comparisons to simdjson's current design throughout.

---

## 1. Background: the canonical two-stage model

simdjson's pipeline is well documented. Stage 1 sweeps the input in 64-byte blocks, computing per-block bitmasks: backslashes, odd-length escape terminations, quotes, an in-string range mask derived via carry-less multiplication (prefix XOR), structural characters (`{ } [ ] : ,`), whitespace, and pseudo-structural scalar starts. The surviving bits are converted to byte offsets — the *structural tape* — via a tzcnt/blsr extraction loop. Stage 2 then walks the tape to build a DOM or, in the On Demand API, lazily materializes only the values the caller touches.

The model's strength is that stage 1 is branch-free and data-parallel; its cost is that it is unconditional. Every byte of every document is classified and indexed before a single value is parsed, regardless of whether the caller's access pattern needs the index at all.

## 2. The Jsonifier position: pay for the tape only when the tape pays for you

Jsonifier's primary API parses JSON directly into user-defined structs registered through compile-time reflection (`jsonifier::core<T>`). In that setting the parser already knows, at compile time, the expected shape of the document: which keys exist, their serialized order, their types. The parse loop is a schema-directed walk, not a blind tree construction.

Consider what the structural tape provides: the positions of every brace, bracket, colon, comma, quote, and scalar start. Now consider what a schema-directed single-pass parser does at each of those same positions: it is *already there*, with its iterator parked on that exact byte, consuming it as part of matching a known key literal or delimiting a value. The tape's information is a strict subset of what the fused pass discovers for free in program order. Building it first means that, for a full-document parse in which every value is materialized:

- every byte of the input is loaded twice (once in stage 1, once in stage 2),
- the tape itself is written and re-read (cache traffic proportional to structural density), and
- stage 2's control flow is driven by indirect loads through the index array rather than by a pointer walking linearly through memory the prefetcher already understands.

(We scope the double-load claim deliberately: a two-stage consumer that *skips* content — simdjson's On Demand API skipping unrequested fields — does not reload the skipped bytes in its second pass. That is exactly the workload where the tape earns its keep, and exactly why Jsonifier retains the two-stage machinery for partial reading. The accounting above describes the full-materialization case, where nothing is skipped and the tape's information is fully redundant with the walk.)

For full-document parsing into known types, the tape is overhead in principle, and in practice whether skipping it is faster comes down to two questions: is the input indented, and is the machine x86? §2.1 through §2.4 measure both paths in one binary on five platform/compiler builds. In the freshly allocated run, the fused path wins all 25 POD-type tests and 46 of the 50 minified documents across all five builds; the other four are Canada and Mesh on Linux/GCC and Mesh on macOS/GCC, which go to the tape, and Random on Linux/GCC, a tie. The two-stage path wins 23 of the 30 prettified documents on x86, and on the M1 the fused path wins 16 of the 20 prettified documents, the tape wins 2 and two are ties.

**The routing rule is simple: the two-stage machinery is engaged for partial reading, prettifying, and minifying — workloads where the caller does *not* want every value, or wants pure structural transformation. Full-document parsing takes the single-pass path.** §2.4 shows where the measurements say that rule should be refined.

One anticipated objection deserves preemption here: that Jsonifier's requirement of ahead-of-time registration (`jsonifier::core<T>`) concedes generality that simdjson retains, since simdjson parses arbitrary documents with no such declaration. For truly dynamic workloads — schemas unknown until runtime, exploratory traversal, structural transformation of unknown documents — this is correct, and simdjson's DOM and On Demand models are the appropriate tools; Jsonifier's registration model simply does not address that problem. But for the workload this paper concerns — parsing documents into concrete types the caller has defined — the objection dissolves on inspection, because the schema knowledge exists at compile time in both programs. A simdjson caller materializing a struct writes the schema into their source as a sequence of field accesses in a fixed order chosen at authoring time; that traversal code is a schema declaration in imperative clothing. The difference is not the presence of compile-time knowledge but its legibility to the library: expressed as hand-written traversal, the knowledge is opaque — simdjson cannot fuse key literals from it, cannot learn permuted orders through it, and cannot skip building the index it implies is unnecessary. Expressed as a reflection registration, the identical knowledge becomes architecture: fused member headers, adaptive order recovery, and the routing rule above. The comparison between the two libraries on known-type workloads is therefore not "declared schema versus no schema" — it is the same schema, declared once where the compiler can consume it versus restated per call site where it cannot.

### 2.1 Method: both paths in one binary

Every comparison in this section comes from the [Json-Performance](https://github.com/nihilai-collective/Json-Performance) sweep published October 11, 2026, with simdjson [7f6f8dc](https://github.com/simdjson/simdjson/commit/7f6f8dc) and BenchmarkSuite [125f8a7](https://github.com/nihilai-collective/benchmarksuite/commit/125f8a7) on every build. Jsonifier is [09747e9](https://github.com/nihilai-collective/jsonifier/commit/09747e9) on every build. The harness registers both Jsonifier paths in the same binary. "jsonifier" is the default fused single-pass path, which the harness calls scalar structural iteration. "jsonifier (two-stage)" makes exactly the same `parseJson` call with `partialRead` set, which routes it through stage 1 and then stage 2. "simdjson (ondemand)" is the reference for §2.2 to §2.4. Glaze runs in the same sweep but is left out here, and `simdjson (reflection)`, simdjson 5's C++26 static-reflection reader, gets its own comparison in §2.5. Because the two Jsonifier paths are compiled together, run back to back on each test, and ranked against each other by the same statistics, the only thing that differs between them is the path.

All libraries parse fully into the target data structures and perform UTF-8 validation. Every test runs twice. In the freshly allocated run, every iteration constructs a new object to parse into and destroys it again inside the timed region, so allocation and deallocation are part of each measurement. In the reused run, labelled "(Reused)" in the sweep, the object is created once and held across iterations; it is cleared, keeping its capacity, outside the timed region before each iteration, so only the parse itself is measured. Parser instances are reused in both. §2.2, §2.3 and §2.5 give both runs; the discussion in §2.4 quotes the freshly allocated run unless it says otherwise. The suite has 25 tests: five POD-type tests (arrays of a single value type: Bool, Double, Int64, String, Uint64), nine corpus documents in minified and prettified form, and two "Marine IK Reverse" tests that request every key in the reverse of its document order. In every other test, all libraries receive keys in document order. The sweep also runs a partial-reading test on the Twitter document, which §2.6 covers on its own, and "Small" cut-down copies of each document (at most 5 KiB minified) that measure per-call overhead; neither is counted in §2.2 to §2.5.

Sampling is adaptive. Iterations start at 100 and double each epoch, and sampling does not stop early: epochs continue until 5 seconds have elapsed or the iteration cap of 100,000 is reached. Every epoch after the first is scored by its relative standard error plus its epoch-over-epoch mean shift, and the lowest-scoring epoch is kept as the result. A result counts as converged only if that epoch has RSE below 5% and mean shift below 2.5% on the three x86 builds (10% and 5% on the virtualized M1), and a test is ranked only if every library in it converges, which would leave a platform with fewer than 25 tests; in this sweep all 25 converged on every build. Ties are declared by Welch's t-test on the kept epoch (two-sided, p < 0.05), and every verdict in this paper is a pairwise test between the two libraries or paths named in its column. Two properties of this rule matter for reading the results. Keeping the quietest epoch favors the least-disturbed stretch of each run, which raises absolute throughput somewhat, but it is applied identically to every library. And because the kept epoch has the smallest variance available, small differences are more often resolved as wins or losses than they would be under a first-to-converge rule: this sweep produced five ties between the two Jsonifier paths across 125 freshly allocated tests and five across 125 reused ones.

### 2.2 Fused against two-stage

Ranked head to head, the fused path wins 92 tests, 5 are statistical ties, and the two-stage path wins 28. The split is a matter of platform and indentation:

| Platform / Compiler | Fused faster | Tie | Two-stage faster | Tests converged |
|---|---|---|---|---|
| Windows / MSVC 19.44 (i9-14900KF, AVX2) | 15 | 0 | 10 | 25 of 25 |
| Linux / Clang 24.0 (i9-14900KF, AVX2) | 19 | 0 | 6 | 25 of 25 |
| Linux / GCC 16.2 (i9-14900KF, AVX2) | 13 | 3 | 9 | 25 of 25 |
| macOS / GCC 16.2 (Apple M1, NEON) | 22 | 0 | 3 | 25 of 25 |
| macOS / Clang 23.1 (Apple M1, NEON) | 23 | 2 | 0 | 25 of 25 |
| **Aggregate** | **92** | **5** | **28** | **125 of 125** |

Split by input shape (fused / tie / two-stage):

| Input shape | Windows / MSVC | Linux (Clang + GCC) | macOS (GCC + Clang) |
|---|---|---|---|
| POD-type tests | 5 / 0 / 0 | 10 / 0 / 0 | 10 / 0 / 0 |
| Minified corpus documents | 10 / 0 / 0 | 17 / 1 / 2 | 19 / 0 / 1 |
| Prettified corpus documents | 0 / 0 / 10 | 5 / 2 / 13 | 16 / 2 / 2 |

Against simdjson, counting a test for Jsonifier by the verdict of whichever of its two paths has the higher throughput against simdjson On Demand:

| Platform / Compiler | Best Jsonifier path (W / T / L) | Fused path alone (W / T / L) |
|---|---|---|
| Windows / MSVC 19.44 (i9-14900KF, AVX2) | 25 / 0 / 0 | 25 / 0 / 0 |
| Linux / Clang 24.0 (i9-14900KF, AVX2) | 25 / 0 / 0 | 21 / 0 / 4 |
| Linux / GCC 16.2 (i9-14900KF, AVX2) | 23 / 0 / 2 | 23 / 0 / 2 |
| macOS / GCC 16.2 (Apple M1, NEON) | 24 / 1 / 0 | 23 / 0 / 2 |
| macOS / Clang 23.1 (Apple M1, NEON) | 25 / 0 / 0 | 25 / 0 / 0 |
| **Aggregate** | **122 / 1 / 2** | **117 / 0 / 8** |

**Reused objects.** With the target object held across iterations:

| Platform / Compiler | Fused faster | Tie | Two-stage faster | Tests converged |
|---|---|---|---|---|
| Windows / MSVC 19.44 (i9-14900KF, AVX2) | 15 | 0 | 10 | 25 of 25 |
| Linux / Clang 24.0 (i9-14900KF, AVX2) | 18 | 1 | 6 | 25 of 25 |
| Linux / GCC 16.2 (i9-14900KF, AVX2) | 14 | 3 | 8 | 25 of 25 |
| macOS / GCC 16.2 (Apple M1, NEON) | 22 | 1 | 2 | 25 of 25 |
| macOS / Clang 23.1 (Apple M1, NEON) | 24 | 0 | 1 | 25 of 25 |
| **Aggregate** | **93** | **5** | **27** | **125 of 125** |

Split by input shape (fused / tie / two-stage):

| Input shape | Windows / MSVC | Linux (Clang + GCC) | macOS (GCC + Clang) |
|---|---|---|---|
| POD-type tests | 5 / 0 / 0 | 10 / 0 / 0 | 10 / 0 / 0 |
| Minified corpus documents | 10 / 0 / 0 | 19 / 0 / 1 | 19 / 0 / 1 |
| Prettified corpus documents | 0 / 0 / 10 | 3 / 4 / 13 | 17 / 1 / 2 |

Against simdjson, counting a test for Jsonifier by the verdict of whichever of its two paths has the higher throughput against simdjson On Demand:

| Platform / Compiler | Best Jsonifier path (W / T / L) | Fused path alone (W / T / L) |
|---|---|---|
| Windows / MSVC 19.44 (i9-14900KF, AVX2) | 25 / 0 / 0 | 25 / 0 / 0 |
| Linux / Clang 24.0 (i9-14900KF, AVX2) | 24 / 0 / 1 | 21 / 0 / 4 |
| Linux / GCC 16.2 (i9-14900KF, AVX2) | 23 / 0 / 2 | 23 / 0 / 2 |
| macOS / GCC 16.2 (Apple M1, NEON) | 23 / 1 / 1 | 23 / 0 / 2 |
| macOS / Clang 23.1 (Apple M1, NEON) | 25 / 0 / 0 | 24 / 0 / 1 |
| **Aggregate** | **120 / 1 / 4** | **116 / 0 / 9** |

Across 125 converged tests the fused path wins 93, ties 5 and loses 27 to the two-stage path, and the x86 split holds: the tape wins 23 of the 30 prettified documents, as it does freshly allocated, and takes minified Mesh on Linux/GCC by 7%. On x86 the fused path wins prettified Google Maps, Marine IK Reverse and Marine IK on Linux/Clang, and the other four prettified x86 results are ties: Canada on Linux/Clang, and Google Maps, Marine IK Reverse and Marine IK on Linux/GCC. On the M1 the tape takes prettified Mesh on both builds (by 9% on macOS/GCC and 57% on macOS/Clang) and minified Mesh on macOS/GCC (by 3%), and prettified Canada on macOS/GCC is a tie. Against simdjson, the faster Jsonifier path wins 120, ties 1 and loses 4, and the fused path alone wins 116 and loses 9. Taking allocation out of the timed region helps every library, so it moves individual margins; on x86 it changes the verdict between the two paths on four tests: prettified Canada on Linux/Clang goes from the fused path to a tie, minified Canada on Linux/GCC from the tape to the fused path, prettified Google Maps on Linux/GCC from the fused path to a tie, and minified Random on Linux/GCC from a tie to the fused path. Under MSVC no verdict changes.

### 2.3 Per-test results

Throughput is in MB/s. "Faster Jsonifier path" is the Welch's t-test verdict between the two Jsonifier paths, "Best Jsonifier path vs simdjson" is the verdict for whichever of the two has the higher throughput.

#### Freshly allocated objects

**Windows / MSVC 19.44 (i9-14900KF, AVX2)**

| Test | Fused (MB/s) | Two-stage (MB/s) | simdjson (MB/s) | Fused ÷ two-stage | Faster Jsonifier path | Best Jsonifier path vs simdjson |
|---|---|---|---|---|---|---|
| Bool (POD) | 519 | 134 | 125 | 3.88× | Fused | Win |
| Double (POD) | 529 | 300 | 168 | 1.76× | Fused | Win |
| Int64 (POD) | 1,512 | 548 | 378 | 2.76× | Fused | Win |
| String (POD) | 967 | 695 | 637 | 1.39× | Fused | Win |
| Uint64 (POD) | 1,541 | 562 | 378 | 2.74× | Fused | Win |
| Canada (minified) | 608 | 574 | 368 | 1.06× | Fused | Win |
| Canada (prettified) | 1,588 | 1,681 | 1,143 | 0.94× | Two-stage | Win |
| CitmCatalog (minified) | 1,515 | 1,266 | 524 | 1.20× | Fused | Win |
| CitmCatalog (prettified) | 2,785 | 3,098 | 1,411 | 0.90× | Two-stage | Win |
| Discord (minified) | 1,121 | 1,017 | 623 | 1.10× | Fused | Win |
| Discord (prettified) | 1,379 | 1,573 | 1,005 | 0.88× | Two-stage | Win |
| Google Maps Response (minified) | 1,193 | 1,155 | 515 | 1.03× | Fused | Win |
| Google Maps Response (prettified) | 2,286 | 2,693 | 1,293 | 0.85× | Two-stage | Win |
| Instruments (minified) | 2,041 | 1,416 | 690 | 1.44× | Fused | Win |
| Instruments (prettified) | 2,119 | 2,552 | 1,287 | 0.83× | Two-stage | Win |
| Marine IK Reverse (minified) | 508 | 443 | 37 | 1.15× | Fused | Win |
| Marine IK Reverse (prettified) | 2,090 | 2,215 | 197 | 0.94× | Two-stage | Win |
| Marine IK (minified) | 514 | 456 | 260 | 1.13× | Fused | Win |
| Marine IK (prettified) | 2,079 | 2,255 | 1,290 | 0.92× | Two-stage | Win |
| Mesh (minified) | 950 | 825 | 432 | 1.15× | Fused | Win |
| Mesh (prettified) | 1,194 | 1,466 | 796 | 0.81× | Two-stage | Win |
| Random (minified) | 912 | 887 | 519 | 1.03× | Fused | Win |
| Random (prettified) | 1,396 | 1,595 | 964 | 0.88× | Two-stage | Win |
| Twitter (minified) | 1,255 | 1,141 | 801 | 1.10× | Fused | Win |
| Twitter (prettified) | 1,418 | 1,644 | 1,126 | 0.86× | Two-stage | Win |

**Linux / Clang 24.0 (i9-14900KF, AVX2)**

| Test | Fused (MB/s) | Two-stage (MB/s) | simdjson (MB/s) | Fused ÷ two-stage | Faster Jsonifier path | Best Jsonifier path vs simdjson |
|---|---|---|---|---|---|---|
| Bool (POD) | 1,752 | 353 | 214 | 4.97× | Fused | Win |
| Double (POD) | 1,259 | 443 | 272 | 2.84× | Fused | Win |
| Int64 (POD) | 2,942 | 1,013 | 559 | 2.90× | Fused | Win |
| String (POD) | 1,792 | 1,165 | 1,108 | 1.54× | Fused | Win |
| Uint64 (POD) | 3,253 | 1,013 | 578 | 3.21× | Fused | Win |
| Canada (minified) | 1,005 | 907 | 650 | 1.11× | Fused | Win |
| Canada (prettified) | 2,646 | 2,557 | 1,952 | 1.03× | Fused | Win |
| CitmCatalog (minified) | 2,451 | 1,822 | 1,220 | 1.35× | Fused | Win |
| CitmCatalog (prettified) | 3,912 | 4,191 | 3,104 | 0.93× | Two-stage | Win |
| Discord (minified) | 2,219 | 1,817 | 1,452 | 1.22× | Fused | Win |
| Discord (prettified) | 2,143 | 2,793 | 2,372 | 0.77× | Two-stage | Win |
| Google Maps Response (minified) | 2,611 | 1,947 | 1,358 | 1.34× | Fused | Win |
| Google Maps Response (prettified) | 3,633 | 3,186 | 3,243 | 1.14× | Fused | Win |
| Instruments (minified) | 2,634 | 1,979 | 1,728 | 1.33× | Fused | Win |
| Instruments (prettified) | 2,776 | 3,366 | 3,147 | 0.82× | Two-stage | Win |
| Marine IK Reverse (minified) | 844 | 753 | 602 | 1.12× | Fused | Win |
| Marine IK Reverse (prettified) | 3,440 | 3,335 | 2,792 | 1.03× | Fused | Win |
| Marine IK (minified) | 871 | 747 | 595 | 1.17× | Fused | Win |
| Marine IK (prettified) | 3,501 | 3,334 | 2,809 | 1.05× | Fused | Win |
| Mesh (minified) | 1,396 | 1,234 | 1,158 | 1.13× | Fused | Win |
| Mesh (prettified) | 2,061 | 2,130 | 2,113 | 0.97× | Two-stage | Win |
| Random (minified) | 1,657 | 1,405 | 1,091 | 1.18× | Fused | Win |
| Random (prettified) | 2,245 | 2,445 | 1,943 | 0.92× | Two-stage | Win |
| Twitter (minified) | 1,982 | 1,808 | 1,735 | 1.10× | Fused | Win |
| Twitter (prettified) | 1,977 | 2,531 | 2,464 | 0.78× | Two-stage | Win |

**Linux / GCC 16.2 (i9-14900KF, AVX2)**

| Test | Fused (MB/s) | Two-stage (MB/s) | simdjson (MB/s) | Fused ÷ two-stage | Faster Jsonifier path | Best Jsonifier path vs simdjson |
|---|---|---|---|---|---|---|
| Bool (POD) | 1,587 | 259 | 183 | 6.12× | Fused | Win |
| Double (POD) | 1,154 | 328 | 228 | 3.53× | Fused | Win |
| Int64 (POD) | 3,308 | 820 | 517 | 4.04× | Fused | Win |
| String (POD) | 1,770 | 1,050 | 1,018 | 1.69× | Fused | Win |
| Uint64 (POD) | 3,349 | 838 | 518 | 4.00× | Fused | Win |
| Canada (minified) | 908 | 918 | 775 | 0.99× | Two-stage | Win |
| Canada (prettified) | 2,420 | 2,600 | 2,287 | 0.93× | Two-stage | Win |
| CitmCatalog (minified) | 2,445 | 1,883 | 1,375 | 1.30× | Fused | Win |
| CitmCatalog (prettified) | 4,000 | 4,301 | 3,353 | 0.93× | Two-stage | Win |
| Discord (minified) | 2,164 | 1,860 | 1,465 | 1.16× | Fused | Win |
| Discord (prettified) | 2,451 | 2,834 | 2,261 | 0.87× | Two-stage | Win |
| Google Maps Response (minified) | 2,621 | 1,892 | 1,113 | 1.39× | Fused | Win |
| Google Maps Response (prettified) | 4,180 | 4,168 | 2,651 | 1.00× | Fused | Win |
| Instruments (minified) | 3,291 | 2,266 | 1,401 | 1.45× | Fused | Win |
| Instruments (prettified) | 3,392 | 3,817 | 2,613 | 0.89× | Two-stage | Win |
| Marine IK Reverse (minified) | 761 | 709 | 602 | 1.07× | Fused | Win |
| Marine IK Reverse (prettified) | 3,242 | 3,253 | 2,792 | 1.00× | Tie | Win |
| Marine IK (minified) | 761 | 723 | 600 | 1.05× | Fused | Win |
| Marine IK (prettified) | 3,253 | 3,240 | 2,685 | 1.00× | Tie | Win |
| Mesh (minified) | 1,091 | 1,151 | 1,212 | 0.95× | Two-stage | Loss |
| Mesh (prettified) | 1,810 | 2,047 | 2,162 | 0.88× | Two-stage | Loss |
| Random (minified) | 1,426 | 1,424 | 859 | 1.00× | Tie | Win |
| Random (prettified) | 2,055 | 2,444 | 1,580 | 0.84× | Two-stage | Win |
| Twitter (minified) | 2,041 | 1,766 | 1,319 | 1.16× | Fused | Win |
| Twitter (prettified) | 2,023 | 2,469 | 1,923 | 0.82× | Two-stage | Win |

**macOS / GCC 16.2 (Apple M1, NEON)**

| Test | Fused (MB/s) | Two-stage (MB/s) | simdjson (MB/s) | Fused ÷ two-stage | Faster Jsonifier path | Best Jsonifier path vs simdjson |
|---|---|---|---|---|---|---|
| Bool (POD) | 1,615 | 196 | 180 | 8.23× | Fused | Win |
| Double (POD) | 848 | 309 | 123 | 2.74× | Fused | Win |
| Int64 (POD) | 2,172 | 649 | 474 | 3.35× | Fused | Win |
| String (POD) | 1,135 | 704 | 788 | 1.61× | Fused | Win |
| Uint64 (POD) | 2,205 | 661 | 447 | 3.34× | Fused | Win |
| Canada (minified) | 651 | 538 | 417 | 1.21× | Fused | Win |
| Canada (prettified) | 1,427 | 1,350 | 1,129 | 1.06× | Fused | Win |
| CitmCatalog (minified) | 1,762 | 1,119 | 937 | 1.57× | Fused | Win |
| CitmCatalog (prettified) | 2,847 | 2,523 | 2,192 | 1.13× | Fused | Win |
| Discord (minified) | 1,495 | 1,038 | 870 | 1.44× | Fused | Win |
| Discord (prettified) | 1,752 | 1,495 | 1,243 | 1.17× | Fused | Win |
| Google Maps Response (minified) | 1,590 | 1,025 | 689 | 1.55× | Fused | Win |
| Google Maps Response (prettified) | 1,241 | 2,055 | 1,561 | 0.60× | Two-stage | Win |
| Instruments (minified) | 2,448 | 1,352 | 943 | 1.81× | Fused | Win |
| Instruments (prettified) | 2,483 | 2,111 | 1,737 | 1.18× | Fused | Win |
| Marine IK Reverse (minified) | 596 | 484 | 424 | 1.23× | Fused | Win |
| Marine IK Reverse (prettified) | 2,330 | 1,969 | 1,832 | 1.18× | Fused | Win |
| Marine IK (minified) | 631 | 512 | 422 | 1.23× | Fused | Win |
| Marine IK (prettified) | 2,415 | 2,247 | 1,836 | 1.07× | Fused | Win |
| Mesh (minified) | 811 | 833 | 795 | 0.97× | Two-stage | Win |
| Mesh (prettified) | 1,341 | 1,440 | 1,444 | 0.93× | Two-stage | Tie |
| Random (minified) | 1,136 | 834 | 616 | 1.36× | Fused | Win |
| Random (prettified) | 1,635 | 1,263 | 943 | 1.30× | Fused | Win |
| Twitter (minified) | 1,746 | 1,261 | 879 | 1.38× | Fused | Win |
| Twitter (prettified) | 2,076 | 1,790 | 1,471 | 1.16× | Fused | Win |

**macOS / Clang 23.1 (Apple M1, NEON)**

| Test | Fused (MB/s) | Two-stage (MB/s) | simdjson (MB/s) | Fused ÷ two-stage | Faster Jsonifier path | Best Jsonifier path vs simdjson |
|---|---|---|---|---|---|---|
| Bool (POD) | 1,058 | 204 | 145 | 5.20× | Fused | Win |
| Double (POD) | 799 | 263 | 164 | 3.04× | Fused | Win |
| Int64 (POD) | 2,009 | 528 | 413 | 3.80× | Fused | Win |
| String (POD) | 1,129 | 576 | 700 | 1.96× | Fused | Win |
| Uint64 (POD) | 1,758 | 521 | 414 | 3.38× | Fused | Win |
| Canada (minified) | 657 | 600 | 399 | 1.09× | Fused | Win |
| Canada (prettified) | 1,718 | 1,612 | 1,187 | 1.07× | Fused | Win |
| CitmCatalog (minified) | 1,636 | 1,247 | 841 | 1.31× | Fused | Win |
| CitmCatalog (prettified) | 3,062 | 2,802 | 2,151 | 1.09× | Fused | Win |
| Discord (minified) | 1,778 | 835 | 1,057 | 2.13× | Fused | Win |
| Discord (prettified) | 1,525 | 1,517 | 1,435 | 1.00× | Tie | Win |
| Google Maps Response (minified) | 1,829 | 996 | 720 | 1.84× | Fused | Win |
| Google Maps Response (prettified) | 2,940 | 1,923 | 1,789 | 1.53× | Fused | Win |
| Instruments (minified) | 2,304 | 1,457 | 1,087 | 1.58× | Fused | Win |
| Instruments (prettified) | 2,737 | 2,377 | 1,932 | 1.15× | Fused | Win |
| Marine IK Reverse (minified) | 580 | 494 | 346 | 1.17× | Fused | Win |
| Marine IK Reverse (prettified) | 2,085 | 1,913 | 1,601 | 1.09× | Fused | Win |
| Marine IK (minified) | 589 | 496 | 345 | 1.19× | Fused | Win |
| Marine IK (prettified) | 2,355 | 2,177 | 1,575 | 1.08× | Fused | Win |
| Mesh (minified) | 981 | 747 | 592 | 1.31× | Fused | Win |
| Mesh (prettified) | 1,455 | 1,456 | 1,347 | 1.00× | Tie | Win |
| Random (minified) | 1,639 | 879 | 782 | 1.87× | Fused | Win |
| Random (prettified) | 2,308 | 1,207 | 1,165 | 1.91× | Fused | Win |
| Twitter (minified) | 2,090 | 1,562 | 1,042 | 1.34× | Fused | Win |
| Twitter (prettified) | 2,167 | 1,984 | 1,973 | 1.09× | Fused | Win |

#### Reused objects

**Windows / MSVC 19.44 (i9-14900KF, AVX2)**

| Test | Fused (MB/s) | Two-stage (MB/s) | simdjson (MB/s) | Fused ÷ two-stage | Faster Jsonifier path | Best Jsonifier path vs simdjson |
|---|---|---|---|---|---|---|
| Bool (POD) | 641 | 142 | 131 | 4.51× | Fused | Win |
| Double (POD) | 685 | 328 | 176 | 2.09× | Fused | Win |
| Int64 (POD) | 1,980 | 599 | 404 | 3.31× | Fused | Win |
| String (POD) | 1,686 | 1,007 | 896 | 1.67× | Fused | Win |
| Uint64 (POD) | 1,994 | 627 | 407 | 3.18× | Fused | Win |
| Canada (minified) | 721 | 670 | 412 | 1.08× | Fused | Win |
| Canada (prettified) | 1,844 | 1,999 | 1,260 | 0.92× | Two-stage | Win |
| CitmCatalog (minified) | 1,825 | 1,468 | 569 | 1.24× | Fused | Win |
| CitmCatalog (prettified) | 2,969 | 3,489 | 1,472 | 0.85× | Two-stage | Win |
| Discord (minified) | 1,600 | 1,402 | 786 | 1.14× | Fused | Win |
| Discord (prettified) | 1,861 | 2,209 | 1,259 | 0.84× | Two-stage | Win |
| Google Maps Response (minified) | 1,371 | 1,295 | 565 | 1.06× | Fused | Win |
| Google Maps Response (prettified) | 2,498 | 2,979 | 1,391 | 0.84× | Two-stage | Win |
| Instruments (minified) | 2,642 | 1,682 | 768 | 1.57× | Fused | Win |
| Instruments (prettified) | 2,426 | 2,962 | 1,431 | 0.82× | Two-stage | Win |
| Marine IK Reverse (minified) | 549 | 497 | 37 | 1.11× | Fused | Win |
| Marine IK Reverse (prettified) | 2,178 | 2,338 | 195 | 0.93× | Two-stage | Win |
| Marine IK (minified) | 555 | 506 | 268 | 1.10× | Fused | Win |
| Marine IK (prettified) | 2,196 | 2,411 | 1,350 | 0.91× | Two-stage | Win |
| Mesh (minified) | 1,002 | 860 | 466 | 1.16× | Fused | Win |
| Mesh (prettified) | 1,220 | 1,554 | 859 | 0.79× | Two-stage | Win |
| Random (minified) | 1,259 | 1,221 | 636 | 1.03× | Fused | Win |
| Random (prettified) | 1,799 | 2,153 | 1,162 | 0.84× | Two-stage | Win |
| Twitter (minified) | 1,804 | 1,573 | 1,002 | 1.15× | Fused | Win |
| Twitter (prettified) | 1,816 | 2,241 | 1,425 | 0.81× | Two-stage | Win |

**Linux / Clang 24.0 (i9-14900KF, AVX2)**

| Test | Fused (MB/s) | Two-stage (MB/s) | simdjson (MB/s) | Fused ÷ two-stage | Faster Jsonifier path | Best Jsonifier path vs simdjson |
|---|---|---|---|---|---|---|
| Bool (POD) | 1,829 | 358 | 217 | 5.11× | Fused | Win |
| Double (POD) | 1,391 | 458 | 276 | 3.04× | Fused | Win |
| Int64 (POD) | 3,274 | 1,048 | 564 | 3.12× | Fused | Win |
| String (POD) | 2,828 | 1,516 | 1,417 | 1.86× | Fused | Win |
| Uint64 (POD) | 3,573 | 1,054 | 587 | 3.39× | Fused | Win |
| Canada (minified) | 1,218 | 1,086 | 764 | 1.12× | Fused | Win |
| Canada (prettified) | 3,084 | 3,080 | 2,237 | 1.00× | Tie | Win |
| CitmCatalog (minified) | 3,013 | 2,155 | 1,409 | 1.40× | Fused | Win |
| CitmCatalog (prettified) | 4,401 | 4,847 | 3,562 | 0.91× | Two-stage | Win |
| Discord (minified) | 3,277 | 2,476 | 1,897 | 1.32× | Fused | Win |
| Discord (prettified) | 2,603 | 3,730 | 2,942 | 0.70× | Two-stage | Win |
| Google Maps Response (minified) | 2,993 | 2,153 | 1,479 | 1.39× | Fused | Win |
| Google Maps Response (prettified) | 3,891 | 3,409 | 3,461 | 1.14× | Fused | Win |
| Instruments (minified) | 3,123 | 2,253 | 1,894 | 1.39× | Fused | Win |
| Instruments (prettified) | 3,037 | 3,766 | 3,436 | 0.81× | Two-stage | Win |
| Marine IK Reverse (minified) | 938 | 820 | 652 | 1.14× | Fused | Win |
| Marine IK Reverse (prettified) | 3,745 | 3,619 | 3,035 | 1.03× | Fused | Win |
| Marine IK (minified) | 966 | 844 | 659 | 1.14× | Fused | Win |
| Marine IK (prettified) | 3,805 | 3,665 | 3,044 | 1.04× | Fused | Win |
| Mesh (minified) | 1,496 | 1,301 | 1,291 | 1.15× | Fused | Win |
| Mesh (prettified) | 2,176 | 2,236 | 2,365 | 0.97× | Two-stage | Loss |
| Random (minified) | 2,546 | 2,010 | 1,410 | 1.27× | Fused | Win |
| Random (prettified) | 2,959 | 3,370 | 2,508 | 0.88× | Two-stage | Win |
| Twitter (minified) | 2,501 | 2,444 | 2,147 | 1.02× | Fused | Win |
| Twitter (prettified) | 2,293 | 3,375 | 3,030 | 0.68× | Two-stage | Win |

**Linux / GCC 16.2 (i9-14900KF, AVX2)**

| Test | Fused (MB/s) | Two-stage (MB/s) | simdjson (MB/s) | Fused ÷ two-stage | Faster Jsonifier path | Best Jsonifier path vs simdjson |
|---|---|---|---|---|---|---|
| Bool (POD) | 1,743 | 261 | 189 | 6.68× | Fused | Win |
| Double (POD) | 1,257 | 332 | 232 | 3.79× | Fused | Win |
| Int64 (POD) | 3,635 | 845 | 523 | 4.30× | Fused | Win |
| String (POD) | 2,621 | 1,303 | 1,296 | 2.01× | Fused | Win |
| Uint64 (POD) | 3,733 | 859 | 524 | 4.35× | Fused | Win |
| Canada (minified) | 1,103 | 1,092 | 934 | 1.01× | Fused | Win |
| Canada (prettified) | 2,784 | 3,040 | 2,702 | 0.92× | Two-stage | Win |
| CitmCatalog (minified) | 3,099 | 2,257 | 1,634 | 1.37× | Fused | Win |
| CitmCatalog (prettified) | 4,497 | 4,995 | 3,972 | 0.90× | Two-stage | Win |
| Discord (minified) | 3,258 | 2,451 | 1,808 | 1.33× | Fused | Win |
| Discord (prettified) | 3,004 | 3,689 | 2,852 | 0.81× | Two-stage | Win |
| Google Maps Response (minified) | 3,088 | 2,123 | 1,175 | 1.45× | Fused | Win |
| Google Maps Response (prettified) | 4,534 | 4,539 | 2,786 | 1.00× | Tie | Win |
| Instruments (minified) | 3,969 | 2,561 | 1,491 | 1.55× | Fused | Win |
| Instruments (prettified) | 3,713 | 4,290 | 2,745 | 0.87× | Two-stage | Win |
| Marine IK Reverse (minified) | 835 | 777 | 664 | 1.07× | Fused | Win |
| Marine IK Reverse (prettified) | 3,512 | 3,507 | 3,003 | 1.00× | Tie | Win |
| Marine IK (minified) | 850 | 800 | 663 | 1.06× | Fused | Win |
| Marine IK (prettified) | 3,556 | 3,546 | 2,957 | 1.00× | Tie | Win |
| Mesh (minified) | 1,128 | 1,209 | 1,327 | 0.93× | Two-stage | Loss |
| Mesh (prettified) | 1,869 | 2,160 | 2,421 | 0.87× | Two-stage | Loss |
| Random (minified) | 2,015 | 1,965 | 1,078 | 1.03× | Fused | Win |
| Random (prettified) | 2,706 | 3,321 | 1,930 | 0.81× | Two-stage | Win |
| Twitter (minified) | 2,615 | 2,212 | 1,560 | 1.18× | Fused | Win |
| Twitter (prettified) | 2,357 | 2,952 | 2,262 | 0.80× | Two-stage | Win |

**macOS / GCC 16.2 (Apple M1, NEON)**

| Test | Fused (MB/s) | Two-stage (MB/s) | simdjson (MB/s) | Fused ÷ two-stage | Faster Jsonifier path | Best Jsonifier path vs simdjson |
|---|---|---|---|---|---|---|
| Bool (POD) | 1,450 | 235 | 175 | 6.17× | Fused | Win |
| Double (POD) | 993 | 282 | 153 | 3.52× | Fused | Win |
| Int64 (POD) | 2,629 | 644 | 428 | 4.08× | Fused | Win |
| String (POD) | 1,817 | 934 | 1,012 | 1.94× | Fused | Win |
| Uint64 (POD) | 2,734 | 688 | 458 | 3.97× | Fused | Win |
| Canada (minified) | 792 | 632 | 331 | 1.25× | Fused | Win |
| Canada (prettified) | 1,708 | 1,711 | 1,487 | 1.00× | Tie | Win |
| CitmCatalog (minified) | 2,167 | 1,305 | 1,069 | 1.66× | Fused | Win |
| CitmCatalog (prettified) | 3,380 | 2,864 | 2,398 | 1.18× | Fused | Win |
| Discord (minified) | 2,088 | 1,273 | 1,001 | 1.64× | Fused | Win |
| Discord (prettified) | 2,220 | 1,832 | 1,517 | 1.21× | Fused | Win |
| Google Maps Response (minified) | 2,107 | 1,112 | 640 | 1.89× | Fused | Win |
| Google Maps Response (prettified) | 3,065 | 2,211 | 1,673 | 1.39× | Fused | Win |
| Instruments (minified) | 2,823 | 1,431 | 1,022 | 1.97× | Fused | Win |
| Instruments (prettified) | 2,703 | 2,257 | 1,928 | 1.20× | Fused | Win |
| Marine IK Reverse (minified) | 683 | 539 | 466 | 1.27× | Fused | Win |
| Marine IK Reverse (prettified) | 2,536 | 2,344 | 1,986 | 1.08× | Fused | Win |
| Marine IK (minified) | 731 | 551 | 423 | 1.33× | Fused | Win |
| Marine IK (prettified) | 2,685 | 2,466 | 1,970 | 1.09× | Fused | Win |
| Mesh (minified) | 854 | 882 | 877 | 0.97× | Two-stage | Tie |
| Mesh (prettified) | 1,367 | 1,491 | 1,570 | 0.92× | Two-stage | Loss |
| Random (minified) | 1,791 | 1,196 | 729 | 1.50× | Fused | Win |
| Random (prettified) | 2,209 | 1,922 | 1,075 | 1.15× | Fused | Win |
| Twitter (minified) | 2,131 | 1,673 | 1,047 | 1.27× | Fused | Win |
| Twitter (prettified) | 2,432 | 2,357 | 1,724 | 1.03× | Fused | Win |

**macOS / Clang 23.1 (Apple M1, NEON)**

| Test | Fused (MB/s) | Two-stage (MB/s) | simdjson (MB/s) | Fused ÷ two-stage | Faster Jsonifier path | Best Jsonifier path vs simdjson |
|---|---|---|---|---|---|---|
| Bool (POD) | 1,069 | 176 | 162 | 6.06× | Fused | Win |
| Double (POD) | 940 | 252 | 177 | 3.73× | Fused | Win |
| Int64 (POD) | 2,454 | 537 | 470 | 4.57× | Fused | Win |
| String (POD) | 1,910 | 841 | 1,001 | 2.27× | Fused | Win |
| Uint64 (POD) | 2,294 | 412 | 431 | 5.56× | Fused | Win |
| Canada (minified) | 892 | 710 | 427 | 1.26× | Fused | Win |
| Canada (prettified) | 2,213 | 2,002 | 1,487 | 1.11× | Fused | Win |
| CitmCatalog (minified) | 1,900 | 1,374 | 741 | 1.38× | Fused | Win |
| CitmCatalog (prettified) | 3,839 | 3,541 | 2,428 | 1.08× | Fused | Win |
| Discord (minified) | 2,218 | 1,253 | 1,074 | 1.77× | Fused | Win |
| Discord (prettified) | 2,729 | 1,942 | 1,859 | 1.41× | Fused | Win |
| Google Maps Response (minified) | 1,916 | 832 | 912 | 2.30× | Fused | Win |
| Google Maps Response (prettified) | 3,063 | 2,149 | 2,127 | 1.43× | Fused | Win |
| Instruments (minified) | 2,768 | 1,523 | 1,063 | 1.82× | Fused | Win |
| Instruments (prettified) | 3,147 | 2,605 | 2,012 | 1.21× | Fused | Win |
| Marine IK Reverse (minified) | 653 | 560 | 386 | 1.17× | Fused | Win |
| Marine IK Reverse (prettified) | 2,513 | 2,202 | 1,739 | 1.14× | Fused | Win |
| Marine IK (minified) | 677 | 509 | 387 | 1.33× | Fused | Win |
| Marine IK (prettified) | 2,727 | 2,326 | 1,783 | 1.17× | Fused | Win |
| Mesh (minified) | 1,023 | 748 | 599 | 1.37× | Fused | Win |
| Mesh (prettified) | 990 | 1,557 | 1,320 | 0.64× | Two-stage | Win |
| Random (minified) | 2,372 | 1,046 | 879 | 2.27× | Fused | Win |
| Random (prettified) | 2,700 | 1,664 | 1,699 | 1.62× | Fused | Win |
| Twitter (minified) | 2,229 | 1,820 | 1,558 | 1.23× | Fused | Win |
| Twitter (prettified) | 2,624 | 1,954 | 2,092 | 1.34× | Fused | Win |

### 2.4 What the results show

Which Jsonifier path is faster depends on two things: whether the document is indented, and whether the machine is x86. On the three x86 builds the fused path wins all POD input and 27 of the 30 minified documents, with two going to the tape and one tie, while the tape wins 23 of the 30 prettified documents, the fused path wins 5 and 2 are ties. MSVC splits cleanly: minified to the fused path, prettified to the tape. On the M1 the fused path wins all POD input, 19 of the 20 minified documents (one to the tape) and 16 of the 20 prettified documents (two to the tape, two ties).

**POD-type tests: the fused path wins all 25 freshly allocated and all 25 reused, on every build.** It runs them 1.4× (String, Windows/MSVC) to 8.2× (Bool, macOS/GCC) faster than the two-stage path, and 1.7× to 6.7× faster with objects reused. These documents are the case §2 argues from: nothing to skip, every value materialized, so the tape is pure overhead. This is the one result that does not depend on the platform.

**Minified documents: the fused path's, apart from Canada and Mesh on Linux/GCC and Mesh on macOS/GCC.** The fused path wins 46 of the 50 minified documents: all 10 under MSVC, all 10 on Linux/Clang, 7 of 10 on Linux/GCC, 9 of 10 on macOS/GCC and all 10 on macOS/Clang. On Linux/GCC the two-stage path wins minified Canada by 1% (918 against 908 MB/s) and minified Mesh by 6%, and minified Random is a tie (1,426 against 1,424 MB/s). On macOS/GCC the two-stage path wins minified Mesh by 3% (833 against 811 MB/s). The fused path's margins run from 3% (Google Maps and Random under MSVC) to 113% (Discord on macOS/Clang, 1,778 MB/s against the two-stage path's 835); on Linux/GCC they are widest on Google Maps (39%) and Instruments (45%), and under MSVC on CitmCatalog (20%) and Instruments (44%). This is the workload the fused key literals of §3 were built for: machine-generated JSON in declared order.

**Prettified documents on x86: the two-stage path wins 23 of 30.** It wins all 10 under MSVC, by 6% (Canada and Marine IK Reverse) to 23% (Mesh); 6 of 10 on Linux/Clang, by 3% (Mesh) to 30% (Discord); and 7 of 10 on Linux/GCC, by 7% (Canada) to 22% (Twitter). The fused path wins the other four on Linux/Clang, Canada and Marine IK Reverse by 3%, Marine IK by 5% and Google Maps by 14%, and wins prettified Google Maps on Linux/GCC by under 1%. Prettified Marine IK Reverse and Marine IK on Linux/GCC are ties (3,242 against 3,253 MB/s, and 3,253 against 3,240). With objects reused the tape again wins 23 of 30, the fused path wins 3 (all on Linux/Clang: Google Maps by 14%, Marine IK Reverse and Marine IK by 3% and 4%) and 4 are ties. The two Marine IK documents, which are mostly numbers, are among the closest on every x86 build (a 6% to 8% tape win under MSVC, a fused win of 3% to 5% on Linux/Clang, a tie on Linux/GCC), and Twitter is among the widest on Linux/Clang and Linux/GCC (28% and 22%). The fused path already predicts the indentation of every line, including the lines that close an object or an array, matches the `": "` after each key as one two-byte constant, and verifies each predicted span with a 16-byte vector loop, a single 8-byte SWAR step and a scalar remainder switch (§3). That keeps the number-heavy documents close, and on Linux/Clang it wins them, but across the three builds it does not win most prettified documents. Stage 1 classifies every byte, whitespace included, in vector blocks at a cost that does not depend on layout, while the fused path still does a small amount of branchy work per line: the newline test, the depth multiplication, the span check and the branch on its result. On a heavily indented document that per-line cost appears to outweigh the second pass over the input that the tape costs. We have not isolated this, and state it as a hypothesis. We also have not isolated why Google Maps goes to the fused path on Linux/Clang by 14% but to the tape by 18% under MSVC.

**Prettified documents on the M1: mostly the fused path.** Of the 20 prettified documents, the fused path wins 16, the tape wins 2 and two are ties. On macOS/GCC the fused path wins 8 of 10, by 6% (Canada) to 30% (Random), and the tape wins Google Maps by 66% (2,055 against 1,241 MB/s) and Mesh by 7% (1,440 against 1,341). On macOS/Clang the fused path wins 8 of 10, by 7% (Canada) to 91% (Random, 2,308 against 1,207 MB/s), and prettified Discord and Mesh are ties (1,525 against 1,517 MB/s, and 1,455 against 1,456). Reused, the fused path wins 17 of the 20, the tape wins 2 (Mesh on both builds, by 9% on macOS/GCC and 57% on macOS/Clang) and one is a tie (Canada on macOS/GCC); the Google Maps result on macOS/GCC does not survive reuse, where the fused path wins it by 39%. NEON escapes most of the prettified penalty seen on x86, and we do not have an explanation for why. One contributing factor is that Jsonifier's stage 1 is comparatively more expensive there, since NEON has no `movemask` and the collectors emulate it with narrowing shifts, but we have not isolated it.

**Mesh is where Jsonifier is weakest.** The two-stage path wins prettified Mesh on all three x86 builds, by 3% (Linux/Clang), 13% (Linux/GCC) and 23% (Windows/MSVC), and minified Mesh on Linux/GCC by 6%. On macOS/GCC it wins Mesh by 3% minified and 7% prettified, and prettified Mesh on macOS/Clang is a tie. Mesh is also the only document on which both Jsonifier paths lose to simdjson On Demand. Freshly allocated, that happens on Linux/GCC in both forms, by 5% minified (1,212 against 1,151 MB/s) and 6% prettified (2,162 against 2,047); on macOS/GCC prettified Mesh is a tie against it (1,440 against 1,444 MB/s). With objects reused it loses on Linux/GCC by 10% minified and 12% prettified, on macOS/GCC prettified by 5%, and on Linux/Clang prettified by 6%, and minified Mesh on macOS/GCC is a tie (882 against 877 MB/s). We have not profiled what is particular about this document.

**MSVC slows simdjson more than either Jsonifier path.** All libraries run slower under MSVC than under Linux/Clang on the same i9-14900KF, but not by the same amount. Across the 9 minified documents in declared key order, the fused path reaches a median 60% of its Linux/Clang throughput, the two-stage path 63% and simdjson 43%. Because the two Jsonifier paths lose about the same share, MSVC routes exactly as Linux/Clang does: minified to the fused path, prettified to the tape. The spread between documents is wider for the fused path (46% on Google Maps to 77% on Instruments) than for the two-stage path (56% to 72%). That fits the fused path being branch-heavy code whose performance rests on the optimizer while stage 1 is branch-free intrinsic code whose shape is fixed by the source, but we have not isolated it.

**Against simdjson, Jsonifier wins 122 of 125 tests with whichever path is faster, ties one and loses two.** It loses two, Mesh in both forms on Linux/GCC, and ties prettified Mesh on macOS/GCC. The fused path alone wins 117 and loses 8: those two Linux/GCC losses, prettified Discord, Instruments, Mesh and Twitter on Linux/Clang, and prettified Google Maps and Mesh on macOS/GCC.

**Reverse key order.** Requesting every key in reverse order is meant to force simdjson's On Demand API into the rescanning behavior described in §3, and under MSVC it does: simdjson parses minified Marine IK Reverse at 37 MB/s, against 260 MB/s for the same document in declared order, so the faster Jsonifier path is 13.7× faster than simdjson freshly allocated (14.7× reused). The prettified form shows the same gap, at 197 MB/s against 1,290, and the faster Jsonifier path is 11.2× faster (12.0× reused). On the other builds simdjson parses the reversed and declared-order documents within 11% of each other, and the faster Jsonifier path is 1.17× to 1.68× faster than simdjson freshly allocated and 1.17× to 1.69× reused. Why the rescan cost shows up only under MSVC has not been profiled.

The routing implication is sharper than §2's rule of thumb. On x86 the fused path is right for POD and minified input, apart from Canada and Mesh on Linux/GCC, and prettified input mostly belongs to the two-stage path on all three compilers, though not on Linux/Clang for the number-heavy documents. On the M1 the fused path is right for POD and minified input, apart from Mesh on macOS/GCC, and for most prettified documents, but not all of them. Routing each test to its faster path would raise the record against simdjson from 117 wins and 8 losses to 122 wins, 1 tie and 2 losses. Six tests change hands, all prettified: four on Linux/Clang and two on macOS/GCC, of which five become wins and prettified Mesh on macOS/GCC becomes a tie. The router does not yet take the architecture or the indentation into account.

### 2.5 Stage 1 + reflection: two-stage Jsonifier against simdjson's reflection reader

simdjson 5 adds a C++26 static-reflection reader (`document.get<T>()`): stage 1 builds the structural index, then the reflected type drives materialization. That is the same shape as Jsonifier's two-stage path, a tape plus a compile-time schema, so ranking those two head to head compares the two stage-1 + reflection designs directly. The reader needs P2996 reflection, so it only runs on GCC, and the harness registers it for corpus documents, not the POD-type tests. The tables list every test where both converged.

#### Freshly allocated objects

**Linux / GCC 16.2 (i9-14900KF, AVX2)**

| Test | Two-stage Jsonifier (MB/s) | simdjson reflection (MB/s) | Two-stage ÷ reflection | Verdict |
|---|---|---|---|---|
| Canada (minified) | 918 | 864 | 1.06× | Win |
| Canada (prettified) | 2,600 | 2,551 | 1.02× | Win |
| CitmCatalog (minified) | 1,883 | 1,419 | 1.33× | Win |
| CitmCatalog (prettified) | 4,301 | 3,475 | 1.24× | Win |
| Discord (minified) | 1,860 | 1,578 | 1.18× | Win |
| Discord (prettified) | 2,834 | 2,478 | 1.14× | Win |
| Google Maps Response (minified) | 1,892 | 1,313 | 1.44× | Win |
| Google Maps Response (prettified) | 4,168 | 3,087 | 1.35× | Win |
| Instruments (minified) | 2,266 | 1,859 | 1.22× | Win |
| Instruments (prettified) | 3,817 | 3,331 | 1.15× | Win |
| Marine IK Reverse (minified) | 709 | 724 | 0.98× | **Loss** |
| Marine IK Reverse (prettified) | 3,253 | 3,263 | 1.00× | Tie |
| Marine IK (minified) | 723 | 714 | 1.01× | Tie |
| Marine IK (prettified) | 3,240 | 3,106 | 1.04× | Win |
| Mesh (minified) | 1,151 | 1,165 | 0.99× | **Loss** |
| Mesh (prettified) | 2,047 | 2,094 | 0.98× | **Loss** |
| Random (minified) | 1,424 | 1,180 | 1.21× | Win |
| Random (prettified) | 2,444 | 2,096 | 1.17× | Win |
| Twitter (minified) | 1,766 | 1,799 | 0.98× | **Loss** |
| Twitter (prettified) | 2,469 | 2,598 | 0.95× | **Loss** |

**macOS / GCC 16.2 (Apple M1, NEON)**

| Test | Two-stage Jsonifier (MB/s) | simdjson reflection (MB/s) | Two-stage ÷ reflection | Verdict |
|---|---|---|---|---|
| Canada (minified) | 538 | 349 | 1.54× | Win |
| Canada (prettified) | 1,350 | 1,353 | 1.00× | Tie |
| CitmCatalog (minified) | 1,119 | 924 | 1.21× | Win |
| CitmCatalog (prettified) | 2,523 | 1,950 | 1.29× | Win |
| Discord (minified) | 1,038 | 938 | 1.11× | Win |
| Discord (prettified) | 1,495 | 1,416 | 1.06× | Win |
| Google Maps Response (minified) | 1,025 | 833 | 1.23× | Win |
| Google Maps Response (prettified) | 2,055 | 1,862 | 1.10× | Win |
| Instruments (minified) | 1,352 | 903 | 1.50× | Win |
| Instruments (prettified) | 2,111 | 1,604 | 1.32× | Win |
| Marine IK Reverse (minified) | 484 | 457 | 1.06× | Win |
| Marine IK Reverse (prettified) | 1,969 | 1,761 | 1.12× | Win |
| Marine IK (minified) | 512 | 450 | 1.14× | Win |
| Marine IK (prettified) | 2,247 | 1,780 | 1.26× | Win |
| Mesh (minified) | 833 | 558 | 1.49× | Win |
| Mesh (prettified) | 1,440 | 1,095 | 1.31× | Win |
| Random (minified) | 834 | 679 | 1.23× | Win |
| Random (prettified) | 1,263 | 1,005 | 1.26× | Win |
| Twitter (minified) | 1,261 | 1,086 | 1.16× | Win |
| Twitter (prettified) | 1,790 | 1,508 | 1.19× | Win |

#### Reused objects

**Linux / GCC 16.2 (i9-14900KF, AVX2)**

| Test | Two-stage Jsonifier (MB/s) | simdjson reflection (MB/s) | Two-stage ÷ reflection | Verdict |
|---|---|---|---|---|
| Canada (minified) | 1,092 | 1,057 | 1.03× | Win |
| Canada (prettified) | 3,040 | 2,993 | 1.02× | Tie |
| CitmCatalog (minified) | 2,257 | 1,676 | 1.35× | Win |
| CitmCatalog (prettified) | 4,995 | 3,977 | 1.26× | Win |
| Discord (minified) | 2,451 | 1,782 | 1.38× | Win |
| Discord (prettified) | 3,689 | 2,807 | 1.31× | Win |
| Google Maps Response (minified) | 2,123 | 1,349 | 1.57× | Win |
| Google Maps Response (prettified) | 4,539 | 3,232 | 1.40× | Win |
| Instruments (minified) | 2,561 | 1,911 | 1.34× | Win |
| Instruments (prettified) | 4,290 | 3,433 | 1.25× | Win |
| Marine IK Reverse (minified) | 777 | 816 | 0.95× | **Loss** |
| Marine IK Reverse (prettified) | 3,507 | 3,579 | 0.98× | **Loss** |
| Marine IK (minified) | 800 | 801 | 1.00× | Tie |
| Marine IK (prettified) | 3,546 | 3,576 | 0.99× | Tie |
| Mesh (minified) | 1,209 | 1,216 | 0.99× | Tie |
| Mesh (prettified) | 2,160 | 2,213 | 0.98× | **Loss** |
| Random (minified) | 1,965 | 1,452 | 1.35× | Win |
| Random (prettified) | 3,321 | 2,596 | 1.28× | Win |
| Twitter (minified) | 2,212 | 2,079 | 1.06× | Win |
| Twitter (prettified) | 2,952 | 2,937 | 1.00× | Win |

**macOS / GCC 16.2 (Apple M1, NEON)**

| Test | Two-stage Jsonifier (MB/s) | simdjson reflection (MB/s) | Two-stage ÷ reflection | Verdict |
|---|---|---|---|---|
| Canada (minified) | 632 | 529 | 1.20× | Win |
| Canada (prettified) | 1,711 | 1,730 | 0.99× | Tie |
| CitmCatalog (minified) | 1,305 | 1,031 | 1.27× | Win |
| CitmCatalog (prettified) | 2,864 | 2,199 | 1.30× | Win |
| Discord (minified) | 1,273 | 1,024 | 1.24× | Win |
| Discord (prettified) | 1,832 | 1,654 | 1.11× | Win |
| Google Maps Response (minified) | 1,112 | 926 | 1.20× | Win |
| Google Maps Response (prettified) | 2,211 | 2,015 | 1.10× | Win |
| Instruments (minified) | 1,431 | 1,004 | 1.43× | Win |
| Instruments (prettified) | 2,257 | 1,732 | 1.30× | Win |
| Marine IK Reverse (minified) | 539 | 508 | 1.06× | Win |
| Marine IK Reverse (prettified) | 2,344 | 2,099 | 1.12× | Win |
| Marine IK (minified) | 551 | 489 | 1.13× | Win |
| Marine IK (prettified) | 2,466 | 1,776 | 1.39× | Win |
| Mesh (minified) | 882 | 615 | 1.44× | Win |
| Mesh (prettified) | 1,491 | 1,069 | 1.39× | Win |
| Random (minified) | 1,196 | 773 | 1.55× | Win |
| Random (prettified) | 1,922 | 1,128 | 1.70× | Win |
| Twitter (minified) | 1,673 | 1,147 | 1.46× | Win |
| Twitter (prettified) | 2,357 | 1,522 | 1.55× | Win |

In the freshly allocated run, two-stage Jsonifier wins 32 of these 40 tests, ties 3 and loses 5. On Linux/GCC it wins 13 of 20, by 2% (Canada, prettified) to 44% (Google Maps, minified), ties prettified Marine IK Reverse and minified Marine IK, and is slower on minified Marine IK Reverse, minified Mesh, prettified Mesh and minified Twitter (by 2%, 1%, 2% and 2%) and on prettified Twitter (by 5%). Mesh is also where Jsonifier is weakest against simdjson On Demand on that build (§2.4); we have not profiled either result. On macOS/GCC it wins 19 of 20, by 6% (Discord, prettified) to 54% (Canada, minified), and prettified Canada is a tie. With the fused path instead of the two-stage one, Jsonifier beats the reflection reader on 13 of 20 tests on Linux/GCC (one tie) and on 19 of 20 on macOS/GCC. With objects reused, two-stage Jsonifier wins 32 of 40, ties 5 and loses 3: on Linux/GCC it wins 13 of 20, by under 1% (Twitter, prettified) to 57% (Google Maps, minified), ties prettified Canada, both forms of Marine IK and minified Mesh, and is slower on both Marine IK Reverse documents (by 5% minified and 2% prettified) and prettified Mesh (by 2%); on macOS/GCC it wins 19 of 20, by 6% (Marine IK Reverse, minified) to 70% (Random, prettified), and prettified Canada is a tie.

### 2.6 Partial reading: where the tape earns its keep

The harness has one test built for the workload the tape exists for. "Twitter Partial" parses the Twitter document into a type that holds three fields per status (`text`, `user.screen_name` and `retweet_count`) and skips everything else. The harness reads the keys without assuming their order, and simdjson On Demand reads the same three fields. The harness requires stage 1 and stage 2 for this test, so both Jsonifier rows run the two-stage path; the table uses the "jsonifier (two-stage)" row. The two rows agree within 8% on 18 of the 20 results, and within 2% on 15; the widest gaps are freshly allocated minified Twitter on macOS/GCC, where the "jsonifier" row runs 34% faster (4,132 against 3,074 MB/s), and reused prettified Twitter on macOS/GCC, where it runs 13% faster (4,791 against 4,235 MB/s). Every build converged both forms in both runs.

#### Freshly allocated objects

| Platform / Compiler | Document form | Jsonifier stage 1 + 2 (MB/s) | simdjson On Demand (MB/s) | Jsonifier ÷ simdjson | Verdict |
|---|---|---|---|---|---|
| Windows / MSVC 19.44 (i9-14900KF, AVX2) | minified | 5,534 | 1,931 | 2.86× | Win |
| Windows / MSVC 19.44 (i9-14900KF, AVX2) | prettified | 7,085 | 2,737 | 2.59× | Win |
| Linux / Clang 24.0 (i9-14900KF, AVX2) | minified | 7,123 | 5,629 | 1.27× | Win |
| Linux / Clang 24.0 (i9-14900KF, AVX2) | prettified | 8,992 | 7,395 | 1.22× | Win |
| Linux / GCC 16.2 (i9-14900KF, AVX2) | minified | 7,498 | 5,469 | 1.37× | Win |
| Linux / GCC 16.2 (i9-14900KF, AVX2) | prettified | 8,919 | 7,186 | 1.24× | Win |
| macOS / GCC 16.2 (Apple M1, NEON) | minified | 3,074 | 2,594 | 1.19× | Win |
| macOS / GCC 16.2 (Apple M1, NEON) | prettified | 4,172 | 3,470 | 1.20× | Win |
| macOS / Clang 23.1 (Apple M1, NEON) | minified | 3,956 | 3,008 | 1.32× | Win |
| macOS / Clang 23.1 (Apple M1, NEON) | prettified | 4,097 | 3,375 | 1.21× | Win |

#### Reused objects

| Platform / Compiler | Document form | Jsonifier stage 1 + 2 (MB/s) | simdjson On Demand (MB/s) | Jsonifier ÷ simdjson | Verdict |
|---|---|---|---|---|---|
| Windows / MSVC 19.44 (i9-14900KF, AVX2) | minified | 5,852 | 1,997 | 2.93× | Win |
| Windows / MSVC 19.44 (i9-14900KF, AVX2) | prettified | 7,373 | 2,824 | 2.61× | Win |
| Linux / Clang 24.0 (i9-14900KF, AVX2) | minified | 7,329 | 5,708 | 1.28× | Win |
| Linux / Clang 24.0 (i9-14900KF, AVX2) | prettified | 9,179 | 7,464 | 1.23× | Win |
| Linux / GCC 16.2 (i9-14900KF, AVX2) | minified | 7,636 | 5,567 | 1.37× | Win |
| Linux / GCC 16.2 (i9-14900KF, AVX2) | prettified | 9,111 | 7,313 | 1.25× | Win |
| macOS / GCC 16.2 (Apple M1, NEON) | minified | 3,410 | 2,683 | 1.27× | Win |
| macOS / GCC 16.2 (Apple M1, NEON) | prettified | 4,235 | 3,365 | 1.26× | Win |
| macOS / Clang 23.1 (Apple M1, NEON) | minified | 3,670 | 2,937 | 1.25× | Win |
| macOS / Clang 23.1 (Apple M1, NEON) | prettified | 4,482 | 3,867 | 1.16× | Win |

Jsonifier wins all 20 results, by 1.16× (prettified on macOS/Clang, reused) to 2.93× (minified under MSVC, reused). Throughput here is counted over the whole document, so these figures run well above the full-parse ones: on minified Twitter the fused path parses at 1,255 MB/s under MSVC and 1,982 MB/s on Linux/Clang when it materializes every value, against 5,534 and 7,123 MB/s here, because the bytes the caller does not ask for are skipped through the tape instead of being walked. The margin over simdjson is 2.59× to 2.93× under MSVC and 1.16× to 1.37× elsewhere. On minified Twitter in the freshly allocated run, simdjson On Demand under MSVC reaches 34% of its Linux/Clang throughput and Jsonifier's stage 1 + 2 path reaches 78%. This is the one test in the sweep where the index is unambiguously the right tool, and it is the reason the two-stage machinery stays in Jsonifier even though full-document parsing does not route through it.

## 3. The single-pass path: `json_cursor` over raw text

The fused path is implemented as `json_cursor<parseOpts, read_buffer_ptr>` — a set of static navigation primitives over a raw `read_buffer_ptr` cursor, with the end pointer, the detected indentation and the string buffer held in a `parse_context` and the nesting depth passed down the call chain, walking the document exactly once. Several design decisions distinguish it:

**Compile-time key fusion.** For each reflected member, the expected token is baked into the binary at compile time as a fused literal. In minified known-order mode, an entire member header — leading comma, quoted key, and colon — collapses into a single constant:

```cpp
template<uint64_t index, typename literal_type> JSONIFIER_INLINE static constexpr auto makeMemberLiteralNew(const literal_type& keyLiteral) noexcept {
	if constexpr (index > 0) {
		return string_literal{ "," } + string_literal{ "\"" } + keyLiteral + string_literal{ "\"" } + string_literal{ ":" };
	} else {
		return string_literal{ "\"" } + keyLiteral + string_literal{ "\"" } + string_literal{ ":" };
	}
}
```

When the document arrives in the expected order — the overwhelmingly common case for machine-generated JSON — matching `,"name":` is one constant-length comparison, and the cursor jumps the entire header in a single add. No tokenizer, no tape, no per-character state machine.

**Adaptive out-of-order recovery.** When the known-order guess misses, the parser falls back to a compile-time hash map over the type's keys (`hash_map<value_type>::findIndex`), dispatches through a generated table, and — crucially — *learns*: a thread-local `antiHashStatesNew` array records which index actually matched at each slot, so a document stream with a consistently permuted key order pays the hash cost once and then rides the corrected fast path.

This is worth dwelling on, because key order is where iterative traversal models pay their hidden tax. simdjson's On Demand API resolves a field lookup by scanning forward through the object from the current cursor position; the project's own documentation (doc/basics.md, field-access guidance) instructs callers to request fields in the order they appear in the document for best performance. When a requested key is *not* next, the cursor scans — and skips — every intervening key/value pair to find it, and a subsequent request for a key that lies *behind* the cursor forces a wrap-around rescan of the object. The degenerate case is a caller requesting fields in an order that consistently mismatches the document: each lookup can traverse a large fraction of the object's raw bytes, turning an O(members) parse into an O(members²) crawl over structural positions — per object, per document, forever. The model has no memory; the millionth permuted document costs exactly what the first one did.

Jsonifier's failure mode for the same situation is: one hash lookup, one dispatch-table indirection, and a learned correction. The mismatch cost is paid once per key slot per thread, not once per object instance. A feed of a billion documents with keys in reversed order parses at effectively the same throughput as a feed in declared order, because after the first document the "expected order" *is* the observed order. The schema-directed model converts key order from a per-document runtime tax into a per-stream calibration.

**Depth-predicted indentation.** The non-minified specialization exploits the fact that pretty-printed JSON indents each line by a fixed unit times its nesting depth. At the root, `collectIndentSize` measures that unit once: the indent character (`wsChar`) and how many of it make one level (`indentSize`). After every `{`, `[` and `,`, `skipWhitespacePredicted` steps over the newline and predicts the next line's indentation as `context.indentSize * depth`. It then verifies the whole predicted span in one call to `spanIsIndent`, which works in three tiers on every architecture: a 16-byte vector loop compares the span against the broadcast indent character while more than 16 bytes remain, a single 8-byte SWAR step (one XOR against the broadcast character in a `uint64_t`) handles 9 to 16 remaining bytes, and a size-class switch with overlapping scalar loads resolves the last 8 or fewer, so no span length needs a byte loop. The tiers were chosen by A/B testing across compilers: a wider 32-byte loop and a remainder case that assembled a 128-bit vector from two 8-byte copies both cost MSVC and GCC measurably, while a pure 8-byte SWAR loop halved prettified Canada's throughput under MSVC because its long spans need the vector tier. Before a closing `}` or `]`, `skipWhitespacePredictedClose` makes the same prediction one level shallower, and after each key `collectObjectColon` matches `": "` as a single two-byte constant. If the span matches and the next byte is not whitespace, the cursor jumps the entire indentation at once. If the prediction misses, the parser falls back to `skipWhitespaceScalar`, a `whitespaceTable` lookup loop that advances one byte per iteration.

The result is a parser whose inner loop is dominated by wide constant comparisons and direct value materialization, with SIMD engaged surgically where it wins (string unescaping, discussed in §6) rather than as a mandatory preprocessing pass.

## 4. Stage 1: structural indexing, Jsonifier style

When the workload *does* justify a structural index, Jsonifier's stage 1 (`simd_string_reader`) implements the Langdale–Lemire bitmask algebra with several architectural departures.

### 4.1 Step geometry as a per-compiler compile-time constant

simdjson classifies input per 64-byte block, and its generic indexer steps through the buffer 128 bytes per iteration — two 64-byte blocks, software-pipelined one block deep, a geometry it retains even on its widest kernel (the Icelake implementation invokes `index<128>`). Jsonifier generalizes the unit of work to a *step* of `simdBlocksPerStep` 64-byte blocks, with the constant chosen per ISA **and per compiler**:

```cpp
template<> struct backend_traits<jsonifier_backend::avx2> : compiler_step_tuning<4, 4, 4, 8, 4, 8> {
	static constexpr const char* name{ "AVX2" };
	using simd_int_t = jsonifier_simd_int_256;
};
```

`compiler_step_tuning` takes (tape step, blocks per step) pairs for Clang, GCC and MSVC in that order and resolves to the current compiler's pair, so the AVX2 traits above carry 4 blocks per step on Clang and 8 on GCC and MSVC, with a tape step of 4 on all three. `simdBlocksPerStep` and `simdTapeStep` are read from the configured backend's traits.

These values are not guesses. They were selected by Cartesian parameter sweeps across five platform/compiler CI targets (Windows/MSVC, Linux/GCC, Linux/Clang, macOS/Clang, plus ARM), validated against popcount histograms of structural density on the benchmark corpus. The finding that Clang and GCC want *different* geometry on the identical ISA reflects real differences in how each compiler schedules the unrolled block bodies: on AVX2 Clang takes four blocks (256 bytes) per step where GCC and MSVC take eight (512 bytes), and on NEON both compilers take four blocks per step with a drain burst of 8. It is only expressible because the entire pipeline is specialized at compile time. A runtime-dispatched kernel gets one shape per ISA; a Cathedral-Architecture kernel gets one shape per (ISA × compiler) cell.

Processing up to 8 blocks (512 bytes) per step before draining amortizes the loop-carried state updates and gives the out-of-order core a deep window of independent block computations — a 2-4x wider scan window than simdjson's fixed 128-byte step.

### 4.2 The classification core: `rope_detector` and the collectors

The per-block bitmask algebra lives in small, composable functor structs. `cmp_eq_op` fans a broadcast comparison across the registers of a block and fuses the per-register movemasks into one `uint64_t` with compile-time shift amounts. The escape/quote/in-string state machine is `rope_detector`, a CRTP mixin over a plain `rope_block` of three masks:

```cpp
JSONIFIER_INLINE void next(const simd_array_t in_01, const jsonifier_simd_int_t bsRegister, const jsonifier_simd_int_t quoteRegister) noexcept {
	const uint64_t escaped = nextEscapeAndTerminalCode(simd::cmp_eq_op::impl(in_01, bsRegister));
	const uint64_t quotes  = (simd::cmp_eq_op::impl(in_01, quoteRegister) & ~escaped);
	rope_block::escaped	   = escaped;
	rope_block::quotes	   = quotes;
	return quotes ? finishNext() : finishNextNoInString();
}
```

The escape logic is the classic odd-length-backslash-run computation with the standard fast exit — a block containing zero backslashes skips the arithmetic and just consumes the carried `nextIsEscaped` bit, the same short-circuit simdjson's `json_escape_scanner` ships by default. Jsonifier extends the principle one level up: the prefix-XOR (`clmul` on x86, a shift-XOR ladder on NEON) that turns the quote mask into an in-string range mask is itself conditional — `next` branches on the quote mask, and a quoteless block bypasses the multiply entirely, inheriting `prevInString` directly via `finishNextNoInString` (the NEON variant names the multiplying branch `finishNextInString`; on AVX it is `finishNext`). Since long stretches of numeric or minified structural data contain no quotes at all, entire regions of such documents never touch the carry-less multiplier.

Whitespace and operator classification use the same `shuffle`-against-lookup-table trick simdjson pioneered, expressed as `ws_collector` and `op_collector` over the block's register array. On NEON, where `movemask` doesn't exist, the collectors use the `vshrn_n_u16`-based 4-bit-per-lane narrowing (with `tzcnt >> 2` index correction in `postCmpTzcnt`) and, in the tuned NEON `op_collector`, a `vqtbl1q_u8` nibble-shuffle keyed on `(byte + 3) >> 4` — the reverse-bits/RBIT strategy is available as a compile-time switch where it profiles faster.

### 4.3 Pseudo-structural promotion

Scalar starts are promoted to structurals exactly as in the original algorithm — `followsNonquoteScalar` carries the cross-block bit — so the tape marks the first byte of every number, `true`/`false`/`null`, and string, giving stage 2 direct seek points to every value:

```cpp
JSONIFIER_INLINE uint64_t getStructurals(const simd_array_t in_01, const jsonifier_simd_int_t opTable, const jsonifier_simd_int_t spaceMask,
	const jsonifier_simd_int_t whitespaceTableLocal) noexcept {
	const uint64_t whitespace  = simd::ws_collector::impl(in_01, whitespaceTableLocal);
	const uint64_t op		   = simd::op_collector::impl(in_01, opTable, spaceMask);
	const uint64_t scalar	   = ~(op | whitespace | simd::rope_detector<rope_block>::quotes);
	const uint64_t follows	   = simd::rope_detector<rope_block>::followsNonquoteScalar(scalar);
	const uint64_t scalarStart = scalar & ~follows;
	return op | simd::rope_detector<rope_block>::quotes | scalarStart;
}
```

Note the overload pair: the minified variant omits the whitespace collector entirely — an entire classification lane deleted at compile time when the caller declares the input minified. This is the same philosophy as the compiler-specific step constants: every fact known before runtime is burned into the instruction stream.

### 4.4 The drain architecture: bits → indices

Extraction — converting each 64-bit structural mask into byte offsets on the tape — is where naive implementations serialize hard, because the classic loop (`tzcnt`, store, `blsr`, repeat) is a loop-carried dependency chain of length popcount.

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

**AVX2/AVX/NEON: the folded stepped drain.** Without byte-compress, extraction must use the tzcnt chain — but the chain's *structure* is still a compile-time decision. `write_indices_functor` emits one extract-advance pair per index; `write_indices_stepped_functor` groups them into unconditional bursts of `simdTapeStep` writes:

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

The `functor_runner`'s `implAnd` expands a stepped range sequence `<0, 64, simdTapeStep>` through an `&&`-fold: each group writes `simdTapeStep` indices *unconditionally* (over-writing garbage past the true count is harmless — the tape cursor only advances by the real popcount), and the fold short-circuits the moment a group's start index reaches the count. One predictable branch per `simdTapeStep` extractions instead of one per extraction.

Credit where due: simdjson's current generic `bit_indexer` implements the same stepped-burst pattern — `write_indexes_stepped<START, END, STEP>` via recursive template expansion, unconditional bursts, `simdjson_unlikely` short-circuit checks at each group boundary — with the burst size exposed as a build macro (`SIMDJSON_STRUCTURAL_INDEXER_STEP`, default 4). The two libraries have converged on the extraction inner loop itself. The divergences are in everything around it:

- **Coverage.** simdjson's stepped expansion runs to a fixed `STEP_UNTIL` of 24 set bits and falls back to a plain scalar loop for denser blocks; Jsonifier's fold covers the full 0..64 range at the same stride.
- **Tuning axis.** simdjson's STEP is one global knob; Jsonifier's `simdTapeStep` is swept and pinned per (ISA × compiler) cell, jointly with `simdBlocksPerStep`, because the two constants interact. AVX2 uses a burst of 4 on all three compilers, with 4 blocks per step on Clang and 8 on GCC and MSVC; NEON uses a burst of 8 on both Clang and GCC, at 4 blocks per step (§4.1).
- **Drain scheduling.** This is the structural difference. simdjson pipelines at a depth of one block: each `next()` call drains the *previous* block's structurals while the current block is being classified. Jsonifier defers draining for an entire step — up to eight masks and their popcounts are materialized in `bitsArr`/`cntsArr` before `add_tape_values` drains them back-to-back, each lane's tape destination precomputed from the popcount prefix. Eight independent tzcnt chains with no interleaved classification dependencies, handed to the out-of-order core as one batch.

Above the per-lane drain sits the same fold pattern at block scope. `add_tape_values` drains all blocks of a step, threading the running tape offset through a fold over the lane indices:

```cpp
JSONIFIER_INLINE static void impl(const array<uint64_t, blocksPerStep>& bitsArr, const array<uint64_t, blocksPerStep>& cnts, write_structural_index_ptr tape,
	size_type strIdx) noexcept {
	uint64_t offset = 0;
	(((drainLane<indices>(bitsArr, cnts, tape + offset, strIdx)), offset += cnts[tag<indices>{}]), ...);
}
```

Because per-block popcounts were captured during classification (`cntsArr[I]`), the drains of successive blocks have no data dependence on each other's tzcnt chains — each lane knows its destination offset up front. The classification of blocks *N+1..7* and the drain of block *N* are independent instruction streams the scheduler is free to interleave.

## 5. Stage 2: the tape-driven iterator

Stage 2 is not a separate parser — it is a *specialization* of the same `json_cursor` interface over `write_structural_index_ptr` instead of `read_buffer_ptr`. The reflection-driven parse machinery (`parse_impl`, the dispatch tables, the anti-hash learning) is written once against the cursor interface: `cursor_t<options, context_type>` picks the specialization from the context's `iterator_type`, and the `json_cursor<parseOpts, write_structural_index_ptr>` specialization supplies the token-navigation primitives:

```cpp
template<typename context_type> JSONIFIER_INLINE static bool skipValue(write_structural_index_ptr& iter, context_type& context) noexcept {
	if (iter >= context.endIter) [[unlikely]] {
		return reject<parse_statuses::unexpected_end_of_input>(iter, context);
	}
	const char first = static_cast<char>(*valuePtr(iter, context));
	if (first == '{' || first == '[') {
		int64_t depth{};
		while (iter < context.endIter) {
			depth += nestingDeltaTable[static_cast<uint8_t>(context.stringRoot[*iter])];
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

This is the payoff that justifies the tape for partial reading: skipping an unwanted value is `++iter`. Skipping an entire unwanted subtree touches only its structural characters — one indexed byte load per structural entry — never the bytes in between. `skipString` is a single increment, because stage 1 already resolved every escape sequence's effect on string extent. In the raw-pointer cursor, by contrast, skipping a string means re-scanning it for an unescaped closing quote (`skipStringImpl`'s memchr-and-check-backslash-parity loop), and skipping a container means walking every byte.

The asymmetry defines the routing rule. When the caller wants *every* value (full-document parse into a reflected type), skips are rare and the tape is overhead. When the caller wants a *few* values from a large document (partial reading), or wants only the structure itself (prettify/minify, where the transformation is literally "copy bytes, adjusting whitespace at structural positions"), skips dominate and the tape converts O(bytes) navigation into O(structurals).

The same routing logic appears in stage 1's own entry point: `reset<minified>` selects between the whitespace-aware and whitespace-free classification pipelines, and the tail-block handling pads with `0x20`, indexes the pad, then retroactively pops any tape entries pointing past the true document length — branchless main loop, exact tape.

## 6. Distributed UTF-8 validation

Let's be precise about what simdjson does, because it is also an in-register scheme: `json_structural_indexer::next` feeds every 64-byte block into `utf8_checker::check_next_input` using the very registers the classifier just loaded. Validation is fused into stage 1, costs no extra pass over memory, and covers the entire input. (The standalone `generic_validate_utf8` exists too, for the buffer-validation API.) On Demand then additionally defers some string-level checks to traversal.

So the distinction is not "in-register versus dedicated pass" — both libraries validate in registers already in flight. The distinction is **scope and location**:

- **simdjson** validates *all input bytes*, in *stage 1*. Elegant and total, but it means the range checker runs over structural characters, whitespace, numbers, and literals — bytes that a JSON parser's own grammar already constrains to ASCII. For a document that is 70% non-string content, 70% of the validation work confirms what token matching would prove for free.
- **Jsonifier** validates *string bytes only*, in the *string parse loop*. Everything outside strings is implicitly ASCII-constrained: structural characters are matched literally, numbers pass through a digit-table parser, `true`/`false`/`null` are compared as packed integer constants — any non-ASCII byte in those positions is a parse error by construction, no range checker required. The only place arbitrary bytes can legally appear is inside strings, and that is exactly — and only — where `utf8_register_validator` runs.

One accounting qualifier belongs in the main text rather than a footnote, because it bounds the size of the claim: simdjson's checker leads with an ASCII fast path — a block whose registers OR to an ASCII-only result skips the multibyte carry chain entirely and pays roughly one reduction and one test. For ASCII-heavy input, then, "validation work proportional to input bytes" is proportional with a small constant, not with the full cost of the multibyte machinery. The scope distinction survives the qualifier — Jsonifier runs *no* validation instructions of any kind over non-string bytes, fast-path or otherwise, and the two approaches still separate cleanly on documents dense in non-ASCII string content — but the honest comparison states the fast path up front rather than burying it.

Jsonifier ships the same range-based algorithm simdjson uses (the `byte1High`/`byte1Low`/`byte2High` nibble-lookup classifier with the `carry`/`tooShort`/`tooLong`/`surrogate` error-bit algebra) in two deployments. The first, the standalone `validateUtf8` built on `utf8_checker`, is the conventional block validator, with the same ASCII fast path (`orAll` the block's registers, one test, skip the multibyte machinery). The second is the distributed one:

```cpp
if (delimiters == static_cast<integer_type>(0)) {
	validator.checkRegister(simdValue);
	string1Start += bytesProcessed;
	string2 += bytesProcessed;
	continue;
}
```

The validator carries `prevInput`/`incompleteRegister` across registers exactly as the block checker carries them across blocks, and `checkPartial` handles the sub-register tail at each quote or backslash boundary by padding with `0x20` (an innocuous ASCII byte) — so multi-byte sequences spanning register boundaries are still caught, and a string ending mid-sequence trips the incomplete carry. Because the string parse loop cascades through progressively narrower SIMD widths, the carry additionally survives width transitions through a compact three-byte `utf8_validation_state` flushed at each width's loop exit and reseeded into the next width's validator — the position-dependent thresholds (last byte ≥ 0xC0, second-to-last ≥ 0xE0, third-to-last ≥ 0xF0) reconstruct exactly the incomplete carry the register-resident validator would have held. The stage-1 companion paper covers this mechanism in full [§6.1 there].

The consequence differs by path. On simdjson's architecture, validation work is proportional to *input bytes* — every block, string or not, at minimum passes through the checker's ASCII test, and any block containing non-ASCII runs the full multibyte carry chain. On Jsonifier's, validation work is proportional to *string bytes*, and on the single-pass full-document path it rides registers the unescaper already loaded, so it never adds a memory pass and never touches non-string content at all. A document that is mostly numbers, structure, and whitespace validates nearly for free; the grammar itself is the validator for everything the range checker skips. There is also a timing difference: simdjson's stage-1 fusion produces its verdict at `check_eof`, after the whole input is scanned; Jsonifier's distributed scheme delivers a strict per-string verdict at each string's closing quote, so an invalid byte fails the parse at the value that contains it. It is always on: there is no option to disable it, because its cost is already confined to string bytes the parser was loading anyway.

## 7. Where the two libraries stand

| Dimension | simdjson | Jsonifier |
|---|---|---|
| Stage-1 usage | Unconditional, all documents | Partial reading, prettify, minify only (benchmarks may force it for comparability; see §2.1) |
| Full-document parse | Stage 1 + On Demand traversal | Single fused pass, schema-directed; faster than its own two-stage path on POD data, on 45 of 50 minified documents and on 17 of 20 prettified documents on the M1; slower on 23 of 30 prettified x86 documents (see §2.2) |
| Target of stage 2 | DOM / lazy generic values | Reflected concrete types via shared cursor interface |
| Out-of-order keys | Forward scan + wrap-around rescan per lookup, per object — no memory across documents | One hash fallback, then learned per-slot order correction (thread-local, per stream) |
| Schema knowledge (known-type workloads) | Exists in caller's traversal code — invisible to the library | Declared once via reflection — consumed by the architecture |
| Step size | 128 bytes (two 64-byte blocks), pipelined one block deep | 256–512 bytes, per-(ISA × compiler) constant |
| Bit extraction | Stepped tzcnt bursts (global STEP macro, scalar tail past 24 bits); VBMI2 compress on Icelake | Same primitives, but full-range folded bursts with per-toolchain-swept stride |
| Drain scheduling | Previous block drained during current block's classification | Whole-step deferred batch drain, lane offsets precomputed from popcounts |
| Escape short-circuit | Yes (zero-backslash fast exit) | Yes (same) |
| In-string prefix-XOR | Computed every block | Skipped for quoteless blocks |
| UTF-8 validation | In-register, fused into stage 1, over all input bytes (ASCII fast path per block); verdict at EOF | In-register, fused into string parsing, over string bytes only, carried across the width cascade; verdict per string |
| Specialization axis | Runtime CPU dispatch through virtual functions (core/DOM API); On Demand mostly fixed at compile time | Runtime tier selection by a cached enum fold over per-tier CRTP instantiations, no virtual calls; everything inside a tier is compile-time (Cathedral Architecture) |

The last row is the root of every other difference. Both libraries can ship one x86 binary that picks its instruction set at run time; they differ in what that choice costs and how much of the code it reaches. Jsonifier's founding constraint is that apart from the data, the only thing decided at run time is the tier, and that it is decided once. The schema, the compiler and the parse options are known at build time, and each ISA tier is a complete compile-time instantiation, so every routing decision in this paper (tape or no tape, whitespace lane or not, validation or not, 4 blocks or 8, burst of 1 or 4) is still resolved before the first byte is read.

On x86, one Jsonifier binary carries every instruction-set tier it was configured for and picks one at run time. The headers are compiled once per tier inside a target-attribute region (`backend_passes.hpp`: `#pragma GCC target` on GCC, `#pragma clang attribute` on Clang), which yields one complete `jsonifier_core_internal<backend>` per tier: AVX-512 (`avx512f`, `avx512bw`, `avx512vbmi2`), AVX2 and AVX. Each of those inherits its tier's parser, serializer, validator, minifier, prettifier and generic iterator through CRTP, so every call inside a tier is a direct, inlinable call into code built for that tier. On first use, `selectSupportedBackend` reads CPUID (leaves 0, 1 and 7, plus the extended leaves for `lzcnt`) and XCR0, so a tier counts only if both the CPU and the operating system support it, and caches the widest supported tier in a function-local `static`. Every public entry point then dispatches with a fold expression over the compiled tiers:

```cpp
static const jsonifier_backend backend{ selectedBackend() };
static_cast<void>(((backend == cores::backendType && (result = cores::template parseJson<options>(internal::forward<value_type>(object), in), true)) || ...));
```

That is a load of a cached enum and at most three compares, once per call. There is no function pointer and no vtable. simdjson's runtime dispatch, by contrast, goes through virtual functions: `implementation` exposes `create_dom_parser_implementation`, `minify` and `validate_utf8` as virtuals, and `dom_parser_implementation` declares `parse`, `stage1`, `stage2` and `parse_string` pure virtual. Its own On Demand documentation notes that the On Demand API "has limited runtime dispatch support" and recommends compiling for a specific x64 target. In Jsonifier the same dispatch covers every API, including typed parsing, serialization and the generic On Demand-style iterator, and inside a tier nothing is virtual, so the per-tier code keeps all of its compile-time specialization.

The scope is x86. ARM builds compile one backend (NEON, or SVE2 when configured), as simdjson's ARM builds do. The set of x86 tiers compiled in is bounded by the configured tier (`JSONIFIER_CONFIGURED_AVX_TIER`), so a build configured for AVX2 carries AVX2 and AVX but not AVX-512. We have not separately measured the cost of the per-call check.

## 8. Conclusion

The two-stage model is a genuinely great algorithm — Jsonifier's stage 1 is an unapologetic descendant of Langdale and Lemire's design, and credits it in source. The contribution here is architectural discipline about *when* to run it. A structural tape is an index, and indexes are worth building exactly when you will not read the whole book. Jsonifier builds it for partial reads and structural transforms, skips it for full parses, validates UTF-8 in the registers it was already holding, and lets the compiler specialize every remaining decision down to per-toolchain loop geometry. §2 shows what skipping the tape is worth, with both paths compiled into one binary and ranked head to head, on both freshly allocated and reused target objects. Across 125 freshly allocated tests on five platforms the fused path wins 92, ties 5 and loses 28, and where it loses is a matter of layout and architecture more than of principle. It wins all 25 POD-type tests, by up to 8.2× (Bool on macOS/GCC), 46 of the 50 minified documents, and 16 of the 20 prettified documents on the M1. The two-stage path wins 23 of the 30 prettified documents on x86, all 10 of them under MSVC, while the fused path wins 5 and 2 are ties. The tape, in other words, is not only an index for partial reads: on indented input on x86 it is also the faster way to read the whole book, and the router should learn when. Taking whichever path is faster, Jsonifier beats simdjson On Demand on 122 of those 125 tests and ties one, its own stage-1 + reflection path beats simdjson's stage-1 + reflection reader on 32 of 40, and on the partial-reading test, where the index is the right tool, it beats simdjson On Demand on all 20 results. The benchmarks are the receipts.

---

*Jsonifier is MIT-licensed and available at github.com/nihilai-collective/Jsonifier. Benchmark methodology and full sweep data: github.com/nihilai-collective/Json-Performance.*