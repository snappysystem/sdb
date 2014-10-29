package sdb

// For efficiency reason, this implementation does not convert
// integers into big endian format in a file. This is usually
// fine since we are only use the same data locally which is
// very unlikely to have an endianess change. But if uses arise
// that requires physically copy the data to other servers
// which potentially have different kind of CPUs, we may have
// to revisit the decision and implement a endianess aware
// implementation

import (
	"sort"
	"unsafe"
)

type BlockBuilder struct {
	data        []byte
	startOffset uint32
	cur         uint32
	dataSize    uint32
	keys        []uint32
}

// A tailer of block, always at the end of a block
type blockTailer struct {
	dataSize      uint32
	numKeys       uint32
	restartOffset uint32
}

type Block struct {
	data          []byte
	firstOffset   uint32
	lastOffset    uint32
	restartOffset uint32
	numKeys       uint32
}

type blockIter struct {
	block *Block
	order Comparator
	idx   int32
}

func (a *Block) NewIterator(o Comparator) Iterator {
	ret := &blockIter{}
	ret.block = a
	ret.order = o
	ret.idx = -1
	return ret
}

// Parse an entry starting at offset @off, returns key, value along with
// how many bytes has been consumed
func parseSimpleEntry(data []byte, off uint32) (key, val []byte, s uint32) {
	keylen := uint32(0)
	vallen := uint32(0)
	pos := int(off)

	// parse key length
	{
		v, l, ok := ParseVarInt(data, pos)
		switch ok.(type) {
		case VarIntOk:
		default:
			return
		}

		keylen = uint32(v)
		s = s + uint32(l)
		pos = pos + l
	}

	// parse value length
	{
		v, l, ok := ParseVarInt(data, pos)
		switch ok.(type) {
		case VarIntOk:
		default:
			return
		}

		vallen = uint32(v)
		s = s + uint32(l)
		pos = pos + l
	}

	// parse key
	{
		key = data[pos : pos+int(keylen)]
		s = s + keylen
		pos = pos + int(keylen)
	}

	// parse value
	{
		val = data[pos : pos+int(vallen)]
		s = s + vallen
		pos = pos + int(vallen)
	}

	return
}

func (a *blockIter) Valid() bool {
	return (a.idx >= 0 && a.idx < int32(a.block.numKeys))
}

func (a *blockIter) SeekToFirst() {
	if a.block.numKeys >= 0 {
		a.idx = 0
	} else {
		a.idx = -1
	}
}

func (a *blockIter) SeekToLast() {
	a.idx = int32(a.block.numKeys)
}

// Find and point to the key. If key does not exist, point to the
// key that immediately follow @key in the index
func (a *blockIter) Seek(mark []byte) {
	b := a.block
	a.idx = int32(sort.Search(
		int(b.numKeys),
		func(n int) bool {
			loc := &b.data[int(b.restartOffset)+n*4]
			offsetPtr := (*uint32)(unsafe.Pointer(loc))
			key, _, consumed := parseSimpleEntry(b.data, *offsetPtr)
			if consumed == 0 {
				panic("corrupted data")
			}

			return (a.order.Compare(key, mark) >= 0)
		}))
}

func (a *blockIter) Next() {
	a.idx++
}

func (a *blockIter) Prev() {
	a.idx--
}

func (a *blockIter) Key() []byte {
	b := a.block
	loc := &b.data[b.restartOffset+uint32(a.idx)*4]
	offsetPtr := (*uint32)(unsafe.Pointer(loc))
	key, _, consumed := parseSimpleEntry(b.data, *offsetPtr)
	if consumed == 0 {
		panic("corrupted data")
	}
	return key
}

func (a *blockIter) Value() []byte {
	b := a.block
	loc := &b.data[b.restartOffset+uint32(a.idx)*4]
	offsetPtr := (*uint32)(unsafe.Pointer(loc))
	_, val, consumed := parseSimpleEntry(b.data, *offsetPtr)
	if consumed == 0 {
		panic("corrupted data")
	}
	return val
}

// create a new BlockBuilder and initialize it
// pass the slice that is going to be used to build the block
// along with the offset from where the block is going to build
func MakeBlockBuilder(data []byte, off uint32) *BlockBuilder {
	ret := &BlockBuilder{}
	ret.data = data
	ret.startOffset = off
	ret.cur = off
	ret.keys = make([]uint32, 0, 16*1024)
	return ret
}

// Add a key and a value at a time, return true if success
func (a *BlockBuilder) Add(key []byte, val []byte) bool {
	keylen := len(key)
	vallen := len(val)

	entryOffset := a.cur

	// append key length
	{
		s, ok := AppendVarInt(a.data, int(a.cur), uint64(keylen))
		switch ok.(type) {
		case VarIntOk:
		default:
			return false
		}
		a.cur = a.cur + uint32(s)
		a.dataSize = a.dataSize + uint32(s)
	}

	// append value length
	{
		s, ok := AppendVarInt(a.data, int(a.cur), uint64(vallen))
		switch ok.(type) {
		case VarIntOk:
		default:
			return false
		}
		a.cur = a.cur + uint32(s)
		a.dataSize = a.dataSize + uint32(s)
	}

	// append key
	{
		newKey := a.data[a.cur : a.cur+uint32(keylen)]
		s := copy(newKey, key)
		if s != keylen {
			return false
		}
		a.cur = a.cur + uint32(s)
		a.dataSize = a.dataSize + uint32(keylen)
	}

	// append value
	{
		s := copy(a.data[a.cur:a.cur+uint32(vallen)], val)
		if s != vallen {
			return false
		}
		a.cur = a.cur + uint32(s)
		a.dataSize = a.dataSize + uint32(vallen)
	}

	a.keys = append(a.keys, entryOffset)
	return true
}

var modelTailer blockTailer

// Finish building the block, return the slice that denotes
// the boundary of the block. Return true if operation succeeds
func (a *BlockBuilder) Finalize() (ret *Block, ok bool) {
	// align starting of restart offset to 8 byte boundary
	restart := a.startOffset + a.dataSize
	restart = (restart + 7) / 8 * 8
	pos := restart

	// save all key offsets
	for _, off := range a.keys {
		intPtr := (*uint32)(unsafe.Pointer(&a.data[pos]))
		pos = pos + 4
		if int(pos) >= len(a.data) {
			return
		}

		*intPtr = uint32(off)
	}

	// prepare tailer
	tail := (*blockTailer)(unsafe.Pointer(&a.data[pos]))
	pos = pos + uint32(unsafe.Sizeof(modelTailer))
	if int(pos) >= len(a.data) {
		return
	}

	tail.dataSize = a.dataSize
	tail.numKeys = uint32(len(a.keys))
	tail.restartOffset = restart

	//prepare result
	ret = &Block{}

	ret.data = a.data
	ret.firstOffset = a.startOffset
	ret.lastOffset = pos
	ret.restartOffset = restart
	ret.numKeys = uint32(len(a.keys))

	ok = true

	return
}
