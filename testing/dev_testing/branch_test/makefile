CC=gcc
CFLAGS=-Wall -Wextra -Werror -fno-stack-protector -fcf-protection -no-pie -mshstk

all: performance_test

performance_test: performance_test.c
	$(CC) $(CFLAGS) performance_test.c -o performance_test

clean:
	rm -f performance_test