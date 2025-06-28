# Names of your source files
LEX = flex
YACC = bison

LEX_FILE = lexer.l
YACC_FILE = parser.yy

LEX_C = scanner.cc
YACC_C = parser.cc
YACC_H = frontend.hh

CXX = g++
CFLAGS = -pedantic -Wall -Wextra -Wcast-align -Wcast-qual -Wctor-dtor-privacy -Wdisabled-optimization -Wformat=2 -Wuninitialized -Winit-self -Wmissing-declarations -Wmissing-include-dirs -Wold-style-cast -Woverloaded-virtual -Wredundant-decls -Wsign-conversion -Wsign-promo -Wstrict-overflow=5 -Wundef -Werror -Wno-unused -Wno-unused-parameter
CPP_SRCS := $(wildcard *.cpp) 
OBJ_SRCS := parser.o lexer.o $(CPP_SRCS:.cpp=.o)

all: compiler

parser.cc: parser.yy
	bison -Werror -Wcounterexamples --graph=parser.dot --defines=frontend.hh -v $<


lexer.yy.cc: lexer.l
	flex --outfile=lexer.yy.cc $<

lexer.o: lexer.yy.cc
	$(CXX) $(CFLAGS) -Wno-sign-compare -Wno-sign-conversion -Wno-old-style-cast -Wno-switch-default -Wno-strict-overflow -g -std=c++14 -c lexer.yy.cc -o lexer.o
parser.o: parser.cc
	$(CXX) $(CFLAGS) -Wno-sign-compare -Wno-sign-conversion -Wno-switch-default -g -std=c++14 -MMD -MP -c -o $@ $<

%.o: %.cpp 
	$(CXX) $(FLAGS) -g -std=c++14 -MMD -MP -c -o $@ $<
# Compile the main program
compiler: $(OBJ_SRCS)
	$(CXX) $(FLAGS) -g -std=c++14 -o $@ $(OBJ_SRCS)
	chmod a+x compiler

run: compiler
	./compiler

graph: run
	dot -Tpng astDigraph.dot -o graph.png
	open graph.png


clean:
	rm -rf *.output *.o *.cc *.hh ac parser.dot parser.png