TARGET = server

OBJDIR := obj/

SRC := $(wildcard *.c)
OBJS := $(SRC:%.c=$(OBJDIR)%.o)

# Build target that depends on evey .o
$(TARGET): $(OBJS)
	gcc $^ -o $@

# Create the obj/ directory if it doesn't exist
$(OBJDIR):
	mkdir -p $(OBJDIR)

# Compile the source files into object files
$(OBJDIR)%.o: %.c | $(OBJDIR)
	gcc -c $< -o $@
