package data

import (
	"github.com/dailycrypto-me/daily-evm/common"
	"github.com/dailycrypto-me/daily-evm/core/vm"
	"github.com/dailycrypto-me/daily-evm/daily/state/state_common"
)

func Parse_eth_mainnet_blocks_0_300000() (ret []struct {
	Hash         common.Hash
	StateRoot    common.Hash
	EVMBlock     vm.BlockInfo
	Transactions []vm.Transaction
	UncleBlocks  []state_common.UncleBlock
}) {
	parse_rlp_file("eth_mainnet_blocks_0_300000", &ret)
	return
}
