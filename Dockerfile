FROM cahirwpz/amigaos-cross-toolchain:latest

WORKDIR /root

ADD https://packagecloud.io/install/repositories/github/git-lfs/script.deb.sh \
    script.deb.sh
RUN bash script.deb.sh
RUN apt-get install -y --no-install-recommends --allow-unauthenticated \
            python3 ctags cscope git-lfs
RUN git clone https://github.com/cahirwpz/demoscene.git && \
    cd demoscene/a500 && make
