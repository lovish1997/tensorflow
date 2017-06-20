/* Copyright 2015 The TensorFlow Authors. All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
==============================================================================*/

#ifndef TENSORFLOW_KERNELS_TILE_FUNCTOR_H_
#define TENSORFLOW_KERNELS_TILE_FUNCTOR_H_

#include "third_party/eigen3/unsupported/Eigen/CXX11/Tensor"
#include "tensorflow/core/framework/tensor.h"
#include "tensorflow/core/framework/tensor_types.h"
#include "tensorflow/core/platform/types.h"

namespace tensorflow {

namespace internal {

// Helper to compute 'strides' given a tensor 'shape'. I.e.,
// strides[i] = prod(shape.dim_size[(i+1):])
template <typename Index>
gtl::InlinedVector<Index, 8> ComputeStride(const TensorShape& shape) {
  const int ndims = shape.dims();
  gtl::InlinedVector<Index, 8> strides(ndims);
  Index stride = 1;
  for (int i = ndims - 1; i >= 0; --i) {
    strides[i] = stride;
    stride *= static_cast<Index>(shape.dim_size(i));
  }
  return strides;
}


// Device-specific naive implementation for tile.
template <typename Device, typename T>
void TileSimple(const Device& d, Tensor* out, const Tensor& in);

template <typename Device, typename T, int NDIM>
void TileUsingEigen(const Device& d, Tensor* out, const Tensor& in,
                    const gtl::ArraySlice<int32>& broadcast_array) {
  typename TTypes<T, NDIM>::ConstTensor x = in.tensor<T, NDIM>();
  typename TTypes<T, NDIM>::Tensor y = out->tensor<T, NDIM>();

  Eigen::array<int32, NDIM> b;
  for (int i = 0; i < NDIM; ++i) b[i] = broadcast_array[i];
  if (Eigen::internal::is_same<Device, Eigen::GpuDevice>::value) {
    // Use 32bit indexing to speed up the computations
    To32Bit(y).device(d) = To32Bit(x).broadcast(b);
  } else {
    y.device(d) = x.broadcast(b);
  }
}

template <typename Device, typename T>
void TileUsingEigen(const Device& d, Tensor* out, const Tensor& in,
                    const gtl::ArraySlice<int32>&) {
  typename TTypes<T, 0>::ConstTensor x = in.tensor<T, 0>();
  typename TTypes<T, 0>::Tensor y = out->tensor<T, 0>();
  // In the scalar case we simply copy the input.
  y.device(d) = x;
}

}  // end namespace internal

namespace functor {

template <typename Device, typename T>
struct Tile {
  void operator()(const Device& d, Tensor* out, const Tensor& in,
                  const gtl::ArraySlice<int32> broadcast_array) const {
    switch (in.dims()) {
      case 0:
        internal::TileUsingEigen<Device, T>(d, out, in, broadcast_array);
        break;
      case 1:
        internal::TileUsingEigen<Device, T, 1>(d, out, in, broadcast_array);
        break;
      case 2:
        internal::TileUsingEigen<Device, T, 2>(d, out, in, broadcast_array);
        break;
      case 3:
        internal::TileUsingEigen<Device, T, 3>(d, out, in, broadcast_array);
        break;
      case 4:
        internal::TileUsingEigen<Device, T, 4>(d, out, in, broadcast_array);
        break;
      case 5:
        internal::TileUsingEigen<Device, T, 5>(d, out, in, broadcast_array);
        break;
      case 6:
        internal::TileUsingEigen<Device, T, 6>(d, out, in, broadcast_array);
        break;
      case 7:
        internal::TileUsingEigen<Device, T, 7>(d, out, in, broadcast_array);
        break;
      default:
        internal::TileSimple<Device, T>(d, out, in);
        break;
    }
  }
};

}  // end namespace functor
}  // end namespace tensorflow

#endif  // TENSORFLOW_KERNELS_TILE_FUNCTOR_H_
