cmake_minimum_required(VERSION 3.21)

project(share-everything CXX)

add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")

enable_language(CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
  if(CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
    string(REGEX REPLACE "/W[0-4]" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
  else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
  endif()
elseif(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wno-long-long -pedantic")
endif()

if(WIN32)
  set(CMAKE_SHARED_LINKER_FLAGS /MANIFEST:NO)
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS")
endif(WIN32)

add_definitions(-D_UNICODE -DUNICODE)

set(ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(SRC_DIR ${ROOT_DIR}/src)

include(FetchContent)

FetchContent_Declare(
  rapidjson
  GIT_REPOSITORY https://github.com/Tencent/rapidjson.git
  GIT_TAG v1.1.0
)
FetchContent_GetProperties(rapidjson)

if(NOT rapidjson_POPULATED)
  FetchContent_Populate(rapidjson)
endif()

include_directories(${rapidjson_SOURCE_DIR}/include)

set(CMAKE_USE_OPENSSL)
include(FindOpenSSL)
include(FindCURL)

include(ExternalProject)

ExternalProject_Add(
  zlib
  GIT_REPOSITORY "https://github.com/madler/zlib"
  GIT_TAG "v1.2.12"
  PREFIX "${CMAKE_BINARY_DIR}/zlib"
  CMAKE_CACHE_ARGS
  "-DBUILD_SHARED_LIBS:BOOL=OFF"
  "-DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_BINARY_DIR}"
)

include_directories(${PROJECT_BINARY_DIR}/include)
link_directories(${PROJECT_BINARY_DIR}/lib)

include_directories(${SRC_DIR})
add_subdirectory(${SRC_DIR})
