FROM resilient-ubuntu16

RUN apt-get install -y \
    gcc-6 \
    g++-6 \
    && update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-6 60 --slave /usr/bin/g++ g++ /usr/bin/g++-6