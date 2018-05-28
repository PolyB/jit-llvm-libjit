# LLVM-LIBJIT
The purpose of this repository is to compare LLVM and libjit by developping a Brainfuck interpreter.

## Building

### Update submodule
`$ git submodule init`
`$ git submodule update`

### Build using cmake
`$ mkdir build`
`$ cd !^`
`$ cmake ..`
`$ make`

## Usage
`$ ./test_jit --llvm file.bf`
or
`$ ./test_jit --libjit file.bf`

this will print the intermediary representation of the jitted function to stderr and the result of the brainfuck program to stdout.

## Exemple

`$ ./test_jit --libjit <(echo ',++.') 2>/dev/null <<<a`
result:
`c`
