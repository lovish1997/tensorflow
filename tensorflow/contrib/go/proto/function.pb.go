// Code generated by protoc-gen-go.
// source: tensorflow/core/framework/function.proto
// DO NOT EDIT!

package tensorflow

import proto "github.com/golang/protobuf/proto"
import fmt "fmt"
import math "math"

// Reference imports to suppress errors if they are not otherwise used.
var _ = proto.Marshal
var _ = fmt.Errorf
var _ = math.Inf

// A library is a set of named functions.
type FunctionDefLibrary struct {
	Function []*FunctionDef `protobuf:"bytes,1,rep,name=function" json:"function,omitempty"`
	Gradient []*GradientDef `protobuf:"bytes,2,rep,name=gradient" json:"gradient,omitempty"`
}

func (m *FunctionDefLibrary) Reset()                    { *m = FunctionDefLibrary{} }
func (m *FunctionDefLibrary) String() string            { return proto.CompactTextString(m) }
func (*FunctionDefLibrary) ProtoMessage()               {}
func (*FunctionDefLibrary) Descriptor() ([]byte, []int) { return fileDescriptor3, []int{0} }

func (m *FunctionDefLibrary) GetFunction() []*FunctionDef {
	if m != nil {
		return m.Function
	}
	return nil
}

func (m *FunctionDefLibrary) GetGradient() []*GradientDef {
	if m != nil {
		return m.Gradient
	}
	return nil
}

// A function can be instantiated when the runtime can bind every attr
// with a value. When a GraphDef has a call to a function, it must
// have binding for every attr defined in the signature.
//
// TODO(zhifengc):
//   * device spec, etc.
type FunctionDef struct {
	// The definition of the function's name, arguments, return values,
	// attrs etc.
	Signature *OpDef `protobuf:"bytes,1,opt,name=signature" json:"signature,omitempty"`
	// The body of the function.
	Node []*FunctionDef_Node `protobuf:"bytes,2,rep,name=node" json:"node,omitempty"`
}

func (m *FunctionDef) Reset()                    { *m = FunctionDef{} }
func (m *FunctionDef) String() string            { return proto.CompactTextString(m) }
func (*FunctionDef) ProtoMessage()               {}
func (*FunctionDef) Descriptor() ([]byte, []int) { return fileDescriptor3, []int{1} }

func (m *FunctionDef) GetSignature() *OpDef {
	if m != nil {
		return m.Signature
	}
	return nil
}

func (m *FunctionDef) GetNode() []*FunctionDef_Node {
	if m != nil {
		return m.Node
	}
	return nil
}

// A node is a multi-value assignment:
//   (ret[0], ret[1], ...) = func(arg[0], arg[1], ...)
//
// By convention, "func" is resolved by consulting with a user-defined
// library first. If not resolved, "func" is assumed to be a builtin op.
type FunctionDef_Node struct {
	// This node produces multiple outputs. They are named ret[0],
	// ret[1], ..., etc.
	//
	// REQUIRES: function.node.ret[*] are unique across all nodes.
	// REQUIRES: ret.size == func/op def's number of output args.
	Ret []string `protobuf:"bytes,1,rep,name=ret" json:"ret,omitempty"`
	// The op/function name.
	Op string `protobuf:"bytes,2,opt,name=op" json:"op,omitempty"`
	// Arguments passed to this func/op.
	//
	// arg[i] must be either one of
	// function.signature.input_args[*].name or one of
	// function.node[*].ret[*].
	//
	// REQUIRES: arg.size == func/op def's number of input args.
	Arg []string `protobuf:"bytes,3,rep,name=arg" json:"arg,omitempty"`
	// Control dependencies.
	//
	// dep[i] must be one of function.node[*].ret[*] or one of
	// function.signature.input_args[*].name.
	Dep []string `protobuf:"bytes,4,rep,name=dep" json:"dep,omitempty"`
	// Attrs.
	//
	// 'attr' maps names defined by 'func's attr defs to attr values.
	// attr values may have placeholders which are substituted
	// recursively by concrete values when this node is instantiated.
	// These placeholders must name an attr listed in the FunctionDef's
	// signature.
	Attr map[string]*AttrValue `protobuf:"bytes,5,rep,name=attr" json:"attr,omitempty" protobuf_key:"bytes,1,opt,name=key" protobuf_val:"bytes,2,opt,name=value"`
}

func (m *FunctionDef_Node) Reset()                    { *m = FunctionDef_Node{} }
func (m *FunctionDef_Node) String() string            { return proto.CompactTextString(m) }
func (*FunctionDef_Node) ProtoMessage()               {}
func (*FunctionDef_Node) Descriptor() ([]byte, []int) { return fileDescriptor3, []int{1, 0} }

func (m *FunctionDef_Node) GetAttr() map[string]*AttrValue {
	if m != nil {
		return m.Attr
	}
	return nil
}

// GradientDef defines the gradient function of a function defined in
// a function library.
//
// A gradient function g (specified by gradient_func) for a function f
// (specified by function_name) must follow the following:
//
// The function 'f' must be a numerical function which takes N inputs
// and produces M outputs. Its gradient function 'g', which is a
// function taking N + M inputs and produces N outputs.
//
// I.e. if we have
//    (y1, y2, ..., y_M) = f(x1, x2, ..., x_N),
// then, g is
//    (dL/dx1, dL/dx2, ..., dL/dx_N) = g(x1, x2, ..., x_N,
//                                      dL/dy1, dL/dy2, ..., dL/dy_M),
// where L is a scalar-value function of (x1, x2, ..., xN) (e.g., the
// loss function). dL/dx_i is the partial derivative of L with respect
// to x_i.
type GradientDef struct {
	FunctionName string `protobuf:"bytes,1,opt,name=function_name,json=functionName" json:"function_name,omitempty"`
	GradientFunc string `protobuf:"bytes,2,opt,name=gradient_func,json=gradientFunc" json:"gradient_func,omitempty"`
}

func (m *GradientDef) Reset()                    { *m = GradientDef{} }
func (m *GradientDef) String() string            { return proto.CompactTextString(m) }
func (*GradientDef) ProtoMessage()               {}
func (*GradientDef) Descriptor() ([]byte, []int) { return fileDescriptor3, []int{2} }

func init() {
	proto.RegisterType((*FunctionDefLibrary)(nil), "tensorflow.FunctionDefLibrary")
	proto.RegisterType((*FunctionDef)(nil), "tensorflow.FunctionDef")
	proto.RegisterType((*FunctionDef_Node)(nil), "tensorflow.FunctionDef.Node")
	proto.RegisterType((*GradientDef)(nil), "tensorflow.GradientDef")
}

var fileDescriptor3 = []byte{
	// 389 bytes of a gzipped FileDescriptorProto
	0x1f, 0x8b, 0x08, 0x00, 0x00, 0x09, 0x6e, 0x88, 0x02, 0xff, 0x7c, 0x92, 0xcf, 0x4a, 0xeb, 0x40,
	0x14, 0xc6, 0x49, 0x9a, 0x5e, 0x6e, 0x4f, 0x7a, 0xcb, 0x75, 0x40, 0x0c, 0xc1, 0x85, 0x54, 0x28,
	0x45, 0x25, 0x91, 0x76, 0x23, 0xdd, 0x59, 0xfc, 0xb3, 0x91, 0x5a, 0xb2, 0xd0, 0x65, 0x48, 0x9b,
	0x49, 0x08, 0xad, 0x33, 0x61, 0x3a, 0xb5, 0x74, 0xe3, 0x0b, 0xfa, 0x02, 0x3e, 0x8e, 0x73, 0x26,
	0x49, 0x1b, 0x90, 0xba, 0x3b, 0xf9, 0xe6, 0xf7, 0x7d, 0x73, 0x4e, 0xce, 0x40, 0x5f, 0x52, 0xb6,
	0xe2, 0x22, 0x59, 0xf2, 0x8d, 0x3f, 0xe7, 0x82, 0xfa, 0x89, 0x88, 0xde, 0xe8, 0x86, 0x8b, 0x85,
	0x9f, 0xac, 0xd9, 0x5c, 0x66, 0x9c, 0x79, 0xb9, 0xe0, 0x92, 0x13, 0xd8, 0x93, 0xee, 0xc5, 0x61,
	0x57, 0x24, 0xa5, 0x08, 0xdf, 0xa3, 0xe5, 0x9a, 0x16, 0x3e, 0xb7, 0x77, 0x98, 0xe5, 0x79, 0x18,
	0xd3, 0xa4, 0xe0, 0xba, 0x1f, 0x40, 0x1e, 0xca, 0x1b, 0xef, 0x68, 0xf2, 0x94, 0xcd, 0x44, 0x24,
	0xb6, 0x64, 0x08, 0x7f, 0xab, 0x3e, 0x1c, 0xe3, 0xac, 0xd1, 0xb7, 0x07, 0x27, 0xde, 0x3e, 0xd0,
	0xab, 0x39, 0x82, 0x1d, 0x88, 0xa6, 0x54, 0x44, 0x71, 0x46, 0x99, 0x74, 0xcc, 0x9f, 0xa6, 0xc7,
	0xf2, 0x4c, 0x9b, 0x2a, 0xb0, 0xfb, 0x69, 0x82, 0x5d, 0x8b, 0x23, 0x3e, 0xb4, 0x56, 0x59, 0xca,
	0x22, 0xb9, 0x16, 0x54, 0x5d, 0x6d, 0xa8, 0x94, 0xa3, 0x7a, 0xca, 0x73, 0x8e, 0xfe, 0x3d, 0x43,
	0xae, 0xc1, 0x62, 0x3c, 0xa6, 0xe5, 0x8d, 0xa7, 0x07, 0xda, 0xf4, 0x26, 0x8a, 0x09, 0x34, 0xe9,
	0x7e, 0x19, 0x60, 0xe1, 0x27, 0xf9, 0x0f, 0x0d, 0x41, 0xa5, 0x1e, 0xb0, 0x15, 0x60, 0x49, 0x3a,
	0x60, 0xf2, 0x5c, 0x45, 0x19, 0x4a, 0x50, 0x15, 0x12, 0x91, 0x48, 0x9d, 0x46, 0x41, 0xa8, 0x12,
	0x95, 0x98, 0xe6, 0x8e, 0x55, 0x28, 0xaa, 0x24, 0x23, 0xb0, 0xf0, 0xef, 0x3b, 0x4d, 0xdd, 0x40,
	0xef, 0xb7, 0x06, 0xbc, 0x5b, 0x05, 0xde, 0x33, 0x29, 0xb6, 0x81, 0xf6, 0xb8, 0x13, 0x68, 0xed,
	0x24, 0x8c, 0x5e, 0xd0, 0xad, 0x1e, 0x5a, 0x45, 0xab, 0x92, 0x5c, 0x42, 0x53, 0xef, 0x54, 0x77,
	0x64, 0x0f, 0x8e, 0xeb, 0xd9, 0xe8, 0x7b, 0xc1, 0xc3, 0xa0, 0x60, 0x46, 0xe6, 0x8d, 0xd1, 0x7d,
	0x05, 0xbb, 0xf6, 0x9b, 0xc9, 0x39, 0xfc, 0xab, 0xb6, 0x13, 0x32, 0xb5, 0xfe, 0x32, 0xbb, 0x5d,
	0x89, 0x13, 0xa5, 0x21, 0x54, 0x6d, 0x23, 0xc4, 0x83, 0x72, 0xfc, 0x76, 0x25, 0xe2, 0x10, 0xe3,
	0x2b, 0x70, 0xb8, 0x48, 0xeb, 0xf7, 0xef, 0xde, 0xd3, 0xb8, 0x53, 0x8d, 0x39, 0xc5, 0x17, 0xb5,
	0x9a, 0x1a, 0xb3, 0x3f, 0xfa, 0x6d, 0x0d, 0xbf, 0x03, 0x00, 0x00, 0xff, 0xff, 0x1c, 0xdd, 0x91,
	0x80, 0xe7, 0x02, 0x00, 0x00,
}
