#include <fstream>
#include <sstream>
#include <stdio.h>
#include <iomanip>
#include "../inc/assembler.h"
#include "../lexer_assembler/MyLexerAssembler.h"

vector<Symbol> Assembler::Symbols = vector<Symbol>();
vector<Section> Assembler::Sections = vector<Section>();

int Assembler::location_counter = 0;
int Assembler::currNumSimbol = 1;
int Assembler::index_of_current_section= 0;

ofstream* Assembler::outputFile = nullptr;

//Definicija enuma i simbola
  #define STRING 0
  #define END_FILE 1
  #define NEW_LINE 2
  #define ESCAPE_SPACES 3
  #define LABEL 4
  #define LITERAL 5
  #define SYMBOL 6
  #define PLUS 7
  #define MINUS 8
  #define OPEN_PARENTHESIS 9
  #define CLOSE_PARENTHESIS 10
  #define OPEN_PARENTHESIS_JMP 11
  #define OPEN_COMMENT 12
  #define REGISTER 13

  enum INSTRUCTION_TYPE{
  INSTRUCTION_NOOP = 14,
  INSTRUCTION_JMP,
  INSTRUCTION_ONE_REG,
  INSTRUCTION_TWO_REG,
  INSTRUCTION_PUSH,
  INSTRUCTION_POP,
  INSTRUCTION_LOAD_STORE
};

enum ARG_VALUE_TYPE{
    ARG_LITERAL_VALUE = 21,
    ARG_SYMBOL_VALUE,
    ARG_SYMBOL_MEM_PCREL,
    ARG_REG_INDIR,
    ARG_REG_LITERAL_MEM,
    ARG_REG_SYMBOL_MEM
  };

  enum JMP_DEST_TYPE{
    JMP_ARG_SYMBOL_VALUE_PCREL = 27,
    JMP_LITERAL_MEM,
    JMP_SYMBOL_MEM,
    JMP_REG_DIR,
    JMP_ARG_REG_INDIR,
    JMP_ARG_REG_LITERAL_MEM,
    JMP_ARG_REG_SYMBOL_MEM
  };

   enum DIRECTIVES_TYPE{
      DIRECTIVES_GLOBAL = 34,
      DIRECTIVES_EXTERN,
      DIRECTIVES_SECTION,
      DIRECTIVES_WORD,
      DIRECTIVES_SKIP,
      DIRECTIVES_ASCII,
      DIRECTIVES_EQU,
      DIRECTIVES_END
    };

  

void Assembler::printAllSymbols(){
  *outputFile << "Symbols" << '\n';
  for(int i= 0 ; i< Symbols.size() ; i++){
    *outputFile<< hex <<Symbols[i].Num<<'\t'
      <<Symbols[i].Value<<'\t'<<Symbols[i].Size
      <<'\t'<<Symbols[i].Type<<'\t'<<Symbols[i].Bind
      <<'\t'<<Symbols[i].Ndx<<'\t'<<Symbols[i].Name<<'\t'<<Symbols[i].isExtern<<'\n';
  }
}

void Assembler::printSections(){
  for(int i= 0 ; i< Sections.size() ; i++){
    //zaglavlje sekcije
    *outputFile<< "rela." << Sections[i].Name<<'\n';
    //Tabela relokacija
    for(int j=0;j<Sections[i].RealocationTable.size();j++){
      *outputFile<< hex<<Sections[i].RealocationTable[j].Offset<<'\t' ; 
      if ( Sections[i].RealocationTable[j].Type ==0)  
        *outputFile <<  "16_ABS" << '\t' ;
      else 
        *outputFile <<  "16_PCREL" << '\t' ;
      *outputFile <<Sections[i].RealocationTable[j].Symbol <<'\t' 
        << Sections[i].RealocationTable[j].Addend <<'\n' ;
    }
  }

  //Ispis sekcije
  outputFile->precision(2);
  for(int i= 0 ; i< Sections.size() ; i++){
    *outputFile<< "bin."<<Sections[i].Name<<'\n';
    for(int j=0;j<Sections[i].SectionText.size();j++){
      if( j % 8 == 0 && j!=0) 
        *outputFile << '\n';   
      if(Sections[i].SectionText[j]<16) 
        *outputFile<<'0';
      *outputFile<< hex<<Sections[i].SectionText[j]<<'\t';  
    }
    *outputFile<< '\n';
  }
}

int Assembler::symbolIndex(string name){
  for(int i=0;i<Symbols.size();i++){
    if(strcmp(name.c_str(), Symbols[i].Name.c_str())==0){
      return i;
    }
  }
  return -1;
}

int Assembler::sectionIndex(string name){
  for(int i=0;i<Sections.size();i++){
    if(strcmp(name.c_str(), Sections[i].Name.c_str())==0){
      return i;
    }
  }
  return -1;
}  

int Assembler::getValue(string text){
  if (text[0] == '0') {
      if (text.length() == 1) {
          return 0;
      } else if (text[1] == 'x' || text[1] == 'X') {
          return stol(text.substr(2), nullptr, 16);
      } else if (text[1] == 'b' || text[1] == 'B') {
          return stoi(text.substr(2), nullptr, 2  );
      } else {
          return stoi(text.substr(2), nullptr, 8);
      }
  } else {
      return stoi(text);
  }
}

int Assembler::numOfReg(string ri){
  return stoi(ri.substr(1));
}

void Assembler::asm_start(void* l, string outFile){ 
  //ubacivanje prvog reda tabele simbola
  Symbols.push_back( Symbol());
  Symbols[0].Type="NOTYP";
  //Alokacija izlaznog tekstualnog fajla
  outputFile = new ofstream(outFile);

  //Alokacija lekasera
  yyFlexLexer* lex = (yyFlexLexer*)l ; 
  int ret = 0;

  //Citanje iz fajla dok se ne dodje do kraja fajla
  while ( ret != END_FILE ){    
    //citanje novog reda
    ret = lex->yylex();

    //Provera ispravnosti ulaza 
    if (ret == -1){
				printf("!!!ERROR");
				exit(-1);
		} 

    //Izvozi simbole navedene u okviru liste parametara. 
    //Lista parametara može sadržati samo jedan simbol ili 
    //više njih razdvojenih zapetama.
    if ( ret == DIRECTIVES_GLOBAL ){
      while(1){
        ret = lex->yylex();

        if ( ret == NEW_LINE || ret==OPEN_COMMENT) break;
        if ( lex->YYText() == "," || ret == ESCAPE_SPACES) continue;

        string symbolName = lex->YYText();
        
        if (symbolIndex(symbolName)==-1){ //ako ne postoji dodaj
          Symbol tmpSymbol = Symbol();
          tmpSymbol.Name= symbolName;
          tmpSymbol.Num = currNumSimbol++;
          tmpSymbol.Type = "NOTYP";
          tmpSymbol.Size = 0;
          tmpSymbol.Ndx = 0;
          tmpSymbol.Bind = "GLOB";
          tmpSymbol.Value=0;

          Symbols.push_back(tmpSymbol);
        }else{ //ako postoji a nije definisan kao global onda postavi da je global ( verovatno je bio labela)
          int i = symbolIndex(symbolName);
          if( Symbols[i].Bind == "GLOB"){
            Symbols[i].Bind = "GLOB";
          } else{
            cerr << "Symbol "<< symbolName << " already declared as extern or global" << endl;
            exit(-1);
          }  
        }   
      } 
    }
    
    //Uvozi simbole navedene u okviru liste parametara. 
    //Lista parametara može sadržati samo jedan simbol ili 
    //više njih razdvojenih zapetama.
    if (ret ==DIRECTIVES_EXTERN){
      while(1){
        ret = lex->yylex();
        if ( ret == NEW_LINE || ret==OPEN_COMMENT) break;
        if ( lex->YYText() == "," || ret == ESCAPE_SPACES) continue;
        
        string symbolName = lex->YYText();

        if (symbolIndex(symbolName)==-1){ //ako ne postoji dodaj
          Symbol tmpSymbol = Symbol();
          tmpSymbol.Name= symbolName;
          tmpSymbol.Num = currNumSimbol++;
          tmpSymbol.Type = "NOTYP";
          tmpSymbol.Size = 0;
          tmpSymbol.Ndx = 0;
          tmpSymbol.Bind = "GLOB";
          tmpSymbol.Value=0;
          tmpSymbol.isExtern=true;

          Symbols.push_back(tmpSymbol);
        }else{ //ne sme da bude koriscen pre definicije
          outputFile->close();
          remove(outFile.c_str());
          delete outputFile;
          cerr << "Symbol "<< symbolName << " already declared as extern or global" << endl;
          exit(-1);
        }   
      }
    }

    //Započinje novu asemblersku sekciju, 
    //čime se prethodno započeta sekcija automatski završava, 
    //proizvoljnog imena navedenog kao parametar asemblerske direktive
    if (ret ==DIRECTIVES_SECTION){
      ret = lex->yylex();    

      while( ret == ESCAPE_SPACES) ret = lex->yylex(); 
      
      string section = lex->YYText();


      //Alocira objekte potrebne za unos sekcije ako sekcija ne postoji
      Assembler::location_counter = 0;
      if ( sectionIndex(section)==-1){
        Section tmpSection = Section();
        tmpSection.Name = section;
        tmpSection.SectionText = vector<int>();
        tmpSection.RealocationTable = vector<Relocation>();
        
        Sections.push_back(tmpSection);          
      }
      else {//Prijavljuje gresku ako sekcija postoji
        outputFile->close();
        remove(outFile.c_str());
        delete outputFile;
        cerr << "Multiple declaration of section " << section << " in sample.s" << endl;
        exit(-1);
      }

      //Upis sekcije u tabelu simbola
      if (symbolIndex(section)==-1){ //ako ne postoji dodaj
        Symbol tmpSymbol = Symbol();
        tmpSymbol.Name= section;
        tmpSymbol.Num = currNumSimbol++;
        tmpSymbol.Type = "SCTN";
        tmpSymbol.Size = 0;
        tmpSymbol.Ndx = ++index_of_current_section;
        tmpSymbol.Bind = "LOC";
        tmpSymbol.Value=0;

        Symbols.push_back(tmpSymbol);
      }else{ //ne sme da bude koriscen pre definicije
        outputFile->close();
        remove(outFile.c_str());
        delete outputFile;
        cerr << "Symbol "<< section << " already declared as extern or global" << endl;
        exit(-1);
      }   
    }

    //.word lista_ simbola_ili_literal>
    //Alocira prostor fiksne veličine po dva bajta za svaki inicijalizator (simbol ili literal)
    if (ret ==DIRECTIVES_WORD){
       while(1){
        ret = lex->yylex();

        if ( ret == NEW_LINE  || ret==OPEN_COMMENT ) break;
        if ( lex->YYText() == "," || ret == ESCAPE_SPACES) continue;
        
        string text = lex->YYText();
        
        if ( Assembler::index_of_current_section != 0){
          if ( ret == LITERAL){ //Ako se alocira literal samo se upisuje na njegovo mesto
            int  value = getValue(text);
            int tmp = (value & 0xff);
            Sections[index_of_current_section-1].SectionText.push_back(tmp); 
            value = value >> 8;
            tmp = (value & 0xff);
            Sections[index_of_current_section-1].SectionText.push_back(tmp);
          }
          else if(ret == SYMBOL){//Za simbol je potrebno u baciti ga u tabeli simbola i u reloacioniu tabelu
            string symbolName = lex->YYText();
            if (symbolIndex(symbolName)==-1){ //ako ne postoji dodaj
              Symbol tmpSymbol = Symbol();
              tmpSymbol.Name= symbolName;
              tmpSymbol.Num = currNumSimbol++;
              tmpSymbol.Type = "NOTYP";
              tmpSymbol.Size = 0; // 0 za sve osim sekcija i 0 do kraja asembleriranja
              tmpSymbol.Ndx = 0;
              tmpSymbol.Bind = "LOC";
              tmpSymbol.Value=0;
             Symbols.push_back(tmpSymbol);
             
            }
              Relocation relocation = Relocation();
              relocation.Symbol = symbolName;
              relocation.Offset = Sections[index_of_current_section-1].SectionText.size();
              relocation.Type = 0;//kad je tip = 1 onda je addent -2
              relocation.Addend = 0;
              Sections[index_of_current_section-1].SectionText.push_back(0);
              Sections[index_of_current_section-1].SectionText.push_back(0);
              Sections[index_of_current_section-1].RealocationTable.push_back(relocation);
          }
        }else {
          outputFile->close();
          remove(outFile.c_str());
          delete outputFile;
          cerr << "Section problem 1" << endl;
          exit(-1);
        }
      }
    }

    //Alocira prostor čija je veličina jednaka broju bajtova
    // definisanom literalom navedenim kao parametar. Asemblerska 
    //direktiva alocirani prostor inicijalizuje nulama.
    if ( ret ==DIRECTIVES_SKIP){
      while(1){
        ret = lex->yylex();
        if ( ret == LITERAL){
          string text = lex->YYText();
          int  value = getValue(text);
          while (value > 0){
            Sections[index_of_current_section-1].SectionText.push_back(0);
            value--;
          }
        }
        else if ( ret == NEW_LINE || ret==OPEN_COMMENT) break;
        else if (ret == ESCAPE_SPACES) continue;
        else {
          outputFile->close();
          remove(outFile.c_str());
          delete outputFile;
          cerr << "Section problem 2" << endl;
          exit(-1);
        }
      }
    }

    //Završava proces asembliranja ulazne datoteke.
    if ( ret ==DIRECTIVES_END){
      break;
    }

    //Svaki space karakter je potrebno ignorisati
    if ( ret == ESCAPE_SPACES) continue;


    //Postavljanje labela 
    if ( ret == LABEL){// moze da se desi da postavi 2 labele u istom redu
      string label = lex->YYText();
      label = label.substr(0, label.size()-1); //Odstaranjivanj ':'
      if (symbolIndex(label)==-1){ //ako ne postoji dodaj
          Symbol tmpSymbol = Symbol();
          tmpSymbol.Name= label;
          tmpSymbol.Num = currNumSimbol++;
          tmpSymbol.Type = "NOTYP";
          tmpSymbol.Size = 0;
          tmpSymbol.Ndx = index_of_current_section;
          tmpSymbol.Bind = "LOC";
          tmpSymbol.Value=Sections[index_of_current_section-1].SectionText.size();
          tmpSymbol.isExtern=true;

          Symbols.push_back(tmpSymbol);
        }else if (!Symbols[symbolIndex(label)].isExtern &&  Symbols[symbolIndex(label)].Ndx == 0 ){
          Symbols[symbolIndex(label)].Ndx = index_of_current_section;
          Symbols[symbolIndex(label)].Value=Sections[index_of_current_section-1].SectionText.size();
        } else {
          outputFile->close();
          remove(outFile.c_str());
          delete outputFile;
          cerr << "Section problem 3" << endl;
          exit(-1);
        }
    }

    //Ukoliko je neka od funkcija za prekid izvrsavanje samo ona stoji u redu
    if (ret == INSTRUCTION_NOOP){
      string instruction = lex->YYText();
      if(instruction == "halt"){
        Sections[index_of_current_section-1].SectionText.push_back(0x0);
      } else  if(instruction == "iret"){
        Sections[index_of_current_section-1].SectionText.push_back(0x20);
      } else if(instruction == "ret"){
        Sections[index_of_current_section-1].SectionText.push_back(0x40);
      }
      while(1){ // do kraja reda ne treba nicega da ima 
        ret = lex->yylex();
        if ( ret == NEW_LINE || ret==OPEN_COMMENT) break;
        else if (ret == ESCAPE_SPACES) continue;
        else {
          outputFile->close();
          remove(outFile.c_str());
          delete outputFile;
          cerr << "Section problem 4" << endl;
          exit(-1);
        }
      }
    }

    //Jednoregistarske instukcije
    if (ret == INSTRUCTION_ONE_REG){
        string instruction = lex->YYText();
        ret = lex->yylex();
        while( ret == ESCAPE_SPACES){ret = lex->yylex();}
         if ( ret != REGISTER) {
            outputFile->close();
            remove(outFile.c_str());
            delete outputFile;
            cerr << "Section problem 5" << ret << endl;
            exit(-1);
          }
          string reg = lex->YYText();
          int numOfRegister = numOfReg(reg.c_str());
          
          if( strcmp(instruction.c_str(),"int") == 0 ) {
            Sections[index_of_current_section-1].SectionText.push_back(0x10);
            Sections[index_of_current_section-1].SectionText.push_back((numOfRegister*16 + 15));
          } else if ( strcmp(instruction.c_str(),"push") == 0 ) {
            Sections[index_of_current_section-1].SectionText.push_back(0xB0);
            Sections[index_of_current_section-1].SectionText.push_back((numOfRegister*16 + 6));
            Sections[index_of_current_section-1].SectionText.push_back(0x12);
          }else if ( strcmp(instruction.c_str(),"not") == 0 ) {
            Sections[index_of_current_section-1].SectionText.push_back(0x80);
            Sections[index_of_current_section-1].SectionText.push_back((numOfRegister*16 + 0xF));
          } else if ( strcmp(instruction.c_str(),"pop") == 0 ) {
            Sections[index_of_current_section-1].SectionText.push_back(0xA0);
            Sections[index_of_current_section-1].SectionText.push_back((numOfRegister*16 + 6));
            Sections[index_of_current_section-1].SectionText.push_back(0x42);
          }
    }

    //Dvoregijstarske instukcije 
    if (ret == INSTRUCTION_TWO_REG){
      string instruction = lex->YYText();
      string regD = "";
      string regS = "";
      int numOfRegisterD;
      int numOfRegisterS; 
      while(1){
        ret = lex->yylex();
        if ( ret == OPEN_COMMENT ) break;
        else if ( ret == NEW_LINE || ret==OPEN_COMMENT) break;
        else if ( ret == ESCAPE_SPACES || lex->YYText() == ",") continue;
        else if ( ret == REGISTER) {
          if ( regD == ""){
            regD = lex->YYText();
            numOfRegisterD = numOfReg(regD.c_str());
          }
          else if ( regS == ""){
            regS = lex->YYText();
            numOfRegisterS = numOfReg(regS);
          }
          else {
            outputFile->close();
            remove(outFile.c_str());
            delete outputFile;
            cerr << "Error in  : " << instruction << ": Too many arguments "<< endl;
            exit (-1);
          } 
        }
        else {
          outputFile->close();
          remove(outFile.c_str());
          delete outputFile;
          cerr << "Error in" << instruction << ": Bad format 1"<< endl;
          exit (-1);
        }
      }
      //prvi bajt instrukcij
      if( strcmp(instruction.c_str(),"xchg") == 0 ) {
            Sections[index_of_current_section-1].SectionText.push_back(0x60);
      } else if( strcmp(instruction.c_str(),"add") == 0 ) {
            Sections[index_of_current_section-1].SectionText.push_back(0x70); 
      } else if( strcmp(instruction.c_str(),"sub") == 0 ) {
            Sections[index_of_current_section-1].SectionText.push_back(0x71); 
      } else if( strcmp(instruction.c_str(),"mul") == 0 ) {
            Sections[index_of_current_section-1].SectionText.push_back(0x72); 
      }else if( strcmp(instruction.c_str(),"div") == 0 ) {
            Sections[index_of_current_section-1].SectionText.push_back(0x73); 
      }else if( strcmp(instruction.c_str(),"cmp") == 0 ) {
            Sections[index_of_current_section-1].SectionText.push_back(0x74); 
      }else if( strcmp(instruction.c_str(),"and") == 0 ) {
            Sections[index_of_current_section-1].SectionText.push_back(0x81); 
      }else if( strcmp(instruction.c_str(),"or") == 0 ) {
            Sections[index_of_current_section-1].SectionText.push_back(0x82); 
      }else if( strcmp(instruction.c_str(),"xor") == 0 ) {
            Sections[index_of_current_section-1].SectionText.push_back(0x83); 
      }else if( strcmp(instruction.c_str(),"test") == 0 ) {
            Sections[index_of_current_section-1].SectionText.push_back(0x84); 
      }else if( strcmp(instruction.c_str(),"shl") == 0 ) {
            Sections[index_of_current_section-1].SectionText.push_back(0x90); 
      }else if( strcmp(instruction.c_str(),"shr") == 0 ) {
            Sections[index_of_current_section-1].SectionText.push_back(0x91); 
      }

      //drugi bajt instrukcije
      Sections[index_of_current_section-1].SectionText.push_back(numOfRegisterD*16+ numOfRegisterS);
    }

    //Load and store instukcija
    if (ret == INSTRUCTION_LOAD_STORE){
      string instruction = lex->YYText();

      //Postavljanje prvog bajta instukcije
      if( strcmp(instruction.c_str(),"ldr") == 0 ) { 
        Sections[index_of_current_section-1].SectionText.push_back(0xA0); 
      } else {
        Sections[index_of_current_section-1].SectionText.push_back(0xB0); 
      }


      string reg =""; 
      int numOfRegister;

      string opearand2="";
      int valueOfOperand2;

      while(1){ 
        
        ret = lex->yylex();
        
        if ( ret == OPEN_COMMENT ) break;
        else if ( ret == NEW_LINE || ret==OPEN_COMMENT) break;
        else if ( ret == ESCAPE_SPACES || lex->YYText() == ",") continue;

        else if ( ret == REGISTER){
          if(reg==""){
              reg = lex->YYText();
              numOfRegister = numOfReg(reg.c_str());
              
          } else if(opearand2==""){
              reg = lex->YYText();
              valueOfOperand2 = numOfReg(reg.c_str());
              Sections[index_of_current_section-1].SectionText.push_back((numOfRegister*16 + valueOfOperand2));
          } 
          else{
            outputFile->close();
            remove(outFile.c_str());
            delete outputFile;
            cerr << "Error" << instruction << ": Bad format 2"<< endl;
            exit (-1);
          }
        } else if (ret == ARG_LITERAL_VALUE){

          if(opearand2=="" && reg!=""){
            Sections[index_of_current_section-1].SectionText.push_back((numOfRegister*16 + 15));
            Sections[index_of_current_section-1].SectionText.push_back(0x00);
            string tmp =  lex->YYText();
            int num = getValue(tmp.substr(1));

            int valueToPush = num & 0xFF;
            Sections[index_of_current_section-1].SectionText.push_back(valueToPush);
            valueToPush = num & 0xFF00;
            valueToPush = valueToPush>> 8;
            Sections[index_of_current_section-1].SectionText.push_back(valueToPush);
          } else{
            outputFile->close();
            remove(outFile.c_str());
            delete outputFile;
            cerr << "Error in" << instruction << ": Bad format 3"<< endl;
            exit (-1);
          }
        } else if (ret == ARG_SYMBOL_VALUE){
          if(opearand2=="" && reg!=""){
            //oeracionui kod drugi deo
            Sections[index_of_current_section-1].SectionText.push_back((numOfRegister*16 + 15));
            Sections[index_of_current_section-1].SectionText.push_back(0x00);
            
            //dodavanje u realokacionu tebelu
            string tmp =  lex->YYText();
            
            Relocation rel = Relocation();
            rel.Symbol = tmp.substr(1);//ovde nije problem
            rel.Offset = Sections[index_of_current_section-1].SectionText.size();
            rel.Type = 0;
            rel.Addend = 0;
            Sections[index_of_current_section-1].RealocationTable.push_back(rel);
            
            //ostavljanje nula da se popuni
            Sections[index_of_current_section-1].SectionText.push_back(0x00);
            Sections[index_of_current_section-1].SectionText.push_back(0x00);
            
          } else{
            outputFile->close();
            remove(outFile.c_str());
            delete outputFile;
            cerr << "Error in" << instruction << ": Bad format 4 "<< endl;
            exit (-1);
          }
        } else if (ret == LITERAL){
          if(strcmp(instruction.c_str(), "str")==0){
            outputFile->close();
            remove(outFile.c_str());
            delete outputFile;
            cerr << "store in literal"<< endl;
            exit (-1);
          }

          if(opearand2=="" && reg!=""){
            Sections[index_of_current_section-1].SectionText.push_back((numOfRegister*16 + 15));
            Sections[index_of_current_section-1].SectionText.push_back(0x04);
            string tmp =  lex->YYText();
           
            int num = getValue(tmp.substr(1));
            int valueToPush = num & 0xFF;
            Sections[index_of_current_section-1].SectionText.push_back(valueToPush);
            valueToPush = num & 0xFF00;
            valueToPush = valueToPush>> 8;
            Sections[index_of_current_section-1].SectionText.push_back(valueToPush);
          } else{
            outputFile->close();
            remove(outFile.c_str());
            delete outputFile;
            cerr << "Error in" << instruction << ": Bad format 5 "<< endl;
            exit (-1);
          }
        } else if (ret == SYMBOL){
          if(opearand2=="" && reg!=""){
            //oeracionui kod drugi deo
            Sections[index_of_current_section-1].SectionText.push_back((numOfRegister*16 + 15));
            Sections[index_of_current_section-1].SectionText.push_back(0x04);
            string tmp =  lex->YYText();
            //dodavanje u tabelu simbola
            if (symbolIndex(tmp)==-1){ //ako ne postoji dodaj
                Symbol tmpSymbol = Symbol();
                tmpSymbol.Name= tmp;
                tmpSymbol.Num = currNumSimbol++;
                tmpSymbol.Type = "NOTYP";
                tmpSymbol.Size = 0; // 0 za sve osim sekcija i 0 do kraja asembleriranja
                tmpSymbol.Ndx = 0;
                tmpSymbol.Bind = "LOC";
                tmpSymbol.Value=Sections[index_of_current_section-1].SectionText.size();;

                Symbols.push_back(tmpSymbol);
              }
            //dodavanje u realokacionu tebelu
            
            Relocation rel = Relocation();
            rel.Symbol = tmp;
            rel.Offset = Sections[index_of_current_section-1].SectionText.size();
            rel.Type = 0;
            rel.Addend = 0;
            Sections[index_of_current_section-1].RealocationTable.push_back(rel);
            
            //ostavljanje nula da se popuni
            Sections[index_of_current_section-1].SectionText.push_back(0x00);
            Sections[index_of_current_section-1].SectionText.push_back(0x00);
            
          } else{
            outputFile->close();
            remove(outFile.c_str());
            delete outputFile;
            cerr << "Error in" << instruction << ": Bad format 6"<< endl;
            exit (-1);
          }
        } else if (ret == ARG_SYMBOL_MEM_PCREL){
          if(opearand2=="" && reg!=""){
            //oeracionui kod drugi deo
            Sections[index_of_current_section-1].SectionText.push_back((numOfRegister*16 + 7));
            Sections[index_of_current_section-1].SectionText.push_back(0x05);
            
            string tmp =  lex->YYText();
            //dodavanje u tabelu simbola
            if (symbolIndex(tmp)==-1){ //ako ne postoji dodaj
                Symbol tmpSymbol = Symbol();
                tmpSymbol.Name= tmp;
                tmpSymbol.Num = currNumSimbol++;
                tmpSymbol.Type = "NOTYP";
                tmpSymbol.Size = 0; // 0 za sve osim sekcija i 0 do kraja asembleriranja
                tmpSymbol.Ndx = 0;
                tmpSymbol.Bind = "LOC";
                tmpSymbol.Value=Sections[index_of_current_section-1].SectionText.size();;

                Symbols.push_back(tmpSymbol);
              }

            //u relokacionu
            Relocation rel = Relocation();
            rel.Symbol = tmp.substr(1);
            rel.Offset = Sections[index_of_current_section-1].SectionText.size();
            rel.Type = 1;
            rel.Addend = -2;
            Sections[index_of_current_section-1].RealocationTable.push_back(rel);
            
            //ostavljanje nula da se popuni
            Sections[index_of_current_section-1].SectionText.push_back(0x00);
            Sections[index_of_current_section-1].SectionText.push_back(0x00);
            
          }
        }  else if (ret == OPEN_PARENTHESIS ){
          bool instructionWroted = false;
          bool plus = false;
          bool  thirdParam = false;
          
          while(1){
            ret = lex->yylex();
            if ( ret == OPEN_COMMENT ) {
              if( instructionWroted) break;
              else {
                outputFile->close();
                remove(outFile.c_str());
                delete outputFile;
                cerr << "Error in" << instruction << ": Bad format 7"<< endl;
                exit (-1);
              }
            }
            else if ( ret == ESCAPE_SPACES ) continue;
            else if ( ret == PLUS){
              if(!plus){
                plus = true;
              continue;
              } else {
                outputFile->close();
                remove(outFile.c_str());
                delete outputFile;
                cerr << "Error in" << instruction << ": Bad format 8 "<< endl;
                exit (-1);
              }   
            } else if ( ret == REGISTER ){
              
              if(opearand2==""){
                  reg = lex->YYText();
                  opearand2=reg;
                  valueOfOperand2 = numOfReg(reg.c_str());
                  Sections[index_of_current_section-1].SectionText.push_back((numOfRegister*16 + valueOfOperand2));
              } 
              else{
                outputFile->close();
                remove(outFile.c_str());
                delete outputFile;
                cerr << "Error in" << instruction << ": Bad format 9"<< endl;
                exit (-1);
              }
            } else if(ret == CLOSE_PARENTHESIS){
              if( opearand2!= "" && !plus){
                Sections[index_of_current_section-1].SectionText.push_back(0x02);
                instructionWroted = true;
                break;
              } else if(thirdParam){
                instructionWroted = true;
                break;
              } else{
                outputFile->close();
                remove(outFile.c_str());
                delete outputFile;
                cerr << "Error in" << instruction << ": Bad format 10"<< endl;
                exit (-1);
              }
            } else if ( ret == LITERAL){
              thirdParam =true;
              string text =  lex->YYText();
              int  value = getValue(text);
              Sections[index_of_current_section-1].SectionText.push_back(0x03);
              int tmp = (value & 0xff);
              Sections[index_of_current_section-1].SectionText.push_back(tmp);
              value = value >> 8;
              tmp = (value & 0xff);
              Sections[index_of_current_section-1].SectionText.push_back(tmp);
            } else if(ret == SYMBOL){
              thirdParam =true;
              string symbolName = lex->YYText();
              if (symbolIndex(symbolName)==-1){ //ako ne postoji dodaj
                Symbol tmpSymbol = Symbol();
                tmpSymbol.Name= symbolName;
                tmpSymbol.Num = currNumSimbol++;
                tmpSymbol.Type = "NOTYP";
                tmpSymbol.Size = 0; // 0 za sve osim sekcija i 0 do kraja asembleriranja
                tmpSymbol.Ndx = 0;
                tmpSymbol.Bind = "LOC";
                tmpSymbol.Value=Sections[index_of_current_section-1].SectionText.size();;

                Symbols.push_back(tmpSymbol);
              }
              Relocation relocation = Relocation();
              relocation.Symbol = symbolName;
              relocation.Offset = Sections[index_of_current_section-1].SectionText.size();
              relocation.Type = 0;//kad je tip = 1 onda je addent -2
              relocation.Addend = 0;
              Sections[index_of_current_section-1].SectionText.push_back(0);
              Sections[index_of_current_section-1].SectionText.push_back(0);
              Sections[index_of_current_section-1].RealocationTable.push_back(relocation);
            } else{
              outputFile->close();
              remove(outFile.c_str());
              delete outputFile;
              cerr << "Error in" << instruction << ": Bad format 11"<< endl;
              exit (-1);
            }
          }    
        } 
      }
    }  

    if( ret== INSTRUCTION_JMP){
      string instruction = lex->YYText();
      if( strcmp(instruction.c_str(),"call") == 0 ) {
            Sections[index_of_current_section-1].SectionText.push_back(0x30);
      } else if( strcmp(instruction.c_str(),"jmp") == 0 ) {
            Sections[index_of_current_section-1].SectionText.push_back(0x50); 
      } else if( strcmp(instruction.c_str(),"jeq") == 0 ) {
            Sections[index_of_current_section-1].SectionText.push_back(0x51); 
      } else if( strcmp(instruction.c_str(),"jnc") == 0 ) {
            Sections[index_of_current_section-1].SectionText.push_back(0x52); 
      }else if( strcmp(instruction.c_str(),"jgt") == 0 ) {
            Sections[index_of_current_section-1].SectionText.push_back(0x53); 
      }

      while(1){
        ret = lex->yylex();
        if( ret == ESCAPE_SPACES) continue;
        else break;
      }

      if( ret== LITERAL){
        Sections[index_of_current_section-1].SectionText.push_back(0xFF);
        Sections[index_of_current_section-1].SectionText.push_back(0x00);

        string tmp =  lex->YYText();
        int num = getValue(tmp);

        int valueToPush = num & 0xFF;
        Sections[index_of_current_section-1].SectionText.push_back(valueToPush);
        valueToPush = valueToPush>> 8;
        Sections[index_of_current_section-1].SectionText.push_back(valueToPush);
          
      }else if( ret == SYMBOL){
        Sections[index_of_current_section-1].SectionText.push_back(0xFf);
        Sections[index_of_current_section-1].SectionText.push_back(0x00);

        string tmp =  lex->YYText();
        
        //dodavanje u tabelu simbola
        if (symbolIndex(tmp)==-1){ //ako ne postoji dodaj
          Symbol tmpSymbol = Symbol();
          tmpSymbol.Name= tmp;
          tmpSymbol.Num = currNumSimbol++;
          tmpSymbol.Type = "NOTYP";
          tmpSymbol.Size = 0; // 0 za sve osim sekcija i 0 do kraja asembleriranja
          tmpSymbol.Ndx = index_of_current_section;
          tmpSymbol.Bind = "LOC";
          
          
          Symbols.push_back(tmpSymbol);
        }
        //dodavanje u realokacionu tebelu
            
        Relocation rel = Relocation();
        rel.Symbol = tmp;
        rel.Offset = Sections[index_of_current_section-1].SectionText.size();
        rel.Type = 0;
        rel.Addend = 0;
        Sections[index_of_current_section-1].RealocationTable.push_back(rel);
        
        //ostavljanje nula da se popuni
        Sections[index_of_current_section-1].SectionText.push_back(0x00);
        Sections[index_of_current_section-1].SectionText.push_back(0x00);
        
      } else if( ret ==ARG_SYMBOL_MEM_PCREL){

        Sections[index_of_current_section-1].SectionText.push_back(0xF7);
        Sections[index_of_current_section-1].SectionText.push_back(0x05);

        string tmp =  lex->YYText();
        tmp = tmp.substr(1);
        //dodavanje u tabelu simbola
        if (symbolIndex(tmp)==-1){ //ako ne postoji dodaj
          Symbol tmpSymbol = Symbol();
          tmpSymbol.Name= tmp;
          tmpSymbol.Num = currNumSimbol++;
          tmpSymbol.Type = "NOTYP";
          tmpSymbol.Size = 0; // 0 za sve osim sekcija i 0 do kraja asembleriranja
          tmpSymbol.Ndx = index_of_current_section-1;
          tmpSymbol.Bind = "LOC";
          tmpSymbol.Value=Sections[index_of_current_section-1].SectionText.size();;

          Symbols.push_back(tmpSymbol);
        }
        //dodavanje u realokacionu tebelu
            
        Relocation rel = Relocation();
        rel.Offset = Sections[index_of_current_section-1].SectionText.size();
        rel.Type = 1;
        rel.Symbol = tmp;
        rel.Addend = -2;
        Sections[index_of_current_section-1].RealocationTable.push_back(rel);
        
        //ostavljanje nula da se popuni
        Sections[index_of_current_section-1].SectionText.push_back(0x00);
        Sections[index_of_current_section-1].SectionText.push_back(0x00);
        
      } else if(ret== JMP_LITERAL_MEM){
        Sections[index_of_current_section-1].SectionText.push_back(0xFF);
        Sections[index_of_current_section-1].SectionText.push_back(0x04);

        string tmp =  lex->YYText();

        int num = getValue(tmp);

        int valueToPush = num & 0xFF;
        Sections[index_of_current_section-1].SectionText.push_back(valueToPush);
        valueToPush = valueToPush>> 8;
        Sections[index_of_current_section-1].SectionText.push_back(valueToPush);

      } else if( ret== OPEN_PARENTHESIS_JMP){
        string reg="";
        int numOfRegister;
        int valueOfOperand2;
        string opearand2="";
        bool instructionWroted = false;
        bool plus = false;
        bool  thirdParam = false;
        while(1){
          ret = lex->yylex();
          if ( ret == OPEN_COMMENT ) {
            if( instructionWroted) break;
            else {
              outputFile->close();
              remove(outFile.c_str());
              delete outputFile;
              cerr << "Error in" << instruction << ": Bad format 12"<< endl;
              exit (-1);
            }
          }
          else if ( ret == ESCAPE_SPACES ) continue;
          else if ( ret == PLUS){
            if(!plus){
              plus = true;
            continue;
            } else {
              outputFile->close();
              remove(outFile.c_str());
              delete outputFile;
              cerr << "Error in" << instruction << ": Bad format 13"<< endl;
              exit (-1);
            }   
          } else if ( ret == REGISTER ){
            if(opearand2==""){
              reg = lex->YYText();
              opearand2=reg;
              valueOfOperand2 = numOfReg(reg.c_str());
              Sections[index_of_current_section-1].SectionText.push_back((15*16 + valueOfOperand2));
            } 
            else{
              outputFile->close();
              remove(outFile.c_str());
              delete outputFile;
              cerr << "Error in" << instruction << ": Bad format 14"<< endl;
              exit (-1);
            }
          } else if(ret == CLOSE_PARENTHESIS){
            if( opearand2!= "" && !plus){
              Sections[index_of_current_section-1].SectionText.push_back(0x02);
              instructionWroted = true;
              break;
            } else if(thirdParam){
              instructionWroted = true;
              break;
            } else{
              outputFile->close();
              remove(outFile.c_str());
              delete outputFile;
              cerr << "Error in" << instruction << ": Bad format 15.1"<< endl;
              exit (-1);
            }
          } else if ( ret == LITERAL){
            if( opearand2!="" && plus){
              thirdParam =true;
              string text =  lex->YYText();
              int  value = getValue(text);
              Sections[index_of_current_section-1].SectionText.push_back(0x03);
              int tmp = (value & 0xff);
              Sections[index_of_current_section-1].SectionText.push_back(tmp);
              value = value >> 8;
              Sections[index_of_current_section-1].SectionText.push_back(tmp);
          
            }else{
              outputFile->close();
              remove(outFile.c_str());
              delete outputFile;
              cerr << "Error in" << instruction << ": Bad format 15.2"<< endl;
              exit (-1);
            }
              
          } else if(ret == SYMBOL){
            if( opearand2!="" && plus){
              thirdParam =true;
            string symbolName = lex->YYText();
            if (symbolIndex(symbolName)==-1){ //ako ne postoji dodaj
              Symbol tmpSymbol = Symbol();
              tmpSymbol.Name= symbolName;
              tmpSymbol.Num = currNumSimbol++;
              tmpSymbol.Type = "NOTYP";
              tmpSymbol.Size = 0; // 0 za sve osim sekcija i 0 do kraja asembleriranja
              tmpSymbol.Ndx = index_of_current_section-1;
              tmpSymbol.Bind = "LOC";
              tmpSymbol.Value=Sections[index_of_current_section-1].SectionText.size();;

              Symbols.push_back(tmpSymbol);
            }
            Relocation relocation = Relocation();
            relocation.Symbol = symbolName;
            Sections[index_of_current_section-1].SectionText.push_back(0x03);
            relocation.Offset = Sections[index_of_current_section-1].SectionText.size();
            relocation.Type = 0;//kad je tip = 1 onda je addent -2
            relocation.Addend = 0;
            
            Sections[index_of_current_section-1].SectionText.push_back(0);
            Sections[index_of_current_section-1].SectionText.push_back(0);
            Sections[index_of_current_section-1].RealocationTable.push_back(relocation);
            }
          } else{
            outputFile->close();
            remove(outFile.c_str());
            delete outputFile;
            cerr << "Error in" << instruction << ": Bad format 16"<< endl;
            exit (-1);
          }
        }    
         
      }  else if( ret ==JMP_REG_DIR){
        string text = lex->YYText();
        int reg = stoi(text.substr(2));
        
        Sections[index_of_current_section-1].SectionText.push_back(0xF0+ reg);
        Sections[index_of_current_section-1].SectionText.push_back(0x01);
      
      } else {
        outputFile->close();
        remove(outFile.c_str());
        delete outputFile;
        cerr << "Error in" << instruction << ": Bad format 17"<< endl;
        exit (-1);
      }

      while(1){
        ret = lex->yylex();
        if( ret == ESCAPE_SPACES) continue;
        else break;
      }

      if( ret != NEW_LINE && ret!=OPEN_COMMENT ) {
        outputFile->close();
        remove(outFile.c_str());
        delete outputFile;
        cerr << "Error in " << instruction << ": Bad format 18"<< endl;
        exit (-1);
      }
    }
  }

  //Kada je zavrseno citanje svih simbola i njihova obrada
  //provera se da li postoji siblo koji nije eksteran a nije definisan je
  for(int i =0; i<Symbols.size() ; i++){
    /*if(!Symbols[i].isExtern && Symbols[i].Ndx ==0){
      outputFile->close();
      remove(outFile.c_str());
      delete outputFile;
      cerr << "Error in" << Symbols[i].Name << ": Bad format 18"<< endl;
      exit (-1);
    }*/
    //Dodeljivanje vlicine simbola u simbol tabeli
    if ( strcmp(Symbols[i].Type.c_str(), "SCTN")==0){
      for( int j = 0 ; j< Sections.size(); j++){
        if(strcmp(Symbols[i].Name.c_str(), Sections[j].Name.c_str())==0){
          Symbols[i].Size= Sections[j].SectionText.size();
        }
      }
    }
  }

  // Postavljanje imena simbola u realokacionoj tabeli u ime sekcije kako bi linker znao koji simbol pripada kojoj sekciji
  for(int i=0 ; i<Sections.size() ; i++ ){
    for( int j = 0 ; j< Sections[i].RealocationTable.size(); j++){
      int sectionNum;
      for (int k= 0 ; k< Symbols.size(); k++){
        if(strcmp(Symbols[k].Name.c_str(), Sections[i].RealocationTable[j].Symbol.c_str())==0  ){
          if(strcmp(Symbols[k].Type.c_str(), "NOTYP")==0 && !(strcmp(Symbols[k].Bind.c_str(),"GLOB")==0)){
            sectionNum=Symbols[k].Ndx;
            for (int m= 0 ; m< Symbols.size(); m++){
              if(sectionNum == Symbols[m].Ndx && strcmp(Symbols[m].Type.c_str(), "SCTN")==0){
                Sections[i].RealocationTable[j].Symbol= Symbols[m].Name; // ime simbola u realokacionoj tabeli postaje ime sekcije
                Sections[i].RealocationTable[j].Addend +=Symbols[k].Value;
                 break;
              }      
            } 
          }
          break;
        }
      }
    }
  } 

  Assembler::printAllSymbols();
  Assembler::printSections();
  outputFile->close();
  delete outputFile;
}