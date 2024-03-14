.PHONY: all

all: fp.c
	clang -o fp -Wall -Wextra -Werror -O0 fp.c

clean:
	$(RM) fp
