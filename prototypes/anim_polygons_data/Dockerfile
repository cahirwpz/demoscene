FROM debian:bullseye-backports

WORKDIR /root

RUN apt-get update && apt-get upgrade -y
RUN apt-get install -y --no-install-recommends \
    build-essential \
    python3 curl git \
    ca-certificates \
    python3-pip xauth \
    libopenexr-dev \
    bzip2 \
    zlib1g-dev \
    libxmu-dev \
    libxi-dev \
    libxxf86vm-dev \
    libfontconfig1 \
    libxrender1 \
    libgl1-mesa-glx \
    xz-utils \
    xvfb

RUN update-ca-certificates
RUN pip3 install pillow

# Download and install Blender
RUN curl -o blender-3.1.0-linux-x64.tar.xz https://mirror.clarkson.edu/blender/release/Blender3.1/blender-3.1.0-linux-x64.tar.xz \
	&& tar -xvf blender-3.1.0-linux-x64.tar.xz --strip-components=1 -C /bin \
	&& rm -rf blender-3.1.0-linux-x64.tar.xz \
	&& rm -rf blender-3.1.0-linux-x64

# install rust
RUN curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | bash -s -- -y
ENV PATH="/root/.cargo/bin:${PATH}"

# install vtracer
RUN git clone https://github.com/visioncortex/vtracer.git
RUN cd vtracer/cmdapp && cargo build --release && cd ../..
ENV PATH="/root/vtracer/target/release:${PATH}"

# Create docker-files directory 
COPY docker-files/* /root/
RUN make
