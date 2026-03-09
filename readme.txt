Компиляция для yocto-18.0.0

. /opt/poky/2.4.4/environment-setup-armv7ahf-neon-poky-linux-gnueabi
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Debug -DBOARD_REV=0 ..
make

BOARD_REV=0 - PBX-MTV-508
BOARD_REV=1 - PN-MTV-581

сборка на x86

mkdir build
cd build
cmake -DCMAKE_PREFIX_PATH=~/Qt5.10.0/5.10.0/gcc_64/lib/cmake/ ..
make

Проверка TSL
------------

В папке tsl лежит "tsl_send.py". Запустить его следующей командой:

./tsl_send.py -a 1 --txt "Input 1.2(tsl)"  192.168.2.241 15000
./tsl_send.py -r -a 10 --txt "Input 1.2(tsl)"  192.168.2.241 15000
./tsl_send.py -g -a 10 --txt "Input 1.2(tsl)"  192.168.2.241 15000
./tsl_send.py -r -g -a 10 --txt "Input 1.2(tsl)"  192.168.2.241 15000

"-r" - цвет TALLY красный
"-g" - цвет TALLY зелёный
