package sdb

import (
	"math"
)

type VarIntOk struct{}

type VarIntBufferBound struct{}

func handleBoundError(ok *interface{}) {
	if thing := recover(); thing != nil {
		err := thing.(error)
		if err.Error() == "runtime error: index out of range" {
			*ok = VarIntBufferBound{}
			return
		}
		panic("Something else goes wrong")
	}
}

func writeValue(data []byte, off, extraSize int, val uint64) {
	flag := int8(-extraSize)
	data[off] = uint8(flag)

	for i := extraSize; i > 0; i-- {
		data[off+i] = uint8(val)
		val = val >> 8
	}
}

// parse data starting from @offset. Return the parsed value, consumed
// bytes and true if succeeds
func ParseVarInt(data []byte, off int) (val uint64, size int, ok interface{}) {
	defer handleBoundError(&ok)

	flag := int8(data[off])
	switch {
	case flag >= 0:
		val, size, ok = uint64(data[off]), 1, VarIntOk{}
	case flag == -2:
		fallthrough
	case flag == -4:
		fallthrough
	case flag == -8:
		positive := int(-flag)
		val = 0
		for i := 1; i <= positive; i++ {
			val = val << 8
			val = val + uint64(data[off+i])
		}
		size, ok = positive+1, VarIntOk{}
	}

	return
}

func AppendVarInt(data []byte, off int, val uint64) (size int, ok interface{}) {
	defer handleBoundError(&ok)
	switch {
	case val <= math.MaxInt8:
		data[off], size = uint8(val), 1
	case val <= math.MaxUint16:
		writeValue(data, off, 2, val)
		size = 3
	case val <= math.MaxUint32:
		writeValue(data, off, 4, val)
		size = 5
	default:
		writeValue(data, off, 8, val)
		size = 9
	}

	ok = VarIntOk{}
	return
}
