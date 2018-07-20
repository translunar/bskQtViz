# Specify the version used
cmake_minimum_required(VERSION 2.8)

# Add source files
add_sources(
    CMakeLists.txt
    TcpServer.cpp
    TcpServer.hpp
    TcpClient.cpp
    TcpClient.hpp
    TcpSerializeClient.hpp
    TcpSerializeServer.hpp
    TcpSerializeConnection.hpp
    TcpJsonClient.hpp
    TcpConnection.hpp
)
