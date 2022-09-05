# Relocatable Program Module - Runtime Library
The Relocatable Program Module Runtime Library, or *libRPM* for short, is a minimal (<10KB) framework for dynamic code loading on embedded systems.
# Features

 - Runs on almost any little-endian 32-bit ARM CPU with a C++ compiler
 - Blazing fast (loads a 10KB, ~200 relocation file in around 5ms on ARM946E-S)
 - Written in pure C++ - no standard libraries required (that's right, not even libstdc++!)
 - Automatically links import symbols between modules
 - Supports Win32 targets for debugging/testing
 - Allows for strict user-defined allocations/heap management
 - File format supports arbitrary user metadata
 - Slightly more storage-efficient than ELF
 - Full control over relocation/symbol table stripping after load
# Building libRPM
LibRPM can be built using the CMake build system. A target platform has to be defined using the `RPM_PLATFORM` variable, for example:

`-DRPM_PLATFORM=ARMv5T`

Currently, only the `ARMv5T` and `Win32` targets are defined. LibRPM is guaranteed to build on `arm-none-eabi-gcc` and `mingw32-gcc`.
The build output can then be used as a static library on the target system.

LibRPM also depends on certain [ExtLib](https://github.com/HelloOO7/ExtLib) sources, wherefore it expects it to be cloned into the parent directory as follows:  
  
` - <root>`  
` └─ libRPM`  
` └─ ExtLib`  

# Linking RPM executables
The stock linker suite for the RPM format is available at https://github.com/HelloOO7/RPMAuthoringTools.
