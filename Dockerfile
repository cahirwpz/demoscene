# To build and publish image run following commands:
# > docker build -t cahirwpz/demoscene:latest .
# > docker login
# > docker push cahirwpz/demoscene:latest

FROM debian:bullseye-backports

WORKDIR /root

ADD http://circleci.com/api/v1/project/cahirwpz/demoscene-toolchain/latest/artifacts/0/demoscene-toolchain.tar.gz \
    demoscene-toolchain.tar.gz
ADD https://packagecloud.io/install/repositories/github/git-lfs/script.deb.sh \
    script.deb.sh
RUN apt-get -q update && apt-get upgrade -y
RUN apt-get install -y --no-install-recommends gnupg && bash script.deb.sh
RUN apt-get install -y --no-install-recommends \
            universal-ctags cscope git-lfs optipng gcc g++ make golang-1.17 \
            python3 python3-pip python3-dev socat tmux
COPY requirements.txt .
RUN pip3 install setuptools wheel && pip3 install -r requirements.txt
RUN tar -C / -xvzf demoscene-toolchain.tar.gz && rm demoscene-toolchain.tar.gz
