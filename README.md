# TM / PDA Simulator
This is a simple simulator for Turing Machines (TM) and Pushdown Automata (PDA).

# Compile
Use the following command to compile the code:
```bash
cmake -S . -B build
cmake --build build
```
to compile.

The binary file will be in the `bin/` directory.

# PDA Simulator

## Usage
```bash
./bin/fla <path_to_grammar_file> input_string
```

## Gramma

### State Definition

Using 
```txt
#Q = {q0, q1, q2, ...}
```
to define the set of states.
### Input Alphabet
Using 
```txt
#S = {a, b, c, ...}
```
to define the set of input symbols.
### Stack Alphabet
Using 
```txt
#G = {a, b, c, ...}
```
to define the set of stack symbols.
### Initial State
Using 
```txt
#q0 = q_init
```
to define the initial state.
### Stack Start Symbol
Using 
```txt
#z0 = z
```
to define the stack start symbol.
### Final State
Using 
```txt
#F = {q_accept, q_reject}
```
to define the set of final states.
### Transition Function
Taking 
```txt
q 0 Z q1 XXZ
```
as an example, the transition function is defined as follows:
- The first symbol is the current state, which is `q` in this case.
- The second symbol is the current input symbol, which is `0` in this case.
- The third symbol is the current stack symbol, which is `Z` in this case.
- The fourth symbol is the next state, which is `q1` in this case.
- The fifth symbol is the stack symbol to be pushed to the stack, which means the top of the stack would be `XXZ` in this case.

> If the input symbol is $\epsilon$ use `_` to represent it.

### Comment
The line starting with `;` will be ignored.

# TM Simulator

## Usage
```bash
./bin/fla <path_to_to_grammar_file> input_string
```

If you want to use the verbose mode, you can add the `-v` option:
```bash
./bin/fla -v <path_to_grammar_file> input_string
```
The verbose mode will print the current state, the current input symbol, the current tape status, and the current head position.

## Gramma

### State Definition

Using 
```txt
#Q = {q0, q1, q2, ...}
```
to define the set of states.
### Input Alphabet
Using 
```txt
#S = {a, b, c, ...}
```
to define the set of input symbols.
### Tape Alphabet
Using 
```txt
#G = {a, b, c, ...}
```
to define the set of tape symbols.
### Initial State
Using 
```txt
#q0 = q_init
```
to define the initial state.
### Blank Symbol
Using 
```txt
#B = _
```
to define the blank symbol. **The symbol _ refers to blank**
### Tape Number
Using 
```txt
#N = 2
```
to define the number of tapes.

### Final State
Using 
```txt
#F = {q_accept, q_reject}
```
to define the set of final states.
### Transition Function
Taking 
```txt
q 01 __ rl reject
```
as an example, the transition function is defined as follows:
- The first symbol is the current state, which is `q` in this case.
- The second symbol is the current tape symbols , which means the first tape has `0` and the second tape has `1`.
- The third symbol is the tape symbols afterwards, which means the first tape has `_` and the second tape has `_`.
- The fourth symbol is the direction of the head, which is `rl` in this case. The first head will move to the right and the second head will move to the left.
- The fifth symbol is the next state, which is `reject` in this case.
> If the input symbol is $\epsilon$ use `_` to represent it.
> You can use `*` to represent any symbol in the tape alphabet. 
 
### Comment
The line starting with `;` will be ignored.