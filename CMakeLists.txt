cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(coffeecaller)

# include custom headers for e.g. drivers or sub-systems sorted by sub-folders
zephyr_include_directories(include)

add_subdirectory(lib)

add_subdirectory(subsys)

add_subdirectory(drivers)

