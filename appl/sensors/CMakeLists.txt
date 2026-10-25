file(GLOB_RECURSE APPL_SENSOR_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

add_library(appl_sensors STATIC
    ${APPL_SENSOR_SOURCES}
)

target_include_directories(appl_sensors PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/inc"
)

target_link_libraries(appl_sensors PUBLIC
    platform
)

target_compile_features(appl_sensors PUBLIC
    cxx_std_20
)

target_compile_options(appl_sensors PRIVATE
    -Wall
    -Wextra
    -Wpedantic
    -fno-exceptions
    -fno-rtti
    -fno-threadsafe-statics
)