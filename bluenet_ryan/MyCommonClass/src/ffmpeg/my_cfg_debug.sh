export TOP_DIR=/cyj/cyj/MyCommonClass

./configure --enable-gpl \
	--enable-version3 \
	--enable-nonfree \
	--disable-ffmpeg\
	--disable-ffplay \
	--disable-ffserver \
	--enable-memalign-hack \
	--enable-debug=3 \
	--disable-stripping \
	--disable-ffprobe \
	--prefix=/usr/local/viewmobile/ffmpeg \
	--libdir=/usr/local/viewmobile/ffmpeg/libD \
	--enable-runtime-cpudetect \
	--enable-hardcoded-tables \
	--enable-postproc 

make clean

make 

make install

