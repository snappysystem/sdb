package sdb

import (
	"strings"
)


type FileInfo struct {
	size uint32
	ref uint32
	level uint32
	minKey []byte
	maxKey []byte
}

// TODO: should we really skip @ref field?
func (fi *FileInfo) EncodeTo(scratch []byte) []byte {
	scratch = EncodeUint32(scratch, fi.size)
	scratch = EncodeUint32(scratch, fi.level)
	scratch = EncodeSlice(scratch, fi.minKey)
	scratch = EncodeSlice(scratch, fi.maxKey)
	return scratch
}

// decode from a byte buffer. Return the remaining slice. If the buffer
// cannot be decoded, return the original buffer
func (fi *FileInfo) DecodeFrom(buffer []byte) (res []byte) {
	fi.size,res = DecodeUint32(buffer)
	if len(res) == len(buffer) {
		return
	}

	oldLen := len(res)
	fi.level,res = DecodeUint32(res)
	if len(res) == oldLen {
		res = buffer
		return
	}

	oldLen = len(res)
	fi.minKey,res = DecodeSlice(res)
	if len(res) == oldLen {
		res = buffer
		return
	}

	oldLen = len(res)
	fi.maxKey,res = DecodeSlice(res)
	if len(res) == oldLen {
		res = buffer
		return
	}

	return
}

type Version struct {
	lastSequence uint64
	logFiles []uint64
	levels [][]uint64
	prev *Version
	next *Version
}

type VersionEdit struct {
	adds map[uint64]FileInfo
	removes []uint64
	lastSequence uint64
	nextFileNumber uint64
}

func (edit *VersionEdit) AddFile(fileNumber uint64, info *FileInfo) {
	edit.adds[fileNumber] = *info
}

func (edit *VersionEdit) RemoveFile(fileNumber uint64) {
	edit.removes = append(edit.removes, fileNumber)
}

func (edit *VersionEdit) SetLastSequence(seq uint64) {
	edit.lastSequence = seq
}

func (edit *VersionEdit) SetNextFileNumber(fileNumber uint64) {
	edit.nextFileNumber = fileNumber
}

func (edit *VersionEdit) EncodeTo(scratch []byte) []byte {
	// encode map
	{
		num := len(edit.adds)
		scratch = EncodeUint32(scratch, uint32(num))

		for k,v := range edit.adds {
			scratch = EncodeUint64(scratch, k)
			scratch = (&v).EncodeTo(scratch)
		}
	}

	// encode removals
	{
		num := len(edit.removes)
		scratch = EncodeUint32(scratch, uint32(num))

		for _,v := range edit.removes {
			scratch = EncodeUint64(scratch, v)
		}
	}

	scratch = EncodeUint64(scratch, edit.lastSequence)
	scratch = EncodeUint64(scratch, edit.nextFileNumber)

	return scratch
}

// decode an edit from a binary buffer, returns the remaining
// buffer after decoding. If the buffer is malformed and nothing
// has been decoded, return false as second return value
func (edit *VersionEdit) DecodeFrom(buffer []byte) (ret []byte, ok bool) {
	var remaining []byte

	// decode map
	{
		num,remaining := DecodeUint32(buffer)
		if len(remaining) == len(buffer) {
			return
		}

		for i := uint32(0); i < num; i++ {
			key,result := DecodeUint64(remaining)
			if len(result) == len(remaining) {
				return
			}

			fi := FileInfo{}
			result2 := fi.DecodeFrom(result)
			if len(result2) == len(result) {
				return
			}

			edit.adds[key] = fi
			remaining = result2
		}
	}

	// decode removal
	{
		oldLen := len(remaining)
		num,remaining := DecodeUint32(remaining)
		if len(remaining) == oldLen {
			return
		}

		for i := uint32(0); i < num; i++ {
			key,result := DecodeUint64(remaining)
			if len(result) == len(remaining) {
				return
			}

			edit.removes = append(edit.removes, key)
			remaining = result
		}
	}

	{
		var result []byte
		edit.lastSequence,result = DecodeUint64(remaining)
		if len(result) == len(remaining) {
			return
		}

		remaining = result
	}

	{
		var result []byte
		edit.nextFileNumber,result = DecodeUint64(remaining)
		if len(result) == len(remaining) {
			return
		}

		ret,ok = result,true
	}

	return
}

type VersionSet struct {
	name string
	lastSequence uint64
	nextFileNumber uint64
	current *Version
	base *Version
	fileMap map[uint64]FileInfo
	env Env
	log SequentialFile
}


func MakeVersionSet(name string, env Env) *VersionSet {
	ret := &VersionSet{}

	ret.base = &Version{}
	ret.base.prev, ret.base.next = ret.base, ret.base

	ret.current = ret.base

	ret.name = name
	ret.env = env

	return ret
}

func (a *VersionSet) AddVersion(b *Version) {
	b.next = a.current
	b.prev = a.current.prev

	a.current.prev = b
	b.prev.next = b

	a.current = b
}

func (a *VersionSet) RemoveVersion(b *Version) {
	if b == a.base {
		panic("Cannot remove base version")
	}

	b.prev.next = b.next
	b.next.prev = b.prev
}

/*func (a *VersionSet) LogAndApply(e *VersionEdit) Status {
}*/

func (a *VersionSet) Recover() Status {
	manifest := strings.Join([]string{a.name, "manifest"}, "/")
	if !a.env.FileExists(manifest) {
		return MakeStatusCorruption("")
	}

	fileSize,status := a.env.GetFileSize(manifest)
	if !status.Ok() {
		return status
	}

	file,status2 := a.env.NewSequentialFile(manifest)
	if !status2.Ok() {
		return status2
	}

	defer file.Close()

	data := make([]byte, fileSize)
	res,status3 := file.Read(data)
	if !status3.Ok() {
		return status3
	}
	if len(res) != len(data) {
		return MakeStatusCorruption("")
	}

	versionLogName := string(res)
	return a.recoverFromLogFile(versionLogName)
}

func (a *VersionSet) recoverFromLogFile(name string) Status {
	logFile,status := a.env.NewSequentialFile(name)
	if !status.Ok() {
		return status
	}

	defer logFile.Close()

	/*edit := VersionEdit{}
	buffer := [4096]byte{}
	reader := Reader{logFile, 0, true}

	for true {
		record,result := reader.ReadRecord(buffer[:])
		switch result {
		case ReadStatusOk:
		case ReadStatusEOF:
		case ReadStatusCorruption:
		default:
			panic("unexpected result")
		}
	}*/

	panic("should not reach here")
	return MakeStatusCorruption("")
}
