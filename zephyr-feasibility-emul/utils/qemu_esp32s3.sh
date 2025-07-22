dd if=/dev/zero of=build/zephyr/zephyr_4mb.bin bs=1M count=4
dd if=build/zephyr/zephyr.bin of=build/zephyr/zephyr_4mb.bin conv=notrunc
qemu-system-xtensa -nographic -machine esp32s3 -drive file=build/zephyr/zephyr_4mb.bin,if=mtd,format=raw
