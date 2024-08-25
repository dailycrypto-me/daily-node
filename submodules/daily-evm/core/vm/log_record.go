package vm

import "github.com/dailycrypto-me/daily-evm/common"

type LogRecord struct {
	Address common.Address
	Topics  []common.Hash
	Data    []byte
}
