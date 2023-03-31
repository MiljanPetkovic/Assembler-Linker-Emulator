#include <fstream>
#include <sstream>
#include <iostream>
#include "../inc/assembler.h"
#include "../lexer_assembler/MyLexerAssembler.h"

using namespace std;

int main(int argc, char** argv)
{
 string inputFile;
	string outputFile;

  if ( argc == 2)
  {
      inputFile = argv[1];
      outputFile = "asm.o";
  }else if ( argc == 4)
  {
    if ( (string)argv[2] == "-o"){
			
			inputFile = argv[1];
			outputFile = argv[3];
		}else 
    {
      printf("Invalid parameter input\n");
		  return -1;
    }
	} else {
		printf("Invalid parameter input\n");
		return -1;
	}

  ifstream file;
  file.open(inputFile);
	if ( !file.is_open())
  {
		printf("Invalid input file.\n");
		return -1;
	}

  string assemblerInput="";

	
	while(!file.eof()){
		string tmp;
		getline(file,tmp); 
		assemblerInput = assemblerInput + tmp + '\n';
	}

  istringstream  flex_data(assemblerInput);
	yyFlexLexer l(&flex_data, &cout);
	
  Assembler::asm_start(&l, outputFile);
	
}