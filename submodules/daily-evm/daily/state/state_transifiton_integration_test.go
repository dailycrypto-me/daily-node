package state

import (
	"math/big"
	"testing"

	"github.com/dailycrypto-me/daily-evm/daily/util/asserts"

	"github.com/dailycrypto-me/daily-evm/daily/util/tests"

	"github.com/schollz/progressbar/v3"

	"github.com/dailycrypto-me/daily-evm/daily/state/chain_config"
	"github.com/dailycrypto-me/daily-evm/daily/state/state_db_rocksdb"

	"github.com/dailycrypto-me/daily-evm/core"
	"github.com/dailycrypto-me/daily-evm/core/types"
	"github.com/dailycrypto-me/daily-evm/params"
	"github.com/dailycrypto-me/daily-evm/daily/data"
)

func TestEthMainnetSmoke(t *testing.T) {
	t.Skip() // TODO[78]
	tc := tests.NewTestCtx(t)
	defer tc.Close()
	statedb := new(state_db_rocksdb.DB).Init(state_db_rocksdb.Opts{
		Path: tc.DataDir(),
	})
	defer statedb.Close()
	blocks := data.Parse_eth_mainnet_blocks_0_300000()
	SUT := new(API).Init(
		statedb,
		func(num types.BlockNum) *big.Int {
			return new(big.Int).SetBytes(blocks[num].Hash[:])
		},
		&chain_config.ChainConfig{
			EVMChainConfig:  *params.MainnetChainConfig,
			GenesisBalances: core.MainnetGenesisBalances(),
		},
		APIOpts{
			ExpectedMaxTrxPerBlock:        300,
			MainTrieFullNodeLevelsToCache: 4,
		},
	)
	defer SUT.Close()
	st := SUT.GetStateTransition()
	asserts.EQ(statedb.GetLatestState().GetCommittedDescriptor().StateRoot.Hex(), blocks[0].StateRoot.Hex())
	progress_bar := progressbar.Default(int64(len(blocks)))
	defer progress_bar.Finish()
	for blk_num := 1; blk_num < len(blocks); blk_num++ {
		blk := blocks[blk_num]
		st.BeginBlock(&blk.EVMBlock)
		for i := range blk.Transactions {
			st.ExecuteTransaction(&blk.Transactions[i])
		}
		st.EndBlock()
		asserts.EQ(st.Commit().Hex(), blk.StateRoot.Hex())
		progress_bar.Add(1)
	}
}
