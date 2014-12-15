# Master makefile for https://github.com/mjbalint/stack.
#
# Tested with Linux Ubuntu 14.04.
# Run 'make' from the same directory as this makefile for best results.
#
# Copyright (c) 2014 by Matthew Balint
# GNU Lesser Public License


# Tools
CC = gcc
CFLAGS = -fPIC -Wall -Wextra -Werror -g
LDFLAGS = -shared
RM = rm -f

# Locations
BINDIR = bin
LIBDIR = lib
OBJDIR = obj
SRCDIR = src

# Source to header file dependencies
# The .d files are generated as a side effect of building object files,
#  from the "-MD" option.  
-include obj/*.d

# Build targets
$(OBJDIR)/%.o: $(SRCDIR)/%.c
	@echo make: Build object $@
	$(CC) $(CFLAGS) -c -MD -o $@ $<
	@echo make: Done object $@

$(LIBDIR)/libstack.so: $(OBJDIR)/stack.o
	@echo make: Build library $@
	$(CC) $(LDFLAGS) -o $@ $<
	@echo make: Done library $@

$(BINDIR)/%: $(OBJDIR)/%.o $(LIBDIR)/libstack.so
	@echo make: Build executable $@
	$(CC) -L$(LIBDIR) -o $@ $< -lstack
	@echo make: Done executable $@

# Master targets
.PHONY: all
all: $(BINDIR)/stack_cmd $(BINDIR)/stack_test

.PHONY: clean
clean:
	@echo make: Clean object files
	-$(RM) $(OBJDIR)/*.[od]
	@echo make: Clean libraries
	-$(RM) $(LIBDIR)/*.so
	@echo make: Clean executables
	-$(RM) $(BINDIR)/stack_cmd $(BINDIR)/stack_test

