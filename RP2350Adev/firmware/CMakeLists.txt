cmake_minimum_required(VERSION 3.13)

set(PICO_SDK_PATH ${CMAKE_CURRENT_LIST_DIR}/pico-sdk)
set(PICO_PLATFORM rp2350-arm-s)

include(pico-sdk/pico_sdk_init.cmake)

project(rp2350a_blink C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

add_executable(rp2350a_blink rp2350a.cpp)
target_link_libraries(rp2350a_blink pico_stdlib)
pico_add_extra_outputs(rp2350a_blink)
