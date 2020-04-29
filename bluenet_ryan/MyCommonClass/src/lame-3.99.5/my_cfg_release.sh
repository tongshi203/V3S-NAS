./configure --prefix=/usr/local/viewmobile/libmp3lame \
  --enable-nasm \
  --disable-gtktest \
  --disable-frontend \
  --disable-decoder \
  --disable-shared

make clean

make

make install
