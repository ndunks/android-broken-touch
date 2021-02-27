build:
	$(CC) main.c -o broken-touch-pc

clean:
	@rm -f broken-touch-pc 2>/dev/null

watch:
	nodemon -V -e c,h -x "make && sudo ./broken-touch-pc 1 || false"

.PHONY: clean watch