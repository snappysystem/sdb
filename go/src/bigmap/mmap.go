package bigmap

import "syscall"

// allocate @length bytes through mmap call
func MmapAlloc(length int) (data []byte, err error) {
  data, err = syscall.Mmap(
    -1, 0, length,
    syscall.PROT_READ | syscall.PROT_WRITE,
    syscall.MAP_ANONYMOUS | syscall.MAP_PRIVATE);
  return
}

func MmapDealloc(bytes []byte) error {
  err := syscall.Munmap(bytes)
  return err
}
