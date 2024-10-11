CROSS=                         # cross compile
CC=$(CROSS)gcc
EXE_SERVER=server             # server executable
EXE_CLIENT=client             # client executable
DEFINES=                      # -DDEBUG
LDFLAGS=-lpthread             # library
CFLAGS=-I./include $(DEFINES) # include option 고정

C_SRC = $(wildcard *.c)       # wildcard
OBJ = $(C_SRC:.c=.o)

all: $(EXE_SERVER) $(EXE_CLIENT)
	@echo completed!!!

$(EXE_SERVER): server.o utils.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(EXE_CLIENT): client.o utils.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

clean:  
	rm -f *.o $(EXE_SERVER) $(EXE_CLIENT)
	rm -f *.swp
good:
	@echo good

