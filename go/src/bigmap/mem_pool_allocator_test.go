package bigmap

import "testing"

func TestSingleMemPoolAlloc(t *testing.T) {
	mp := memPoolAllocator{}
	mp.init()

	x := mp.allocate(512)
	if x == nil {
		t.Error("Fails to allocate block")
	}
}

func TestManyMemPoolAllocDealloc(t *testing.T) {
	mp := memPoolAllocator{}
	mp.init()

	for i := 0; i < 2048; i++ {
		x := mp.allocate(16 * 1024 * 1024)
		if x == nil {
			t.Error("Fails to allocate block")
		}
	}

	mp.deallocateAll()
}
