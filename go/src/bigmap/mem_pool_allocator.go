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
func MakeMemPoolAllocator(bytes ...int) Allocator {
	ret := &memPoolAllocator{}
	switch len(bytes) {
	case 0:
		ret.bytesPerAlloc = kBytesPerAlloc
	case 1:
		ret.bytesPerAlloc = bytes[0]
	default:
		panic("init only takes 0 or 1 parameter!")
	}

	ret.pool = make([][]byte, kNumBlocks)
	ret.current,_ = MmapAlloc(ret.bytesPerAlloc)
	return ret
}

// allocate @size bytes and return the space in the form
// of a byte slice
func (a *memPoolAllocator) Allocate(size int) []byte {
	if len(a.current) >= size {
		ret := a.current[:size]
		a.current = a.current[size:]
		return ret
	} else if len(a.current) > a.bytesPerAlloc {
		panic("Too big allocation")
	} else {
		a.pool = append(a.pool, a.current)
		a.current, _ = MmapAlloc(a.bytesPerAlloc)
		return a.Allocate(size)
	}
}

// release all memories that has been allocated
func (a *memPoolAllocator) DeallocateAll() {
	for _, a := range a.pool {
		MmapDealloc(a)
	}
}
