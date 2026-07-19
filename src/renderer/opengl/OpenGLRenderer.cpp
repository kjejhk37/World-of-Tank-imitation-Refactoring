#include "renderer/opengl/OpenGLRenderer.h"

bool OpenGLRenderer::Initialize(HWND /*windowHandle*/, int /*width*/, int /*height*/)
{
    return true;
}

void OpenGLRenderer::RenderFrame()
{
}

void OpenGLRenderer::OnResize(int /*width*/, int /*height*/)
{
}

void OpenGLRenderer::Shutdown()
{
}

bool OpenGLRenderer::HandleUiMessage(HWND /*windowHandle*/, UINT /*message*/, WPARAM /*wParam*/, LPARAM /*lParam*/)
{
    return false;
}
