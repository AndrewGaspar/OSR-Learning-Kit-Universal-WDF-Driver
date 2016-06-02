#pragma once

#include "type_traits.h"

namespace ktl
{
    template<typename T>
    constexpr __declspec(code_seg(".text")) remove_reference_t<T>&& move(T&& value)
    {
        return static_cast<remove_reference_t<T>&&>(value);
    }
}