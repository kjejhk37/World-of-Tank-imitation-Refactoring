#include "renderer/RendererBackendParser.h"

// 로컬 상수 분리 기준: 이 파일이 500줄을 넘거나 아래 상수가 5개를 넘으면
// RendererBackendNames.h 같은 별도 헤더로 분리한다.
namespace
{
    constexpr std::string_view kDirectXName = "directx";
    constexpr std::string_view kOpenGLName = "opengl";
}

namespace RendererBackendParser
{
    std::optional<RendererBackend> TryParse(std::string_view value)
    {
        if (value == kDirectXName)
        {
            return RendererBackend::DirectX;
        }
        if (value == kOpenGLName)
        {
            return RendererBackend::OpenGL;
        }
        return std::nullopt;
    }
}
