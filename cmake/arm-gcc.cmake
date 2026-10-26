
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)

set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

# Find the ARM GNU toolchain from the host PATH.
find_program(ARM_GNU_GCC
    NAMES arm-none-eabi-gcc
    REQUIRED
)

get_filename_component(
    ARM_GNU_BIN
    "${ARM_GNU_GCC}"
    DIRECTORY
)

message(STATUS "ARM GNU toolchain: ${ARM_GNU_BIN}")

# Compilers
set(CMAKE_C_COMPILER
    "${ARM_GNU_BIN}/arm-none-eabi-gcc"
)

set(CMAKE_CXX_COMPILER
    "${ARM_GNU_BIN}/arm-none-eabi-g++"
)

set(CMAKE_ASM_COMPILER
    "${ARM_GNU_BIN}/arm-none-eabi-gcc"
)

# Binary utilities
set(CMAKE_AR
    "${ARM_GNU_BIN}/arm-none-eabi-ar"
)

set(CMAKE_RANLIB
    "${ARM_GNU_BIN}/arm-none-eabi-ranlib"
)

set(CMAKE_OBJCOPY
    "${ARM_GNU_BIN}/arm-none-eabi-objcopy"
)

set(CMAKE_SIZE
    "${ARM_GNU_BIN}/arm-none-eabi-size"
)
