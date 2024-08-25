# Introducing Daily

Daily is a Practical Byzantine Fault Tolerance blockchain.


# Whitepaper
You can read the Daily Whitepaper at https://www.dailycrypto.me/whitepaper.


# Quickstart
Just want to get up and running quickly? We have pre-built docker images for your convenience.
More details are in our [quickstart guide](doc/quickstart_guide.md).


# Downloading
There are 2 options how to run the latest version of daily-node:

### Docker image
Download and run daily docker image with pre-installed dailyd binary [here](https://hub.docker.com/r/daily/daily-node).

### Ubuntu binary
Download and run statically linked dailyd binary [here](https://github.com/dailycrypto-me/daily-node/releases).


# Building
If you would like to build from source, we do have [build instructions](doc/building.md) for Linux (Ubuntu LTS) and macOS.


# Running

### Inside docker image
    dailyd --conf_daily /etc/dailyd/dailyd.conf

### Pre-built binary or manual build:
    ./dailyd --conf_daily /path/to/config/file


# Contributing
Want to contribute to Daily repository ? We in Daily highly appreciate community work so if you feel like you want to
participate you are more than welcome. You can start by reading [contributing tutorial](doc/contributing.md).


# Useful doc
- [Git practices](doc/git_practices.md)
- [Coding practices](doc/coding_practices.md)
- [EVM incompatibilities](doc/evm_incompatibilities.md)
