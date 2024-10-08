# Daily CLI Tools

## Install

```bash
pip install -r requirements.txt
```

## Daily Local Net

You can use the `local-net` script to start multiple nodes locally using a prebuilt binary.


```
Usage: local-net start [OPTIONS] BINARY

  Start a local testnet

Options:
  --consensus-nodes INTEGER  Number of consensus nodes
  --rpc-nodes INTEGER   Number of RPC nodes
  --tps INTEGER         Number of transactions per second (if zero the faucet
                        will not start)

  --help                Show this message and exit.
```

The script can be run from anywhere but keep in mind that it will create a new directory called `local-net-data` that contains the data and config files for the nodes in the current path.

For example if you want to test the new binary on a network with 3 consensus nodes and 1 RPC node you can run the following command in the root of the current repo:

```bash
./for_devs/local-net start --consensus-nodes 3 --rpc-nodes 1 --tps 1 ./cmake-build/bin/dailyd
```

Network can be stopped, config files for nodes adjusted and redeployed... It can be used for debugging, for example:

- Deploy new network with 3 consensus nodes and 1 rpc node:
```bash
./for_devs/local-net start --consensus-nodes 3 --rpc-nodes 1 --tps 1 ./cmake-build/bin/dailyd
```

- Let it run for 1 minute so consensus nodes create a few pbft blocks. Then stop the network, increase number of rpc nodes(let's say to 5) and redeploy the networK:
```bash
./for_devs/local-net start --consensus-nodes 3 --rpc-nodes 5 --tps 1 ./cmake-build/bin/dailyd
```

New rpc nodes start syncing with original nodes as they are behind with pbft and we can debug this process. Network can be stopped at any time,
config files adjusted and redeployed with the same command.

!!! Note: For existing network only rpc nodes number can be increased, in case you want to increase consensus nodes number, network must deployed from scratch.