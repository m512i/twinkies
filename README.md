# Twinkies

A modern compiler for the Twink programming language that transpiles to C and assembly. Twink is a statically typed language with C-like syntax, designed for simplicity and performance.

![Twinkies Logo](image.png)

## Project Structure / Flow

More diagrams in the `Twinkies-Flow` folder.

```mermaid
graph TD
    subgraph "Phase 1: Lexical Analysis"
        S1[Source String] --> L1[Character Scanner]
        L1 --> L2[Token Recognizer]
        L2 --> L3[Keywords]
        L2 --> L4[Identifiers]
        L2 --> L5[Literals]
        L2 --> L6[Operators]
        L3 --> T[Token Stream]
        L4 --> T
        L5 --> T
        L6 --> T
    end
    
    subgraph "Phase 2: Syntax Analysis"
        T --> P1[Recursive Descent Parser]
        P1 --> P2[Expression Parser]
        P1 --> P3[Statement Parser]
        P1 --> P4[Function Parser]
        P2 --> AST1[Expr Nodes]
        P3 --> AST2[Stmt Nodes]
        P4 --> AST3[Function Nodes]
        AST1 --> AST[Program AST]
        AST2 --> AST
        AST3 --> AST
    end
    
    subgraph "Phase 3: Semantic Analysis"
        AST --> SA1[Symbol Table Builder]
        SA1 --> SA2[Type Checker]
        SA2 --> SA3[Error Collector]
        SA3 --> AAST[Annotated AST]
        SA1 --> ST[Symbol Table]
    end
    
    subgraph "Phase 4: IR Generation"
        AAST --> IR1[Function IR Gen]
        IR1 --> IR2[Statement IR Gen]
        IR2 --> IR3[Expression IR Gen]
        IR3 --> IR4[IR Instructions]
        IR4 --> IRP[IRProgram]
    end
    
    subgraph "Phase 5: Code Generation"
        IRP --> CG1[Strategy Selection]
        CG1 --> CG2[C Generator]
        CG1 --> CG3[Assembly Generator]
        CG2 --> CODE[C Code]
        CG3 --> ASM[Assembly]
    end
```

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
