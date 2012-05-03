GCC=gcc
CCFLAGS=-Wall -pedantic -std=c99
LDFLAGS=-lavcodec -lavutil -lavformat -lswscale
PROG=vcat
MODS=vcat.o

all: $(PROG)

%.o: %.c
	$(GCC) -c $(CCFLAGS) -o $@ $<

$(PROG): $(MODS)
	$(GCC) -o $@ $< $(LDFLAGS)

clean:
	-rm -f $(PROG) $(MODS)



