CC=gcc
CFLAGS=-Wall -Wextra -Werror -fno-stack-protector -fcf-protection -no-pie

all: vuln

vuln: vuln.c
	$(CC) $(CFLAGS) vuln.c -o vuln

clean:
	rm -f vuln