# Compiler for twink lang
CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -g -Iinclude
LDFLAGS = 

SRCDIR = src
BUILDDIR = build
INCLUDEDIR = include

SOURCES = $(wildcard $(SRCDIR)/*.c) \
          $(wildcard $(SRCDIR)/common/*.c) \
          $(wildcard $(SRCDIR)/lexer/*.c) \
          $(wildcard $(SRCDIR)/parser/*.c) \
          $(wildcard $(SRCDIR)/ast/*.c) \
          $(wildcard $(SRCDIR)/semantic/*.c) \
          $(wildcard $(SRCDIR)/ir/*.c) \
          $(wildcard $(SRCDIR)/codegen/*.c)

OBJECTS = $(SOURCES:$(SRCDIR)/%.c=$(BUILDDIR)/%.o)

TARGET = $(BUILDDIR)/compiler

all: $(BUILDDIR) $(TARGET)

$(BUILDDIR):
	mkdir -p $(BUILDDIR)
	mkdir -p $(BUILDDIR)/common
	mkdir -p $(BUILDDIR)/lexer
	mkdir -p $(BUILDDIR)/parser
	mkdir -p $(BUILDDIR)/ast
	mkdir -p $(BUILDDIR)/semantic
	mkdir -p $(BUILDDIR)/ir
	mkdir -p $(BUILDDIR)/codegen

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
	@echo "  clean    - Remove build files"
	@echo "  debug    - Build with debug flags"
	@echo "  release  - Build optimized release version"
	@echo "  install  - Install compiler to /usr/local/bin"
	@echo "  uninstall- Remove installed compiler"
	@echo "  help     - Show this help message"

$(BUILDDIR)/main.o: $(SRCDIR)/main.c $(INCLUDEDIR)/common.h $(INCLUDEDIR)/lexer.h $(INCLUDEDIR)/parser.h $(INCLUDEDIR)/ast.h $(INCLUDEDIR)/semantic.h $(INCLUDEDIR)/ir.h $(INCLUDEDIR)/codegen.h

$(BUILDDIR)/common/common.o: $(SRCDIR)/common/common.c $(INCLUDEDIR)/common.h

$(BUILDDIR)/lexer/lexer.o: $(SRCDIR)/lexer/lexer.c $(INCLUDEDIR)/lexer.h $(INCLUDEDIR)/common.h

$(BUILDDIR)/parser/parser.o: $(SRCDIR)/parser/parser.c $(INCLUDEDIR)/parser.h $(INCLUDEDIR)/lexer.h $(INCLUDEDIR)/ast.h $(INCLUDEDIR)/common.h

$(BUILDDIR)/ast/ast.o: $(SRCDIR)/ast/ast.c $(INCLUDEDIR)/ast.h $(INCLUDEDIR)/common.h $(INCLUDEDIR)/lexer.h

$(BUILDDIR)/semantic/semantic.o: $(SRCDIR)/semantic/semantic.c $(INCLUDEDIR)/semantic.h $(INCLUDEDIR)/ast.h $(INCLUDEDIR)/common.h

$(BUILDDIR)/ir/ir.o: $(SRCDIR)/ir/ir.c $(INCLUDEDIR)/ir.h $(INCLUDEDIR)/ast.h $(INCLUDEDIR)/common.h

$(BUILDDIR)/codegen/codegen.o: $(SRCDIR)/codegen/codegen.c $(INCLUDEDIR)/codegen.h $(INCLUDEDIR)/ir.h $(INCLUDEDIR)/common.h

.PHONY: all test example clean install uninstall debug release help 