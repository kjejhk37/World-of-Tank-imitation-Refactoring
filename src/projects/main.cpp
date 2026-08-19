#include <memory>
#include <stdexcept>
#include <system_error>

#include <string>

#include "projects/config/ConfigManager.h"
#include "projects/config/LaunchConfigParser.h"
#include "projects/config/LaunchConfigStore.h"
#include "projects/engine/InstanceUpdateWorker.h"
#include "projects/ErrorCode.h"
#include "platform/logging/Log.h"
#include "platform/Win32Window.h"
#include "graphics/renderer/RendererFactory.h"
#include "graphics/ui/widgets/ButtonUI.h"
#include "graphics/ui/widgets/ImageUI.h"
#include "graphics/ui/widgets/LabelUI.h"
#include "graphics/ui/widgets/ProgressUI.h"
#include "graphics/ui/widgets/StringFieldUI.h"
#include "graphics/ui/widgets/UiElementRegistry.h"
#include "platform/serialization/JsonDataStore.h"

// 로컬 상수 분리 기준: 이 파일이 500줄을 넘거나 아래 상수가 5개를 넘으면
// 별도 헤더로 분리한다.
namespace
{
    constexpr const char* kDefaultConfigFilePath = "config.json";

    // Author: Claude
    // Description: Intro 화면에 표시할 2D UI 위젯 5종 + UiElementRegistry를 한데 묶은 구성 단위 -
    //              "Intro 화면에 뭐가 있는가"라는 책임을 main()의 부트스트랩 오케스트레이션에서
    //              분리하기 위함(SRP).
    // Input: (해당 없음 - BuildIntroUi()의 Input 참고)
    // Output: (해당 없음)
    // Notes: 위젯/registry는 non-owning 계약이라(IUiElementRegistry.h 참고) 서로의 주소를 참조한다 -
    //        그래서 이 구조체는 항상 std::unique_ptr<IntroUi>로만 다룬다(BuildIntroUi() 참고). 값으로
    //        복사/이동하면 멤버들의 실제 주소가 바뀌어 registry가 옛 주소를 가리키는 댕글링 포인터가
    //        된다 - 힙에 한 번 배치한 뒤 포인터만 옮기면 이 문제가 없다.
    // Date: 2026-08-19
    struct IntroUi
    {
        UiElementRegistry registry;
        LabelUI titleLabel{"World of Tank Imitation - Intro"};
        float loadingProgress = 0.5f;
        ProgressUI loadingProgressBar{&loadingProgress, "Loading"};
        std::string playerName;
        StringFieldUI playerNameField{"Player Name", &playerName};
        ButtonUI startButton{"Start", []() { Log::Info("Intro: Start button clicked"); }};
        void* logoTextureHandle = nullptr;
        std::unique_ptr<ImageUI> logoImage;
    };

    // Author: Claude
    // Description: IntroUi를 힙에 생성하고, 로드 가능한 위젯을 전부 registry에 등록해 돌려준다.
    // Input: renderer - 로고 텍스처 로딩(LoadTexture)에 쓸 IRenderer
    // Output: 구성 완료된 IntroUi(registry에 위젯이 전부 등록된 상태)에 대한 소유권
    // Notes: 로고 텍스처 로딩이 실패하면(LoadTexture가 nullptr 반환) IntroLogoImage는 만들지 않고
    //        경고만 남긴다 - ImageUI에 무효한 핸들을 넘기지 않기 위함. intro_logo.png는 임시
    //        placeholder 에셋이다(CMakeLists.txt 주석 참고).
    // Date: 2026-08-19
    std::unique_ptr<IntroUi> BuildIntroUi(IRenderer& renderer)
    {
        auto ui = std::make_unique<IntroUi>();

        ui->logoTextureHandle = renderer.LoadTexture("assets/textures/intro_logo.png");
        if (ui->logoTextureHandle)
        {
            ui->logoImage = std::make_unique<ImageUI>(ui->logoTextureHandle, 64.0f, 64.0f);
        }
        else
        {
            Log::Warning("Intro: failed to load assets/textures/intro_logo.png - IntroLogoImage skipped");
        }

        ui->registry.Add(&ui->titleLabel);
        ui->registry.Add(&ui->loadingProgressBar);
        ui->registry.Add(&ui->playerNameField);
        ui->registry.Add(&ui->startButton);
        if (ui->logoImage)
        {
            ui->registry.Add(ui->logoImage.get());
        }

        return ui;
    }
}

// Author: Claude
// Description: 진입점. JSON Config 파일로 AppConfig를 읽고, argv로 이를 override해 LaunchConfigStore를 초기화한 뒤,
//              Win32Window와 RendererFactory로 선택된 렌더러를 초기화하고, Intro 화면용 2D UI 위젯을 등록한 채
//              메시지 루프를 돈다.
// Input: argc, argv (OS가 전달하는 실행 인자)
// Output: 종료 코드 (0: 정상, 1: 인자 파싱/창 생성 실패 또는 렌더러 생성/초기화 실패)
// Notes: 설정 적용 순서는 하드코딩 기본값(AppConfig) -> config.json -> argv 순이다.
//        introUi(BuildIntroUi() 참고)는 renderer->RenderFrame()이 도는 이 스레드(메인/렌더 스레드)에
//        산다 - GameManager/IntroScene(engine 스레드)에 두지 않는 이유는, SetUiElementRegistry로
//        등록된 registry가 렌더 스레드에서 RenderAll()로 호출되기 때문이다(ImGui가 스레드 세이프하지
//        않고, 3D InstanceSnapshot과 달리 UI 위젯엔 DoubleBufferPublisher 같은 안전한 스레드 간 전달
//        장치가 없다 - docs/strategy/게임_기본기능_채우기_20260819_1928.md 참고, IntroScene.h Notes도
//        참고).
//        LaunchConfigStore::Init은 반드시 이 함수 안에서, 다른 스레드가 뜨기 전에 1회만 호출되어야 한다.
//        Log::Init은 다른 어떤 try/catch보다도 먼저 호출한다 — 이후의 모든 에러가 Log::Error(ErrorCode, ...)로 기록되게 하기 위함.
//        창 리사이즈는 Win32Window의 콜백을 통해 renderer->OnResize로 연결한다 — Win32Window는 IRenderer를 알지 못한다(SRP).
//        같은 이유로 Win32 메시지는 SetMessageHook을 통해 renderer->HandleUiMessage로 연결한다 —
//        각 렌더러가 소유한 ImGui 프레임워크가 이 메시지를 소비할 수 있게 하되, Win32Window는 ImGui의 존재를 모른다.
//        InstanceUpdateWorker(Engine 스레드)는 renderer->Initialize() 성공 이후, 메시지 루프 진입
//        전에 Start()한다 - main/렌더러는 IFrameDataPublisher의 동기화 정책을 몰라도 되고, 매 프레임
//        worker.GetPublisher().AcquireReadSnapshot()으로 얻은 순수 데이터만 renderer->RenderFrame에
//        넘긴다(SRP). worker.Stop()은 renderer->Shutdown() 이전에 정확히 1회 호출한다 - Stop()은
//        Start()가 호출된 적 없어도 안전(no-op)하므로, 창 생성/렌더러 초기화 실패로 인한 조기
//        반환 경로 뒤에 worker가 아직 없더라도(scope 밖) 문제 없다. std::thread 생성이 OS 자원
//        고갈 등으로 실패하면 std::system_error를 던지는데, 이 스레드는 인스턴스 스냅샷을 만드는
//        보조 워커일 뿐이라(렌더러는 빈 스냅샷일 때 이미 Baseline 단일 드로우로 폴백한다) 실패해도
//        앱 전체를 종료시키지 않고 로그만 남긴 뒤 계속 진행한다.
// Date: 2026-07-19
int main(int argc, char** argv)
{
    JsonDataStore configStore;
    const ConfigManager configManager(configStore);
    const AppConfig appConfig = configManager.LoadOrDefault(kDefaultConfigFilePath);

    Log::Init(appConfig.logFilePath);

    try
    {
        LaunchConfigStore::Init(ParseLaunchConfig(argc, argv, appConfig.renderer));
    }
    catch (const std::invalid_argument& e)
    {
        Log::Error(static_cast<int>(ErrorCode::LaunchConfigParseFailed), e.what());
        return 1;
    }

    std::unique_ptr<Win32Window> window;
    try
    {
        window = std::make_unique<Win32Window>(appConfig.width, appConfig.height, appConfig.windowTitle,
                                                 appConfig.fullscreen);
    }
    catch (const std::runtime_error& e)
    {
        Log::Error(static_cast<int>(ErrorCode::WindowCreationFailed), e.what());
        return 1;
    }

    auto renderer = RendererFactory::Create(LaunchConfigStore::Get().backend);
    if (!renderer)
    {
        Log::Error(static_cast<int>(ErrorCode::UnknownRendererBackend), "Unknown renderer backend");
        return 1;
    }

    if (!renderer->Initialize(window->Handle(), window->ClientWidth(), window->ClientHeight()))
    {
        Log::Error(static_cast<int>(ErrorCode::RendererInitializationFailed), "Failed to initialize renderer");
        return 1;
    }

    window->SetResizeCallback([&renderer](int width, int height) { renderer->OnResize(width, height); });
    window->SetMessageHook([&renderer](HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
        return renderer->HandleUiMessage(hwnd, message, wParam, lParam);
    });

    std::unique_ptr<IntroUi> introUi = BuildIntroUi(*renderer);
    renderer->SetUiElementRegistry(introUi->registry);

    InstanceUpdateWorker worker;
    try
    {
        worker.Start();
    }
    catch (const std::system_error& e)
    {
        Log::Error(static_cast<int>(ErrorCode::EngineWorkerStartFailed), e.what());
    }

    while (window->PumpMessages())
    {
        renderer->RenderFrame(worker.GetPublisher().AcquireReadSnapshot());
    }

    worker.Stop();
    if (introUi->logoTextureHandle)
    {
        renderer->UnloadTexture(introUi->logoTextureHandle);
    }
    renderer->Shutdown();

    return 0;
}
