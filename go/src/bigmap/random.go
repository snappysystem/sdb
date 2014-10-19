package bigmap

import (
	"time"
  "sort"
	"math/rand"
)

var levels = [...]int{
  8, 64, 512, 4096, 32768, 262144, 2097152, 16777216, 134217728,
}

const maxLevel = len(levels)

type randomGenerator struct {
	r *rand.Rand
}


func makeRandomGenerator() *randomGenerator {
  x := randomGenerator{}
  seed := time.Now().UTC().UnixNano()
	x.r = rand.New(rand.NewSource(seed))
  return &x
}

func (x *randomGenerator) get() int {
  max := levels[maxLevel - 1]
  val := x.r.Intn(max)
	i := sort.Search(maxLevel, func(i int) bool {
    return levels[i] >= val
	})
	return (maxLevel - i - 1)
}
