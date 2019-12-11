cmake_minimum_required(VERSION 3.10)

project(miniDHT)

find_package(Boost REQUIRED COMPONENTS
    thread
    program_options
    filesystem
    system
    date_time
    chrono
    regex
)
find_package(Protobuf REQUIRED)
find_package(sqlite3 CONFIG REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(ZLIB REQUIRED)

include_directories(
    ${PROJECT_SOURCE_DIR}/Sources
    ${PROJECT_SOURCE_DIR}/Tests
    ${PROTOBUF_INCLUDE_DIRS}
    ${CMAKE_CURRENT_BINARY_DIR}
)

protobuf_generate_cpp(
    PROTO_SRCS
    PROTO_HDRS
    ${PROJECT_SOURCE_DIR}/Sources/miniDHT_proto.proto
)

add_library(miniDHT STATIC
    ${PROTO_SRCS}
    ${PROTO_HDRS}
    ${PROJECT_SOURCE_DIR}/Sources/miniDHT.cpp
    ${PROJECT_SOURCE_DIR}/Sources/miniDHT.h
    ${PROJECT_SOURCE_DIR}/Sources/miniDHT_bucket.cpp
    ${PROJECT_SOURCE_DIR}/Sources/miniDHT_bucket.h
    ${PROJECT_SOURCE_DIR}/Sources/miniDHT_const.cpp
    ${PROJECT_SOURCE_DIR}/Sources/miniDHT_const.h
    ${PROJECT_SOURCE_DIR}/Sources/miniDHT_db.cpp
    ${PROJECT_SOURCE_DIR}/Sources/miniDHT_db.h
    ${PROJECT_SOURCE_DIR}/Sources/miniDHT_proto.proto
    ${PROJECT_SOURCE_DIR}/Sources/miniDHT_search.cpp
    ${PROJECT_SOURCE_DIR}/Sources/miniDHT_search.h
    ${PROJECT_SOURCE_DIR}/Sources/miniDHT_session.h
)

add_executable(aes_crypt_test
    ${PROJECT_SOURCE_DIR}/Tests/aes_crypt.h
    ${PROJECT_SOURCE_DIR}/Tests/aes_crypt_test.cpp
)

add_executable(db_key_value
    ${PROJECT_SOURCE_DIR}/Tests/db_key_value.cpp
)

add_executable(db_multi_key_data
    ${PROJECT_SOURCE_DIR}/Tests/db_multi_key_data.cpp
)

add_executable(server
    ${PROJECT_SOURCE_DIR}/Tests/server.cpp
)

add_executable(session
    ${PROJECT_SOURCE_DIR}/Tests/session.cpp
)

add_executable(dht_test
    ${PROJECT_SOURCE_DIR}/Tests/test.cpp
)

target_link_libraries(aes_crypt_test
    ${Boost_LIBRARIES}
    ZLIB::ZLIB
    OpenSSL::SSL OpenSSL::Crypto
)

target_link_libraries(db_key_value
    miniDHT
    protobuf::libprotoc protobuf::libprotobuf protobuf::libprotobuf-lite
    ${Boost_LIBRARIES}
    sqlite3
)

target_link_libraries(db_multi_key_data
    miniDHT
    protobuf::libprotoc protobuf::libprotobuf protobuf::libprotobuf-lite
    ${Boost_LIBRARIES}
    sqlite3
    ZLIB::ZLIB
    OpenSSL::SSL OpenSSL::Crypto
)

target_link_libraries(server
    miniDHT
    protobuf::libprotoc protobuf::libprotobuf protobuf::libprotobuf-lite
    ${Boost_LIBRARIES}
    sqlite3
    ZLIB::ZLIB
    OpenSSL::SSL OpenSSL::Crypto
)

target_link_libraries(session
    miniDHT
    protobuf::libprotoc protobuf::libprotobuf protobuf::libprotobuf-lite
    ${Boost_LIBRARIES}
    sqlite3
    ZLIB::ZLIB
    OpenSSL::SSL OpenSSL::Crypto
)

target_link_libraries(dht_test
    miniDHT
    protobuf::libprotoc protobuf::libprotobuf protobuf::libprotobuf-lite
    ${Boost_LIBRARIES}
    sqlite3
    ZLIB::ZLIB
    OpenSSL::SSL OpenSSL::Crypto
)
