package sdb

import "testing"

func TestVarIntMultiTypeAppendAndParse(t *testing.T) {
	data := [...]uint64{0x5, 0xf7, 0x2301, 0x34562301, 0x234562301}
	b := make([]byte, 16)

	for i,x := range(data) {
		var size int

		{
			s,ok := AppendVarInt(b, 0, x)
			switch ok.(type) {
				case VarIntOk:
				default:
					t.Error("Fails to append int ", i)
			}

			size = s
		}

		{
			val,s,ok := ParseVarInt(b, 0)
			switch ok.(type) {
				case VarIntOk:
				default:
					t.Error("Fails to append int ", i)
			}

			if (val != x) {
				t.Error("value mismatch ", i)
			}
			if (s != size) {
				t.Error("size mismatch ", i, s)
			}
		}
	}
}

func TestMultiAppendAndParse(t *testing.T) {
	data := [...]uint64{0x5, 0xf7, 0x2301, 0x34562301, 0x234562301}
	b := make([]byte, 64)
	sizes := make([]int, len(data))
	off := 0

	for i,x := range(data) {
		s,ok := AppendVarInt(b, off, x)
		switch ok.(type) {
			case VarIntOk:
			default:
				t.Error("Fails to append int ", i)
		}
		sizes[i] = s
		off = off + s
	}

	off = 0
	for i,x := range(data) {
		val,s,ok := ParseVarInt(b, off)
		switch ok.(type) {
			case VarIntOk:
			default:
				t.Error("Fails to append int ", i)
		}

		if (val != x) {
			t.Error("value mismatch ", i)
		}
		if (s != sizes[i]) {
			t.Error("size mismatch ", i, s)
		}
		off = off + s
	}
}

