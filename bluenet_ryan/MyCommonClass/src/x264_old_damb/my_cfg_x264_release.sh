export TOP_DIR=/usr/local/viewmobile/x264
export DESTDIR=

rm -f libx264.a
./configure --enable-static --enable-debug \
  --prefix=$TOP_DIR\
  --enable-strip
make clean

make

make install
