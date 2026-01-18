cmake_minimum_required(VERSION 3.16)
project(curl_to_padded_string)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

enable_testing()

# Find curl
find_package(CURL REQUIRED)

# Fetch simdjson
include(FetchContent)
FetchContent_Declare(
  simdjson
  GIT_REPOSITORY https://github.com/simdjson/simdjson.git
  GIT_TAG 182eac0092aa587b6b2b3718da897d2c40642360
)
FetchContent_MakeAvailable(simdjson)

# Header-only library
add_library(curl_to_padded_string INTERFACE)
target_include_directories(curl_to_padded_string INTERFACE include)
target_link_libraries(curl_to_padded_string INTERFACE CURL::libcurl simdjson::simdjson)

# Tests
add_subdirectory(test)