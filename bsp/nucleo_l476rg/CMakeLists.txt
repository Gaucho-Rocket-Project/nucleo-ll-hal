add_library(bsp_board STATIC
    src/board.cpp
    src/system_clock.cpp
    src/error_handler.cpp
)
target_include_directories(bsp_board PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/inc")
target_link_libraries(bsp_board PUBLIC bsp_common)
target_compile_options(bsp_board PRIVATE
    -Wall -Wextra -Wpedantic
    -fno-exceptions -fno-rtti -fno-threadsafe-statics
)

# Link strong handlers directly so startup's weak aliases cannot satisfy them
# before the linker extracts objects from a static library.
target_sources(${PROJECT_NAME} PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/src/interrupt_vectors.cpp"
)
