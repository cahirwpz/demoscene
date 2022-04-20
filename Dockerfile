# To build and publish image run following commands:
# > docker build -t cahirwpz/demoscene:latest .
# > docker login
# > docker push cahirwpz/demoscene:latest

FROM debian:bullseye-backports

WORKDIR /root

ADD https://github.com/cahirwpz/demoscene-toolchain/releases/download/2022-04-18/demoscene-toolchain.tar.gz \
    demoscene-toolchain.tar.gz
RUN apt-get -q update && apt-get upgrade -y
RUN apt-get install -y --no-install-recommends -t bullseye-backports \
            universal-ctags cscope git-lfs optipng gcc g++ make golang \
            python3 python3-pip python3-dev socat tmux
COPY requirements.txt .
RUN pip3 install setuptools wheel && pip3 install -r requirements.txt
RUN git lfs install
RUN tar -C / -xvzf demoscene-toolchain.tar.gz && rm demoscene-toolchain.tar.gz
