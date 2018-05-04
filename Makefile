CC=gcc
CFLAGS=-m32 -g
DriverProto: Driver_main.c DriverLibrary.o AssistantLibrary.o
	$(CC) $(CFLAGS) Driver_main.c DriverLibrary.c AssistantLibrary.c -L/home/balyo/Documents/DriverProto2/SUSIDriver -lSUSI-4.00 -lz -o DriverProto
.PHONY: clean
clean:
	rm -rf *.o
