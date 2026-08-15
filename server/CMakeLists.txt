cmake_minimum_required(VERSION 3.16)
project(file_server LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

include(FetchContent)

# Header-only HTTP server. The tag is pinned deliberately, for reproducible builds.
FetchContent_Declare(
  httplib
  GIT_REPOSITORY https://github.com/yhirose/cpp-httplib.git
  GIT_TAG        v0.18.3
  GIT_SHALLOW    TRUE
)

FetchContent_Declare(
  nlohmann_json
  GIT_REPOSITORY https://github.com/nlohmann/json.git
  GIT_TAG        v3.11.3
  GIT_SHALLOW    TRUE
)

FetchContent_MakeAvailable(httplib nlohmann_json)

# SHA-256 comes from OpenSSL rather than a hand-rolled implementation:
# less code, fewer places to get it wrong.
find_package(OpenSSL REQUIRED)

add_executable(file_server
  src/main.cpp
  src/path_resolver.cpp
  src/fs_service.cpp
  src/api.cpp
)

target_include_directories(file_server PRIVATE src)

target_link_libraries(file_server
  PRIVATE
    httplib::httplib
    nlohmann_json::nlohmann_json
    OpenSSL::Crypto
)

target_compile_options(file_server PRIVATE -Wall -Wextra -Wpedantic)
