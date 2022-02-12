cmake_minimum_required(VERSION 3.10)

project(libSMRP C CXX ASM)

#添加静态库
add_library(SMRP STATIC)

#添加包含目录
target_include_directories(SMRP PUBLIC ./)

#添加源代码
file(GLOB  LIBSMRP_C_FILES *.c *.C)

target_sources(SMRP PUBLIC ${LIBSMRP_C_FILES})

