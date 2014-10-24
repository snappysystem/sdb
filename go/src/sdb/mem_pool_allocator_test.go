package sdb

import "testing"

func TestSingleMemPoolAlloc(t *testing.T) {
	mp := MakeMemPoolAllocator()
	x := mp.Allocate(512)
	if x == nil {
		t.Error("Fails to allocate block")
	}
}

func TestManyMemPoolAllocDealloc(t *testing.T) {
	mp := MakeMemPoolAllocator()

	for i := 0; i < 2048; i++ {
		x := mp.Allocate(16 * 1024 * 1024)
		if x == nil {
			t.Error("Fails to allocate block")
		}
	}

	mp.DeallocateAll()
}
