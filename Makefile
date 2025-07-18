# Compiler for twink lang
CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -g -Iinclude
LDFLAGS = 

SRCDIR = src
BUILDDIR = build
INCLUDEDIR = include

SOURCES = $(wildcard $(SRCDIR)/*.c) \
          $(wildcard $(SRCDIR)/frontend/*/*.c) \
          $(wildcard $(SRCDIR)/analysis/*/*.c) \
          $(wildcard $(SRCDIR)/backend/*/*.c) \
          $(wildcard $(SRCDIR)/common/*.c) \
          $(wildcard $(SRCDIR)/modules/*.c)

OBJECTS = $(SOURCES:$(SRCDIR)/%.c=$(BUILDDIR)/%.o)

TARGET = $(BUILDDIR)/compiler

all: $(BUILDDIR) $(TARGET)

$(BUILDDIR):
	mkdir -p $(BUILDDIR)

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -o $(TARGET) $(LDFLAGS)

$(BUILDDIR)/%.o: $(SRCDIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

test: $(TARGET)
	@echo "Running tests..."
	@if [ -d "tests" ]; then \
		for test_file in tests/*.tl; do \
			if [ -f "$$test_file" ]; then \
				echo "Testing $$test_file"; \
				$(TARGET) $$test_file -o $(BUILDDIR)/test_output.c; \
			fi; \
		done; \
	else \
		echo "No tests directory found"; \
	fi

example: $(TARGET)
	@echo "Compiling examples..."
	@if [ -d "examples" ]; then \
		for example_file in examples/*.tl; do \
			if [ -f "$$example_file" ]; then \
				echo "Compiling $$example_file"; \
				$(TARGET) $$example_file -o $(BUILDDIR)/$$(basename $$example_file .tl).c; \
			fi; \
		done; \
	else \
		echo "No examples directory found"; \
	fi

modules: $(TARGET)
	@echo "Compiling module examples..."
	@if [ -d "examples/modules" ]; then \
		echo "Compiling main.tl with module system"; \
		$(TARGET) examples/modules/main.tl -o $(BUILDDIR)/main.c --modules; \
	else \
		echo "No modules examples directory found"; \
	fi

clean:
	rm -rf $(BUILDDIR)

install: $(TARGET)
	cp $(TARGET) /usr/local/bin/tl-compiler

uninstall:
	rm -f /usr/local/bin/tl-compiler

debug: CFLAGS += -DDEBUG -O0
debug: $(TARGET)

release: CFLAGS += -O2 -DNDEBUG
release: clean $(TARGET)

help:
	@echo "Available targets:"
	@echo "  all      - Build the compiler (default)"
	@echo "  test     - Run tests"
	@echo "  example  - Compile examples"
	@echo "  modules  - Compile module examples"
	@echo "  clean    - Remove build files"
	@echo "  debug    - Build with debug flags"
	@echo "  release  - Build optimized release version"
	@echo "  install  - Install compiler to /usr/local/bin"
	@echo "  uninstall- Remove installed compiler"
	@echo "  help     - Show this help message"

$(BUILDDIR)/main.o: $(SRCDIR)/main.c $(INCLUDEDIR)/common.h $(INCLUDEDIR)/frontend/lexer.h $(INCLUDEDIR)/frontend/parser.h $(INCLUDEDIR)/frontend/ast.h $(INCLUDEDIR)/analysis/semantic.h $(INCLUDEDIR)/backend/ir.h $(INCLUDEDIR)/backend/codegen.h

$(BUILDDIR)/common.o: $(SRCDIR)/common/common.c $(INCLUDEDIR)/common.h

$(BUILDDIR)/frontend/lexer/lexer.o: $(SRCDIR)/frontend/lexer/lexer.c $(INCLUDEDIR)/frontend/lexer.h $(INCLUDEDIR)/common.h

$(BUILDDIR)/frontend/parser/parser.o: $(SRCDIR)/frontend/parser/parser.c $(INCLUDEDIR)/frontend/parser.h $(INCLUDEDIR)/frontend/lexer.h $(INCLUDEDIR)/frontend/ast.h $(INCLUDEDIR)/common.h

$(BUILDDIR)/frontend/ast/ast.o: $(SRCDIR)/frontend/ast/ast.c $(INCLUDEDIR)/frontend/ast.h $(INCLUDEDIR)/frontend/astexpr.h $(INCLUDEDIR)/frontend/aststmt.h $(INCLUDEDIR)/common.h $(INCLUDEDIR)/frontend/lexer.h

$(BUILDDIR)/frontend/ast/astexpr.o: $(SRCDIR)/frontend/ast/astexpr.c $(INCLUDEDIR)/frontend/astexpr.h $(INCLUDEDIR)/frontend/ast.h $(INCLUDEDIR)/common.h $(INCLUDEDIR)/frontend/lexer.h

$(BUILDDIR)/frontend/ast/aststmt.o: $(SRCDIR)/frontend/ast/aststmt.c $(INCLUDEDIR)/frontend/aststmt.h $(INCLUDEDIR)/frontend/astexpr.h $(INCLUDEDIR)/frontend/ast.h $(INCLUDEDIR)/common.h $(INCLUDEDIR)/frontend/lexer.h

$(BUILDDIR)/analysis/semantic/semantic.o: $(SRCDIR)/analysis/semantic/semantic.c $(INCLUDEDIR)/analysis/semantic.h $(INCLUDEDIR)/frontend/ast.h $(INCLUDEDIR)/common.h

$(BUILDDIR)/backend/ir/ir.o: $(SRCDIR)/backend/ir/ir.c $(INCLUDEDIR)/backend/ir.h $(INCLUDEDIR)/frontend/ast.h $(INCLUDEDIR)/common.h

$(BUILDDIR)/backend/codegen/codegen.o: $(SRCDIR)/backend/codegen/codegen.c $(INCLUDEDIR)/backend/codegen.h $(INCLUDEDIR)/backend/ir.h $(INCLUDEDIR)/common.h

$(BUILDDIR)/backend/assembly/codegenasm.o: $(SRCDIR)/backend/assembly/codegenasm.c $(INCLUDEDIR)/backend/codegen.h $(INCLUDEDIR)/backend/ir.h $(INCLUDEDIR)/common.h

$(BUILDDIR)/flags.o: $(SRCDIR)/flags.c $(INCLUDEDIR)/flags.h $(INCLUDEDIR)/common.h

$(BUILDDIR)/utils.o: $(SRCDIR)/utils.c $(INCLUDEDIR)/utils.h $(INCLUDEDIR)/flags.h $(INCLUDEDIR)/common.h $(INCLUDEDIR)/frontend/lexer.h $(INCLUDEDIR)/frontend/parser.h $(INCLUDEDIR)/frontend/ast.h $(INCLUDEDIR)/analysis/semantic.h $(INCLUDEDIR)/backend/ir.h $(INCLUDEDIR)/backend/codegen.h

$(BUILDDIR)/modules/modules.o: $(SRCDIR)/modules/modules.c $(INCLUDEDIR)/modules.h $(INCLUDEDIR)/utils.h $(INCLUDEDIR)/common.h

$(BUILDDIR)/backend/ir/iroperands.o: $(SRCDIR)/backend/ir/iroperands.c $(INCLUDEDIR)/backend/iroperands.h $(INCLUDEDIR)/common.h $(INCLUDEDIR)/backend/ir.h

$(BUILDDIR)/backend/ir/irinstructions.o: $(SRCDIR)/backend/ir/irinstructions.c $(INCLUDEDIR)/backend/irinstructions.h $(INCLUDEDIR)/common.h $(INCLUDEDIR)/backend/ir.h $(INCLUDEDIR)/backend/iroperands.h

.PHONY: all test example clean install uninstall debug release help 