build:
	$(CC) main.c -o broken-touch-pc

clean:
	@rm -f broken-touch-pc 2>/dev/null

watch:
	
	nodemon -V -e c,h -x "sudo killall broken-touch-pc & make && \
	sudo ./broken-touch-pc 192.168.1.3 1 || false"

.PHONY: clean watch