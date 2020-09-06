all:
	arm-none-eabi-as -mthumb --fatal-warnings -o target/shellcode.o target/shellcode.S
	arm-none-eabi-objcopy -O binary target/shellcode.o target/shellcode.bin
	rm target/shellcode.o

	arm-none-eabi-as -mthumb --fatal-warnings -o target/prepare_and_jump.o target/prepare_and_jump.S
	arm-none-eabi-objcopy -O binary target/prepare_and_jump.o target/prepare_and_jump.bin
	rm target/prepare_and_jump.o