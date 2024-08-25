package tests

import (
	"encoding/binary"
	"os"
	"runtime"
	"testing"

	"github.com/dailycrypto-me/daily-evm/daily/util/asserts"

	"github.com/stretchr/testify/assert"

	"github.com/dailycrypto-me/daily-evm/common"

	"github.com/dailycrypto-me/daily-evm/daily/util/bin"
	"github.com/dailycrypto-me/daily-evm/daily/util/files"
	"github.com/dailycrypto-me/daily-evm/daily/util/keccak256"
)

type TestCtx struct {
	*testing.T
	Assert   assert.Assertions
	data_dir string
}

func NewTestCtx(t *testing.T) (ret TestCtx) {
	ret.T = t
	ret.Assert = *assert.New(t)
	return
}

func (self *TestCtx) Close() {
	if len(self.data_dir) != 0 {
		files.RemoveAll(self.data_dir)
	}
}

func (self *TestCtx) DataDir() string {
	if len(self.data_dir) != 0 {
		return self.data_dir
	}
	_, test_file_path, _, _ := runtime.Caller(1)
	h := keccak256.HashAndReturnByValue(bin.BytesView(test_file_path), bin.BytesView(self.Name()))
	self.data_dir = files.CreateDirectoriesClean(os.TempDir(), h.Hex())
	return self.data_dir
}

func Addr(i uint64) (ret common.Address) {
	asserts.Holds(i > 0)
	binary.BigEndian.PutUint64(ret[:], uint64(i))
	return
}

func AddrP(i uint64) *common.Address {
	ret := Addr(i)
	return &ret
}

func Noop(...interface{}) {}
