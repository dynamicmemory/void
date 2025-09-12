void:
	gcc main.c session.c state.c input.c file.c -o void

test: main.c session.c state.c input.c file.c statusbar.c
	gcc main.c session.c state.c input.c file.c statusbar.c -o test

clean:
	rm -rf void refactor *.txt
