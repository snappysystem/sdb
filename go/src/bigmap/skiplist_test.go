package bigmap

import (
	"testing"
	"bytes"
	"math/rand"
	"time"
	"fmt"
)

func TestSkiplistPutGetLess(t *testing.T) {
	slist := MakeSkiplist()
	data := [...]string{"hello", "world", "go", "language"}

	for _,s := range(data) {
		arr := []byte(s)
		_,ok := slist.Put(arr, arr)
		if !ok {
			t.Error("Fails to put ", s)
		}
	}

	for _,s := range(data) {
		arr := []byte(s)
		val,ok := slist.Get(arr)
		if !ok || bytes.Compare(arr, val) != 0 {
			t.Error("Fails to find key ", s)
		}
	}
}

func genRandomBytes() []byte {
	size := 2 + rand.Intn(16)
	ret := make([]byte, size)
	for i := 0; i < size; i++ {
		ret[i] = byte(rand.Intn(25)) + 'a'
	}

	return ret
}

func TestSkiplistPutGetMore(t *testing.T) {
	const numElements = 5000
	data := make([][]byte, 0, numElements)
	slist := MakeSkiplist()

	for i := 0; i < numElements; i++ {
		key := genRandomBytes()
		data = append(data, key)
		slist.Put(key, key)
	}

	for i,k := range(data) {
		val,ok := slist.Get(k)
		if !ok || bytes.Compare(val, k) != 0 {
			t.Error("Fails to find key ", i, " ", string(k))
		}
	}
}

func TestSkiplistPutPerf(t *testing.T) {
	const numElements = 500

	data := make(map[string][]byte)
	slist := MakeSkiplist()

	mapTime := int64(0)
	skiplistTime := int64(0)

	for i := 0; i < numElements; i++ {
		key := genRandomBytes()
		str := string(key)

		{
			t1 := time.Now()
			data[str] = key
			t2 := time.Now()
			delta := t2.Sub(t1).Nanoseconds()
			mapTime = mapTime + delta
		}

		{
			t1 := time.Now()
			slist.Put(key, key)
			t2 := time.Now()
			delta := t2.Sub(t1).Nanoseconds()
			skiplistTime = skiplistTime + delta
		}
	}

	fmt.Println("map uses ", mapTime/numElements, " nanoseconds per op")
	fmt.Println("ski uses ", skiplistTime/numElements, " nanoseconds per op")
}

