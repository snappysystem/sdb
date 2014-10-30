package sdb

import (
	"hash/crc32"
	"unsafe"
)

const (
	// size of a log block
	kBlockSize = 32768
	// header includes checksum (4 bytes), type (1 byte), length (2 bytes)
	kHeaderSize = 4 + 1 + 2

	// A single, full record
	kFullType = 1

	// Following are for fragments
	kFirstType  = 2
	kMiddleType = 3
	kLastType   = 4
)

type Writer struct {
	file WritableFile
}

func (w *Writer) AddRecord(record []byte) Status {
	header := [kHeaderSize]byte{}

	for firstIter := true; true; firstIter = false {
		off := w.file.Size()
		offInBlock := int(off % kBlockSize)
		availInBlock := kBlockSize - offInBlock
		totalBytes := kHeaderSize + len(record)

		switch {
		case totalBytes <= availInBlock:
			p32 := (*uint32)(unsafe.Pointer(&header[0]))
			*p32 = crc32.ChecksumIEEE(record)

			if firstIter {
				// In most case, entire record fit into a block
				header[4] = kFullType
			} else {
				// sometimes, the last piece of a record is in a new block
				header[4] = kLastType
			}

			p16 := (*uint16)(unsafe.Pointer(&header[5]))
			*p16 = uint16(totalBytes)

			s := w.file.Append(header[:])
			if !s.Ok() {
				return s
			}

			return w.file.Append(record)

		case availInBlock > kHeaderSize:
			fragment := availInBlock - kHeaderSize

			p32 := (*uint32)(unsafe.Pointer(&header[0]))
			*p32 = crc32.ChecksumIEEE(record[:fragment])

			if firstIter {
				header[4] = kFirstType
			} else {
				header[4] = kMiddleType
			}

			p16 := (*uint16)(unsafe.Pointer(&header[5]))
			*p16 = uint16(availInBlock)

			s := w.file.Append(header[:])
			if !s.Ok() {
				return s
			}

			s = w.file.Append(record[:fragment])
			if !s.Ok() {
				return s
			}

			record = record[fragment:]

		case firstIter:
			// if there is too little space in current block,
			// skip the remaining bytes and start in a new block
			s := w.file.Append(header[:availInBlock])
			if !s.Ok() {
				return s
			}

		default:
			panic("too little space left should only occur at first iter")
		}
	}

	panic("Should not reach here")
	return MakeStatusOk()
}

type Reader struct {
	file     *SequentialFile
	off      int64
	checksum bool
}

/*func (r *Reader) ReadRecord(scratch []byte) (ret []byte, ok bool) {
}*/
