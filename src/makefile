# Tools
CC = gcc
CFLAGS = -fPIC -Wall -Wextra -Werror -g
LDFLAGS = -shared
RM = rm -f

# Locations
BINDIR = ../bin
LIBDIR = ../lib
OBJDIR = ../obj

# Header file dependencies
-include ../obj/*.d

# Build targets

$(OBJDIR)/%.o: %.c
	$(CC) $(CFLAGS) -c -MD -o $@ $<

../lib/libstack.so: ../obj/stack.o
	$(CC) $(LDFLAGS) -o $@ $<

../bin/stack_cmd: ../obj/stack_cmd.o ../lib/libstack.so
	$(CC) -L$(LIBDIR) -o $@ ../obj/stack_cmd.o -lstack

../bin/stack_test: ../obj/stack_test.o ../lib/libstack.so
	$(CC) -L$(LIBDIR) -o $@ ../obj/stack_test.o -lstack

.PHONY: all
all: ../bin/stack_cmd ../bin/stack_test 

.PHONY: clean
clean:
	-$(RM) ../lib/*.so ../obj/*.o ../obj/*.d

