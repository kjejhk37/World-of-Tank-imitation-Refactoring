#pragma once

#include <string>
#include <unordered_map>

// Author: Claude
// Description: IDataStore가 다루는 저장 단위. 필드 이름 -> 문자열로 직렬화된 값의 평면 맵.
// Input: (해당 없음 - 타입 별칭)
// Output: (해당 없음 - 타입 별칭)
// Notes: 값은 항상 문자열로 저장/조회하며, 실제 타입(int/bool/float 등) 변환은 호출부(ConfigManager 등)의 책임이다.
//        중첩/목록 구조가 필요한 값은 호출부에서 구분자로 인코딩해 하나의 문자열 필드로 저장한다.
// Date: 2026-07-19
using DataRecord = std::unordered_map<std::string, std::string>;
