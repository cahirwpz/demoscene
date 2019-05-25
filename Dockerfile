FROM cahirwpz/amigaos-cross-toolchain:latest

WORKDIR /root

ADD https://packagecloud.io/install/repositories/github/git-lfs/script.deb.sh \
    script.deb.sh
# Temporary solution. Need to update amigaos-cross-toolchain image.
RUN apt-get -q update && apt-get upgrade -y
RUN apt-get install -y --no-install-recommends gnupg && bash script.deb.sh
RUN apt-get install -y --no-install-recommends \
            ctags cscope git-lfs optipng \
            python3 python3-setuptools python3-prompt-toolkit \
            python3-pil python3-pip python3-wheel python3-dev
RUN pip3 install zopflipy
RUN git clone https://github.com/cahirwpz/amigaos-dev-toolkit.git && \
    cd amigaos-dev-toolkit && python3 setup.py install && \
    cd .. && rm -rf amigaos-dev-toolkit
