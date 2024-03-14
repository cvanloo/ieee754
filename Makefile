.PHONY: all

all: fp

fp: fp.c
	clang -o fp -Wall -Wextra -Werror -O0 fp.c

test: fp
	@./fp

clean:
	$(RM) fp
