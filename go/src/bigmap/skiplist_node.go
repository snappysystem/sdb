package bigmap

import "unsafe"


// general interface for a skiplistNode in skip list
type skiplistNode interface {
  getKey() []byte
  getNext() skiplistNode
  getChild() skiplistNode
	setKey(key []byte)
	setNext(next skiplistNode)
	setChild(child skiplistNode)
}

// The real skiplistNode in a skip list
type skiplistLeafNode struct {
  key []byte
  value []byte
  next skiplistNode
}

func (a *skiplistLeafNode) getKey() []byte {
  return a.key
}

func (a *skiplistLeafNode) getNext() skiplistNode {
  return a.next
}

func (a *skiplistLeafNode) getChild() skiplistNode {
  return nil
}

func (a *skiplistLeafNode) setKey(key []byte) {
  a.key = key
}

func (a *skiplistLeafNode) setNext(next skiplistNode) {
  a.next = next
}

func (a *skiplistLeafNode) setChild(child skiplistNode) {
	panic("should not set child on leaf node")
}

// additional links in a skip list skiplistNode is represented by a skiplistPointerNode
type skiplistPointerNode struct {
  key []byte
	next skiplistNode
  child skiplistNode
}

func (a *skiplistPointerNode) getKey() []byte {
  return a.key
}

func (a *skiplistPointerNode) getNext() skiplistNode {
  return a.next
}

func (a *skiplistPointerNode) getChild() skiplistNode {
  return a.child
}

func (a *skiplistPointerNode) setKey(key []byte) {
  a.key = key
}

func (a *skiplistPointerNode) setNext(next skiplistNode) {
  a.next = next
}

func (a *skiplistPointerNode) setChild(child skiplistNode) {
  a.child = child
}


type skiplistNodeAllocator struct {
  leafSize int
  pointerSize int
  pool memPoolAllocator
}


// init memory pool, must be called after the allocator is created
func (a *skiplistNodeAllocator) init() {
  x1 := skiplistLeafNode{}
  x2 := skiplistPointerNode{}
	a.leafSize = int(unsafe.Sizeof(x1))
  a.pointerSize = int(unsafe.Sizeof(x2))
	a.pool.init()
}

// allocate a new leaf
func (a *skiplistNodeAllocator) newLeaf() *skiplistLeafNode {
	b := a.pool.allocate(a.leafSize)
  return (*skiplistLeafNode)(unsafe.Pointer(&b[0]))
}

// allocate a new pointer skiplistNode
func (a *skiplistNodeAllocator) newPointer() *skiplistPointerNode {
  b := a.pool.allocate(a.pointerSize)
  return (*skiplistPointerNode)(unsafe.Pointer(&b[0]))
}

// deallocate all skiplistNodes
func (a *skiplistNodeAllocator) deallocateAll() {
	a.pool.deallocateAll()
}

