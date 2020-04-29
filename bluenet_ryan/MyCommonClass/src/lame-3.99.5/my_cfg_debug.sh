./configure --prefix=/usr/local/viewmobile/libmp3lame \
  --libdir=/usr/local/viewmobile/libmp3lame/libD \
  --enable-nasm \
  --disable-gtktest \
  --disable-frontend \
  --enable-debug=alot \
  --disable-decoder \
  --disable-shared

make clean

make

make install
