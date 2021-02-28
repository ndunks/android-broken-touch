build:
	$(CC) $(APPEND) main.c -I../shared -o brokentouch-pc

clean:
	@rm -f brokentouch-pc 2>/dev/null

watch:
	
	nodemon -V -e c,h -x "sudo killall brokentouch-pc & APPEND=-DDEBUG make  && \
	sudo ./brokentouch-pc 192.168.1.3 2 || false"

.PHONY: clean watch