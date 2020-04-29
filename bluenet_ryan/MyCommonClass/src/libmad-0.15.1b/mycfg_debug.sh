export TOP_DIR=/usr/local/viewmobile/libmad

./configure --prefix=$TOP_DIR \
  --libdir=$TOP_DIR/libD \
  --disable-shared \
  --enable-debugging

make clean
make
make install

