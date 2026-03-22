# SAS-C Compiler

## TODOs

Next time

- comments
- better testing

Soon:

- float literals
- if statements
- loops
- functions
	- function definitions/declaration

Before self-hosting:

- global vars
	- global init dependency checking
- structs
- traits
- stdlib

Eventually:

- try doing UTF8
- labelled code blocks (for early breaks)
  
## Notes for self

- RISC-V addi only supports a 12-bit signed immediate. The code generator will eventually have to "expand" a large LoadImm into a sequence like lui and addi.

## Notes for documentation

- only supporting 32-bit
- ok main return values: void, i32, u32
- all top level functions are hoisted (global var initialization is calculated with DAG)
- exit codes found in utils/common.hpp