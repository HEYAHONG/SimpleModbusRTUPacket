cmake_minimum_required(VERSION 3.12)

project(ModbusMasterTestWin32 C CXX ASM)


#添加可执行文件
add_executable(ModbusMasterTestWin32)

#设置C++标准
set_property(TARGET ModbusMasterTestWin32 PROPERTY CXX_STANDARD 20)

#添加SimpleModbusRTUPacket
add_subdirectory(../../ lib)
target_link_libraries(ModbusMasterTestWin32 SMRP)

#添加argtable3库
file(GLOB  ARGTABLE3_C_FILES 3rdparty/argtable3/src/*.c)
target_sources(ModbusMasterTestWin32 PUBLIC ${ARGTABLE3_C_FILES})
target_include_directories(ModbusMasterTestWin32 PRIVATE 3rdparty/argtable3/src)

#添加线程库
FIND_PACKAGE(Threads REQUIRED)
TARGET_LINK_LIBRARIES(ModbusMasterTestWin32  ${CMAKE_THREAD_LIBS_INIT})

#添加串口库
if(WIN32)
file(GLOB  SERIALPORT_C_FILES 3rdparty/SPinGW/serialport.c)
target_sources(ModbusMasterTestWin32 PUBLIC ${SERIALPORT_C_FILES})
target_include_directories(ModbusMasterTestWin32 PRIVATE 3rdparty/SPinGW/)
else()
message(FATAL_ERROR "只支持Windows")
endif()

#添加源代码
file(GLOB  ModbusMasterTestWin32_C_FILES *.cpp *.CPP *.c *.C)
target_sources(ModbusMasterTestWin32 PUBLIC ${ModbusMasterTestWin32_C_FILES})
