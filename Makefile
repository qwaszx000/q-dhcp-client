CC=gcc
CFLAGS=-O2 -Wall 
DEBUG_DIR_PATH=-D DEBUG_DIR_PATH=\"/var/log/q-dhcp-client/\"
OUTPUT_FILE=./q-dhcp-client

#tests
TEST_OUTPUT_FILE=./q-dhcp-client.test
DEVICE=enp2s0

build:
	$(CC) $(CFLAGS) $(DEBUG_DIR_PATH) src/*.c -o $(OUTPUT_FILE)


clean:
	rm $(OUTPUT_FILE)


check:
	mkdir ./tmp
	$(CC) $(CFLAGS) -D DEBUG_DIR_PATH=\"./tmp/\" -D ARP_ROUTER src/*.c -o $(TEST_OUTPUT_FILE)
	./tests.bash $(TEST_OUTPUT_FILE) $(DEVICE)
	rm $(TEST_OUTPUT_FILE)
	rm ./tmp/*.bin
	rmdir ./tmp

all: build
