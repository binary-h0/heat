CC=gcc
CFLAGS=-I include -lrt
LDFLAGS=
SOURCE=option_handler.c sentinel.c heat.c circular_linked_list.c
OBJS=$(SOURCE:c=o)
EXECUTABLE=heat

all: $(OBJS)

# all: $(EXECUTABLE)

# $(EXECUTABLE): $(OBJS)
# 	$(CC) $(addprefix bin/, $(OBJS)) -o $@

heat: heat.o option_handler.o
	$(CC) bin/heat.o bin/option_handler.o -o $@

sentinel: sentinel.o circular_linked_list.o
	$(CC) bin/sentinel.o bin/circular_linked_list.o -o $@
	mv $@ bin/

%.o: $(addprefix src/, %.c)
	$(CC) $(CFLAGS) -c $<
	mv $@ bin/


clean:
	rm -f $(addprefix bin/, $(OBJS))