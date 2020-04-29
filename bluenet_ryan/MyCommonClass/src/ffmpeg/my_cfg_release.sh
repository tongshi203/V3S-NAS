./configure --enable-gpl \
	--enable-version3 \
	--enable-nonfree \
	--disable-ffmpeg\
	--disable-ffplay \
	--disable-ffserver \
	--enable-memalign-hack \
  	--disable-debug \
	--disable-ffprobe \
	--prefix=/usr/local/viewmobile/ffmpeg \
	--enable-runtime-cpudetect \
	--enable-hardcoded-tables \
	--enable-postproc

make clean

make 

make install

