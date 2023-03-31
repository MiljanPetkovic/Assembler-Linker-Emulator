//Locirati se u lexer_assembler
flex --header-file=MyLexerAssembler.h lexerAssembler.lext

g++ ./src/main_assembler.cpp ./lexer_assembler/lex.yy.cc  ./src/assembler.cpp -o assembler

./assembler  files_asm/main.s -o files_obj_txt/main.o
./assembler  files_asm/isr_reset.s -o files_obj_txt/isr_reset.o
./assembler  files_asm/isr_terminal.s -o files_obj_txt/isr_terminal.o
./assembler  files_asm/isr_timer.s -o files_obj_txt/isr_timer.o
./assembler  files_asm/isr_user0.s -o files_obj_txt/isr_user0.o
./assembler  files_asm/ivt.s -o files_obj_txt/ivt.o
./assembler  files_asm/math.s -o files_obj_txt/math.o

//Locirati se u lexer_linker	
flex --header-file=MyLexerLinker.h lexerLinker.lext

g++ ./src/main_linker.cpp ./lexer_linker/lex.yy.cc  ./src/Linker.cpp -o linker

./linker -hex files_obj_txt/ivt.o files_obj_txt/math.o files_obj_txt/main.o files_obj_txt/isr_reset.o files_obj_txt/isr_terminal.o files_obj_txt/isr_timer.o files_obj_txt/isr_user0.o


g++ -g  src/emulator.cpp src/main_emulator.cpp -o emulator
./emulator linkerOutput.hex 