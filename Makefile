build: printf
printf: printf.c
	gcc -o printf printf.c
check: printf
	./printf
clean:
	rm printf
