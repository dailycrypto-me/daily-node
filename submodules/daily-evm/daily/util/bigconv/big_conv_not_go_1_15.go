// +build !go1.15

package bigconv

import (
	"math/big"
	"unsafe"

	"github.com/dailycrypto-me/daily-evm/common"
)

type BigConv struct {
	buf common.Hash
}

func (self *BigConv) ToHash(b *big.Int) (ret *common.Hash) {
	self.buf = common.Hash{}
	return self.buf.SetBytes(b.Bytes())
}

func (self *BigConv) ToAddr(b *big.Int) (ret *common.Address) {
	self.buf = common.Hash{}
	return (*common.Address)(unsafe.Pointer(&self.buf)).SetBytes(b.Bytes())
}
