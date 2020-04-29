export TOP_DIR=/usr/local/viewmobile/libmad

./configure --prefix=$TOP_DIR \
  --disable-shared \
  --disable-debugging

make clean
make
make install

