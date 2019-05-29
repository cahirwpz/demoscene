# To build and publish image run following commands:
# > docker build -t cahirwpz/demoscene:latest .
# > docker login
# > docker push cahirwpz/demoscene:latest

FROM cahirwpz/demoscene-toolchain:latest

WORKDIR /root

ADD https://packagecloud.io/install/repositories/github/git-lfs/script.deb.sh \
    script.deb.sh
RUN apt-get install -y --no-install-recommends gnupg && bash script.deb.sh
RUN apt-get install -y --no-install-recommends \
            ctags cscope git-lfs optipng gcc g++ make \
            python3 python3-setuptools python3-prompt-toolkit \
            python3-pil python3-pip python3-wheel python3-dev
RUN pip3 install pycodestyle zopflipy
RUN git clone https://github.com/cahirwpz/amigaos-dev-toolkit.git && \
    cd amigaos-dev-toolkit && python3 setup.py install && \
    cd .. && rm -rf amigaos-dev-toolkit
