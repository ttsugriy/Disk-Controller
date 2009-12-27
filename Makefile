SRCS = *.c
HDRS = *.h
CC = gcc

all: student

student : $(SRCS) $(HDRS)
	$(CC) $(SRCS) $(HDRS) -o $@

clean:
	rm -f student *.o
