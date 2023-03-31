#include <fstream>
#include <sstream>
#include <iostream>
#include <stdio.h>
#include <iomanip>
#include "../inc/emulator.h"

using namespace std;


int main(int argc, char** argv)
{
  char *inputFileName;
  if (argc != 2)
  {
    printf("%s\n", "invalid arguments");
    return 0;
  }
  
  string input;
  input = argv[1];

  ifstream inputFile;
  inputFile.open(input);
	if ( !inputFile.is_open())
  {
		printf("Invalid input file.\n");
		return -1;
	}

  vector<uint16_t> data;
  uint16_t value;
  while (inputFile.read(reinterpret_cast<char *>(&value), sizeof(value)))
  {
      data.push_back(value);
  }
  inputFile.close();

  
	
  Emulator::emulator_start(data);
  Emulator::emulator_processing();
  Emulator::print();
}




