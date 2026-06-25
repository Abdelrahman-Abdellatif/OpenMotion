CC ?= gcc
CFLAGS ?= -Wall -Wextra
TARGET = openmotion-host
SRC = main.c

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

clean:
	rm -f $(TARGET)
