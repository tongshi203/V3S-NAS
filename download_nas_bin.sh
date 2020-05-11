#!/bin/sh

rm flashimg.bin

dd if=/dev/zero of=flashimg.bin bs=32M count=1

dd if=./u-boot/u-boot-sunxi-with-spl.bin of=flashimg.bin bs=1K conv=notrunc

dd if=./linux-zero-5.2.y/arch/arm/boot/dts/sun8i-v3s-licheepi-zero-nas.dtb of=flashimg.bin bs=1K seek=1024   conv=notrunc

dd if=./linux-zero-5.2.y/arch/arm/boot/zImage of=flashimg.bin bs=1K seek=1088  conv=notrunc

dd if=./jffs2.img of=flashimg.bin  bs=1K seek=6208  conv=notrunc

cd sunxi-tools
sudo ./sunxi-fel -p spiflash-write 0 ../flashimg.bin
