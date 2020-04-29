#!/bin/sh
function help() {
	echo "****************************************************"
        echo "usage: ./buildall.sh 0/1/2"
        echo "0:build demo  1:buid TSIPRec2+demo  2:build all"
	echo "****************************************************"
}

buildMyCommon=0

if [ $# == 0 ]; then
	help;
	exit;
else
        if [ $# == 1 ]; then
                if [ $1 == 1 -o $1 == 2 -o $1 == 0 ]; then
			buildMyCommon=$1
		fi
	fi
fi


if [ $buildMyCommon == 2 ]; then
        cd /home/wcl/Project/bluenet/bluenet_wcl/MyCommonClass;

	make clean;make;

        cp /home/wcl/Project/bluenet/bluenet_wcl/MyCommonClass/lib64/lib* /home/wcl/Project/bluenet/bluenet_wcl/TSIPRec2/sdk/lib/;
fi

if [ $buildMyCommon == 1 -o $buildMyCommon == 2 ]; then
        cd /home/wcl/Project/bluenet/bluenet_wcl/TSIPRec2/;
	make clean;
	make;make CFG=DEBUG;
        sudo cp bin/lib* /lib/;

        cp bin/lib* /home/wcl/Project/bluenet/bluenet_wcl/demo/Linux/;
fi

if [ $buildMyCommon == 0 -o $buildMyCommon == 1 -o $buildMyCommon == 2 ]; then
        cd /home/wcl/Project/bluenet/bluenet_wcl/demo/Linux;
	make clean;make
fi
