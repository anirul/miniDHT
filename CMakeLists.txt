cmake_minimum_required(VERSION 2.8)

project(miniDHT)

include("cmake/FindCXXFeatures.cmake")

if(APPLE)
    set(CMAKE_C_COMPILER clang)
    set(CMAKE_CPP_COMPILER clang++)
    set(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LANGUAGE_STANDARD c++11)
    set(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LIBRARY libc++)
    set(CMAKE_OSX_ARCHITECTURES x86_64)
    set(CMAKE_LIBRARY_PREFIX /opt/local /usr/local)
endif(APPLE)

if(WIN32 OR APPLE)
    include_directories(
        /opt/local/include
        /opt/local/include/freetype2
        /usr/local/include
        /usr/include
        ${Boost_INCLUDE_DIRS}
        ${PROJECT_SOURCE_DIR}/Sources
        ${PROJECT_SOURCE_DIR}/Tests
        ${PROTOBUF_INCLUDE_DIRS}
        ${CMAKE_CURRENT_BINARY_DIR}
    )
endif(WIN32 OR APPLE)

if(UNIX AND NOT APPLE)
    include_directories(
        /usr/local/include
        /usr/include
        /usr/include/irrlicht
        ${Boost_INCLUDE_DIRS}
        ${PROJECT_SOURCE_DIR}/Sources
        ${PROJECT_SOURCE_DIR}/Tests
        ${PROTOBUF_INCLUDE_DIRS}
        ${CMAKE_CURRENT_BINARY_DIR}
    )
endif(UNIX AND NOT APPLE)

if(WIN32)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_WIN32_WINNT=0x0501")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DBOOST_DATE_TIME_NO_LIB")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DBOOST_REGEX_NO_LIB")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DBOOST_SERIALIZATION_NO_LIB")
    set(BOOST_ROOT "${PROJECT_SOURCE_DIR}/Thirdparty/boost_1_55_0")
endif(WIN32)

if(WIN32 OR APPLE)
    set(Boost_USE_STATIC_LIBS ON)
endif(WIN32 OR APPLE)

find_package(Boost 1.54.0 REQUIRED COMPONENTS
    thread
    program_options
    filesystem
    system
    date_time
    chrono
    regex
    python
)

find_package(Protobuf REQUIRED)
PROTOBUF_GENERATE_CPP(
    PROTO_SRCS
    PROTO_HDRS
    ${PROJECT_SOURCE_DIR}/Sources/miniDHT_proto.proto
)

if(NOT Boost_FOUND)
    message(FATAL_ERROR "Boost not found")
endif()

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

if(APPLE)
    find_library(Z_LIBRARY
        libz.a
        /opt/local/lib
    )

    find_library(SSL_LIBRARY
        libssl.a
        /opt/local/lib
    )

    find_library(CRYPTO_LIBRARY
        libcrypto.a
        /opt/local/lib
    )

    find_library(SQLITE_LIBRARY
        libsqlite3.a
        /opt/local/lib
    )

    find_library(PROTOBUF_LIBRARY
        libprotobuf.a
        /opt/local/lib
    )
endif(APPLE)

target_link_libraries(aes_crypt_test
    ${Boost_LIBRARIES}
    ${Z_LIBRARY}
    ${CRYPTO_LIBRARY}
    ${SSL_LIBRARY}
)

target_link_libraries(db_key_value
    miniDHT
    ${PROTOBUF_LIBRARY}
    ${Boost_LIBRARIES}
    ${SQLITE_LIBRARY}
)

target_link_libraries(db_multi_key_data
    miniDHT
    ${PROTOBUF_LIBRARY}
    ${Boost_LIBRARIES}
    ${SQLITE_LIBRARY}
    ${Z_LIBRARY}
    ${CRYPTO_LIBRARY}
    ${SSL_LIBRARY}
)

target_link_libraries(server
    miniDHT
    ${PROTOBUF_LIBRARY}
    ${Boost_LIBRARIES}
    ${SQLITE_LIBRARY}
    ${Z_LIBRARY}
    ${CRYPTO_LIBRARY}
    ${SSL_LIBRARY}
)

target_link_libraries(session
    miniDHT
    ${PROTOBUF_LIBRARY}
    ${Boost_LIBRARIES}
    ${SQLITE_LIBRARY}
    ${Z_LIBRARY}
    ${CRYPTO_LIBRARY}
    ${SSL_LIBRARY}
)

target_link_libraries(dht_test
    miniDHT
    ${PROTOBUF_LIBRARY}
    ${Boost_LIBRARIES}
    ${SQLITE_LIBRARY}
    ${Z_LIBRARY}
    ${CRYPTO_LIBRARY}
    ${SSL_LIBRARY}
)
