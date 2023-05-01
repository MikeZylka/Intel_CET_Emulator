CC=gcc
CFLAGS=-Wall -Wextra -fno-stack-protector -fcf-protection -no-pie -mshstk

all: vuln_JOP

vuln: vuln_JOP.c
	$(CC) $(CFLAGS) vuln_JOP.c -o vuln_JOP

clean:
	rm -f vuln_JOP