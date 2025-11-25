# Twinkies

A modern compiler for the Twinkies programming language that transpiles to C and assembly. Twinkies is a statically typed language with C-like syntax, designed for simplicity and performance.

![Twinkies Logo](image.png)

## Project Structure / Flow

More diagrams in the `Twinkies-Flow` folder.

<p align="center">
  <img src="Twinkies-Flow/p1.png" width="240" style="margin: 8px;">
  <img src="Twinkies-Flow/p2.png" width="170" style="margin: 8px;">
  <img src="Twinkies-Flow/p3.png" width="170" style="margin: 8px;">
  <img src="Twinkies-Flow/p4.png" width="165" style="margin: 8px;">
  <img src="Twinkies-Flow/p5.png" width="170" style="margin: 8px;">
</p>

## Features

- **Static typing** with type inference
- **C-like syntax** that's familiar and easy to learn
- **FFI support** for calling external libraries
- **Inline Assembly** with GCC-style syntax
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
