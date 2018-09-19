# https://stackoverflow.com/questions/16192087/undefined-reference-to-initscr-ncurses#16192128
# https://stackoverflow.com/questions/2418157/c-error-undefined-reference-to-clock-gettime-and-clock-settime

.PHONY: all

all: bin/stop-watch

bin/stop-watch: src/stop-watch.c
	@echo "	CC	$@"
	@gcc $< -o $@ -lncurses -lrt
