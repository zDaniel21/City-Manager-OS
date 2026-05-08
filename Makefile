CC      = gcc
CFLAGS  = -Wall -Wextra -g -Isrc
SRCDIR  = src

# city_manager
CM_TARGET = city_manager
CM_SRCS   = $(SRCDIR)/main.c \
             $(SRCDIR)/report.c \
             $(SRCDIR)/district.c \
             $(SRCDIR)/permissions.c \
             $(SRCDIR)/log.c \
             $(SRCDIR)/filter.c \
             $(SRCDIR)/monitor.c
CM_OBJS   = $(CM_SRCS:.c=.o)

# monitor_reports
MON_TARGET = monitor_reports
MON_SRCS   = $(SRCDIR)/monitor_main.c \
              $(SRCDIR)/monitor.c
MON_OBJS   = $(MON_SRCS:.c=.o)

all: $(CM_TARGET) $(MON_TARGET)

$(CM_TARGET): $(CM_OBJS)
	$(CC) $(CFLAGS) -o $@ $^

$(MON_TARGET): $(MON_OBJS)
	$(CC) $(CFLAGS) -o $@ $^

$(SRCDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -c -o $@ $

clean:
	rm -f $(CM_OBJS) $(MON_OBJS) $(CM_TARGET) $(MON_TARGET)

.PHONY: all clean
