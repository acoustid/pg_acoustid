#pragma once

#include <cmath>

#include "array.hpp"

namespace numlib {

template<typename T>
void GaussianWindow(T& window, double sigma, bool symetric = true)
{
    auto size = Size(window);
    typename T::value_type center = symetric ? (size - 1) / 2.0 : size / 2.0;
    for (size_t i = 0; i < size; i++)
    {
        window[i] = exp(-0.5 * pow((i - center) / sigma, 2));
    }
}

template<typename T = double>
Array<T> GaussianWindow(size_t size, double sigma, bool symetric = true)
{
    Array<T> window(size);
    GaussianWindow(window, sigma, symetric);
    return window;
}

}
