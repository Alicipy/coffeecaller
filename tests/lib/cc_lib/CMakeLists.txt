cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(aldrin_buzzer_test)

target_sources(app PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src/test_aldrin.c
        ${ZEPHYR_COFFEECALLER_MODULE_DIR}/lib/cc_lib/aldrin.c
)
