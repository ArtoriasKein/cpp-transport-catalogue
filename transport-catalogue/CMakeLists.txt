cmake_minimum_required(VERSION 3.18)
project(transport_catalogue)

set(CMAKE_CXX_STANDARD 17)

set(Protobuf_LIBRARY_DEBUG "D:/Course/pb/package/lib/libprotobufd.lib")
set(Protobuf_LIBRARY_RELEASE "D:/Course/pb/package/lib/libprotobuf.lib")
set(Protobuf_PROTOC_EXECUTABLE "D:/Course/pb/package/bin/protoc.exe")
set(Protobuf_INCLUDE_DIR "D:/Course/pb/package/include")

find_package(Protobuf REQUIRED)
find_package(Threads REQUIRED)

file(GLOB HEADERS "*.h")
file(GLOB SOURCES "*.cpp")
file(GLOB PROTO_FILES "*.proto")

protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${PROTO_FILES})

add_executable(
    ${PROJECT_NAME}
    ${PROTO_SRCS}
    ${PROTO_HDRS}
    ${HEADERS}
    ${SOURCES}
)

target_include_directories(transport_catalogue PUBLIC ${Protobuf_INCLUDE_DIRS})
target_include_directories(transport_catalogue PUBLIC ${CMAKE_CURRENT_BINARY_DIR})

string(REPLACE "libprotobuf.lib" "libprotobufd.lib" "Protobuf_LIBRARY_DEBUG" "${Protobuf_LIBRARY_DEBUG}")
string(REPLACE "protobuf.a" "protobufd.a" "Protobuf_LIBRARY_DEBUG" "${Protobuf_LIBRARY_DEBUG}")

target_link_libraries(transport_catalogue "$<IF:$<CONFIG:Debug>,${Protobuf_LIBRARY_DEBUG},${Protobuf_LIBRARY}>" Threads::Threads)
