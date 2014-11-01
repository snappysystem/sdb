package sdb

import (
	"unsafe"
)

// encode an integer value to the end of @scratch, returns
// the resulting slice.
func EncodeUint32(scratch []byte, val uint32) []byte {
	c, size := cap(scratch), len(scratch)
	if c < size + 4 {
		newCap := c*2
		if (newCap < size + 4) {
			newCap = size + 4
		}
		tmp := make([]byte, size, newCap)
		copy(tmp, scratch)
		scratch = tmp
	}

	scratch = scratch[:(size+4)]
	*(*uint32)(unsafe.Pointer(&scratch[size])) = val
	return scratch
}

// encode an integer value to the end of @scratch, returns
// the resulting slice.
func EncodeUint64(scratch []byte, val uint64) []byte {
	c, size := cap(scratch), len(scratch)
	if c < size + 8 {
		newCap := c*2
		if (newCap < size + 8) {
			newCap = size + 8
		}
		tmp := make([]byte, size, newCap)
		copy(tmp, scratch)
		scratch = tmp
	}

	scratch = scratch[:(size+8)]
	*(*uint64)(unsafe.Pointer(&scratch[size])) = val
	return scratch
}

// encode a slice @data into @scratch, and return the resulting slice
func EncodeSlice(scratch []byte, data []byte) []byte {
	dsize := len(data)
	c, size := cap(scratch), len(scratch)
	if c < size + dsize + 4 {
		newCap := c*2
		if (newCap < size + dsize + 4) {
			newCap = size + dsize + 4
		}
		tmp := make([]byte, size, newCap)
		copy(tmp, scratch)
		scratch = tmp
	}

	scratch = scratch[:(size + dsize + 4)]
	*(*uint32)(unsafe.Pointer(&scratch[size])) = uint32(dsize)
	copy(scratch[size+4:], data)

	return scratch
}

// decode a uint32 value and return the slice after the bytes
// have been consumed by the decode process
func DecodeUint32(data []byte) (val uint32, result []byte) {
	if len(data) < 4 {
		result = data
	} else {
		val = *(*uint32)(unsafe.Pointer(&data[0]))
		result = data[4:]
	}
	return
}


// decode a uint64 value and return the slice after the bytes
// have been consumed by the decode process
func DecodeUint64(data []byte) (val uint64, result []byte) {
	if len(data) < 8 {
		result = data
	} else {
		val = *(*uint64)(unsafe.Pointer(&data[0]))
		result = data[8:]
	}
	return
}

// decode a slice and return it after the bytes
// have been consumed by the decode process
func DecodeSlice(data []byte) (val []byte, result []byte) {
	origin := len(data)
	if origin < 4 {
		result = data
		return
	}

	sliceLen := *(*uint32)(unsafe.Pointer(&data[0]))

	if uint32(origin) < sliceLen + 4 {
		result = data
		return
	}

	val = data[4:sliceLen+4]
	result = data[sliceLen+4:]
	return
}


