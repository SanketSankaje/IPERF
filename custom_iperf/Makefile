CC = gcc
CFLAGS = -fPIC -Wall -g
LDFLAGS = -shared

TARGET = lib_custom_iperf.so
SOURCES = support_func.c

OBJECTS = $(SOURCES:.c=.o)

all:
	make clean
	make $(TARGET)
	sudo gcc test.c -o test -L. -l_custom_iperf

$(TARGET): $(OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $^
	sudo cp ./*.so /usr/lib/

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(TARGET) $(OBJECTS)
	# sudo rm /usr/lib/lib_custom_iperf.so

