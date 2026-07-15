#pragma once

// 지원하는 렌더링 백엔드 종류.
// 이번 사이클에서는 선택 구조만 확보하며, DirectX/OpenGL 둘 다 실제 초기화 로직은 스텁 상태.
enum class RendererBackend
{
    DirectX,
    OpenGL
};
