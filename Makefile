main:
	clang -std=c11 -I./include src/map.c main.c -o main

user_deleter_stack:
	clang -std=c11 -I./include src/map.c examples/user_deleter_stack.c -o user_deleter_stack

user_deleter_heap:
	clang -std=c11 -I./include src/map.c examples/user_deleter_heap.c -o user_deleter_heap

compare_strings:
	clang -std=c11 -I./include src/map.c examples/compare_strings.c -o compare_strings

maptests:
	clang -std=c11 -I./include src/map.c maptests.c -o maptests

clean:
	rm -f main maptests user_deleter_heap user_deleter_stack compare_strings

.PHONY: main user_deleter_stack user_deleter_heap compare_strings maptests clean