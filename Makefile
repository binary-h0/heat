CC=gcc
CFLAGS=-I include
LDFLAGS=
SOURCE=option_handler.c sentinel.c heat.c circular_linked_list.c signal_mapper.c
OBJS=$(SOURCE:c=o)
EXECUTABLE=heat

all: $(OBJS)

# all: $(EXECUTABLE)

# $(EXECUTABLE): $(OBJS)
# 	$(CC) $(addprefix bin/, $(OBJS)) -o $@

heat: heat.o option_handler.o signal_mapper.o
	$(CC) bin/heat.o bin/option_handler.o bin/signal_mapper.o -o $@ -lrt

sentinel: sentinel.o circular_linked_list.o
	$(CC) bin/sentinel.o bin/circular_linked_list.o -o $@ -lrt
	mv $@ bin/

%.o: $(addprefix src/, %.c)
	$(CC) $(CFLAGS) -D_GNU_SOURCE -c $<
	mv $@ bin/


clean:
	rm -f $(addprefix bin/, $(OBJS))