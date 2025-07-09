# Compiler for twink lang
CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -g -Iinclude
LDFLAGS = 

SRCDIR = src
BUILDDIR = build
INCLUDEDIR = include

SOURCES = $(wildcard $(SRCDIR)/*.c)

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

$(BUILDDIR)/common.o: $(SRCDIR)/common.c $(INCLUDEDIR)/common.h

$(BUILDDIR)/lexer.o: $(SRCDIR)/lexer.c $(INCLUDEDIR)/lexer.h $(INCLUDEDIR)/common.h

$(BUILDDIR)/parser.o: $(SRCDIR)/parser.c $(INCLUDEDIR)/parser.h $(INCLUDEDIR)/lexer.h $(INCLUDEDIR)/ast.h $(INCLUDEDIR)/common.h

$(BUILDDIR)/ast.o: $(SRCDIR)/ast.c $(INCLUDEDIR)/ast.h $(INCLUDEDIR)/common.h $(INCLUDEDIR)/lexer.h

$(BUILDDIR)/semantic.o: $(SRCDIR)/semantic.c $(INCLUDEDIR)/semantic.h $(INCLUDEDIR)/ast.h $(INCLUDEDIR)/common.h

$(BUILDDIR)/ir.o: $(SRCDIR)/ir.c $(INCLUDEDIR)/ir.h $(INCLUDEDIR)/ast.h $(INCLUDEDIR)/common.h

$(BUILDDIR)/codegen.o: $(SRCDIR)/codegen.c $(INCLUDEDIR)/codegen.h $(INCLUDEDIR)/ir.h $(INCLUDEDIR)/common.h

$(BUILDDIR)/codegenasm.o: $(SRCDIR)/codegenasm.c $(INCLUDEDIR)/codegen.h $(INCLUDEDIR)/ir.h $(INCLUDEDIR)/common.h

.PHONY: all test example clean install uninstall debug release help 