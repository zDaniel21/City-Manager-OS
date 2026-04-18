CC      = gcc
CFLAGS  = -Wall -Wextra -g -Isrc
TARGET  = city_manager
SRCDIR  = src

SRCS    = $(SRCDIR)/main.c \
          $(SRCDIR)/report.c \
          $(SRCDIR)/district.c \
          $(SRCDIR)/permissions.c \
          $(SRCDIR)/log.c \
          $(SRCDIR)/filter.c

OBJS    = $(SRCS:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

$(SRCDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -c -o $@ $

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean