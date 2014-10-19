package bigmap

const (
	kBytesPerAlloc = 128 * 1024 * 1024
	kNumBlocks     = 128
)

type memPoolAllocator struct {
	bytesPerAlloc int
	pool          [][]byte
	current       []byte
}

// takes 0 or 1 parameters. If there is no parameter, the default
// block allocation size is used. Otherwise, use the specified
// value as block allocation size
func (a *memPoolAllocator) init(bytes ...int) {
	switch len(bytes) {
	case 0:
		a.bytesPerAlloc = kBytesPerAlloc
	case 1:
		a.bytesPerAlloc = bytes[0]
	default:
		panic("init only takes 0 or 1 parameter!")
	}

	a.pool = make([][]byte, kNumBlocks)
	a.current, _ = MmapAlloc(a.bytesPerAlloc)
}

// allocate @size bytes and return the space in the form
// of a byte slice
func (a *memPoolAllocator) allocate(size int) []byte {
	if len(a.current) >= size {
		ret := a.current[:size]
		a.current = a.current[size:]
		return ret
	} else if len(a.current) > a.bytesPerAlloc {
		panic("Too big allocation")
	} else {
		a.pool = append(a.pool, a.current)
		a.current, _ = MmapAlloc(a.bytesPerAlloc)
		return a.allocate(size)
	}
}

// release all memories that has been allocated
func (a *memPoolAllocator) deallocateAll() {
	for _, a := range a.pool {
		MmapDealloc(a)
	}
}
