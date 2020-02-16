# To build and publish image run following commands:
# > docker build -t cahirwpz/demoscene:latest .
# > docker login
# > docker push cahirwpz/demoscene:latest

FROM debian:buster

WORKDIR /root

ADD http://circleci.com/api/v1/project/cahirwpz/demoscene-toolchain/latest/artifacts/0/demoscene-toolchain.tar.gz \
    demoscene-toolchain.tar.gz
ADD https://packagecloud.io/install/repositories/github/git-lfs/script.deb.sh \
    script.deb.sh
RUN apt-get -q update && apt-get upgrade -y
RUN apt-get install -y --no-install-recommends gnupg && bash script.deb.sh
RUN apt-get install -y --no-install-recommends \
            ctags cscope git-lfs optipng gcc g++ make libc6-i386 \
            python3 python3-setuptools python3-prompt-toolkit \
            python3-pil python3-pip python3-wheel python3-dev
RUN tar -C / -xvzf demoscene-toolchain.tar.gz && rm demoscene-toolchain.tar.gz
RUN pip3 install pycodestyle zopflipy
