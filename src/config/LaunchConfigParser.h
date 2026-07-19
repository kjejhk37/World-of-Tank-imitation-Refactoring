#pragma once

#include "config/LaunchConfig.h"

// Author: Claude
// Description: 커맨드라인 인자(argv)를 파싱해 LaunchConfig를 만든다.
// Input: argc, argv (main의 인자를 그대로 전달) / defaultBackend (--renderer 플래그가 없을 때 사용할 기본값)
// Output: 파싱된 LaunchConfig 값
// Notes: --renderer 플래그가 없으면 defaultBackend를 사용한다 (JSON Config에서 읽은 값을 넘겨 파일 -> argv override 순서를 구성할 수 있다).
//        --renderer=<알 수 없는 값>이 들어오면 std::invalid_argument를 던진다 — 호출부(main)에서 반드시 catch해야 한다.
// Date: 2026-07-19
LaunchConfig ParseLaunchConfig(int argc, char** argv, RendererBackend defaultBackend = RendererBackend::DirectX11);
