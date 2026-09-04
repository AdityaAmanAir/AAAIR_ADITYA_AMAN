cmake_minimum_required(VERSION 3.16)

project(portfolio_server LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Threads REQUIRED)
find_package(OpenSSL REQUIRED)

file(GLOB BACKEND_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/backend/src/*.cpp"
)

add_executable(server ${BACKEND_SOURCES})

target_compile_definitions(server PRIVATE CPPHTTPLIB_OPENSSL_SUPPORT)

target_include_directories(server PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/backend/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/backend/third_party"
)

target_compile_options(server PRIVATE -O2 -Wall -Wextra)
target_link_libraries(server PRIVATE
    Threads::Threads
    OpenSSL::SSL
    OpenSSL::Crypto
)