#!/bin/bash
rm targt/payload.elf target/payload.bin;
arm-none-eabi-gcc target/payload.c -fpie -W -pedantic -Wno-long-long -Os -L. -nostdlib -emain -march=armv7-a -mthumb -o target/payload.elf;
arm-none-eabi-objcopy -O binary target/payload.elf target/payload.bin; rm target/payload.elf
HASH=`openssl sha1 -binary target/payload.bin | xxd -p`
printf "SHA1: $HASH\n";
printf "DATA: ";
printf %s `xxd -p target/payload.bin`
printf "\n"
