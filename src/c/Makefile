CC = gcc
CFLAGS = -Wall -O2
TARGET = iopp
SRC = iopp-c.c

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

install: $(TARGET)
	install -m 755 $(TARGET) /usr/local/bin/

uninstall:
	rm -f /usr/local/bin/$(TARGET)

clean:
	rm -f $(TARGET) *.o

.PHONY: install uninstall clean
