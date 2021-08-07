#pragma once
#include "stdint.h"

class TokenGenerator {
public:
    TokenGenerator();
    uint32_t GetNextTypeDefToken();
    uint32_t GetNextParamToken();
    uint32_t GetNextFieldToken();
    uint32_t GetNextPropToken();
    uint32_t GetNextMethodToken();
private:
    uint32_t typeDefToken;
    uint32_t paramToken;
    uint32_t fieldToken;
    uint32_t propToken;
    uint32_t methodToken;
};