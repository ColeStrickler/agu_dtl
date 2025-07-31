# Tools
LEX = flex
YACC = bison

# Source files
LEX_FILE = lexer.l
YACC_FILE = parser.yy

# Generated files
LEX_C = lexer.yy.cc
YACC_C = parser.cc
YACC_H = frontend.hh

# Compilers
CXX_RISCV = riscv64-unknown-linux-gnu-g++
CXX_X86   = g++

CFLAGS_COMMON = -O2 -fno-math-errno -fstrict-aliasing -g -std=c++17 -I./
CFLAGS_RISCV  = $(CFLAGS_COMMON)
CFLAGS_X86    = $(CFLAGS_COMMON)

# All C++ source files in current directory (excluding main.cpp)
CPP_SRCS := $(filter-out main.cpp, $(wildcard *.cpp))

# Object files for each build
OBJ_RISCV = parser.o lexer.o $(CPP_SRCS:.cpp=.o)
OBJ_X86   = parser.x86.o lexer.x86.o $(CPP_SRCS:.cpp=.x86.o) main.x86.o

# Outputs
LIB_NAME = libdtl.a
DTL_COMPILER = dtlcompiler

# Default target: build both
all: lib dtlcompiler


run: dtlcompiler
	chmod +x ./dtlcompiler
	./dtlcompiler

# ============================================================
# === RISC-V library build (via "make lib") ==================
# ============================================================
lib: $(LIB_NAME)

$(LIB_NAME): $(OBJ_RISCV)
	ar rcs $@ $^

parser.o: parser.cc frontend.hh
	$(CXX_RISCV) $(CFLAGS_RISCV) -MMD -MP -c $< -o $@

lexer.o: lexer.yy.cc
	$(CXX_RISCV) $(CFLAGS_RISCV) -Wno-sign-compare -Wno-sign-conversion \
	    -Wno-old-style-cast -Wno-switch-default -Wno-strict-overflow \
	    -c $< -o $@

%.o: %.cpp
	$(CXX_RISCV) $(CFLAGS_RISCV) -MMD -MP -c $< -o $@

# ============================================================
# === x86 dtlcompiler build (via "make dtlcompiler") =========
# ============================================================
dtlcompiler: $(OBJ_X86)
	$(CXX_X86) $(CFLAGS_X86) -o $@ $(OBJ_X86)

parser.x86.o: parser.cc frontend.hh
	$(CXX_X86) $(CFLAGS_X86) -MMD -MP -c $< -o $@

lexer.x86.o: lexer.yy.cc
	$(CXX_X86) $(CFLAGS_X86) -Wno-sign-compare -Wno-sign-conversion \
	    -Wno-old-style-cast -Wno-switch-default -Wno-strict-overflow \
	    -c $< -o $@

%.x86.o: %.cpp
	$(CXX_X86) $(CFLAGS_X86) -MMD -MP -c $< -o $@

main.x86.o: main.cpp
	$(CXX_X86) $(CFLAGS_X86) -c $< -o $@

# ============================================================
# === Lex & Yacc rules =======================================
# ============================================================
parser.cc frontend.hh: parser.yy
	$(YACC) -Werror -Wcounterexamples --graph=parser.dot \
	        --defines=frontend.hh -v $<

lexer.yy.cc: lexer.l
	$(LEX) --outfile=lexer.yy.cc $<

# ============================================================
# === Clean ==================================================
# ============================================================
clean:
	rm -rf *.o *.x86.o *.d *.output *.cc *.hh parser.dot parser.png \
	       $(LIB_NAME) $(DTL_COMPILER) lexer.yy.cc frontend.hh
