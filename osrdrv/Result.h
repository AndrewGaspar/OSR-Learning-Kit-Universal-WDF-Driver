#pragma once

#include <ntdef.h>

#define REQUIRE_SEMICOLON (true)

#define RETURN_IF_NT_FAILED(_StatusExpression) { \
    auto _status = (_StatusExpression); \
    if(!NT_SUCCESS(_status)) \
    { \
        return _status; \
    } \
} REQUIRE_SEMICOLON