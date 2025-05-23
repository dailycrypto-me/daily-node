ARG BUILD_OUTPUT_DIR=cmake-docker-build-debug

#############################################
# builder image - contains all dependencies #
#############################################
FROM ubuntu:24.04@sha256:e3f92abc0967a6c19d0dfa2d55838833e947b9d74edbcb0113e48535ad4be12a as builder

# deps versions
ARG LLVM_VERSION=17

# Avoid prompts from apt
ENV DEBIAN_FRONTEND=noninteractive

# Install standard packages
RUN apt-get update && apt-get install -y --no-install-recommends \
    tzdata \
    && apt-get install -y \
    tar \
    git \
    curl \
    wget \
    python3-pip \
    lsb-release \
    software-properties-common \
    && rm -rf /var/lib/apt/lists/*

# install solc for py_test if arch is not arm64 because it is not available

# Install solc 0.8.24 as we do not support 0.8.25 yet
RUN \
if [ `arch` != "aarch64" ]; \
then  \
curl -L -o solc-0.8.25 https://github.com/ethereum/solidity/releases/download/v0.8.25/solc-static-linux \
    && chmod +x solc-0.8.25 \
    && mv solc-0.8.25 /usr/bin/solc; \
fi

# install standard tools
RUN add-apt-repository ppa:ethereum/ethereum \
    && apt-get update \
    && apt-get install -y \
    clang-format-$LLVM_VERSION \
    clang-tidy-$LLVM_VERSION \
    llvm-$LLVM_VERSION \
    golang-go \
    ca-certificates \
    libtool \
    autoconf \
    binutils \
    cmake \
    ccache \
    libgmp-dev \
    libmpfr-dev \
    libmicrohttpd-dev \
    libgoogle-perftools-dev \
    # these libs are required for arm build by go part
    libzstd-dev \
    libsnappy-dev \
    # replace this with conan dependency
    rapidjson-dev \
    && rm -rf /var/lib/apt/lists/*

ENV CXX="clang++-${LLVM_VERSION}"
ENV CC="clang-${LLVM_VERSION}"

# HACK remove this when update to conan 2.0
RUN ln -s /usr/bin/clang-${LLVM_VERSION} /usr/bin/clang
RUN ln -s /usr/bin/clang++-${LLVM_VERSION} /usr/bin/clang++

# Install conan
RUN apt-get remove -y python3-distro
RUN pip3 install conan==1.64.1 --break-system-packages

# Install conan deps
WORKDIR /opt/daily/
COPY conanfile.py .

RUN conan profile new clang --detect \
    && conan profile update settings.compiler=clang clang  \
    && conan profile update settings.compiler.version=$LLVM_VERSION clang  \
    && conan profile update settings.compiler.libcxx=libstdc++11 clang \
    && conan profile update settings.build_type=RelWithDebInfo clang \
    && conan profile update env.CC=clang-$LLVM_VERSION clang  \
    && conan profile update env.CXX=clang++-$LLVM_VERSION clang  \
    && conan install --build missing -pr=clang .

###################################################################
# Build stage - use builder image for actual build of daily node #
###################################################################
FROM builder as build

# Default output dir containing build artifacts
ARG BUILD_OUTPUT_DIR

# Build daily-node project
WORKDIR /opt/daily/
COPY . .

RUN mkdir $BUILD_OUTPUT_DIR && cd $BUILD_OUTPUT_DIR \
    && cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo \
    -DDAILY_ENABLE_LTO=OFF \
    -DDAILY_STATIC_BUILD=OFF \
    -DDAILY_GPERF=ON \
    ../

RUN cd $BUILD_OUTPUT_DIR && make -j$(nproc) all \
    # Copy CMake generated Testfile to be able to trigger ctest from bin directory
    && cp tests/CTestTestfile.cmake bin/;
    # \
    # keep only required shared libraries and final binaries
    # && find . -maxdepth 1 ! -name "lib" ! -name "bin" -exec rm -rfv {} \;

# Set LD_LIBRARY_PATH so dailyd binary finds shared libs
ENV LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/local/lib

###############################################################################
##### Daily image containing dailyd binary + dynamic libraries + config #####
###############################################################################
FROM ubuntu:24.04@sha256:e3f92abc0967a6c19d0dfa2d55838833e947b9d74edbcb0113e48535ad4be12a

# Install curl and jq
RUN apt-get update \
    && apt-get install -y \
        curl \
        jq \
        python3 \
        python3-pip \
        python3-virtualenv \
        libgoogle-perftools4t64 \
    && rm -rf /var/lib/apt/lists/*

# Install required Python packages
RUN pip3 install click eth-account eth-utils typing-extensions --break-system-packages

ARG BUILD_OUTPUT_DIR
WORKDIR /root/.daily

# Copy required binaries
COPY --from=build /opt/daily/$BUILD_OUTPUT_DIR/bin/dailyd /usr/local/bin/dailyd
COPY --from=build /opt/daily/$BUILD_OUTPUT_DIR/bin/daily-bootnode /usr/local/bin/daily-bootnode
COPY --from=build /opt/daily/$BUILD_OUTPUT_DIR/lib/*.so* /usr/local/lib/

# Copy scripts
COPY scripts/daily-sign.py /usr/local/bin/daily-sign

# Set LD_LIBRARY_PATH so dailyd binary finds shared libs
ENV LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/local/lib

COPY docker-entrypoint.sh /entrypoint.sh

ENTRYPOINT ["/entrypoint.sh"]
