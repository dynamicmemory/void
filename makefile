void: main.c session.c input.c file.c sysops.c display.c
	gcc main.c session.c input.c file.c sysops.c display.c -o void

# test: main.c session.c input.c file.c sysops.c display.c
# 	gcc main.c session.c input.c file.c sysops.c display.c -o test

clean:
	rm -rf void test *.txt
