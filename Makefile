.PHONY: all test clean .FORCE

all: fp

fp: fp.c
	clang -o fp -Wall -Wextra -Werror -O0 fp.c

test: fp .FORCE
	@./fp

clean:
	-$(RM) fp

.FORCE:
