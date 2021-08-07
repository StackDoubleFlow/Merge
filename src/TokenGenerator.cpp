#include "TokenGenerator.h"

TokenGenerator::TokenGenerator() {
    // Mono.Cecil.TokenType
    typeDefToken = 0x02000000;
    fieldToken = 0x04000000;
    methodToken = 0x06000000;
    paramToken = 0x08000000;
    propToken = 0x17000000;
}


uint32_t TokenGenerator::GetNextTypeDefToken() {
    return typeDefToken++;
}

uint32_t TokenGenerator::GetNextParamToken() {
    return paramToken++;
}

uint32_t TokenGenerator::GetNextFieldToken() {
    return fieldToken++;
}

uint32_t TokenGenerator::GetNextPropToken() {
    return propToken++;
}

uint32_t TokenGenerator::GetNextMethodToken() {
    return methodToken++;
}
