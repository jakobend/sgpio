CC=arm-linux-gnueabihf-gcc
LD=arm-linux-gnueabihf-ld

OBJECTS=sgpio.o nespi.o

nespi: $(OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)

all: nespi
clean:
	$(RM) $(OBJECTS) nespi
