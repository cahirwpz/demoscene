FROM debian:bookworm-backports

WORKDIR /root

RUN apt-get update && apt-get upgrade -y
RUN apt-get install -y --no-install-recommends \
  build-essential \
  curl \
  git \
  ca-certificates \
  imagemagick \
  python3 \
  xauth \
  libopenexr-dev \
  bzip2 \
  zlib1g-dev \
  libxmu-dev \
  libxi-dev \
  libxxf86vm-dev \
  libfontconfig1 \
  libxkbcommon0 \
  libxrender1 \
  libgl1-mesa-glx \
  xz-utils \
  xvfb

RUN update-ca-certificates

# Download and install Blender
RUN curl -o blender-4.2.0-linux-x64.tar.xz https://mirrors.dotsrc.org/blender/release/Blender4.2/blender-4.2.0-linux-x64.tar.xz \
  && tar -xvf blender-4.2.0-linux-x64.tar.xz --strip-components=1 -C /bin \
  && rm -rf blender-4.2.0-linux-x64.tar.xz \
  && rm -rf blender-4.2.0-linux-x64

# install rust
RUN curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | bash -s -- -y
ENV PATH="/root/.cargo/bin:${PATH}"

# install vtracer
RUN git clone https://github.com/visioncortex/vtracer.git
RUN cd vtracer/cmdapp && cargo build --release && cd ../..
ENV PATH="/root/vtracer/target/release:${PATH}"
