// Copyright (c) 2017 Advanced Micro Devices, Inc. All rights reserved.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

#ifndef HIPCUB_ROCPRIM_BLOCK_BLOCK_STORE_HPP_
#define HIPCUB_ROCPRIM_BLOCK_BLOCK_STORE_HPP_

#include <type_traits>

#include "../../config.hpp"

#include "block_store_func.hpp"

BEGIN_HIPCUB_NAMESPACE

namespace detail
{
    inline constexpr
    typename std::underlying_type<::rocprim::block_store_method>::type
    to_BlockStoreAlgorithm_enum(::rocprim::block_store_method v)
    {
        using utype = std::underlying_type<::rocprim::block_store_method>::type;
        return static_cast<utype>(v);
    }
}

enum BlockStoreAlgorithm
{
    BLOCK_STORE_DIRECT
        = detail::to_BlockStoreAlgorithm_enum(::rocprim::block_store_method::block_store_direct),
    BLOCK_STORE_VECTORIZE
        = detail::to_BlockStoreAlgorithm_enum(::rocprim::block_store_method::block_store_vectorize),
    BLOCK_STORE_TRANSPOSE
        = detail::to_BlockStoreAlgorithm_enum(::rocprim::block_store_method::block_store_transpose),
    BLOCK_STORE_WARP_TRANSPOSE
        = detail::to_BlockStoreAlgorithm_enum(::rocprim::block_store_method::block_store_warp_transpose),
    BLOCK_STORE_WARP_TRANSPOSE_TIMESLICED
        = detail::to_BlockStoreAlgorithm_enum(::rocprim::block_store_method::block_store_warp_transpose)
};

template<
    typename T,
    int BLOCK_DIM_X,
    int ITEMS_PER_THREAD,
    BlockStoreAlgorithm ALGORITHM = BLOCK_STORE_DIRECT,
    int BLOCK_DIM_Y = 1,
    int BLOCK_DIM_Z = 1,
    int ARCH = HIPCUB_ARCH /* ignored */
>
class BlockStore
    : private ::rocprim::block_store<
        T,
        BLOCK_DIM_X * BLOCK_DIM_Y * BLOCK_DIM_Z,
        ITEMS_PER_THREAD,
        static_cast<::rocprim::block_store_method>(ALGORITHM)
      >
{
    static_assert(
        BLOCK_DIM_X * BLOCK_DIM_Y * BLOCK_DIM_Z > 0,
        "BLOCK_DIM_X * BLOCK_DIM_Y * BLOCK_DIM_Z must be greater than 0"
    );

    using base_type =
        typename ::rocprim::block_store<
            T,
            BLOCK_DIM_X * BLOCK_DIM_Y * BLOCK_DIM_Z,
            ITEMS_PER_THREAD,
            static_cast<::rocprim::block_store_method>(ALGORITHM)
        >;

    // Reference to temporary storage (usually shared memory)
    typename base_type::storage_type& temp_storage_;

public:
    using TempStorage = typename base_type::storage_type;

    HIPCUB_DEVICE inline
    BlockStore() : temp_storage_(private_storage())
    {
    }

    HIPCUB_DEVICE inline
    BlockStore(TempStorage& temp_storage) : temp_storage_(temp_storage)
    {
    }

    template<class OutputIteratorT>
    HIPCUB_DEVICE inline
    void Store(OutputIteratorT block_iter,
               T (&items)[ITEMS_PER_THREAD])
    {
        base_type::store(block_iter, items, temp_storage_);
    }

    template<class OutputIteratorT>
    HIPCUB_DEVICE inline
    void Store(OutputIteratorT block_iter,
               T (&items)[ITEMS_PER_THREAD],
               int valid_items)
    {
        base_type::store(block_iter, items, valid_items, temp_storage_);
    }

private:
    HIPCUB_DEVICE inline
    TempStorage& private_storage()
    {
        HIPCUB_SHARED_MEMORY TempStorage private_storage;
        return private_storage;
    }
};

END_HIPCUB_NAMESPACE

#endif // HIPCUB_ROCPRIM_BLOCK_BLOCK_STORE_HPP_
