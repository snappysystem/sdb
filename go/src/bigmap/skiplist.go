package bigmap

import "bytes"


// compare two binaries, return -1 if a is less than b, 0 if a is the same
// as b, and 1 if a is greater than b
type SkiplistOrder interface {
  Compare(a []byte, b []byte) int
}


type BytesSkiplistOrder struct {
}

func (x BytesSkiplistOrder) Compare(a, b []byte) int {
  return bytes.Compare(a, b)
}


type Skiplist struct {
	levels []skiplistNode
  allocator skiplistNodeAllocator
  gen *randomGenerator
	order SkiplistOrder
  numNodes int
}


// Create a new skiplist
func MakeSkiplist(args ...SkiplistOrder) *Skiplist {
	ret := Skiplist{}

  switch (len(args)) {
		case 0:
			ret.order = &BytesSkiplistOrder{}
		case 1:
			ret.order = args[0]
    default:
			panic("args is either 0 or 1")
  }

  ret.levels = make([]skiplistNode, 0, 16)
  ret.allocator.init()
  ret.gen = makeRandomGenerator()
  ret.numNodes = 0
  return &ret
}

// insert a key value pair into skip list. If the key is already
// in the list, the entry will not be updated. The orginal value
// of the key will be returned
func (a *Skiplist) Put(key []byte, val []byte) (old []byte, ok bool) {
	prevList,found := a.trace(key)
  if found {
		leaf := prevList[0].(*skiplistLeafNode)
		ok,old = false,leaf.value
		return
  }

  height := a.gen.get() + 1
  var child skiplistNode

  for i := 0; i < height; i++ {
		var newNode skiplistNode
		if i == 0 {
			newLeaf := a.allocator.newLeaf()
			newLeaf.value = val
			newNode = newLeaf
    } else {
			newPointer := a.allocator.newPointer()
			newNode = newPointer
		}

		newNode.setKey(key)

		if (prevList[i] != nil) {
			newNode.setNext(prevList[i].getNext())
			prevList[i].setNext(newNode)
		} else {
			newNode.setNext(a.levels[i])
			if (a.levels[i] != nil) {
				a.levels[i].setNext(newNode)
			} else {
				a.levels[i] = newNode
			}
		}

		if (child != nil) {
			newNode.setChild(child)
		}

		child = newNode
	}

	ok = true
	return
}

// Look up a key in the skiplist. Return the corresponding value and true
// if the key is in the skiplist. Otherwise return an empty slice and
// false
func (a *Skiplist) Get(key []byte) (value []byte, ok bool) {
	prevList,ok := a.trace(key)
  if ok {
		leaf := prevList[0].(*skiplistLeafNode)
		value, ok = leaf.value, true
	} else {
		ok = false
	}
	return
}

func (a *Skiplist) trace(key []byte) (ret []skiplistNode, found bool) {
  numLevels := len(a.levels)
  ret = make([]skiplistNode, numLevels)
  var prev skiplistNode

  for cur,i := a.levels[numLevels - 1], numLevels -1; i >= 0; {
    if (cur == nil) {
			i--
			cur = a.levels[i]
      continue
		}

		switch a.order.Compare(cur.getKey(), key) {
			case -1:
				prev = cur
				cur = cur.getNext()
				if (cur == nil) {
					ret[i] = prev
					i--
					cur = prev.getChild()
					prev = nil
				}
			case 0:
				found = true
				ret[i] = cur
				i--
				cur = cur.getChild()
        prev = nil
			case 1:
				ret[i] = prev
				i--
				if (prev != nil) {
				  cur = prev.getChild()
				  prev = nil
        } else {
					cur = a.levels[i]
				}
			default:
				panic("Invaid comparison value")
		}
	}

	return
}

