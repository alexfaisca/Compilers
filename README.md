# MML compiler (Project of Compilers @ IST)
## Final delivery graded at 18/20

Check out the [MML Language Reference Guide](https://web.tecnico.ulisboa.pt/~david.matos/w/pt/index.php/Compiladores/Projecto_de_Compiladores/Projecto_2022-2023/Manual_de_Refer%C3%AAncia_da_Linguagem_MML)

The MML compiler is comprised of:
* scanner (`mml_scanner.l`)
* parser (`mml_parser.y`)
* symbol (`targets/symbol.h`)
* type checker (`targets/type_checker.cpp`)
* XML writer (for the middle delivery: `targets/xml_writer.cpp`)
* Postfix writer (for the final delivery: `targets/postfix_writer.cpp`)

This compiler relies on an external library for the printing to and reading from the terminal operations (linking process detailed below).

## Usage

- Compile source code to .asm file

    ./mml test.mml

- Assemble .asm file onto .o file

    yasm -felf32 test.asm
    
- Link .o file onto executable

    ld -m elf_i386 -o test test.o -L base/lib/ -lrts
