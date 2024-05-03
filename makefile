CC := cc

CFLAGS := -Wall -pedantic
SRC := cwatch.c

OBJ := $(SRC:.c=.o)

EXEC := cwatch

$(EXEC) : $(OBJ)
	$(CC)  $(CFLAGS) $(OBJ) -o $(EXEC)




clean :
	rm $(OBJ)

