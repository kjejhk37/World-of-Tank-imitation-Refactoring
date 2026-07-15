cmake_minimum_required(VERSION 3.20)

project(WorldOfTankImitationRefactoring LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
    # 소스 내 한글 주석이 로컬 코드페이지(949)로 오인되어 C4819가 나는 것을 방지 — 소스/실행 문자셋을 UTF-8로 고정.
    add_compile_options(/utf-8)
endif()

add_library(app_lib STATIC
    src/config/LaunchConfigParser.cpp
    src/config/LaunchConfigStore.cpp
    src/renderer/RendererFactory.cpp
    src/renderer/DirectXRenderer.cpp
    src/renderer/OpenGLRenderer.cpp
)
target_include_directories(app_lib PUBLIC src)

add_executable(main src/main.cpp)
target_link_libraries(main PRIVATE app_lib)

# GoogleTest는 tests 타겟에만 링크한다 — main/app_lib은 테스트 프레임워크에 의존하지 않는다.
# (docs/dependency/googletest_20260715.md 참고)
include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.17.0.zip
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(tests
    tests/LaunchConfigParserTest.cpp
    tests/LaunchConfigStoreTest.cpp
    tests/RendererFactoryTest.cpp
)
target_link_libraries(tests PRIVATE app_lib gtest_main)

include(GoogleTest)
gtest_discover_tests(tests)
