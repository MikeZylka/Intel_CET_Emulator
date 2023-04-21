CC = icx-cc
CFLAGS = -fcf-protection=full -march=native -fstack-protector-strong

sum: sum.c
	$(CC) $(CFLAGS) -o sum sum.c

clean:
	rm -f sum