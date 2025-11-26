FROM ubuntu:24.04

# WARN: STILL UNTESTED

ENV DEBIAN_FRONTEND=noninteractive

# Install dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    wget \
    curl \
    python3 \
    python3-pip \
    pkg-config \
    libssl-dev \
    libomp-dev \
    clang \
    && rm -rf /var/lib/apt/lists/*


COPY pyproject.toml ./
COPY uv.lock ./

RUN pip install uv

RUN uv pip install --system --no-cache .

# TODO: setting a copy in docker: binary, src, weights-sst2
COPY . .

# Build and install OpenFHE
RUN mkdir -p /opt && cd /opt && \
    git clone https://github.com/openfheorg/openfhe-development.git && \
    cd openfhe-development && \
    mkdir build && cd build && \
    cmake -DCMAKE_INSTALL_PREFIX=/usr/local .. && \
    make -j$(nproc) && \
    make install && \
    ldconfig

# Set environment variables
# ENV LD_LIBRARY_PATH=/usr/local/lib:$LD_LIBRARY_PATH
ENV OpenFHE_DIR=/usr/local/lib/cmake/OpenFHE

# Create working directory
WORKDIR /workspace

# Copy project files
COPY . /workspace/

# Build project
RUN mkdir -p build && cd build && \
    cmake .. && \
    make

CMD ["/bin/bash"]
