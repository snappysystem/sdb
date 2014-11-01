package sdb


type FileInfo struct {
	size uint32
	ref uint32
	minKey []byte
	maxKey []byte
}

type Version struct {
	maxSequence uint64
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

/*func (edit *VersionEdit) EncodeTo(scratch []byte) []byte {
}*/

type VersionSet struct {
	lastSequence uint64
	lastAvailFileNumber uint64
	current *Version
	base *Version
	fileMap map[uint64]FileInfo
}


func MakeVersionSet() *VersionSet {
	ret := &VersionSet{}

	ret.base = &Version{}
	ret.base.prev, ret.base.next = ret.base, ret.base

	ret.current = ret.base
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
