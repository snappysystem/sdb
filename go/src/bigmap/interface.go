package bigmap


type Iterator interface {
	Valid() bool
	SeekToFirst()
	SeekToLast()
	Seek()
	Next()
	Prev()
	Key() []byte
	Value() []byte
}

// compare two binaries, return -1 if a is less than b, 0 if a is the same
// as b, and 1 if a is greater than b
type Order interface {
	Compare(a []byte, b []byte) int
}
