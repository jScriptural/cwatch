CC := cc

CFLAGS := -Wall -pedantic -Werror
SRC := cwatch.c utils.c

OBJ := $(SRC:.c=.o)

EXEC := cwatch

$(EXEC) : $(OBJ)
	$(CC)  $(CFLAGS) $(OBJ) -o $(EXEC)


clean :
	rm $(OBJ)

