cmake_minimum_required(VERSION 3.20)

project(WorldOfTankImitationRefactoring LANGUAGES CXX C)

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

# Dear ImGui는 nlohmann_json/googletest와 달리 소비자용 CMakeLists.txt를 제공하지 않는다 —
# FetchContent는 소스만 받아오고(add_subdirectory 없음), 필요한 .cpp만 app_lib에 직접 추가한다.
# (docs/dependency/imgui_20260719.md 참고)
FetchContent_Declare(
    imgui
    URL https://github.com/ocornut/imgui/archive/refs/tags/v1.92.8.zip
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_MakeAvailable(imgui)

# tinyobjloader는 소비자용 CMakeLists.txt를 제공한다 - nlohmann_json과 동일한 패턴.
# tinyobjloader v1.0.7의 CMakeLists.txt가 cmake_minimum_required(VERSION 2.8.11)을 선언하는데,
# 최신 CMake(4.x)는 3.5 미만의 호환성을 완전히 제거했다 - 이 서브프로젝트에 한해 정책 최소 버전을 낮춰준다.
# (docs/dependency/tinyobjloader_20260723.md 참고)
set(CMAKE_POLICY_VERSION_MINIMUM 3.5)
FetchContent_Declare(
    tinyobjloader
    URL https://github.com/tinyobjloader/tinyobjloader/archive/refs/tags/v1.0.7.zip
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_MakeAvailable(tinyobjloader)
unset(CMAKE_POLICY_VERSION_MINIMUM)

# ufbx는 imgui와 마찬가지로 소비자용 CMakeLists.txt를 제공하지 않는 단일 소스 라이브러리다 -
# FetchContent는 소스만 받아오고, ufbx.c를 app_lib에 직접 추가한다.
# (docs/dependency/ufbx_20260723.md 참고)
FetchContent_Declare(
    ufbx
    URL https://github.com/ufbx/ufbx/archive/refs/tags/v0.23.0.zip
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_MakeAvailable(ufbx)

# cgltf는 단일 헤더 라이브러리다 - CgltfWrapper.cpp 안에서 CGLTF_IMPLEMENTATION을 정의해 구현부를
# 생성하므로, 여기서는 include 경로만 추가한다(소스 파일을 app_lib에 넣지 않는다).
# (docs/dependency/cgltf_20260723.md 참고)
FetchContent_Declare(
    cgltf
    URL https://github.com/jkuhlmann/cgltf/archive/refs/tags/v1.15.zip
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_MakeAvailable(cgltf)

# src 계층 구조(docs/architecture/platform_graphics_projects.md)를 빌드 타겟 수준에서 강제한다 —
# platform_lib는 어떤 상위 타겟도 링크하지 않는다(platform은 graphics/projects를 몰라야 함).
add_library(platform_lib STATIC
    src/platform/logging/LogFormatter.cpp
    src/platform/logging/ConsoleLogSink.cpp
    src/platform/logging/FileLogSink.cpp
    src/platform/logging/Logger.cpp
    src/platform/logging/Log.cpp
    src/platform/Win32Window.cpp
    src/platform/serialization/DataRecordAccess.cpp
    src/platform/serialization/JsonDataStore.cpp
    src/platform/serialization/SqliteDataStore.cpp
    src/platform/persistence/SaveLoadManager.cpp
    src/platform/math/Matrix4x4.cpp
    src/platform/geometry/Intersections.cpp
    src/platform/geometry/Geometry.cpp
    src/platform/collision/gjk_epa/Gjk.cpp
    src/platform/collision/gjk_epa/Epa.cpp
    src/platform/collision/gjk_epa/GjkEpa.cpp
    src/platform/collision/sweep_and_prune/SweepAndPrune.cpp
    src/platform/collision/bvh/Bvh.cpp
    src/platform/collision/rtree/RTree.cpp
    src/platform/collision/voxel_grid/VoxelGrid.cpp
    src/platform/collision/spatial_hash/SpatialHash.cpp
    src/platform/collision/octree/Octree.cpp
    src/platform/collision/kdtree/KdTree.cpp
    src/platform/collision/mpr/Mpr.cpp
    src/platform/collision/conservative_advancement/ConservativeAdvancement.cpp
    src/platform/collision/fcl/Fcl.cpp
    src/platform/entity_component/Entity.cpp
    src/platform/entity_component/EntityManager.cpp
    src/platform/model_import/TinyObjWrapper.cpp
    src/platform/model_import/UfbxWrapper.cpp
    src/platform/model_import/CgltfWrapper.cpp
    src/platform/model_import/ModelLoader.cpp
    src/platform/model_import/RefCountingCachePolicy.cpp
    src/platform/model_import/LruCachePolicy.cpp
    src/platform/model_import/Win32FileWatcher.cpp
    src/platform/model_import/HotReloadCachePolicy.cpp
    src/platform/model_import/NaiveSignatureDeduplicator.cpp
    src/platform/model_import/MeshManager.cpp
    ${ufbx_SOURCE_DIR}/ufbx.c
)
target_include_directories(platform_lib PUBLIC src)
target_include_directories(platform_lib PRIVATE ${ufbx_SOURCE_DIR} ${cgltf_SOURCE_DIR})
target_link_libraries(platform_lib PRIVATE nlohmann_json::nlohmann_json tinyobjloader)

# graphics_lib는 platform_lib에는 의존해도 되지만 projects(app_lib)는 몰라야 한다.
add_library(graphics_lib STATIC
    src/graphics/renderer/RendererFactory.cpp
    src/graphics/renderer/RendererBackendParser.cpp
    src/graphics/renderer/ShaderBytecodeLoader.cpp
    src/graphics/renderer/WicImageLoader.cpp
    src/graphics/renderer/directx9/DirectX9Renderer.cpp
    src/graphics/renderer/directx11/DirectX11Renderer.cpp
    src/graphics/renderer/directx11/DirectX11ComputeDemo.cpp
    src/graphics/renderer/directx12/DirectX12Renderer.cpp
    src/graphics/renderer/directx12/DirectX12ComputeDemo.cpp
    src/graphics/renderer/opengl/OpenGLRenderer.cpp
    src/graphics/ui/directx9/ImGuiManagerDX9.cpp
    src/graphics/ui/directx11/ImGuiManagerDX11.cpp
    src/graphics/ui/directx12/ImGuiManagerDX12.cpp
    src/graphics/ui/widgets/UiElementRegistry.cpp
    src/graphics/ui/widgets/ButtonUI.cpp
    src/graphics/ui/widgets/ProgressUI.cpp
    src/graphics/ui/widgets/StringFieldUI.cpp
    src/graphics/ui/widgets/LabelUI.cpp
    src/graphics/ui/widgets/ImageUI.cpp
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_win32.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_dx9.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_dx11.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_dx12.cpp
)
target_include_directories(graphics_lib PUBLIC src)
target_include_directories(graphics_lib PRIVATE ${imgui_SOURCE_DIR} ${imgui_SOURCE_DIR}/backends)
target_link_libraries(graphics_lib PUBLIC platform_lib)

# DirectX 9/11/12는 Windows SDK에 포함된 시스템 라이브러리라 FetchContent 대상이 아니다(googletest/nlohmann::json과 다름).
target_link_libraries(graphics_lib PUBLIC d3d9.lib d3d11.lib d3d12.lib dxgi.lib windowscodecs.lib)

# app_lib(projects)만 platform_lib/graphics_lib를 모두 의존해도 된다 — 역방향은 금지.
add_library(app_lib STATIC
    src/projects/config/LaunchConfigParser.cpp
    src/projects/config/LaunchConfigStore.cpp
    src/projects/config/ConfigManager.cpp
    src/projects/persistence/PlayerProgress.cpp
    src/projects/engine/InstanceUpdateWorker.cpp
    src/projects/game/SceneManager.cpp
    src/projects/game/IntroScene.cpp
    src/projects/game/GameManager.cpp
    src/projects/game/SceneSnapshotBuilder.cpp
)
target_include_directories(app_lib PUBLIC src)
target_link_libraries(app_lib PUBLIC platform_lib graphics_lib)

# --- 셰이더 빌드 파이프라인 (오프라인 컴파일, fxc.exe) ---
# 배포 시 D3DCompiler_47.dll 의존성을 없애기 위해 HLSL을 런타임이 아닌 빌드 시점에 fxc.exe로 컴파일한다.
# (docs/strategy/DirectX_Shader_기초연결_20260726_1400.md 체크리스트 1 참고)
# fxc.exe는 Windows SDK에 포함되지만 PATH에 없는 경우가 많아, 설치된 모든 SDK 버전의 x64 디렉터리를
# 검색 경로 후보로 넘겨 find_program이 찾게 한다.
file(GLOB _wot_fxc_candidates "C:/Program Files (x86)/Windows Kits/10/bin/*/x64/fxc.exe")
set(_wot_fxc_search_dirs "")
foreach(_wot_fxc_candidate ${_wot_fxc_candidates})
    get_filename_component(_wot_fxc_dir "${_wot_fxc_candidate}" DIRECTORY)
    list(APPEND _wot_fxc_search_dirs "${_wot_fxc_dir}")
endforeach()

find_program(FXC_EXECUTABLE fxc PATHS ${_wot_fxc_search_dirs})
if(NOT FXC_EXECUTABLE)
    message(FATAL_ERROR "fxc.exe(HLSL 오프라인 셰이더 컴파일러)를 찾을 수 없습니다 - Windows SDK 설치를 확인하세요.")
endif()

# 셰이더 소스는 백엔드별 폴더(src/graphics/renderer/<backend>/shaders/<name>.hlsl)에 두고, <backend>는 그
# 경로에서 자동으로 추론한다 - "각 렌더러는 자기 폴더의 셰이더만 참조한다"는 기존 원칙과 같은 결이다.
# 컴파일된 .cso는 실행 파일과 같은 위치(<config>/shaders/<backend>/)에 출력해, main.exe가 상대
# 경로로 바로 로드할 수 있게 한다. WOT_SHADER_OUTPUTS는 이후 체크리스트(6/8/9)에서 각 셰이더가
# 등록될 때마다 누적되고, 마지막에(체크리스트 11) `shaders` 커스텀 타겟으로 한데 묶인다.
set(WOT_SHADER_OUTPUTS "")

function(wot_add_hlsl_shader OUT_VAR SOURCE ENTRY_POINT PROFILE)
    # 주의: NAME_WE는 "가장 긴 확장자"를 통째로 잘라내므로 "Baseline.vs.hlsl" -> "Baseline"이 되어
    # vs/ps 두 셰이더가 같은 출력 파일명으로 충돌한다(실제로 재현 확인함) - 파일 전체 이름에서
    # 마지막 ".hlsl" 확장자만 정규식으로 제거해 "Baseline.vs"/"Baseline.ps"를 그대로 보존한다.
    get_filename_component(_shader_full_name "${SOURCE}" NAME)
    string(REGEX REPLACE "\\.hlsl$" "" _shader_name "${_shader_full_name}")
    get_filename_component(_shaders_dir "${SOURCE}" DIRECTORY)
    get_filename_component(_backend_dir "${_shaders_dir}" DIRECTORY)
    get_filename_component(_backend "${_backend_dir}" NAME)

    # 주의: $<TARGET_FILE_DIR:main> 같은 "다른 타겟을 참조하는" 제너레이터 표현식은 Visual Studio
    # 멀티 컨피그 생성기에서 add_custom_command의 OUTPUT 경로로 쓰면 "No target main" 생성 오류가
    # 난다(다른 타겟의 속성에 의존하는 OUTPUT 경로는 CMake가 안정적으로 평가하지 못함 - 실제로 재현
    # 확인함). main.exe가 실제로 출력되는 위치(${CMAKE_BINARY_DIR}/$<CONFIG>/, 이 프로젝트는
    # CMAKE_RUNTIME_OUTPUT_DIRECTORY를 별도로 설정하지 않아 이게 VS 생성기의 기본 출력 위치다)를
    # $<CONFIG>만 사용해 직접 구성한다 - $<CONFIG>는 특정 타겟을 참조하지 않는 단순 제너레이터
    # 표현식이라 OUTPUT 경로에 안전하게 쓸 수 있다.
    set(_output_dir "${CMAKE_BINARY_DIR}/$<CONFIG>/shaders/${_backend}")
    set(_output_file "${_output_dir}/${_shader_name}.cso")

    add_custom_command(
        OUTPUT "${_output_file}"
        COMMAND ${CMAKE_COMMAND} -E make_directory "${_output_dir}"
        COMMAND ${FXC_EXECUTABLE} /nologo /T ${PROFILE} /E ${ENTRY_POINT} /Fo "${_output_file}" "${SOURCE}"
        DEPENDS "${SOURCE}"
        COMMENT "HLSL: ${SOURCE} (${PROFILE}, entry=${ENTRY_POINT}) -> ${_output_file}"
        VERBATIM
    )

    set(${OUT_VAR} "${_output_file}" PARENT_SCOPE)
endfunction()

add_executable(main src/projects/main.cpp)
target_link_libraries(main PRIVATE app_lib)

# wot_add_hlsl_shader()가 만드는 add_custom_command는 $<TARGET_FILE_DIR:main>을 참조하므로,
# main 타겟이 add_executable로 이미 선언된 뒤에 호출해야 한다.
wot_add_hlsl_shader(_wot_dx9_baseline_vs "${CMAKE_SOURCE_DIR}/src/graphics/renderer/directx9/shaders/Baseline.vs.hlsl" VSMain vs_3_0)
wot_add_hlsl_shader(_wot_dx9_baseline_ps "${CMAKE_SOURCE_DIR}/src/graphics/renderer/directx9/shaders/Baseline.ps.hlsl" PSMain ps_3_0)
wot_add_hlsl_shader(_wot_dx11_baseline_vs "${CMAKE_SOURCE_DIR}/src/graphics/renderer/directx11/shaders/Baseline.vs.hlsl" VSMain vs_5_0)
wot_add_hlsl_shader(_wot_dx11_baseline_ps "${CMAKE_SOURCE_DIR}/src/graphics/renderer/directx11/shaders/Baseline.ps.hlsl" PSMain ps_5_0)
wot_add_hlsl_shader(_wot_dx12_baseline_vs "${CMAKE_SOURCE_DIR}/src/graphics/renderer/directx12/shaders/Baseline.vs.hlsl" VSMain vs_5_0)
wot_add_hlsl_shader(_wot_dx12_baseline_ps "${CMAKE_SOURCE_DIR}/src/graphics/renderer/directx12/shaders/Baseline.ps.hlsl" PSMain ps_5_0)
wot_add_hlsl_shader(_wot_dx9_instancing_vs "${CMAKE_SOURCE_DIR}/src/graphics/renderer/directx9/shaders/Instancing.vs.hlsl" VSMain vs_3_0)
wot_add_hlsl_shader(_wot_dx11_instancing_vs "${CMAKE_SOURCE_DIR}/src/graphics/renderer/directx11/shaders/Instancing.vs.hlsl" VSMain vs_5_0)
wot_add_hlsl_shader(_wot_dx12_instancing_vs "${CMAKE_SOURCE_DIR}/src/graphics/renderer/directx12/shaders/Instancing.vs.hlsl" VSMain vs_5_0)
wot_add_hlsl_shader(_wot_dx11_transform_cs "${CMAKE_SOURCE_DIR}/src/graphics/renderer/directx11/shaders/Transform.cs.hlsl" CSMain cs_5_0)
wot_add_hlsl_shader(_wot_dx12_transform_cs "${CMAKE_SOURCE_DIR}/src/graphics/renderer/directx12/shaders/Transform.cs.hlsl" CSMain cs_5_0)
list(APPEND WOT_SHADER_OUTPUTS
    ${_wot_dx9_baseline_vs} ${_wot_dx9_baseline_ps}
    ${_wot_dx11_baseline_vs} ${_wot_dx11_baseline_ps}
    ${_wot_dx12_baseline_vs} ${_wot_dx12_baseline_ps}
    ${_wot_dx9_instancing_vs} ${_wot_dx11_instancing_vs} ${_wot_dx12_instancing_vs}
    ${_wot_dx11_transform_cs} ${_wot_dx12_transform_cs}
)

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
    tests/LogFormatterTest.cpp
    tests/ConsoleLogSinkTest.cpp
    tests/FileLogSinkTest.cpp
    tests/LoggerTest.cpp
    tests/LogTest.cpp
    tests/Win32WindowTest.cpp
    tests/DirectX9RendererTest.cpp
    tests/DirectX11RendererTest.cpp
    tests/DirectX12RendererTest.cpp
    tests/ImGuiManagerDX9Test.cpp
    tests/ImGuiManagerDX11Test.cpp
    tests/ImGuiManagerDX12Test.cpp
    tests/Vec3Test.cpp
    tests/QuaternionTest.cpp
    tests/Matrix4x4Test.cpp
    tests/TransformTest.cpp
    tests/AABBTest.cpp
    tests/OBBTest.cpp
    tests/IntersectionsTest.cpp
    tests/GeometryTest.cpp
    tests/GjkEpaTest.cpp
    tests/SweepAndPruneTest.cpp
    tests/BvhTest.cpp
    tests/RTreeTest.cpp
    tests/VoxelGridTest.cpp
    tests/SpatialHashTest.cpp
    tests/OctreeTest.cpp
    tests/KdTreeTest.cpp
    tests/MprTest.cpp
    tests/ConservativeAdvancementTest.cpp
    tests/FclTest.cpp
    tests/EntityTest.cpp
    tests/EntityManagerTest.cpp
    tests/ModelLoaderTest.cpp
    tests/RefCountingCachePolicyTest.cpp
    tests/LruCachePolicyTest.cpp
    tests/Win32FileWatcherTest.cpp
    tests/HotReloadCachePolicyTest.cpp
    tests/NaiveSignatureDeduplicatorTest.cpp
    tests/MeshManagerTest.cpp
    tests/DoubleBufferPublisherTest.cpp
    tests/InstanceUpdateWorkerTest.cpp
    tests/SceneManagerTest.cpp
    tests/IntroSceneTest.cpp
    tests/GameManagerTest.cpp
    tests/SceneSnapshotBuilderTest.cpp
    tests/ShaderBytecodeLoaderTest.cpp
    tests/DirectX11ComputeDemoTest.cpp
    tests/DirectX12ComputeDemoTest.cpp
)
target_link_libraries(tests PRIVATE app_lib graphics_lib platform_lib gtest_main)
target_compile_definitions(tests PRIVATE MODEL_IMPORT_FIXTURES_DIR="${CMAKE_SOURCE_DIR}/tests/fixtures/model_import/")
target_compile_definitions(tests PRIVATE WOT_SHADER_OUTPUT_DIR="${CMAKE_BINARY_DIR}/$<CONFIG>/shaders/")

# WORKING_DIRECTORY 미지정 시 gtest_discover_tests는 (멀티 컨피그 생성기에서 tests.exe가 실제로
# 있는 build/<Config>/가 아니라) 최상위 빌드 디렉터리를 각 테스트의 실행 위치로 쓴다 - 렌더러가
# 상대 경로("shaders/<backend>/...", "assets/models/cube.obj")로 셰이더/에셋을 찾으므로, ctest로
# 실행할 때도 tests.exe와 같은 디렉터리에서 실행되도록 명시해야 한다(실제로 재현 확인함).
include(GoogleTest)
gtest_discover_tests(tests WORKING_DIRECTORY "$<TARGET_FILE_DIR:tests>")

# 하드코딩 삼각형(체크리스트 6)에서 MeshManager로 로드한 실제 큐브 모델(체크리스트 7)로 교체하며,
# 렌더러가 상대 경로 "assets/models/cube.obj"로 읽을 수 있게 실행 파일 옆에 복사해 둔다.
# TARGET POST_BUILD 커스텀 커맨드는(순수 add_custom_command(OUTPUT ...)와 달리) $<TARGET_FILE_DIR:...>
# 제너레이터 표현식을 안전하게 지원한다 - 위 wot_add_hlsl_shader()가 이 방식 대신
# ${CMAKE_BINARY_DIR}/$<CONFIG>를 쓴 이유(OUTPUT 절에서의 제약)와는 다른 경우다.
add_custom_command(TARGET main POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:main>/assets/models"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_SOURCE_DIR}/tests/fixtures/model_import/cube.obj"
        "$<TARGET_FILE_DIR:main>/assets/models/cube.obj"
    COMMENT "Copying cube.obj next to main.exe"
)
add_custom_command(TARGET tests POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:tests>/assets/models"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_SOURCE_DIR}/tests/fixtures/model_import/cube.obj"
        "$<TARGET_FILE_DIR:tests>/assets/models/cube.obj"
    COMMENT "Copying cube.obj next to tests.exe"
)

# IntroScene의 ImageUI가 renderer->LoadTexture("assets/textures/intro_logo.png")로 상대 경로로
# 읽을 수 있게 실행 파일 옆에 복사해 둔다 - cube.obj와 동일한 패턴. intro_logo.png는 임시 placeholder
# (graphics 저장소의 WicImageLoaderTest 픽스처를 그대로 복사한 4x4 테스트용 이미지)이며, 실제 로고
# 에셋이 정해지면 이 파일을 교체하면 된다.
add_custom_command(TARGET main POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:main>/assets/textures"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_SOURCE_DIR}/tests/fixtures/textures/intro_logo.png"
        "$<TARGET_FILE_DIR:main>/assets/textures/intro_logo.png"
    COMMENT "Copying intro_logo.png next to main.exe"
)

# 이 시점까지 등록된 모든 wot_add_hlsl_shader() 호출(체크리스트 6/8/9)의 .cso 출력을 하나의
# 커스텀 타겟으로 묶는다 - main/tests 둘 다 셰이더가 빌드된 뒤에 실행되어야 하므로 의존시킨다.
# (docs/strategy/DirectX_Shader_기초연결_20260726_1400.md 체크리스트 11 참고)
add_custom_target(shaders DEPENDS ${WOT_SHADER_OUTPUTS})
add_dependencies(main shaders)
add_dependencies(tests shaders)
