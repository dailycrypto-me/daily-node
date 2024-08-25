package state_common

import (
	"github.com/dailycrypto-me/daily-evm/common"
	"github.com/dailycrypto-me/daily-evm/consensus/ethash"
	"github.com/dailycrypto-me/daily-evm/crypto"
	"github.com/dailycrypto-me/daily-evm/rlp"
)

type UncleBlock = ethash.BlockNumAndCoinbase

var EmptyRLPListHash = func() common.Hash {
	return crypto.Keccak256Hash(rlp.MustEncodeToBytes([]byte(nil)))
}()

func IsEmptyStateRoot(state_root *common.Hash) bool {
	return state_root == nil || *state_root == EmptyRLPListHash || *state_root == common.ZeroHash
}
