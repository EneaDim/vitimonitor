cd
sudo apt update
sudo apt install libgcrypt20-dev libglib2.0-dev libpixman-1-dev zlib1g-dev ninja-build
sudo apt upgrade
git clone --recursive https://github.com/espressif/qemu.git
cd qemu
./configure     --target-list=xtensa-softmmu,arm-softmmu,riscv32-softmmu,riscv64-softmmu     --enable-gcrypt     --enable-slirp     --enable-debug     --enable-sdl     --disable-strip --disable-user     --disable-capstone --disable-vnc     --disable-gtk
make -j 4
build/qemu-system-xtensa --version
sudo ln -s build/qemu-system-xtensa /usr/local/bin/qemu-system-xtensa
which qemu-system-xtensa
cd
cd zephyrproject/
source .venv/bin/activate
cd
cd github/introduction-to-zephyr/workspace/apps/01_blink/
export ZEPHYR_BASE=/home/eneadim/zephyrproject/zephyr
source $ZEPHYR_BASE/zephyr-env.sh
west build -p always -b esp32s3_devkitc/esp32s3/procpu -- -DDTC_OVERLAY_FILE=boards/esp32s3_devkitc.overlay
dd if=/dev/zero of=build/zephyr/zephyr_4mb.bin bs=1M count=4
dd if=build/zephyr/zephyr.bin of=build/zephyr/zephyr_4mb.bin conv=notrunc
qemu-system-xtensa -machine help
qemu-system-xtensa -machine esp32s3,help
qemu-system-xtensa -nographic -machine esp32s3 -drive file=build/zephyr/zephyr_4mb.bin,if=mtd,format=raw
