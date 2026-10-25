file(GLOB_RECURSE APPL_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

set(FIRMWARE_TEST "" CACHE STRING
    "Hardware test name in tests/hardware (empty builds the application)")

if(FIRMWARE_TEST STREQUAL "")
    target_sources(${PROJECT_NAME} PRIVATE ${APPL_SOURCES})
    message(STATUS "Firmware entry point: appl/src/main.cpp")
else()
    if(NOT FIRMWARE_TEST MATCHES "^[A-Za-z0-9_]+$")
        message(FATAL_ERROR "FIRMWARE_TEST must be a test name, e.g. uart or uart_dma")
    endif()
    set(TEST_SOURCE "${PROJECT_SOURCE_DIR}/tests/hardware/${FIRMWARE_TEST}.cpp")
    if(NOT EXISTS "${TEST_SOURCE}")
        message(FATAL_ERROR "Hardware test does not exist: ${TEST_SOURCE}")
    endif()
    target_sources(${PROJECT_NAME} PRIVATE "${TEST_SOURCE}")
    message(STATUS "Firmware entry point: ${TEST_SOURCE}")
endif()

add_subdirectory(sensors)

target_link_libraries(${PROJECT_NAME} PRIVATE
    appl_sensors
)
