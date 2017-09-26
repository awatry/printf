build: printf
printf: printf.c
	gcc -o printf printf.c -lm
check: printf
	./printf
clean:
	rm printf
