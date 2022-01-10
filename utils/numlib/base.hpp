#pragma once

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifndef NUMLIB_ALLOC
#define NUMLIB_ALLOC(n) ::malloc(n)
#endif

#ifndef NUMLIB_FREE
#define NUMLIB_FREE(p) ::free(p)
#endif

#ifndef NUMLIB_REALLOC
#define NUMLIB_REALLOC(p, n) ::realloc(p, n)
#endif

#include <vector>
#include <array>

#include "array.hpp"

namespace numlib {

template <typename T>
void _ignore_unused(const T&) {}

template <typename T>
size_t Max(const T& a, const T& b) {
  return a > b ? a : b;
}

template <typename T>
size_t Min(const T& a, const T& b) {
    return a < b ? a : b;
}

template<typename T>
size_t Size(const T &v) { return v.size(); }

template<typename T1, typename T2>
size_t Size(const std::pair<T1, T2> &v) { _ignore_unused(v); return 2u; }

template<typename... Types>
size_t Size(const std::tuple<Types...> &v) { _ignore_unused(v); return std::tuple_size<std::tuple<Types...>>{}; }

template<typename T>
void Resize(T &v, size_t n) { v.resize(n); }

template<typename T>
T &Vector(T &v) { return v; }

template<typename T>
const T &Vector(const T &v) { return v; }

template<typename T>
std::vector<T> Vector(std::initializer_list<T> v) { return std::vector<T>(v); }

template<typename T>
typename T::value_type Mean(T input) {
    size_t n = Size(input);
    typename T::value_type sum = 0;
    for (size_t i = 0; i < n; ++i) {
        sum += input[i];
    }
    return sum / n;
}

} // namespace numlib
