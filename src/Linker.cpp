#include <fstream>
#include <sstream>
#include <stdio.h>
#include <iomanip>
#include "../inc/Linker.h"
#include "../lexer_linker/MyLexerLinker.h"

enum READ{ 
  HEX_VALUE = 0,
  SYMBOL,
  SYMBOLS_TABLE,
  RELOCATION_TABLE,
  SECTIONS_TEXT,
  ESCAPE_SPACES,
  NEW_LINE,
  END_FILE,
  PCREL,
  ABS,
  T
};


vector<Symbol> Linker::Symbols = vector<Symbol>();
vector<Symbol> Linker::localSymbols = vector<Symbol>();
vector<Section> Linker::Sections = vector<Section>();
vector<int> Linker::outputVector = vector<int>();
ofstream* Linker::outputFile = nullptr;
ofstream* Linker::outputFileBinary = nullptr;

int Linker::SymbolIndex(vector<Symbol> Symbols, string symbolName){
  for(int i = 0; i< Symbols.size() ; i++){
    if( strcmp(symbolName.c_str(), Symbols[i].Name.c_str())==0){
      return i;
    }
  }
  return -1;
}

int Linker::SectionIndex(vector<Section> Sections, string sectionName){
  for(int i = 0; i< Sections.size() ; i++){
    if( strcmp(sectionName.c_str(), Sections[i].Name.c_str())==0){
      return i;
    }
  }
  return -1;
}

int Linker::SectionIndexInSymbolTable(vector<Symbol> Symbols, int ndx){
  for(int i = 0; i< Symbols.size() ; i++){
    if( strcmp("SCTN", Symbols[i].Type.c_str())==0 && Symbols[i].Ndx == ndx){
      return i;
    }
  }
  return -1;
}

int Linker::LocalSectionIndexInLocalSymbolTable(vector<Symbol> localSymbols, int ndx , int fileId){
  for(int i = 0 ; i<localSymbols.size(); i++){
    if( localSymbols[i].Ndx == ndx && localSymbols[i].fileId == fileId){
      return i;
    }
  } 
  return -1;
}

int Linker::LocalSectionIndexInLocalSymbolTableNumFileId(vector<Symbol> localSymbols, int num , int fileId){
  for(int i = 0 ; i<localSymbols.size(); i++){
    if( localSymbols[i].Num == num && localSymbols[i].fileId == fileId){
      return i;
    }
  } 
  return -1;
}

int Linker::LocalSectionIndexInLocalSymbolTablebyName(vector<Symbol> localSymbols, string name  , int fileId){
  for(int i = 0 ; i<localSymbols.size(); i++){
    if( strcmp(localSymbols[i].Name.c_str(), name.c_str())==0 && localSymbols[i].fileId == fileId){
      return i;
    }
  }
  return -1;
}

void Linker::linker_load(vector<string> inputFiles){
  int GlobalSectionIndex=0;
  for ( int i = 0 ; i < inputFiles.size(); i++){ //za svaki fajl
    
    ifstream file;
    file.open(inputFiles[i]);

    if ( !file.is_open()){
      printf("Invalid file as input.\n");
      exit(-1);
    }
    
    string linkerInput = "";
    
    while(!file.eof()){
      string tmp;
      getline(file,tmp); //Get 1 line from the file and place into temp
      linkerInput = linkerInput + tmp + '\n';
    }

    file.close();
    
    istringstream  flex_data(linkerInput);
    yyFlexLexer *lex =new yyFlexLexer(&flex_data, &cout);
    int ret = 0;
    
    while(1){
      
      ret = lex->yylex();
      

      if(ret == END_FILE) break;
      else if (ret == SYMBOLS_TABLE ){
        
        ret = lex->yylex(); // novi red
        
        while (1){
          ret = lex->yylex(); // cita regex
          if ( ret !=NEW_LINE && ret!=ESCAPE_SPACES && ret !=SYMBOL && ret!=HEX_VALUE) break;
          
          Symbol symbol = Symbol();
          
          symbol.Num = stoi(lex->YYText(),nullptr, 16);

          ret = lex->yylex();
          ret = lex->yylex();
          symbol.Value  = stoi(lex->YYText(),nullptr, 16);

          ret = lex->yylex();
          ret = lex->yylex();
          symbol.Size  = stoi(lex->YYText(),nullptr, 16);

          ret = lex->yylex();
          ret = lex->yylex();
          symbol.Type  = lex->YYText();

          ret = lex->yylex();
          ret = lex->yylex();
          symbol.Bind  = lex->YYText();

          ret = lex->yylex();
          ret = lex->yylex();
          symbol.Ndx  = stoi(lex->YYText(),nullptr, 16);

          ret = lex->yylex();
          ret = lex->yylex();
          symbol.Name = lex->YYText();

          ret = lex->yylex();
          ret = lex->yylex();
          symbol.isExtern = stoi(lex->YYText(),nullptr, 16);
          symbol.fileId=i;
          
                    //cerr<< hex <<symbol.Num<<'\t'<<symbol.Value<<'\t'<<symbol.Size<<'\t'<<symbol.Type<<'\t'<<symbol.Bind<<'\t'<<symbol.Ndx<<'\t'<<symbol.Name<<'\n';  
          ret=lex->yylex();
          if(strcmp(symbol.Bind.c_str(), "GLOB")==0){
            if( SymbolIndex(Symbols, symbol.Name) == -1){
              Symbols.push_back(symbol);
            } else{
              int index = SymbolIndex(Symbols, symbol.Name);
              if( Symbols[index].Ndx == 0){
                if(symbol.Ndx!=0){
                  Symbols[index].Ndx = symbol.Ndx;
                  Symbols[index].Value = symbol.Value;
                  if(symbol.isExtern == 0){
                    Symbols[index].fileId= i;
                  }
                }
              } else{
                if(symbol.Ndx!=0){
                  outputFile->close();
                  //remove(output.c_str());
                  delete &outputFile;
                  cerr << "Symbol "<< symbol.Name << " already defined" << endl;
                  exit(-1);
                }
              }
            }
          } else if (strcmp(symbol.Type.c_str(), "SCTN")==0){
            if( SymbolIndex(Symbols, symbol.Name) == -1){
              symbol.Num=++GlobalSectionIndex;
              localSymbols.push_back(symbol);
              symbol.Bind="GLOB";
              symbol.fileId = i;
              Symbols.push_back(symbol);
            }else{
              int index = SymbolIndex(Symbols, symbol.Name);
              symbol.Value= Symbols[index].Size;
              Symbols[index].Size += symbol.Size;
              Symbols[index].Ndx= symbol.Ndx;
              Symbols[index].fileId= i ;
              
              for(int i = index+1; i < Symbols.size(); i++){//Pomeranje sekcija i simbola ukoliko je ispred ubacena neka nova
                if (strcmp(Symbols[index].Type.c_str(), "SCTN")==0){
                  Symbols[index].Value +=symbol.Size;
                }
              }

              for(int i = 0; i < Symbols.size(); i++){//Pomeranje  simbola ukoliko je ispred ubacena neka nova
                if (strcmp(Symbols[index].Type.c_str(), "NOTYP")==0){
                  if(Symbols[index].Num > symbol.Num)
                    Symbols[index].Value +=symbol.Size;
                }
              }

              symbol.Num=++GlobalSectionIndex;
              Symbols[index].Num = symbol.Num;
              localSymbols.push_back(symbol);
            }
            
            //promena svih simbola koji pripadaju datoj sekcijji u lokalnom fajlu da je ndx iz tabele lokalnih simbola
            
          }   
        }
        
      } 
      if (ret == RELOCATION_TABLE ){
        //Obrada tabele simbola
        for(int k = 0; k< Symbols.size(); k++){
              if (strcmp(Symbols[k].Type.c_str(), "SCTN")==0){
                if(Symbols[k].fileId==i ){//za svaku sekciju iz ovog fajla 
                  for(int m = 0; m< Symbols.size(); m++){
                    if (strcmp(Symbols[m].Type.c_str(), "SCTN")!=0){
                      if(Symbols[k].fileId == Symbols[m].fileId && Symbols[k].Ndx == Symbols[m].Ndx){
                        Symbols[m].Num = Symbols[k].Num;
                      }
                    }
                  }
                }
              }
            }
        while(1){
          if ( ret!=RELOCATION_TABLE) break;
          string nameTmp = lex->YYText();
          int pos = nameTmp.find('.');
          string name = nameTmp.substr(pos+1);
          Section section = Section();
          section.Name= name;
          if(SectionIndex(Sections, name)==-1)
            Sections.push_back(section);
          
          ret = lex->yylex();
          while(1){
            ret = lex->yylex();
            
            
            if ( ret !=NEW_LINE && ret!=ESCAPE_SPACES && ret !=SYMBOL && ret!=HEX_VALUE) break;
            
            Relocation relocation = Relocation();
            
            relocation.Offset= stoi(lex->YYText(),nullptr, 16);
            
            ret = lex->yylex();
            ret = lex->yylex();
            if(ret == PCREL) {
              relocation.Type=0;
            } else {
              relocation.Type=1;
            }

            ret = lex->yylex();
            ret = lex->yylex();
            relocation.Symbol  = lex->YYText();

            ret = lex->yylex();
            ret = lex->yylex();
            string num = lex->YYText();
            relocation.Addend  = stoi(num,nullptr, 16);
            relocation.fileId =i;
            
            Sections.back().RealocationTable.push_back(relocation);
            
            ret=lex->yylex();
            
          }

        }
      }

    if( ret == SECTIONS_TEXT){
        
        while(1){
          
          if(ret!=SECTIONS_TEXT) break;
          string nameTmp = lex->YYText();
          int pos = nameTmp.find('.');
          string name = nameTmp.substr(pos+1);
          int index = SectionIndex(Sections, name);

          ret = lex->yylex();//newline
          
          while(1){
            ret = lex->yylex();
            
            
            if ( ret!=ESCAPE_SPACES && ret !=SYMBOL && ret!=HEX_VALUE) break;
            
            Sections[index].SectionText.push_back(stoi(lex->YYText(),nullptr, 16));
            
            ret = lex->yylex();
            ret = lex->yylex();
            if ( ret ==NEW_LINE) break;
            Sections[index].SectionText.push_back(stoi(lex->YYText(),nullptr, 16));
            
            
            
            ret = lex->yylex();
            ret = lex->yylex();
            if ( ret ==NEW_LINE) break;
            Sections[index].SectionText.push_back(stoi(lex->YYText(),nullptr, 16));
            
            ret = lex->yylex();
            ret = lex->yylex();
            if ( ret ==NEW_LINE) break;
            Sections[index].SectionText.push_back(stoi(lex->YYText(),nullptr, 16));

            ret = lex->yylex();
            ret = lex->yylex();
            if ( ret ==NEW_LINE) break;
            Sections[index].SectionText.push_back(stoi(lex->YYText(),nullptr, 16));

            ret = lex->yylex();
            ret = lex->yylex();
            if ( ret ==NEW_LINE) break;
            Sections[index].SectionText.push_back(stoi(lex->YYText(),nullptr, 16));

            ret = lex->yylex();
            ret = lex->yylex();
            if ( ret ==NEW_LINE) break;
            Sections[index].SectionText.push_back(stoi(lex->YYText(),nullptr, 16));

            ret = lex->yylex();
            ret = lex->yylex();
            if ( ret ==NEW_LINE) break;
            
            Sections[index].SectionText.push_back(stoi(lex->YYText(),nullptr, 16));
            
            ret = lex->yylex();//space
            ret = lex->yylex();//\n
          }
          
        }
      }
    }
  }

  //Da li su svi definisani
  for(int i =1; i< Symbols.size(); i++){
    if( Symbols[i].Ndx==0){
      outputFile->close();
      delete &outputFile;
      cerr << "Symbol "<< Symbols[i].Name << " is not defined" << endl;
      exit(-1);
    }
  }

  //Redjanje svih sekcija (jedna za drugim) i odredivaneje offseta svake
  int counter=0;
  for(int i =1; i< Symbols.size(); i++){
    if( strcmp("SCTN", Symbols[i].Type.c_str())==0 ){
      //postavljanje velicinei offseta svake sekcije
      Symbols[i].Value = counter;
      counter+=Symbols[i].Size;

      //popunjavanje izlaznog vektora
      for( int j = 0; j< Sections.size(); j++){
        if( strcmp( Sections[j].Name.c_str() , Symbols[i].Name.c_str())==0){ //ako za globalnu sekciju u tabeli nadjes sekciju u sekcijama 
        outputVector.insert(end(outputVector), begin(Sections[j].SectionText), end(Sections[j].SectionText)  );
        }
      }
    }
  }

  setSectionOffsets(); 

 
int i=4;

  for( int j = 0; j< Sections.size(); j++){
    for(int k = 0 ; k< Sections[j].RealocationTable.size(); k++){
      int relocationOffset = Sections[j].RealocationTable[k].Offset;//mesto u koje se upisuje
      string name = Sections[j].Name;
      int fileIdRela = Sections[j].RealocationTable[k].fileId;
      int index = LocalSectionIndexInLocalSymbolTablebyName(localSymbols, name, fileIdRela);
      relocationOffset+=localSymbols[index].Value;

      Sections[j].RealocationTable[k].Offset = relocationOffset;
      int symbolIndex = SymbolIndex(Symbols, Sections[j].Name);//nadji simbol u tabeli simbola

      //Vrednost za upisivanje u zapis
      //Trazenje iz kog fajla je simbol



      int val_symIndex = SymbolIndex(Symbols, Sections[j].RealocationTable[k].Symbol);
      int val_num_sym = Symbols[val_symIndex].Num;
      int val_file_sym = Symbols[val_symIndex].fileId;
      int relSymSecIndex = LocalSectionIndexInLocalSymbolTableNumFileId(localSymbols, val_num_sym, val_file_sym); //indeks sekcije u lokalo tabeli simbola
     

     
      if (strcmp(Symbols[val_symIndex].Type.c_str(), "SCTN")!=0){
        Symbols[val_symIndex].Size = localSymbols[relSymSecIndex].Value;
      }
      

      //cout<< name << " " << relSymFile<< "  "  << "index in localSymbols:" << relSymSecIndex << " value:  " << "returned value" << localSymbols[relSymSecIndex].Value<<  endl;
      int value;
      if (strcmp(Symbols[val_symIndex].Type.c_str(), "SCTN")!=0){
        value = Symbols[val_symIndex].Size + Symbols[val_symIndex].Value  ;
      } else {
        value =  Symbols[val_symIndex].Value ;
      }
      
      if(  Sections[j].RealocationTable[k].Type==0){//pcrel
        value -= Sections[j].RealocationTable[k].Offset  - Sections[j].RealocationTable[k].Addend ;
      } else{
        value +=Sections[j].RealocationTable[k].Addend;
      }

      int tmp = (value & 0xff);
      outputVector[relocationOffset] = tmp; 
      value = value >> 8;
      tmp = (value & 0xff);
      outputVector[relocationOffset+1] = tmp; 
    }
    


  }

  printOutputFile();
}

//Postavljanje vrednosti offseta svim lokalnim sekcijama
void Linker::setSectionOffsets(){
  for(int i = 0 ; i < localSymbols.size() ; i++){
    int Section_parents_index = SymbolIndex(Symbols, localSymbols[i].Name);
    localSymbols[i].Value += Symbols[Section_parents_index].Value;
  } 
}

void Linker::printOutputFile(){
  int prefix = 0;
  *outputFile << std::setbase(16);
  for ( int i = 0; i < outputVector.size() ; i++){
    if ( i % 8 == 0) {
      if( i!= 0 ) *outputFile << endl;
      *outputFile << std::setw(4) << std::setfill('0') << prefix;
      *outputFile << ": ";
      prefix += 8;
    }
    *outputFile << std::setw(2) << std::setfill('0') << outputVector[i] << " ";
  }
  outputFile->close();

  for (const auto &i : outputVector)
    {
        uint16_t value = static_cast<uint16_t>(i);
        outputFileBinary->write(reinterpret_cast<char *>(&value), sizeof(value));
    }
    outputFileBinary->close();
   
}

void Linker::linker_start(vector<string> inputFiles, string output){
  outputFile = new ofstream(output + ".txt");
  outputFileBinary = new ofstream(output + ".hex");

  linker_load(inputFiles);
  //printInputFile();
}

void Linker::printInputFile(){
  outputFile = new ofstream("ProcitanaTabelaSimbola.txt");
  *outputFile << "Symbols" << '\n';
  for(int i= 0 ; i< Symbols.size() ; i++){
    *outputFile<< hex <<Symbols[i].Num<<'\t'
      <<Symbols[i].Value<<'\t'<<Symbols[i].Size
      <<'\t'<<Symbols[i].Type<<'\t'<<Symbols[i].Bind
      <<'\t'<<Symbols[i].Ndx<<'\t'<<Symbols[i].Name<<'\n';
  }

  for(int i= 0 ; i< Sections.size() ; i++){
    //zaglavlje sekcije
    *outputFile<< "rela." << Sections[i].Name<<'\n';
    //Tabela relokacija
    for(int j=0;j<Sections[i].RealocationTable.size();j++){
      *outputFile<< hex<<Sections[i].RealocationTable[j].Offset<<'\t' ; 
      if ( Sections[i].RealocationTable[j].Type ==1)  
        *outputFile <<  "16_PCREL" << '\t' ;
      else 
        *outputFile <<  "16_ABS" << '\t' ;
      *outputFile <<Sections[i].RealocationTable[j].Symbol <<'\t' 
        << Sections[i].RealocationTable[j].Addend <<'\n' ;
    }
  }

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

  outputFile->close();
  delete outputFile;
}