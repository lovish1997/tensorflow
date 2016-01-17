// Code generated by protoc-gen-go.
// source: tensorflow/core/util/event.proto
// DO NOT EDIT!

/*
Package tensorflow is a generated protocol buffer package.

It is generated from these files:
	tensorflow/core/util/event.proto
	tensorflow/core/util/saved_tensor_slice.proto

It has these top-level messages:
	Event
	SavedSliceMeta
	SavedTensorSliceMeta
	SavedSlice
	SavedTensorSlices
*/
package tensorflow

import proto "github.com/golang/protobuf/proto"
import fmt "fmt"
import math "math"
import tensorflow7 "tensorflow/core/framework"
import tensorflow8 "tensorflow/core/framework"

// Reference imports to suppress errors if they are not otherwise used.
var _ = proto.Marshal
var _ = fmt.Errorf
var _ = math.Inf

// This is a compile-time assertion to ensure that this generated file
// is compatible with the proto package it is being compiled against.
const _ = proto.ProtoPackageIsVersion1

// Protocol buffer representing an event that happened during
// the execution of a Brain model.
type Event struct {
	// Timestamp of the event.
	WallTime float64 `protobuf:"fixed64,1,opt,name=wall_time" json:"wall_time,omitempty"`
	// Global step of the event.
	Step int64 `protobuf:"varint,2,opt,name=step" json:"step,omitempty"`
	// Types that are valid to be assigned to What:
	//	*Event_FileVersion
	//	*Event_GraphDef
	//	*Event_Summary
	What isEvent_What `protobuf_oneof:"what"`
}

func (m *Event) Reset()                    { *m = Event{} }
func (m *Event) String() string            { return proto.CompactTextString(m) }
func (*Event) ProtoMessage()               {}
func (*Event) Descriptor() ([]byte, []int) { return fileDescriptor0, []int{0} }

type isEvent_What interface {
	isEvent_What()
}

type Event_FileVersion struct {
	FileVersion string `protobuf:"bytes,3,opt,name=file_version,oneof"`
}
type Event_GraphDef struct {
	GraphDef *tensorflow7.GraphDef `protobuf:"bytes,4,opt,name=graph_def,oneof"`
}
type Event_Summary struct {
	Summary *tensorflow8.Summary `protobuf:"bytes,5,opt,name=summary,oneof"`
}

func (*Event_FileVersion) isEvent_What() {}
func (*Event_GraphDef) isEvent_What()    {}
func (*Event_Summary) isEvent_What()     {}

func (m *Event) GetWhat() isEvent_What {
	if m != nil {
		return m.What
	}
	return nil
}

func (m *Event) GetFileVersion() string {
	if x, ok := m.GetWhat().(*Event_FileVersion); ok {
		return x.FileVersion
	}
	return ""
}

func (m *Event) GetGraphDef() *tensorflow7.GraphDef {
	if x, ok := m.GetWhat().(*Event_GraphDef); ok {
		return x.GraphDef
	}
	return nil
}

func (m *Event) GetSummary() *tensorflow8.Summary {
	if x, ok := m.GetWhat().(*Event_Summary); ok {
		return x.Summary
	}
	return nil
}

// XXX_OneofFuncs is for the internal use of the proto package.
func (*Event) XXX_OneofFuncs() (func(msg proto.Message, b *proto.Buffer) error, func(msg proto.Message, tag, wire int, b *proto.Buffer) (bool, error), func(msg proto.Message) (n int), []interface{}) {
	return _Event_OneofMarshaler, _Event_OneofUnmarshaler, _Event_OneofSizer, []interface{}{
		(*Event_FileVersion)(nil),
		(*Event_GraphDef)(nil),
		(*Event_Summary)(nil),
	}
}

func _Event_OneofMarshaler(msg proto.Message, b *proto.Buffer) error {
	m := msg.(*Event)
	// what
	switch x := m.What.(type) {
	case *Event_FileVersion:
		b.EncodeVarint(3<<3 | proto.WireBytes)
		b.EncodeStringBytes(x.FileVersion)
	case *Event_GraphDef:
		b.EncodeVarint(4<<3 | proto.WireBytes)
		if err := b.EncodeMessage(x.GraphDef); err != nil {
			return err
		}
	case *Event_Summary:
		b.EncodeVarint(5<<3 | proto.WireBytes)
		if err := b.EncodeMessage(x.Summary); err != nil {
			return err
		}
	case nil:
	default:
		return fmt.Errorf("Event.What has unexpected type %T", x)
	}
	return nil
}

func _Event_OneofUnmarshaler(msg proto.Message, tag, wire int, b *proto.Buffer) (bool, error) {
	m := msg.(*Event)
	switch tag {
	case 3: // what.file_version
		if wire != proto.WireBytes {
			return true, proto.ErrInternalBadWireType
		}
		x, err := b.DecodeStringBytes()
		m.What = &Event_FileVersion{x}
		return true, err
	case 4: // what.graph_def
		if wire != proto.WireBytes {
			return true, proto.ErrInternalBadWireType
		}
		msg := new(tensorflow7.GraphDef)
		err := b.DecodeMessage(msg)
		m.What = &Event_GraphDef{msg}
		return true, err
	case 5: // what.summary
		if wire != proto.WireBytes {
			return true, proto.ErrInternalBadWireType
		}
		msg := new(tensorflow8.Summary)
		err := b.DecodeMessage(msg)
		m.What = &Event_Summary{msg}
		return true, err
	default:
		return false, nil
	}
}

func _Event_OneofSizer(msg proto.Message) (n int) {
	m := msg.(*Event)
	// what
	switch x := m.What.(type) {
	case *Event_FileVersion:
		n += proto.SizeVarint(3<<3 | proto.WireBytes)
		n += proto.SizeVarint(uint64(len(x.FileVersion)))
		n += len(x.FileVersion)
	case *Event_GraphDef:
		s := proto.Size(x.GraphDef)
		n += proto.SizeVarint(4<<3 | proto.WireBytes)
		n += proto.SizeVarint(uint64(s))
		n += s
	case *Event_Summary:
		s := proto.Size(x.Summary)
		n += proto.SizeVarint(5<<3 | proto.WireBytes)
		n += proto.SizeVarint(uint64(s))
		n += s
	case nil:
	default:
		panic(fmt.Sprintf("proto: unexpected type %T in oneof", x))
	}
	return n
}

func init() {
	proto.RegisterType((*Event)(nil), "tensorflow.Event")
}

var fileDescriptor0 = []byte{
	// 218 bytes of a gzipped FileDescriptorProto
	0x1f, 0x8b, 0x08, 0x00, 0x00, 0x09, 0x6e, 0x88, 0x02, 0xff, 0x74, 0xce, 0xcd, 0x4a, 0xc4, 0x30,
	0x10, 0xc0, 0xf1, 0x8d, 0xdb, 0x5d, 0xe9, 0xb8, 0x17, 0xa3, 0x48, 0xd8, 0xd3, 0x22, 0xf8, 0x75,
	0x69, 0x40, 0xdf, 0x40, 0x14, 0xf7, 0xee, 0x03, 0x94, 0xa8, 0x13, 0x37, 0x98, 0x34, 0x65, 0x92,
	0x6d, 0xf0, 0x59, 0x7c, 0x59, 0xd3, 0x56, 0xa8, 0x08, 0x1e, 0x33, 0xf3, 0x0b, 0xff, 0x81, 0x4d,
	0xc4, 0x26, 0x78, 0xd2, 0xd6, 0x27, 0xf9, 0xea, 0x09, 0xe5, 0x3e, 0x1a, 0x2b, 0xb1, 0xc3, 0x26,
	0x56, 0x2d, 0xf9, 0xe8, 0x39, 0x4c, 0x62, 0x7d, 0xf1, 0x57, 0x6b, 0x52, 0x0e, 0x93, 0xa7, 0x0f,
	0xf9, 0x4e, 0xaa, 0xdd, 0x8d, 0x5f, 0xd6, 0x57, 0xff, 0xb3, 0xb0, 0x77, 0x4e, 0xd1, 0xe7, 0x08,
	0xcf, 0xbf, 0x18, 0x2c, 0x1e, 0xfb, 0x16, 0x3f, 0x86, 0x32, 0x29, 0x6b, 0xeb, 0x68, 0x1c, 0x0a,
	0xb6, 0x61, 0xd7, 0x8c, 0xaf, 0xa0, 0x08, 0x11, 0x5b, 0x71, 0x90, 0x5f, 0x73, 0x7e, 0x06, 0x2b,
	0x6d, 0x2c, 0xd6, 0x1d, 0x52, 0x30, 0xbe, 0x11, 0xf3, 0x3c, 0x2d, 0xb7, 0x33, 0x7e, 0x03, 0xe5,
	0x90, 0xae, 0xdf, 0x50, 0x8b, 0x22, 0x0f, 0x8f, 0x6e, 0x4f, 0xab, 0xa9, 0x5f, 0x3d, 0xf5, 0xcb,
	0x07, 0xd4, 0x99, 0x5e, 0xc2, 0xe1, 0x4f, 0x5e, 0x2c, 0x06, 0x78, 0xf2, 0x1b, 0x3e, 0x8f, 0xab,
	0xed, 0xec, 0x7e, 0x09, 0x45, 0xda, 0xa9, 0xf8, 0xb2, 0x1c, 0x8e, 0xbc, 0xfb, 0x0e, 0x00, 0x00,
	0xff, 0xff, 0x2d, 0xd7, 0xf3, 0xfa, 0x24, 0x01, 0x00, 0x00,
}
