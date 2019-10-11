cmake_minimum_required(VERSION 3.10)

if(NOT DEFINED ENV{VCPKG_TARGET_TRIPLET})
	if(DEFINED ENV{VCPKG_DEFAULT_TRIPLET})
		set(VCPKG_TARGET_TRIPLET "$ENV{VCPKG_DEFAULT_TRIPLET}")
		message("Set the target triplet to := " ${VCPKG_TARGET_TRIPLET})
	endif()
endif()

if(DEFINED ENV{VCPKG_ROOT} AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
      CACHE STRING "")
endif()

set(VCPKG_TARGET_ARCHITECTURE x64)

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
    find_path(BOOST_INCLUDE_DIR boost)
	include_directories(
        /opt/local/include
        /opt/local/include/freetype2
        /usr/local/include
        /usr/include
        ${BOOST_INCLUDE_DIR}
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
endif(WIN32)

if(WIN32 OR APPLE)
    set(Boost_USE_STATIC_LIBS ON)
endif(WIN32 OR APPLE)

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
protobuf_generate_cpp(
    PROTO_SRCS
    PROTO_HDRS
    ${PROJECT_SOURCE_DIR}/Sources/miniDHT_proto.proto
)

if(WIN32)
	find_package(Sqlite3 CONFIG REQUIRED)
	find_package(OpenSSL REQUIRED)
	find_package(ZLIB REQUIRED)
endif(WIN32)

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

if(WIN32)
	set(SQLITE_LIBRARY sqlite3)
	set(PROTOBUF_LIBRARY protobuf::libprotoc protobuf::libprotobuf protobuf::libprotobuf-lite)
	set(SSL_LIBRARY OpenSSL::SSL)
	set(CRYPTO_LIBRARY OpenSSL::Crypto)
	set(Z_LIBRARY ZLIB::ZLIB)
endif(WIN32)

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
