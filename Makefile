CC=gcc
CFLAGS=-I include -lrt
LDFLAGS=
SOURCE=option_handler.c sentinel.c heat.c
OBJS=$(SOURCE:c=o)
EXECUTABLE=heat

all: $(OBJS)

# all: $(EXECUTABLE)

# $(EXECUTABLE): $(OBJS)
# 	$(CC) $(addprefix bin/, $(OBJS)) -o $@

heat: heat.o option_handler.o
	$(CC) bin/heat.o bin/option_handler.o -o $@

sentinel: sentinel.o
	$(CC) bin/sentinel.o -o $@
	mv $@ bin/

%.o: $(addprefix src/, %.c)
	$(CC) $(CFLAGS) -c $<
	mv $@ bin/


clean:
	rm -f $(addprefix bin/, $(OBJS))