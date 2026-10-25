file(GLOB_RECURSE PLATFORM_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

# Interrupt vectors must be linked directly into the firmware executable.
# Leaving them only inside a static library can allow the linker to retain
# the weak handlers from the STM32 startup file instead.
set(INTERRUPT_VECTOR_SOURCE
    "${CMAKE_CURRENT_SOURCE_DIR}/src/interrupt_vectors.cpp"
)

list(REMOVE_ITEM PLATFORM_SOURCES
    "${INTERRUPT_VECTOR_SOURCE}"
)

add_library(platform STATIC
    ${PLATFORM_SOURCES}
)

target_include_directories(platform PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/inc"
)

target_link_libraries(platform PUBLIC
    mcal_config
)

target_compile_features(platform PUBLIC
    cxx_std_20
)

target_compile_options(platform PRIVATE
    -Wall
    -Wextra
    -Wpedantic
    -fno-exceptions
    -fno-rtti
    -fno-threadsafe-statics
)

# Add the vector implementation directly to nucleo_hal.elf.
target_sources(${PROJECT_NAME} PRIVATE
    "${INTERRUPT_VECTOR_SOURCE}"
)