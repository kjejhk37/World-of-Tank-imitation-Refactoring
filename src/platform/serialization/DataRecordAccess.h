#pragma once

#include <string>

#include "platform/serialization/DataRecord.h"

// Author: Claude
// Description: DataRecord에서 필드를 안전하게 꺼내는 공용 유틸리티. ConfigManager/PlayerProgress 등 DataRecord를 다루는 여러 클래스가 공유한다.
// Input: 조회할 DataRecord, 필드 이름, 필드가 없거나 파싱에 실패했을 때 쓸 기본값
// Output: 파싱된 값, 실패 시 전달받은 기본값
// Notes: 예외를 던지지 않는다 (std::from_chars 기반) — 손상되거나 형식이 다른 저장 데이터에도 안전하게 기본값으로 폴백한다.
//        도메인 타입(RendererBackend 등)으로의 변환은 이 유틸리티의 책임이 아니다 — 호출부에서 처리한다.
// Date: 2026-07-19
namespace DataRecordAccess
{
    int GetIntOr(const DataRecord& record, const std::string& field, int fallback);
    bool GetBoolOr(const DataRecord& record, const std::string& field, bool fallback);
    float GetFloatOr(const DataRecord& record, const std::string& field, float fallback);
    std::string GetStringOr(const DataRecord& record, const std::string& field, const std::string& fallback);
}
