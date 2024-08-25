package state_evm

import (
	"math/big"

	"github.com/dailycrypto-me/daily-evm/daily/state/state_db"

	"github.com/dailycrypto-me/daily-evm/daily/util/bigutil"

	"github.com/dailycrypto-me/daily-evm/common"
)

type AccountChange struct {
	state_db.Account
	StorageDirty    EVMStorage
	RawStorageDirty RawStorage
	Code            []byte
	CodeDirty       bool
}
type EVMStorage = map[bigutil.UnsignedStr]*big.Int
type RawStorage = map[common.Hash][]byte

type Input interface {
	GetCode(*common.Hash) []byte
	GetAccount(addr *common.Address, cb func(state_db.Account))
	GetAccountStorage(*common.Address, *common.Hash, func([]byte))
}
type Output interface {
	StartMutation(*common.Address) AccountMutation
	Delete(*common.Address)
}
type AccountMutation interface {
	Update(AccountChange)
	Commit()
}
