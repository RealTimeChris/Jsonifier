### CPU Architecture Detection Configuration
----
The CPU architecture detection and configuration in Jsonifier's CMakeLists.txt file are designed to support the following architectures: x64, AVX, AVX2, AVX-512, and ARM-NEON. Let's explore each architecture in detail:

#### x64 Architecture
----
The x64 architecture, also known as x86-64 or AMD64, is a 64-bit extension of the x86 instruction set architecture. It provides increased memory addressability and larger general-purpose registers, enabling more efficient processing of 64-bit data. The x64 architecture is widely used in modern CPUs, offering improved performance and expanded capabilities compared to its 32-bit predecessor.

#### ARM-NEON
----
ARM-NEON (ARM Advanced SIMD) is a set of SIMD (Single Instruction, Multiple Data) instructions for ARM processors, providing similar capabilities to x86's SSE and AVX. NEON offers parallel processing of 64-bit and 128-bit data types, enabling efficient multimedia and signal processing operations on ARM-based devices. Jsonifier leverages ARM-NEON instructions to optimize performance on ARM architectures.

#### AVX (Advanced Vector Extensions)
----
AVX, short for Advanced Vector Extensions, is an extension to the x86 instruction set architecture. AVX provides SIMD (Single Instruction, Multiple Data) instructions for performing parallel processing on vectors of data. It introduces 128-bit vector registers (XMM registers) and new instructions to accelerate floating-point and integer calculations. AVX is supported by many modern CPUs and offers significant performance benefits for applications that can utilize parallel processing.

#### AVX2 (Advanced Vector Extensions 2)
----
AVX2 is an extension of the AVX instruction set architecture. It builds upon the foundation of AVX and introduces additional instructions and capabilities for SIMD processing. AVX2 expands the vector register size to 256 bits (YMM registers) and introduces new integer and floating-point operations, enabling further optimization of vectorized code. CPUs that support AVX2 offer enhanced performance for applications that leverage these advanced instructions.

#### AVX-512 (Advanced Vector Extensions 512-bit)
----
AVX-512 is an extension of the AVX instruction set architecture, designed to provide even higher levels of vector parallelism. AVX-512 introduces 512-bit vector registers (ZMM registers) and a broad range of new instructions for both floating-point and integer operations. With AVX-512, CPUs can process larger amounts of data in parallel, offering significant performance improvements for applications that can effectively utilize these capabilities.

### Manual Configuration
----
In addition to automatic CPU architecture detection, Jsonifier's CMake configuration also allows for manual control over specific CPU instructions. You can manually set the `JSONIFIER_CPU_FLAGS` variable in the CMake configuration to fine-tune the instruction sets used. This variable can be set to one of the following options, or combined for optimal performance:

#### Setting `JSONIFIER_CPU_FLAGS`
You can configure the instruction sets by specifying the `JSONIFIER_CPU_FLAGS` in your CMake build configuration:

- **For AVX-512**: Set `JSONIFIER_CPU_FLAGS=-DJSONIFIER_CPU_FLAGS=/arch:AVX512`  OR `JSONIFIER_CPU_FLAGS=-DJSONIFIER_CPU_FLAGS=-march=avx512`
- **For AVX2**: Set `JSONIFIER_CPU_FLAGS=-DJSONIFIER_CPU_FLAGS=/arch:AVX2`  OR `JSONIFIER_CPU_FLAGS=-DJSONIFIER_CPU_FLAGS=-march=avx2`
- **For AVX**: Set `JSONIFIER_CPU_FLAGS=-DJSONIFIER_CPU_FLAGS=/arch:AVX`  OR `JSONIFIER_CPU_FLAGS=-DJSONIFIER_CPU_FLAGS=-march=avx`
- **For ARM-NEON**: Set `JSONIFIER_CPU_FLAGS=-DJSONIFIER_CPU_FLAGS=-mfpu=neon`
- **For BMI1**: Set `JSONIFIER_CPU_FLAGS=-DJSONIFIER_CPU_FLAGS=-mbmi`
- **For LZCNT**: Set `JSONIFIER_CPU_FLAGS=-DJSONIFIER_CPU_FLAGS=-mlzcnt`
- **For POPCNT**: Set `JSONIFIER_CPU_FLAGS=-DJSONIFIER_CPU_FLAGS=-mpopcnt`

NOTE: BMI1, LZCNT, and POPCNT can be combined with any of themselves or the other flags here, however, AVX, AVX2, and AVX512 cannot be combined with NEON.

This configuration allows you to manually or automatically optimize Jsonifier for your specific CPU architecture, ensuring that the code generated will make the best use of the available hardware features for maximum performance.