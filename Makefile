all: parser.o lexer.o lex.o main-sub.o
	g++ -std=c++20 -o racket2tmp -Wno-write-strings parser.o lexer.o lex.o main-sub.o -lreadline

parser.o: parser.cc parser.h
	g++ -std=c++20 -Wno-write-strings -c parser.cc -o parser.o

lexer.o: lexer.cc lexer.h
	g++ -std=c++20 -Wno-write-strings -c lexer.cc -o lexer.o

lex.o: lex.cc lex.h
	g++ -std=c++20 -c lex.cc -o lex.o

parser.cc : parser.yy
	bison --output=parser.cc --defines=parser.h -Wcounterexamples -v parser.yy

lexer.cc: lexer.lex
	flex --outfile=lexer.cc --header-file=lexer.h lexer.lex

main-sub.o: main-sub.cc
	g++ -std=c++20 -c main-sub.cc -o main-sub.o

main-sub.cc: main.cc eval.cc
	sed 's/"/\\\\"/g' eval.cc | sed 's/$$/\\\\n\\\\/g' | sed '$$ s/..$$//' | sed 's/&/\\\\&/g' > eval-sub.cc
	awk -v r="$$(cat eval-sub.cc)" '{gsub(/__SOURCE_FROM_EVAL__/,r)}1' main.cc > main-sub.cc
