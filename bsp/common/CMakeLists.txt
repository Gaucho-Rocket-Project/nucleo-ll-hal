file(GLOB_RECURSE BSP_COMMON_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

add_library(bsp_common STATIC ${BSP_COMMON_SOURCES})
target_include_directories(bsp_common PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/inc")
target_link_libraries(bsp_common PUBLIC mcal_config)
target_compile_features(bsp_common PUBLIC cxx_std_20)
target_compile_options(bsp_common PRIVATE
    -Wall -Wextra -Wpedantic
    -fno-exceptions -fno-rtti -fno-threadsafe-statics
)
