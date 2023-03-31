#ifndef _assembler_h_
#define _assembler_h_

#include <vector>
#include <string>
#include <algorithm>
#include <unordered_map>
#include <cstring>

using namespace std;

enum Symbol_Type{
  SCTN=100,
  NOTYP = 101
};
enum Symbol_Bind{
  LOC= 110,
  GLOB = 111
};


struct Symbol{
  int Num;//identifikator simbola
  int Value=0;//Vrednost
  int Size=0;//Velicina (samo za sekcije)
  string Type="SCTN"; //Tip Sybola (SCTN ili NOTYP)
  string Bind="LOC";  //Povezivanje (LOC ili GLOB)
  int Ndx;  //indeks sekcije 
  string Name = "UND";  //Ime
  bool isExtern= false; //Da li je simbol eksteran
};

struct Symbol_Bin{
  public:
  Symbol_Bin(Symbol sym){
    Num=sym.Num;
    Value=sym.Value;
    Size=sym.Size;
    if(strcmp(sym.Type.c_str(), "SCTN")==0){
      Type=0;
    } else {
      Type=1;
    }
    if(strcmp(sym.Bind.c_str(), "LOC")==0){
      Bind=0;
    } else {
      Bind=1;
    }
    Ndx=sym.Ndx;
    Name=sym.Name;
    isExtern=sym.isExtern;
  }

  int Num;//identifikator simbola
  int Value=0;//Vrednost
  int Size=0;//Velicina (samo za sekcije)
  int Type=0; //Tip Sybola (SCTN==0 ili NOTYP==1)
  int Bind=0;  //Povezivanje (LOC==0 ili GLOB==1)
  int Ndx;  //indeks sekcije 
  string Name = "UND";  //Ime
  bool isExtern= false; //Da li je simbol eksteran
};

  struct Relocation{
    int Offset; 
    int Type; //kad je tip = 1 onda je addent -2
    string Symbol;
    short Addend;
  };



  struct Section{
    string Name;
    vector<int> SectionText;	
    vector<Relocation> RealocationTable;
  };

class Assembler
{
  public:
    Assembler(){}
    //Tabela simbola i sekcija
    static vector<Symbol> Symbols;
    static vector<Section> Sections;

    //
    static int location_counter;
    static int currNumSimbol;
    static int index_of_current_section;

    //Izlazni tekstualni fajl
    static ofstream*   outputFile;

    //Korisni funkcije
    static void printAllSymbols();
    static void printSections();

    //Vrace redni broj simbola u tabeli simbola sa imenom name
    static int symbolIndex(string name);
    //Vrace redni broj sekcije u tabeli sekcija sa imenom name
    static int sectionIndex(string name);
    
    //Vraca celobrojnu vrednost izbinarno, decimalnog ili heksadecimalnog zapisa
    static int getValue(string text);

    //Vraca redni broj i registra ri
    static int numOfReg(string ri);

    //Obrada ulaznog asemblerskog fajla
    static void asm_start(void* l, string outFile);
    
    
    // static int trimFirstSymbol(string); ??????
};



#endif