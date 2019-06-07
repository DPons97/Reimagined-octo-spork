#!/usr/bin/env bash

if [ -n "$(ls -A opencv-3.4.5/build)" ];
then
    ## Using cached version of opencv. Try build
    cd opencv-3.4.5/build/;
else
    ## No cached build present
    rm -r opencv-3.4.5;
    rm -r opencv_contrib-3.4.5;
    wget https://github.com/opencv/opencv_contrib/archive/3.4.5.tar.gz;
    tar -xvzf 3.4.5.tar.gz;
    rm 3.4.5.tar.gz;
    wget https://github.com/opencv/opencv/archive/3.4.5.tar.gz;
    tar -xvzf 3.4.5.tar.gz;
    rm 3.4.5.tar.gz;
    cd opencv-3.4.5;
    mkdir build;
    cd build;
fi
cmake -D CMAKE_BUILD_TYPE=Release \
          -D CMAKE_INSTALL_PREFIX=/usr/local \
          -DOPENCV_EXTRA_MODULES_PATH=../../opencv_contrib-3.4.5/modules/ \
          -DBUILD_opencv_dnn=ON \
          -DBUILD_EXAMPLES=OFF \
          -DCPU_DISPATCH= \
          -DBUILD_TESTS=OFF \
          -DBUILD_JAVA=OFF \
          -DBUILD_PERF_TESTS=OFF ..
make -j8;
