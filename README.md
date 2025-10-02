# Twinkies

A modern compiler for the Twink programming language that transpiles to C and assembly. Twink is a statically typed language with C-like syntax, designed for simplicity and performance.

![Twinkies Logo](Z.png)

## Features

- **Static typing** with type inference
- **C-like syntax** that's familiar and easy to learn
- **FFI support** for calling external libraries
- **Module system** with header files
- **Multiple output formats** - transpile to C or compile to assembly
- **Built-in debugging** tools and memory profiling
- **VS Code syntax highlighting** extension included

## Language Overview

Twink supports modern programming constructs while maintaining simplicity:

```twink
func main() -> int {
    let message: string = "Hello, World!";
    let numbers: int[5];
    
    numbers[0] = 1;
    numbers[1] = 2;
    numbers[2] = 3;
    numbers[3] = 4;
    numbers[4] = 5;
    
    print(message);
    
    let i: int = 0;
    while (i < 5) {
        print("Number:", numbers[i]);
        i = i + 1;
    }
    
    return 0;
}
}
```

### Type System
- **Primitives**: `int`, `float`, `double`, `bool`, `string`
- **Arrays**: Fixed-size arrays with bounds checking
- **Functions**: First-class with return type annotations

### FFI Example
```twink
extern "cdecl" from "kernel32.dll" {
    func GetTickCount() -> int;
}

func main() -> int {
    let ticks: int = GetTickCount();
    print("System ticks:", ticks);
    return 0;
}
```

## Building

```bash
make all          # Build the compiler
make debug        # Build with debug symbols
make release      # Build optimized version
```

## Usage

```bash
# Compile to C
./compiler input.tl -o output.c

# Compile to assembly (experimental)
./compiler input.tl -o output.s --asm

# Enable module system
./ompiler main.tl -o output.c --modules

# Debug options
./compiler input.tl -o output.c --debug --verbose
```

### Compiler Options

- `-o <file>` - Specify output file
- `--asm` - Generate assembly output
- `--modules` - Enable module system
- `--debug` - Enable debug output
- `--verbose` - Verbose compilation
- `--tokens` - Print token stream
- `--ast` - Print abstract syntax tree
- `--ir` - Print intermediate representation

## Examples

The `examples/` directory contains sample programs:

- `basic/` - Simple programs demonstrating language features
- `advanced/` - Complex examples with arrays, functions, and control flow
- `ffi/` - Foreign function interface examples
- `modules/` - Module system demonstrations
- `debugging/` - Error handling and debugging examples

Try them out:
```bash
make example      # Compile all basic examples
make modules      # Test module system
```

## Project Structure

```
├── src/
│   ├── frontend/     # Lexer, parser, AST
│   ├── analysis/     # Semantic analysis
│   ├── backend/      # IR generation, code generation
│   └── modules/      # Module system and FFI
├── include/          # Header files
├── examples/         # Sample programs
└── TwinkSyntax/      # VS Code extension
```

## Development

The compiler follows a traditional pipeline:
1. **Lexical Analysis** - Tokenization
2. **Parsing** - AST generation  
3. **Semantic Analysis** - Type checking
4. **IR Generation** - Intermediate representation
5. **Code Generation** - C/Assembly output

## VS Code Extension

Install the included syntax highlighter from `TwinkSyntax/twink-syntax-0.0.2.vsix` for full language support in VS Code.

## Status

- ✅ Core language features implemented
- ✅ C code generation working
- ⚠️ Assembly generation (experimental)
- ✅ FFI system functional
- ✅ Module system operational

## License

See [LICENSE](LICENSE) for details.