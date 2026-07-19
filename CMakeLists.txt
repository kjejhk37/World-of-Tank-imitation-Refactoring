cmake_minimum_required(VERSION 3.20)

project(WorldOfTankImitationRefactoring LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
    # 소스 내 한글 주석이 로컬 코드페이지(949)로 오인되어 C4819가 나는 것을 방지 — 소스/실행 문자셋을 UTF-8로 고정.
    add_compile_options(/utf-8)
endif()

include(FetchContent)

# nlohmann::json은 JsonDataStore 구현(내부)에서만 쓰인다 — app_lib에 PRIVATE로 링크해
# main/tests가 이 라이브러리의 존재를 include 경로 상으로도 알 수 없게 한다.
# (docs/dependency/nlohmann_json_20260719.md 참고)
FetchContent_Declare(
    nlohmann_json
    URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_MakeAvailable(nlohmann_json)

add_library(app_lib STATIC
    src/config/LaunchConfigParser.cpp
    src/config/LaunchConfigStore.cpp
    src/config/ConfigManager.cpp
    src/platform/Win32Window.cpp
    src/renderer/RendererFactory.cpp
    src/renderer/RendererBackendParser.cpp
    src/renderer/directx9/DirectX9Renderer.cpp
    src/renderer/directx11/DirectX11Renderer.cpp
    src/renderer/directx12/DirectX12Renderer.cpp
    src/renderer/opengl/OpenGLRenderer.cpp
    src/serialization/DataRecordAccess.cpp
    src/serialization/JsonDataStore.cpp
    src/serialization/SqliteDataStore.cpp
    src/persistence/SaveLoadManager.cpp
    src/persistence/PlayerProgress.cpp
)
target_include_directories(app_lib PUBLIC src)
target_link_libraries(app_lib PRIVATE nlohmann_json::nlohmann_json)

# DirectX 9/11/12는 Windows SDK에 포함된 시스템 라이브러리라 FetchContent 대상이 아니다(googletest/nlohmann::json과 다름).
target_link_libraries(app_lib PUBLIC d3d9.lib d3d11.lib d3d12.lib dxgi.lib)

add_executable(main src/app/main.cpp)
target_link_libraries(main PRIVATE app_lib)

# GoogleTest는 tests 타겟에만 링크한다 — main/app_lib은 테스트 프레임워크에 의존하지 않는다.
# (docs/dependency/googletest_20260715.md 참고)
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
    tests/RendererBackendParserTest.cpp
    tests/DataRecordAccessTest.cpp
    tests/JsonDataStoreTest.cpp
    tests/SqliteDataStoreTest.cpp
    tests/ConfigManagerTest.cpp
    tests/SaveLoadManagerTest.cpp
    tests/PlayerProgressTest.cpp
    tests/Win32WindowTest.cpp
    tests/DirectX9RendererTest.cpp
    tests/DirectX11RendererTest.cpp
    tests/DirectX12RendererTest.cpp
)
target_link_libraries(tests PRIVATE app_lib gtest_main)

include(GoogleTest)
gtest_discover_tests(tests)
