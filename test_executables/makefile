CC=gcc
CFLAGS=-Wall -Wextra -Werror -fno-stack-protector -fcf-protection=full -no-pie -mindirect-branch=thunk-extern

all: vuln

vuln: vuln.c
	$(CC) $(CFLAGS) vuln.c -o vuln

clean:
	rm -f vuln