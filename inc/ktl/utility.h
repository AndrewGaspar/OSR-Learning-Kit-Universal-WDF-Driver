#pragma once

namespace ktl
{
    template<typename T>
    auto move(T & value) -> T&&
    {
        return static_cast<T&&>(value);
    }

    template<typename T>
    auto move(T&& value) -> T&&
    {
        return value;
    }
}