FROM cahirwpz/amigaos-cross-toolchain:latest

WORKDIR /root

ADD https://packagecloud.io/install/repositories/github/git-lfs/script.deb.sh \
    script.deb.sh
RUN apt-get install -y --no-install-recommends gnupg && bash script.deb.sh
RUN apt-get install -y --no-install-recommends \
            ctags cscope git-lfs optipng \
            python3 python3-setuptools python3-prompt-toolkit \
            python-pil cython 
RUN git clone https://github.com/cahirwpz/amigaos-dev-toolkit.git && \
    cd amigaos-dev-toolkit && python3 setup.py install && \
    cd .. && rm -rf amigaos-dev-toolkit
