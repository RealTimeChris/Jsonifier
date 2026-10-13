# CPU Architecture Selection

Jsonifier is a SIMD-heavy library, and getting the right SIMD backend selected for your target CPU is what separates near-peak performance from a slow fallback path. By default, everything is automatic: Jsonifier detects your CPU features at configure time and generates a binary specialized for exactly what your machine supports. One backend is compiled in, and there is no runtime CPU dispatch. When automatic detection isn't the right choice (cross-compilation, portable binaries, deployment mismatches), you can override the detection with a single CMake variable.

## How Auto-Detection Works

At configure time, Jsonifier's CMake build fetches [voided-hw-detection](https://github.com/nihilai-collective/voided-hw-detection), which builds and runs a small standalone helper program on the host machine. The program calls `cpuid` (on x64) or queries NEON, SVE2, and PMULL/crypto (carry-less multiply) support via `getauxval`/`sysctlbyname` (on ARM64), then prints one `KEY=VALUE` line per property: the thread count and cache sizes, a `HAS_*` flag for every feature it checks (including FMA, F16C, BMI2 and the AVX-512 F/BW/VBMI2 subsets, which have no bit of their own), and `CPU_INSTRUCTION_MASK`, the raw bitfield of every [feature bit](#the-feature-bits) it found.

CMake parses those lines into `JSONIFIER_HAS_*` cache variables and picks `JSONIFIER_SIMD_TIER`: `AVX512` when AVX-512F and AVX-512BW are both present, otherwise `AVX2`, `AVX`, `SVE2` (only with a known vector length), `NEON` or `NONE`. It then builds the selected bitfield from the raw mask. On x64 that is the LZCNT/POPCNT/BMI/PCLMULQDQ bits as detected, plus the detected AVX bits up to and including the tier. On ARM64 it is the PCLMULQDQ bit plus the single NEON or SVE2 bit. The selected value is written into `include/jsonifier-incl/simd/jsonifier_cpu_instructions.hpp` as `#define JSONIFIER_CPU_INSTRUCTIONS <value>`, alongside `#define JSONIFIER_SVE2_VECTOR_BITS <value>`. The detected alignment (64 for AVX-512, 32 for AVX2, the SVE vector length in bytes for SVE2, otherwise 16) is written into the generated `jsonifier_cpu_properties.hpp`, and allocations use it as their alignment.

CMake translates the tier and the `HAS_*` flags into compiler flags (`/arch:AVX`, `/arch:AVX2` or `/arch:AVX512` on MSVC; on GCC/Clang, the tier's `-mavx...` flags plus `-mfma`, `-mf16c`, `-mlzcnt`, `-mpopcnt`, `-mbmi`, `-mbmi2` and `-mpclmul` for each feature that was found) and attaches them to the `voided_hw_jsonifier_cpu` interface target. The Jsonifier target links that target, and it is installed and exported with it, so every consumer that links Jsonifier compiles with the flags that match the configured feature bits. The configured backend is the only one compiled in, on every architecture.

## Backends and the Core

Every SIMD backend is a value of `jsonifier::jsonifier_backend` (`avx512`, `avx2`, `avx`, `neon`, `sve2`, `fallback`). `simd/simd_config.hpp` defines a `jsonifier::backend_traits<backend>` specialization for each backend the build can use. It holds the register type (`simd_int_t`), the display `name`, and the per-compiler stage-1 tuning: `blocksPerStep` (64-byte blocks scanned per step) and `tapeStep` (burst size of the scalar tape drain). The derived helpers `backendBytesPerRegister<backend>`, `backendRegistersPerBlock<backend>` and `backendBytesPerStep<backend>` come from those traits.

### One tier per build

A build compiles exactly one backend, and the configured feature bits choose it. On x64, `core/defines.hpp` reduces the bitfield to `JSONIFIER_CONFIGURED_AVX_TIER`: 3 for AVX-512, 2 for AVX2, 1 for AVX. It then sets `JSONIFIER_CPU_INSTRUCTIONS` to that tier's cumulative mask. The AVX-512 mask also carries the AVX2 and AVX bits, and every tier's mask carries whichever of LZCNT/POPCNT/BMI/PCLMULQDQ were configured. `simd/simd_config.hpp` turns that mask into `jsonifier::defaultBackend`, and `jsonifier::cpuArchName` is that backend's display name.

Define `JSONIFIER_MAX_AVX_TIER` (1, 2 or 3) before including Jsonifier to cap the tier below what was detected. For example, define it as 2 to build the AVX2 backend on an AVX-512 machine.

The whole library is compiled once, in `jsonifier` and `jsonifier::internal`, with the translation unit's own flags. There are no target-attribute regions and no per-tier copies, so every force-inlined function can be inlined into every caller. That includes callables handed to standard-library templates, such as the contexts passed to `std::basic_string::resize_and_overwrite`.

### Flag consistency check

On x64, `core/defines.hpp` checks at compile time that the translation unit is actually compiled for the configured tier, and stops with `#error` if it is not:

| Configured tier | Required predefined macros |
|-----------------|----------------------------|
| AVX-512 | `__AVX512F__`, `__AVX512BW__`, plus `__AVX512VBMI2__` on GCC/Clang |
| AVX2 | `__AVX2__` |
| AVX | `__AVX__` |

MSVC's `/arch:AVX512` never defines `__AVX512VBMI2__`, so MSVC is checked for F and BW only. Linking the `voided_hw_jsonifier_cpu` target supplies matching flags automatically. The check catches the remaining cases, such as headers used without that target, or a stale generated header next to different compiler flags. Without it they would show up as cryptic target-feature errors or as `SIGILL` at run time.

### The core

`jsonifier::jsonifier_core<initialBufferSize>` is a single class. It inherits the printer, prettifier, serializer, validator, minifier, parser and generic iterator through CRTP, and it owns everything those bases work on:

- the fused-path structural reader (`pod_simd_string_reader<defaultBackend, simdBytesPerStep>`) and the two-stage structural reader (`simd_string_reader<initialBufferSize>`)
- the string buffer and the error list
- the generic-parsing storage (field index, document state and string arena), held inline; its buffers are allocated on the first `iterate`

Every call is a direct call into code built for the configured backend; nothing is selected at run time. A core can be moved but not copied. `jsonifier::activeBackendName()` returns `cpuArchName`.

### Limits

- **Target requirements:** a binary requires the tier it was configured for on every machine that runs it. A build configured for AVX-512 stops with `SIGILL` on an AVX2-only CPU. For binaries distributed to mixed hardware, configure for the lowest tier you need to support (see [Overriding the Auto-Detected Value](#overriding-the-auto-detected-value)).
- **Generic parsing:** `iterate` and `iterateMany` run on the same backend as every other call, and they reuse the core's own structural readers. They are members of `jsonifier_core<>` itself. A `document` points into the core's tape and generic-parsing state, so a later `parseJson`, `validateJson` or `iterate` call on the same core, or moving the core, invalidates it.

## The Feature Bits

| Feature  | Bit | Value | Macro |
|----------|-----|-------|-------|
| LZCNT    | 0 | 1   | `JSONIFIER_LZCNT` |
| POPCNT   | 1 | 2   | `JSONIFIER_POPCNT` |
| BMI      | 2 | 4   | `JSONIFIER_BMI` |
| PCLMULQDQ| 3 | 8   | `JSONIFIER_CLMUL` |
| NEON     | 4 | 16  | `JSONIFIER_NEON` |
| AVX      | 5 | 32  | `JSONIFIER_AVX` |
| AVX2     | 6 | 64  | `JSONIFIER_AVX2` |
| AVX-512  | 7 | 128 | `JSONIFIER_AVX512` |
| SVE2     | 8 | 256 | `JSONIFIER_SVE2` |

**Important detail:** the AVX bits are cumulative. A CPU with AVX-512 reports the AVX, AVX2 and AVX-512 bits together, and the selected value keeps every detected AVX bit up to the tier, on every compiler. `core/defines.hpp` keys the build off the highest AVX bit, so the lower ones never change which backend is compiled.

The AVX-512 bit is only detected when AVX-512F, AVX-512BW **and** AVX-512VBMI2 are all present, while the `AVX512` tier only needs F and BW. On a CPU with F and BW but no VBMI2 (Skylake-X, for example), CMake applies the AVX-512 compiler flags, but bit 7 is absent from the written value, so `core/defines.hpp` builds the AVX2 backend.

**On MSVC, CMake also ORs in the AVX2 and AVX bits below the selected tier** (the MSVC cascade). Detected values already carry those bits, so the cascade only changes an override that names a single tier bit: `1|2|4|128` is written as `135` on GCC/Clang and as `231` on MSVC. The cascade never adds the AVX-512 bit.

NEON and SVE2 are mutually exclusive with each other (every SVE2-capable part also reports NEON, but only one backend bit is ever set) and are exclusive to ARM64. The bit-manipulation features (LZCNT, POPCNT, BMI) are independent and can co-exist with any SIMD tier. PCLMULQDQ (carry-less multiplication) is independent on x64; on ARM64 its presence upgrades the SVE2 build's `-march` flag to include `+aes`.

## OS-Level Requirements

On x64, having AVX/AVX2/AVX-512 available in the CPU is not enough; the operating system also has to enable state-saving for those register sets. Jsonifier's detector checks this via `xgetbv`:

- **AVX/AVX2** require the OS to have XMM and YMM state enabled
- **AVX-512** additionally requires ZMM and OpMask state enabled

If a CPU supports AVX-512 but the OS hasn't enabled the state (common on some older Windows configurations, hypervisors, or containerized environments), the detector correctly reports it as unavailable and falls back to AVX2 or lower. This prevents illegal-instruction crashes at runtime.

## Overriding the Auto-Detected Value

When automatic detection isn't right for your use case, set `JSONIFIER_CPU_INSTRUCTIONS` explicitly at CMake configure time. Two forms are accepted.

**Pipe-separated flags (readable):**

```bash
cmake -B build -DJSONIFIER_CPU_INSTRUCTIONS="1|2|4|64"
```

**Or the numeric OR of the values:**

```bash
cmake -B build -DJSONIFIER_CPU_INSTRUCTIONS=71
```

Both produce identical results. The detector still runs for the thread count and cache sizes, but CMake treats your value as the detected mask: each bit sets the matching `HAS_*` flag (bit 7 sets all three AVX-512 subsets, while FMA, F16C and BMI2 are turned off), and the tier and the written value are derived from it exactly as above. When targeting SVE2, also pass `-DJSONIFIER_SVE2_VECTOR_BITS=<bits>` with the target's SVE vector length. SVE2 is only enabled when that length is known, since the backend uses fixed-length SVE types (`-msve-vector-bits`). When cross-compiling without `JSONIFIER_CPU_INSTRUCTIONS` defined, configure warns and falls back to the scalar build (`0`).

The pipe form is self-documenting: pass it into your CI or build scripts and future-you will thank present-you.

Common override values:

| Target | Pipe form | Numeric |
|--------|-----------|---------|
| ARM64 with NEON | `16` | `16` |
| x64 with AVX-512 | `1\|2\|4\|128` | `135` |
| x64 with AVX2 | `1\|2\|4\|64` | `71` |
| x64 with AVX only | `1\|2\|4\|32` | `39` |
| x64 with bit-ops + PCLMULQDQ only (no SIMD) | `1\|2\|4\|8` | `15` |
| x64 with bit-ops only (no SIMD) | `1\|2\|4` | `7` |
| Pure scalar fallback | `0` | `0` |

An override only needs the highest AVX bit you want: `1|2|4|64` and `1|2|4|32|64` configure the same AVX2 build. On MSVC the cascade then adds the lower AVX bits to the written value, so `71` lands in the header as `103` (see the "Important detail" note above).

## The Pure-Scalar Fallback (`JSONIFIER_CPU_INSTRUCTIONS = 0`)

Setting the value to `0` produces a fully portable build with no SIMD and no hardware bit-manipulation intrinsics. Every operation falls back to `std::countl_zero`, `std::popcount`, and scalar C++20 stdlib equivalents.

This mode is slower than any SIMD-enabled build, but it is **fully supported**: every code path has an `#else` branch that reaches for the stdlib scalar version. Use it when:

- You're producing a maximally portable binary for unknown-CPU deployment
- You're building for a target where the intrinsics aren't available
- You want to verify Jsonifier's correctness independent of any SIMD-specific code

## When to Override Detection

**Cross-compiling.** You're building on machine A for machine B. Auto-detection would tell you about A's CPU; the binary needs to run on B. Set `JSONIFIER_CPU_INSTRUCTIONS` to B's feature set.

**Portable binaries.** You're building a binary that will be distributed to machines with different CPU generations. Pick the lowest common denominator across your target audience (often AVX2, sometimes just AVX for maximum compatibility) and override to that.

**Testing lower-tier code paths.** You want to benchmark or debug Jsonifier's AVX2 backend on a machine that has AVX-512. Override to `1|2|4|64` (AVX2 tier) instead of the auto-detected AVX-512 value to force the AVX2 code path.

**Rare OS/CPU mismatches.** The host CPU has AVX-512 but the OS doesn't have ZMM state enabled, as on some older Windows configurations, older hypervisors. Auto-detection handles this correctly, but if you're overriding for another reason, remember to match reality.

## ⚠️ Skipping the CMake Build

If you drop Jsonifier's headers into a project without running its CMake configure step (hand-rolled Makefile, non-CMake build system, or copying headers into a monorepo), the `jsonifier_cpu_instructions.hpp` file will be empty, stale, or wrong, and you'll get one of:

- Compilation failures because the SIMD detection macros aren't defined
- The [flag consistency check](#flag-consistency-check) stopping the build because the header names a tier your compiler flags don't enable
- A silently-selected fallback backend (much slower than expected)
- A binary that uses instructions your CPU doesn't support (crashes at runtime with `SIGILL`)

If you're bypassing CMake, **you must manually edit `include/jsonifier-incl/simd/jsonifier_cpu_instructions.hpp`**, set `JSONIFIER_CPU_INSTRUCTIONS` to a valid value, and pass the matching `-m...` or `/arch:` flags yourself. See [Installation](Installation.md) for the details on this footgun.

## Verifying What Got Selected

After configuring, check the generated header:

```bash
cat include/jsonifier-incl/simd/jsonifier_cpu_instructions.hpp
```

You'll see something like:

```cpp
#define JSONIFIER_CPU_INSTRUCTIONS 111

#define JSONIFIER_SVE2_VECTOR_BITS 0
```

(`111` = `1|2|4|8|32|64`: LZCNT, POPCNT, BMI, PCLMULQDQ, AVX and AVX2, a typical AVX2-tier x64 machine on any compiler. An AVX-512 machine with VBMI2 writes `239`. `JSONIFIER_SVE2_VECTOR_BITS` is `0` unless SVE2 is the selected tier.)

The CMake configure output summarizes the result in two lines, for example:

```
-- voided-hw-detection [JSONIFIER] CPU: AVX2, 16 threads, L1 32768B, L2 1048576B, L3 33554432B, alignment 32, instructions 111, detected TRUE
-- voided-hw-detection [JSONIFIER] CPU features: LZCNT POPCNT BMI1 BMI2 CLMUL FMA F16C AVX AVX2
```

The first line names the tier, the alignment and the value written to the header. `detected FALSE` means no detector output and no override was available (cross-compiling, or the detector failed to build or run), so every property used its fallback. The second line lists every `HAS_*` flag that is set, and those flags are what the compiler flags are built from. When `JSONIFIER_CPU_INSTRUCTIONS` is overridden, an extra `voided-hw-detection [JSONIFIER] CPU: using manually set JSONIFIER_CPU_INSTRUCTIONS` line appears just before them.

## The Check Macros

For internal code paths (and for anyone extending Jsonifier), the header exposes compile-time predicates:

```cpp
#if JSONIFIER_CHECK_FOR_INSTRUCTION(JSONIFIER_POPCNT)
    // POPCNT is available
#endif
```

`JSONIFIER_CHECK_FOR_INSTRUCTION` is a bitwise AND against a specific feature bit. The header also `#error`s if NEON and SVE2 are both set, or if SVE2 is set while `JSONIFIER_SVE2_VECTOR_BITS` is `0`.

Convenience masks for common groups:

- `JSONIFIER_ANY_AVX`: any AVX tier (AVX, AVX2, or AVX-512)
- `JSONIFIER_ANY_SIMD`: any SIMD backend (AVX, AVX2, AVX-512, NEON, or SVE2)

## What's Next

- **[Installation](Installation.md)**: includes the ⚠️ warning about the `jsonifier_cpu_instructions.hpp` header when bypassing CMake
- **[Serializing & Parsing](Usage_Serializing_Parsing.md)**: the runtime API that benefits from correct SIMD selection

---