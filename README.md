# llvmbf
GCC-like Brainfuck compiler with llvm
## Building bfcc (the llvmbf compiler)
1. Install all the necessary llvm tools and sdk's
2. Clone this repo: ```git clone https://github.com/aceinetx/llvmbf```
3. Cd into it and create the build directory: ```cd llvmbf && mkdir build```
4. Configure cmake: ```cmake ..```
5. Build it: ```make -j4```

bfcc is now built
## Using bfcc
### Compile .bf
- ```./bfcc a.bf```
### Flags that you might find useful
- ```--help```: Displays all the flags
- ```--ir``` (or ```-i```): Generate only IR code
- ```--asm``` (or ```-S```): Generate only assembly
- ```--obj``` (or ```-c```): Generate only object file
