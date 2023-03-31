#ifndef _linker_h_
#define _linker_h_

#include <vector>
#include <string>
#include <algorithm>
#include <unordered_map>

using namespace std;

struct Symbol{
  int Num;//identifikator simbola
  int Value=0;//Vrednost
  int Size=0;//Velicina (samo za sekcije)
  string Type="SCTN"; //Tip Sybola (SCTN ili NOTYP)
  string Bind="LOC";  //Povezivanje (LOC ili GLOB)
  int Ndx;  //indeks sekcije 
  string Name = "UND";  //Ime
  bool isExtern= false; //Da li je simbol eksteran
  int fileId;
};

struct Relocation{
  int Offset; 
  int Type; //kad je tip = 1 onda je addent -2
  string Symbol;
  short Addend;
  int fileId;
};

struct Section{
  string Name;
  vector<int> SectionText;	
  vector<Relocation> RealocationTable;
};

class Linker{
  public:
    //Izlazni tekstualni fajl
    static ofstream* outputFile;
    static ofstream* outputFileBinary;
    static vector<int> outputVector;//Iz vektora se upisuje u fajl
    
    //Tabela simbola i sekcija
    static vector<Symbol> Symbols;
    static vector<Section> Sections;

    //Tabela lokalnih simbola 
    static vector<Symbol> localSymbols;
    
    //vraca index simbola u vektoru sibola, ako ne postoji vraca -1
    static int SymbolIndex(vector<Symbol>, string);
    
    //vraca index sekcije u vektoru sekcija, ako ne postoji vraca -1
    static int SectionIndex(vector<Section>, string);
    
    //vraca index sekcije u vektoru symbola, ako ne postoji vraca -1
    static int SectionIndexInSymbolTable(vector<Symbol>, int);
    static int LocalSectionIndexInLocalSymbolTable(vector<Symbol>, int, int);
    static int  LocalSectionIndexInLocalSymbolTablebyName(vector<Symbol>, string, int);
    static int LocalSectionIndexInLocalSymbolTableNumFileId(vector<Symbol>, int, int);
    //static int indexOfSectionInSymbolTable(int ndx);
    //static int indexOfSectionInLocalSymbolTable(string name, int fileId);    
    static void linker_start(vector<string> inputFiles, string outFile);

    static void linker_load(vector<string> inputFiles);


    //DebugFunkcija 
    static void printInputFile();

    static void setSectionOffsets(); 

    static void printOutputFile();
};













#endif