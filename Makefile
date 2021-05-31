CC = gcc
P = -pthread
BUILD_DIR := ./bin
SRC_DIR := ./source

compile:
	mkdir -p $(BUILD_DIR)
	echo "## Binaries" > $(BUILD_DIR)/README.md
	$(CC)  $(SRC_DIR)/client.c -o bin/client  -pthread
	$(CC)  $(SRC_DIR)/server.c -o bin/server
	$(CC)  $(SRC_DIR)/server_async.c -o bin/server_async
	$(CC)  $(SRC_DIR)/server_fork.c -o bin/server_fork
	$(CC)  $(SRC_DIR)/client_fork.c -o bin/client_fork
	$(CC)  $(SRC_DIR)/udpclient.c -o bin/udpclient
	$(CC)  $(SRC_DIR)/client_concurrent_udp.c -o bin/client_concurrent_udp
	$(CC)  $(SRC_DIR)/udpserver.c -o bin/udpserver
	$(CC)  $(SRC_DIR)/server_concurrent.c -o bin/server_concurrent -pthread
	$(CC)  $(SRC_DIR)/server_concurrent_udp.c -o bin/server_concurrent_udp -pthread

clean:
	$(RM) -r $(BUILD_DIR)
