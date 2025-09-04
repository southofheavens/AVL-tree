main:
	clang -std=c11 -I./include src/map.c main.c -o main

user_deleter:
	clang -std=c11 -I./include src/map.c examples/user_deleter.c -o user_deleter

user_deleter_ptr:
	clang -std=c11 -I./include src/map.c examples/user_deleter_ptr.c -o user_deleter_ptr

compare_strings:
	clang -std=c11 -I./include src/map.c examples/compare_strings.c -o compare_strings

maptests:
	clang -std=c11 -I./include src/map.c maptests.c -o maptests

clean:
	rm -f main maptests user_deleter_ptr user_deleter compare_strings

.PHONY: main user_deleter user_deleter_ptr compare_strings maptests clean