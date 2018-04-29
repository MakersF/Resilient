FROM resilient-ubuntu16

RUN apt-get install -y \
    gcc-7 \
    g++-7 \
    && update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-7 60 --slave /usr/bin/g++ g++ /usr/bin/g++-7