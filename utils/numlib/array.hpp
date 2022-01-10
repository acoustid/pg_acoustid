#pragma once

#include "base.hpp"

#include <iostream>
#include <algorithm>

namespace numlib {

template<typename T>
class Allocator {
 public:
    typedef T value_type;
    typedef T* pointer;
    typedef const T* const_pointer;
    typedef T& reference;
    typedef const T& const_reference;

    Allocator() = default;
    Allocator(const Allocator &other) = default;

    template<typename U>
    Allocator(const Allocator<U> &other) { _ignore_unused(other); }

    pointer alloc(size_t n) {
        return static_cast<pointer>(NUMLIB_ALLOC(n * sizeof(T)));
    }

    pointer realloc(pointer p, size_t n) {
        return static_cast<pointer>(NUMLIB_REALLOC(p, n * sizeof(T)));
    }

    void free(pointer p) {
        NUMLIB_FREE(p);
    }

    template<typename U>
    struct rebind {
        typedef Allocator<U> other;
    };
};

template <typename T, typename Alloc = Allocator<T>>
class Array {
 public:
    typedef T value_type;
    typedef T* iterator;
    typedef const T* const_iterator;
    typedef T* pointer;
    typedef const T* const_pointer;
    typedef T& reference;
    typedef const T& const_reference;
    typedef Alloc allocator_type;

 public:

    Array(const Alloc &alloc = Allocator<T>()) : alloc_(alloc) {}
    Array(size_t n, const Alloc &alloc = Allocator<T>()) : alloc_(alloc) {
        resize(n);
    }
    Array(size_t n, T value, const Alloc &alloc = Allocator<T>()) : alloc_(alloc) {
        resize(n);
        std::fill(begin(), end(), value);
    }
    Array(std::initializer_list<T> init, const Alloc &alloc = Allocator<T>()) : alloc_(alloc) {
        resize(init.size());
        std::copy(init.begin(), init.end(), data_);
    }
    
    ~Array() {
        if (data_) {
            alloc_.free(data_);
        }
    }
    
    Array(const Array& other) : alloc_(other.alloc_), size_(other.size_), capacity_(other.capacity_) {
        data_ = alloc_.alloc(other.size_);
        std::copy(other.data_, other.data_ + other.size_, data_);
    }

    Array& operator=(const Array& other) = delete;
    
    Array(Array&& other) : alloc_(other.alloc_), data_(other.data_), size_(other.size_), capacity_(other.capacity_) {
        other.data_ = nullptr;
        other.size_ = 0;
        other.capacity_ = 0;
    }

    Array& operator=(Array&& other) {
        data_ = other.data_;
        size_ = other.size_;
        capacity_ = other.capacity_;
        other.data_ = nullptr;
        other.size_ = 0;
        other.capacity_ = 0;
        return *this;
    }

    Alloc allocator() const {
        return alloc_;
    }

    reference operator[](size_t i) {
        return data_[i];
    }

    const_reference operator[](size_t i) const {
        return data_[i];
    }

    pointer begin() {
        return data_;
    }

    pointer end() {
        return data_ + size_;
    }

    const_pointer begin() const {
        return data_;
    }

    const_pointer end() const {
        return data_ + size_;
    }

    size_t size() const {
        return size_;
    }

    void resize(size_t n) {
        if (n > capacity_) {
            reserve(n);
        }
        size_ = n;
    }
    
    void reserve(size_t n) {
        if (n <= capacity_) {
            return;
        }
        if (capacity_ == 0) {
            data_ = alloc_.alloc(n);
        } else {
            data_ = alloc_.realloc(data_, n);
        }
        capacity_ = n;
    }

    void push_back(const T& value) {
        Append(value);
    }

    void Append(const T& value) {
        if (size_ == capacity_) {
            auto new_capacity = capacity_ == 0 ? 2 : capacity_ * 2;
            reserve(new_capacity);
        }
        data_[size_++] = value;
    }

 private:
  allocator_type alloc_;
  pointer data_{nullptr};
  size_t size_{0};
  size_t capacity_{0};
};

template <typename T>
class ArrayView {
 public:
    typedef T value_type;
    typedef T* iterator;
    typedef const T* const_iterator;
    typedef T* pointer;
    typedef const T* const_pointer;
    typedef T& reference;
    typedef const T& const_reference;

  ArrayView(T* ptr, size_t size) : ptr_(ptr), size_(size) {}

  T &operator[](size_t i) { return ptr_[i]; }
  const T &operator[](size_t i) const { return ptr_[i]; }

    T* data() { return ptr_; }
    const T* data() const { return ptr_; }

    size_t size() const { return size_; }

    pointer begin() {
        return ptr_;
    }

    pointer end() {
        return ptr_ + size_;
    }

    const_pointer begin() const {
        return ptr_;
    }

    const_pointer end() const {
        return ptr_ + size_;
    }

 private:
    T* ptr_;
    size_t size_;
};

template <typename T>
class ConstArrayView {
 public:
    typedef T value_type;
    typedef T* iterator;
    typedef const T* const_iterator;
    typedef T* pointer;
    typedef const T* const_pointer;
    typedef T& reference;
    typedef const T& const_reference;

  ConstArrayView(const T* ptr, size_t size) : ptr_(ptr), size_(size) {}

  const T &operator[](size_t i) const { return ptr_[i]; }

    const T* data() const { return ptr_; }

    size_t size() const { return size_; }

    const_pointer begin() const {
        return ptr_;
    }

    const_pointer end() const {
        return ptr_ + size_;
    }

 private:
    const T* ptr_;
    size_t size_;
};

template <typename T, typename Deletor>
class OwnedArrayView {
 public:
    OwnedArrayView(T* ptr, size_t size, Deletor d) : ptr_(ptr), size_(size), d_(d) {}

    OwnedArrayView(const OwnedArrayView& o) = delete;
    OwnedArrayView& operator=(const OwnedArrayView& o) = delete;
    void operator=(OwnedArrayView&& o) = delete;
    
    ~OwnedArrayView() {
        d_(ptr_);
    }

    T &operator[](size_t i) { return ptr_[i]; }
    const T &operator[](size_t i) const { return ptr_[i]; }
    
        T* data() { return ptr_; }
        const T* data() const { return ptr_; }
    
        size_t size() const { return size_; }
    
     private:
        T* ptr_;
        size_t size_;
        Deletor d_;
    };

template<typename T>
ArrayView<T> MakeArray(T* ptr, size_t size) {
    return ArrayView<T>(ptr, size);
}

template<typename T>
ConstArrayView<T> MakeArray(const T* ptr, size_t size) {
    return ConstArrayView<T>(ptr, size);
}

template<typename T, typename Deletor>
OwnedArrayView<T, Deletor> MakeArray(T* ptr, size_t size, Deletor d) {
    return OwnedArrayView<T, Deletor>(ptr, size, d);
}

template<typename T, typename Allocator = Allocator<T>>
Array<T> Zeros(size_t n, Allocator allocator = Allocator()) {
    return Array<T>(n, 0, allocator);
}

template<typename T, typename Allocator = Allocator<T>>
Array<T> Ones(size_t n, Allocator allocator = Allocator()) {
    return Array<T>(n, 0, allocator);
}

template<typename T, typename Allocator = Allocator<T>>
Array<T> Range(size_t n, Allocator allocator = Allocator()) {
    auto arr = Array<T>(n, allocator);
    for (size_t i = 0; i < n; ++i) {
        arr[i] = i;
    }
    return arr;
}

template<typename T>
bool operator==(const Array<T>& a, const Array<T>& b) {
    if (a.size() != b.size()) {
        return false;
    }
    for (size_t i = 0; i < a.size(); ++i) {
        if (a[i] != b[i]) {
            return false;
        }
    }
    return true;
}

template<typename T>
std::ostream& operator<<(std::ostream& stream, const Array<T> &a) {
    stream << "{";
    for (size_t i = 0; i < a.size(); ++i) {
        stream << a[i];
        if (i != a.size() - 1) {
            stream << ", ";
        }
    }
    stream << "}";
    return stream;
}

template<typename T>
std::ostream& operator<<(std::ostream& stream, const ArrayView<T> &a) {
    stream << "{";
    for (size_t i = 0; i < a.size(); ++i) {
        stream << a[i];
        if (i != a.size() - 1) {
            stream << ", ";
        }
    }
    stream << "}";
    return stream;
}

} // namespace numlib
