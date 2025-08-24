cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

project(cc_coffeecall_shell_sample)

target_sources(app PRIVATE
  ${CMAKE_CURRENT_LIST_DIR}/src/main.c
)
