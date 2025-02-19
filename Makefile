OBJS = lex.yy.o C.tab.o symbol_table.o nodes.o stack.o interpreter.o tac_generator.o mips_generator.o main.o
SRCS = lex.yy.c C.tab.c symbol_table.c nodes.c stack.c interpreter.c tac_generator.c mips_generator.c main.c
CC = gcc

all:	mycc

clean:
	rm ${OBJS}

mycc:	${OBJS}
	${CC} -g -o mycc ${OBJS} 

C.tab.c:	C.y
	bison -d -t -v C.y

lex.yy.c: C.flex
	flex C.flex

.c.o:
	${CC} -g -c $*.c

depend:	
	${CC} -M $(SRCS) > .deps
	cat Makefile .deps > makefile

dist:	symbol_table.c nodes.c main.c Makefile C.flex C.y nodes.h token.h
	tar cvfz mycc.tgz symbol_table.c nodes.c main.c Makefile C.flex C.y \
		nodes.h token.h
