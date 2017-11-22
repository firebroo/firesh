TARGET = firesh

SRCS = firesh.c prompt.c

OBJS = $(SRCS:.c=.o)

INSTALLDIR = /bin/

CC = cc
CFLAGS = -O2 -lreadline

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

install:
	mv $(TARGET) $(INSTALLDIR)

clean:
	rm -rf $(TARGET) $(OBJS)

depclean:
	sudo rm -rf $(TARGET) $(OBJS) $(INSTALLDIR)$(TARGET)


%.o:%.c
	$(CC) $(CFLAGS) -o $@ -c $<
