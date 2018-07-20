# Specify the version used
cmake_minimum_required(VERSION 2.8)

# Add source files
add_sources(
    CMakeLists.txt
# Basic communication files
    BasicIoDevice.hpp
    SerialConnection.cpp
    SerialConnection.hpp
# ADCS simulation specific files
    AdcsSimTcpServer.cpp
    AdcsSimTcpServer.hpp
)
