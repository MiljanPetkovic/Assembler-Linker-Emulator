#include <fstream>
#include <sstream>
#include <iostream>
#include "../inc/Linker.h"
#include "../lexer_linker/MyLexerLinker.h"


using namespace std;

  
int main(int argc, char** argv)
{
  vector<string> inputFiles = vector<string>();
	string outputFile ="linkerOut";

  bool hex = false;

  for ( int i = 1; i < argc; i++){
    if ( (string)argv[i] == "-hex"){
      hex = true;
    }
  }
  if (!hex){
    cerr<< "Wrong input format 1";
    return(-1);
  }

  for ( int i = 1; i < argc; i++){
    if ( (string)argv[i] == "-o" && i < argc - 1 ){
      if( strcmp("linkerOut", outputFile.c_str())==0){
        outputFile = argv[i+1];
        i++; //preskakanje izlaznog fajla
      }
      else{
        cerr<< "Wrong input format 2";
        return(-1);
      }
    } else if((string)argv[i] != "-hex" ){
      inputFiles.push_back((string)argv[i]);
    }
  }
  if( inputFiles.size() > 0 ){
      Linker::linker_start(inputFiles, outputFile);
  }else {
      cerr<< "Wrong input format 3";
       return(-1);
  }





}