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
In addition to automatic CPU architecture detection, Jsonifier's CMake configuration also allows for manual control over specific CPU instructions. You can manually set the JSONIFIER_CPU_INSTRUCTIONS variable in the CMake configuration to fine-tune the instruction sets used. Here are the values you can use for different instruction sets:

- JSONIFIER_CPU_INSTRUCTIONS for AVX-512: Set to 1 << 7
- JSONIFIER_CPU_INSTRUCTIONS for AVX2: Set to 1 << 6
- JSONIFIER_CPU_INSTRUCTIONS for AVX: Set to 1 << 5
- JSONIFIER_CPU_INSTRUCTIONS for ARM-NEON: Set to 1 << 4
- JSONIFIER_CPU_INSTRUCTIONS for BMI2: Set to 1 << 3
- JSONIFIER_CPU_INSTRUCTIONS for BMI: Set to 1 << 2
- JSONIFIER_CPU_INSTRUCTIONS for POPCNT: Set to 1 << 1
- JSONIFIER_CPU_INSTRUCTIONS for LZCOUNT: Set to 1 << 0

You can combine LZCNT, BMI, BMI2, and POPCNT with each other or any of the AVX types (AVX, AVX2, AVX-512, ARM-NEON) to optimize Jsonifier for your specific use case. However, please note that you cannot combine multiple AVX/ARM-NEON types together, as they are distinct and cannot be used simultaneously. This flexibility in instruction set configuration allows you to tailor Jsonifier's performance to your target CPU architecture and application requirements effectively.

### Configuration Explanation
----
The configuration script in Jsonifier's CMakeLists.txt file detects the CPU architecture and sets the appropriate compiler flags based on the supported architectures. It ensures that the generated code takes full advantage of the available instruction sets and achieves the best possible performance on the target CPU. Additionally, the manual configuration option allows you to customize the instruction sets for further optimization according to your specific needs.
