export TOP_DIR=/usr/local/viewmobile/libmpeg2

./configure --prefix=$TOP_DIR\
  --libdir=$TOP_DIR/libD \
  --enable-debug \
  --disable-shared \
  --disable-sdl

make clean
make
make install

