package data

import (
	"os"

	"github.com/dailycrypto-me/daily-evm/rlp"
	"github.com/dailycrypto-me/daily-evm/daily/util"
	"github.com/dailycrypto-me/daily-evm/daily/util/files"
)

var this_dir = files.ThisDirRelPath()

func parse_rlp_file(short_file_name string, out interface{}) {
	f, err := os.Open(files.Path(this_dir, short_file_name+".rlp"))
	util.PanicIfNotNil(err)
	util.PanicIfNotNil(rlp.Decode(f, out))
}
