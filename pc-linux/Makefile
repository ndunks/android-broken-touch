LDFLAGS =
CFLAGS = -Wall

build: main

clean:
	@rm -f main 2>/dev/null

watch:
	nodemon -V -e c,h -x "make && sudo ./main 1 || false"

.PHONY: build clean watch