CC=gcc
CFLAGS=-I include
LDFLAGS=
SOURCE=option_handler.c sentinel.c heat.c circular_linked_list.c signal_mapper.c dummy.c
OBJS=$(SOURCE:c=o)
EXECUTABLE=heat

all: heat sentinel dummy
	cp ./heat /usr/local/bin/heat
	cp ./dummy /usr/local/bin/dummy

heat: heat.o option_handler.o signal_mapper.o
	$(CC) bin/heat.o bin/option_handler.o bin/signal_mapper.o -o $@ -lrt

sentinel: sentinel.o circular_linked_list.o
	$(CC) bin/sentinel.o bin/circular_linked_list.o -o $@ -lrt
	mv $@ bin/

dummy: dummy.o
	$(CC) bin/dummy.o -o $@ -lrt

%.o: $(addprefix src/, %.c)
	$(CC) $(CFLAGS) -D_GNU_SOURCE -c $<
	mv $@ bin/


clean:
	rm -f $(addprefix bin/, $(OBJS))