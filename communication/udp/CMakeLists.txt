# Specify the version used
cmake_minimum_required(VERSION 2.8)

# Add source files
add_sources(
    CMakeLists.txt
    UdpServer.cpp
    UdpServer.hpp
    UdpClient.cpp
    UdpClient.hpp
    UdpSerializeServer.hpp
    UdpSerializeClient.hpp
)
