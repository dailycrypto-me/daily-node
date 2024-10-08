Quickstart Guide
-------------------

### Pre-requisites

* A sufficient-sized and secure machine (Physical, VM or in the Cloud) with Ubuntu pre-installed on it
* System requirements:
    * CPU:  min. 2 cores
    * RAM: min. 4 GB
    * Disk space –  > 50GB (blockchain and OS)
    * Disk Partitions  
      Recommended to create dedicated partition for storing blockchain database (data); sizing to be adjusted according to the size of the actual blockchain.

      for other partitions, setup and sizing, follow the best practice for ubuntu installation

    * Network – the network should be reliable and fast (high speed connection with low latency), and the machine accessible via public IP
    * 2 x 1Gb >= Network adapter (second Network Interface Card (NIC) as a standby or failover interface in case the primary NIC fails. Can also be used for load balancing)

    * The following are the port(s) to be opened for incoming traffic or connections:
  #### MANDATORY PORT
  TODO: add

  #### OPTIONAL PORTS   
  TODO: add

### Config
You can see example config [here](doc/example_config.json)

#### Param1
#### Param2
#### ...
TODO: add explanation of all config parameters


# Daily docker image

There are 2 daily docker images:

### daily-builder:latest 
Contains all dependencies required for building daily-node project, you can create local daily-builder image 
by running this command in root directory:

    DOCKER_BUILDKIT=1 docker build --progress=plain --target builder -t daily-builder:latest .
    

### daily-node:latest
Contains final dailyd binary + config. All dependencies are linked statically. To create daily-node image, run: 

    DOCKER_BUILDKIT=1 docker build --progress=plain -t daily-node:latest .
