#pragma once

#include <cstdlib>
#include <memory>

struct free_deleter {
    template <typename T>
    void operator()(T* p) const
    {
        std::free(const_cast<std::remove_const_t<T>*>(p));
    }
};
template <typename T>
using unique_cptr = std::unique_ptr<T, free_deleter>;
