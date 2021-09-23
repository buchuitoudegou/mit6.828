# JOS

## preparing tool on Linux
install gcc
```
sudo apt-get install -y build-essential gdb
```
install 32-bit library: please note that, if there are multiple versions of `gcc` in the system, the `multilib` module is not general for all the compilers. For example, the `gcc-4.8-multilib` is incompatible with the `gnu-gcc-10`.
```
sudo apt-get install gcc-[VERSION]-multilib
```

## QEMU Dependencies
```
sudo apt-get install libsdl1.2-dev libtool-bin libglib2.0-dev libz-dev libpixman-1-dev
```