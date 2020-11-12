// RUN: kernel-gen-opt %s --embed-memref-prints | FileCheck %s

func @print_memrefs(
    %ctx: !tf_framework.op_kernel_context, %input: memref<*xf16>)
    -> memref<*xf16> attributes {tf_entry} {
  %c0 = constant 0 : index
  %c1 = constant 1 : index
  %rank = rank %input : memref<*xf16>
  %shape = alloca(%rank) : memref<?xindex>
  scf.for %i = %c0 to %rank step %c1 {
    %dim = dim %input, %i : memref<*xf16>
    store %dim, %shape[%i] : memref<?xindex>
  }

  %c9000 = constant 9000 : index
  %num_elem = alloca() : memref<1xindex>
  store %c9000, %num_elem[%c0] : memref<1xindex>
  %flat_input = memref_reshape %input(%num_elem)
    : (memref<*xf16>, memref<1xindex>) -> memref<?xf16>

  %flat_output = tf_framework.alloc(%ctx, %c9000) : memref<?xf16>
  %output = memref_reshape %flat_output(%shape)
    : (memref<?xf16>, memref<?xindex>) -> memref<*xf16>
  return %output : memref<*xf16>
}

// CHECK:   func private @print_memref_index(memref<*xindex>)

// CHECK-LABEL: func @print_memrefs

// CHECK: [[SHAPE:%.*]] = alloca({{%.*}}) : memref<?xindex>
// CHECK: scf.for
// CHECK: [[NUM_ELEM:%.*]] = alloca() : memref<1xindex>
// CHECK: store {{%.*}}, [[NUM_ELEM]]

// CHECK: [[UNRANKED_NUM_ELEM:%.*]] = memref_cast [[NUM_ELEM]]
// CHECK-NEXT: call @print_memref_index([[UNRANKED_NUM_ELEM]])

// CHECK: memref_reshape
// CHECK: tf_framework.alloc

// CHECK: [[UNRANKED_SHAPE:%.*]] = memref_cast [[SHAPE]]
// CHECK-NEXT: call @print_memref_index([[UNRANKED_SHAPE]])
// CHECK: memref_reshape
