CC=gcc
CFLAGS=-Wall -I include
LDFLAGS=
SOURCE=option_handler.c main.c
OBJS=$(SOURCE:c=o)
EXECUTABLE=main

all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJS)
	$(CC) $(addprefix bin/, $(OBJS)) -o $@

%.o: $(addprefix src/, %.c)
	$(CC) $(CFLAGS) -c $<
	mv $@ bin/
clean:
	rm -f $(OBJS) $(TARGET)