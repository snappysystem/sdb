package sdb

// A class to enumerate all entries in the key value store
type Iterator interface {
	Valid() bool
	SeekToFirst()
	SeekToLast()
	Seek(key []byte)
	Next()
	Prev()
	Key() []byte
	Value() []byte
}

// A pool style allocator that does increamental allocation and
// reclaim all allocated space in a single call
type Allocator interface {
	Allocate(size int) []byte
	DeallocateAll()
}

// compare two binaries, return -1 if a is less than b, 0 if a is the same
// as b, and 1 if a is greater than b
type Order interface {
	Compare(a []byte, b []byte) int
}
