build:
	gcc src/*.c -o q-dhcp-client
clean:
	rm ./q-dhcp-client
all: build
