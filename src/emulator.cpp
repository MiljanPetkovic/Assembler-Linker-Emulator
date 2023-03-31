#include <fstream>
#include <sstream>
#include <stdio.h>
#include <iomanip>
#include "../inc/emulator.h"

  // #define HEX_VALUE  0
  // #define SYMBOL 1
  // #define SYMBOLS_TABLE 2
  // #define RELOCATION_TABLE 3 
  // #define SECTIONS_TEXT 4
  // #define ESCAPE_SPACES 5 
  // #define NEW_LINE 6
  #define PC 7
  // #define END_FILE 8
  // #define PCREL 9
  // #define ABS 10
  // #define T 11

  vector<uint16_t> Emulator::memory = vector<uint16_t>(65536);
  vector<uint16_t> Emulator::reg = vector<uint16_t>(8);
  uint16_t Emulator::psw = 0;
  ofstream* Emulator::outputFile = nullptr;
  bool Emulator::finished = false;
  //uint16_t Emulator::reg7old=0;
  bool Emulator::error= false;
  int Emulator::programSize = 0;



enum INSTRUCTION{
  HALT = 0,
  INTREG = 16,
  IRET = 32,
  CALL = 48,
  RET = 64,
  JMP =80,
  JEQ = 81,
  JNE = 82,
  JGT = 83,
  XCHG = 96,
  NOT = 128,  
  ADD = 112, 
  SUB = 113,
  MUL = 114,
  DIV = 115,
  CMP = 116,
  AND = 129,
  OR = 130,
  XOR = 131,
  TEST = 132,
  SHL = 144,
  SHR = 145,
  LDR = 160,
  STR = 176
};


void Emulator::emulator_start(vector<uint16_t> data){
  reg[6]=65280; //sp registar

  
  //Prebacivanje ulaznog fajla u memoriju
  for (int i=0 ; i <  data.size(); i++)
  {
    memory[i] = data[i];
  }

  Emulator::programSize = data.size();
  

}

void Emulator::emulator_processing(){
  reg[PC] = (memory[1] << 8) + memory[0];

  while( !finished && !error){

    int ins = memory[reg[PC]];
    reg[PC]++;
    if (ins == HALT)
    {
      halt();
    }
    else if (ins == INTREG)
    { 
      intreg();
    }
    else if (ins == IRET)
    { 
      iret();
    }
    else if (ins == RET)
    {
      ret();
    }
    else if (ins == CALL)
    {
      call();
    }
    else if (ins == JMP)
    {
      jmp();
    }
    else if (ins == JEQ)
    {
      jeq();
    }
    else if (ins == JNE)
    {
      jne();
    }
    else if (ins == JGT)
    {
      jgt();
    }
    else if (ins == XCHG)
    {
      xchg();
    }
    else if (ins == NOT)
    {
      notins();
    }
    else if (ins == ADD)
    {
      add();
    }
    else if (ins == SUB)
    {
      sub();
    }
    else if (ins == MUL)
    {
      mul();
    }
    else if (ins == DIV)
    {
      div();
    }
    else if (ins == CMP)
    {
      cmp();
    }
    else if (ins == ADD)
    {
      andins();
    }
    else if (ins == OR)
    {
      orins();
    }
    else if (ins == XOR)
    {
      xorins();
    }
    else if (ins == TEST)
    {
      test();
    }
    else if (ins == SHL)
    {
      shl();
    }
    else if (ins == SHR)
    {
      shr();
    }
    else if (ins == LDR)
    {
       
      uint16_t registers = memory[reg[PC]];
      reg[PC]++;
      uint16_t code = memory[reg[PC]];
      reg[PC]++;
      ldr(registers, code);
    }
    else if (ins == STR)
    {
      uint16_t registers = memory[reg[PC]];
      reg[PC]++;
      uint16_t code = memory[reg[PC]];
      reg[PC]++;
      str(registers, code);
    }
    else
    {
      error = true;
    }
  }
  if(!error){
    //print();
  }


}

void Emulator::print(){
  //cout<< "Program finished succesfully"<< endl;
  cout<< "--------------------------"<<endl;
  cout<< "Registars: "<< endl;
  
  for(int i = 0; i < 8 ; i++){
    cout<< hex << "REG"<< i << "= "<< reg[i]<< endl;
  }
  //cout<< hex << "PSW" << "= "<< psw<< endl;

  // cout<< "Memory: "<< endl;
  // int prefix = 0;
  // cout<< std::setbase(16);
  // for ( int i = 0; i < Emulator::programSize ; i++){
  //   if ( i % 8 == 0) {
  //     if( i!= 0 ) cout<< endl;
  //     cout << std::setw(4) << std::setfill('0') << prefix;
  //     cout << ": ";
  //     prefix += 8;
  //   }
  //   cout << std::setw(2) << std::setfill('0') << memory[i] << " ";
  // }

  // cout<<"Stek: "<< endl;
  // for( int i = 0xfef0 ; i< 0xfefe ; i++){
  //   cout<< i << ": ";
  //   cout << std::setw(2) << std::setfill('0') <<  memory[i] << " " << endl;
  // }
  
}

void  Emulator::halt(){
  Emulator::finished =true;
}

void  Emulator::intreg(){// push psw; pc <= mem16[(regD mod 8)*2];
  uint16_t value_of_next_pc = reg[PC] + 1;
  
  reg[6]--;
  memory[reg[6]] = value_of_next_pc >> 8;
  reg[6]--;
  memory[reg[6]] = value_of_next_pc & 0xff;

  reg[6]--;
  memory[reg[6]] = psw >> 8;
  reg[6]--;
  memory[reg[6]] = psw & 0xff;


  uint16_t registers = memory[reg[PC]];
  reg[PC]++;

  uint16_t DDDD = registers >> 4;
  reg[PC] = memory[(reg[DDDD] % 8) * 2];
  reg[PC] = reg[7] + (memory[(reg[DDDD] % 8) * 2 + 1] << 8); 
}

void  Emulator::iret(){// pop psw; pop pc;
  //pop psw
  psw = memory[reg[6]];
  reg[6]++;
  psw = psw + (uint16_t)(memory[reg[6]] << 8);
  reg[6]++;

  reg[PC] = Emulator::pop(PC); 
}

void  Emulator::ret(){// pop pc


  reg[PC] = Emulator::pop(PC);


} 


void  Emulator::call(){// push pc; pc <= operand;
  uint16_t value_of_next_pc;
  if (memory[reg[PC]+1] == 2 || memory[reg[PC]+1] == 1  ){
     value_of_next_pc = reg[PC] + 2;
  } else{
     value_of_next_pc = reg[PC] + 4;
  }

  
  
  reg[6]--;
  memory[reg[6]] = value_of_next_pc >> 8;
  reg[6]--;
  memory[reg[6]] = value_of_next_pc & 0xff;


  jmp();
}


void  Emulator::jmp(){//pc operand
  uint16_t registers = memory[reg[PC]];
  reg[PC]++;

  uint16_t SSSS = registers & 0xf;
  uint16_t operand;

  if (SSSS == 15)
  {
    uint16_t code = memory[reg[PC]];
    reg[PC]++;
    if (code == 0)
    {
      operand = memory[reg[PC]];
      reg[PC]++;
      operand = operand + (memory[reg[PC]] << 8);
      reg[PC]++;
    }
    else if (code == 4)
    {
      uint16_t address = memory[reg[PC]];
      reg[PC]++;
      address = address + (memory[reg[PC]] << 8);
      reg[PC]++;
      operand = memory[address] + (memory[address + 1] << 8);
    }
    else
    {
      error = true;
      cout<< "Error in jmp"<< endl;
    }
  }
  else
  {
    int code = memory[reg[PC]];
    reg[PC]++;
    if (SSSS == 7 && code == 5)
    {
      operand = reg[PC] + (memory[reg[PC]]);
      reg[PC]++;
      operand = operand + (memory[reg[PC]] << 8);
      reg[PC]++;
      operand += 2;
    }
    else if (code == 1)
    {
      operand = reg[SSSS];
    }
    else if (code == 2)
    {
      operand = reg[SSSS];
      operand = memory[operand] + (memory[operand + 1] << 8);

    }
    else if (code == 3)
    {
      operand = reg[SSSS];
      uint16_t pom = memory[reg[PC]];
      reg[PC]++;
      pom = pom + (memory[reg[PC]] << 8);
      reg[PC]++;
      operand = memory[operand + pom] + (memory[operand + pom + 1] << 8);
    
    }
    else
    {
      error = true;
      return;
    }
  }
  reg[PC] = operand;

}

void  Emulator::jeq(){//if (equal) pc operand
  if (psw & 0x0001 == 0x0001)
  {
    jmp();
  }
  else
  {
    reg[PC]++;
    int code = memory[reg[7]];
    if (code == 0 || code == 3 || code == 4 || code == 5)
    {
      reg[PC] += 2;
    }
    reg[PC]++;
  }
}

void  Emulator::jne(){//if (NOT equal) pc operand
 if (psw & 0x0001 != 0x0001)
  {
    jmp();
  }
  else
  {
    reg[PC]++;
    int code = memory[reg[7]];
    if (code == 0 || code == 3 || code == 4 || code == 5)
    {
      reg[PC] += 2;
    }
    reg[PC]++;
  }
}


void  Emulator::jgt(){//if (signedgreater) pc operand
  if (!((psw & 0x0001 == 0x0000) && (((psw >> 2) & 0b0010) == (psw & 0b0010))))
  {
    jmp();
  }
  else
  {
    reg[PC]++;
    int code = memory[reg[7]];
    if (code == 0 || code == 3 || code == 4 || code == 5)
    {
      reg[PC] += 2;
    }
    reg[PC]++;
  }
}


void  Emulator::xchg(){
  uint16_t registers = memory[reg[PC]];
  reg[PC]++;

  uint16_t temp;
  uint16_t regS = registers & 0x0f;
  uint16_t regD = registers >> 4;
  
  temp = reg[regD];
  reg[regD] = reg[regS];
  reg[regS] = temp;

}

void  Emulator::notins(){
  uint16_t registers = memory[reg[PC]];
  reg[PC]++;

  uint16_t DDDD = registers >> 4;
  reg[DDDD] = ~reg[DDDD];
}

void  Emulator::add(){//
  uint16_t registers = memory[reg[PC]];
  reg[PC]++;

  uint16_t SSSS = registers & 0x0f;
  uint16_t DDDD = registers >> 4;
 
  reg[DDDD] += reg[SSSS];
 
}


void  Emulator::sub(){
    uint16_t registers = memory[reg[PC]];
  reg[PC]++;

  uint16_t SSSS = registers & 0x0f;
  uint16_t DDDD = registers >> 4;
 
  reg[DDDD] -= reg[SSSS];
}

void  Emulator::mul(){
  uint16_t registers = memory[reg[PC]];
  reg[PC]++;

  uint16_t SSSS = registers & 0x0f;
  uint16_t DDDD = registers >> 4;
 
  reg[DDDD] *= reg[SSSS];
}

void  Emulator::div(){
    uint16_t registers = memory[reg[PC]];
  reg[PC]++;

  uint16_t SSSS = registers & 0x0f;
  uint16_t DDDD = registers >> 4;
 
  reg[DDDD] /= reg[SSSS];
}

void  Emulator::cmp(){
    uint16_t registers = memory[reg[PC]];
  reg[PC]++;

  uint16_t SSSS = registers & 0x0f;
  uint16_t DDDD = registers >> 4;
 
  uint16_t ds = reg[DDDD] - reg[SSSS];

  if (ds == 0)
  {
    psw = psw | 0x0001;
  }
  else
  {
    psw = psw & 0xfffe;
  }

  if (ds > 0x7fff)
  {
    psw = psw | 0x0008;
  }
  else
  {
    psw = psw & 0xfff7;
  }

  if ((reg[DDDD] <= 0x7fff && reg[SSSS] > 0x7fff && ds > 0x7fff) || (reg[DDDD] > 0x7fff && reg[SSSS] <= 0x7fff && ds <= 0x7fff))
  {
    psw = psw | 0x0002;
  }
  else
  {
    psw = psw & 0xfffd;
  }

  uint16_t checkD = (uint16_t)reg[DDDD];
  uint16_t checkS = (uint16_t)reg[SSSS];
  if ((checkD - checkS) & 0x00010000 == 0x00010000)
  {
    psw = psw | 0x0004;
  }
  else
  {
    psw = psw & 0xffffb;
  }
}

void  Emulator::andins(){
   uint16_t registers = memory[reg[PC]];
  reg[PC]++;

  uint16_t SSSS = registers & 0x0f;
  uint16_t DDDD = registers >> 4;
 
  reg[DDDD] &= reg[SSSS];
}

void  Emulator::orins(){
 uint16_t registers = memory[reg[PC]];
  reg[PC]++;

  uint16_t SSSS = registers & 0x0f;
  uint16_t DDDD = registers >> 4;
 
  reg[DDDD] |= reg[SSSS];
}

void  Emulator::xorins(){
  uint16_t registers = memory[reg[PC]];
  reg[PC]++;

  uint16_t SSSS = registers & 0x0f;
  uint16_t DDDD = registers >> 4;
 
  reg[DDDD] ^= reg[SSSS];
}

void  Emulator::test(){
  uint16_t registers = memory[reg[PC]];
  reg[PC]++;

  uint16_t SSSS = registers & 0x0f;
  uint16_t DDDD = registers >> 4;
 
  uint16_t temp= reg[DDDD] & reg[SSSS];
  if (temp == 0)
    {
      psw = psw | 0x0001;
    }
    else
    {
      psw = psw & 0xfffe;
    }
    if (temp > 0x7fff)
    {
      psw = psw | 0x0008;
    }
    else
    {
      psw = psw & 0xfff7;
    }
}

void  Emulator::shl(){
  uint16_t registers = memory[reg[PC]];
  reg[PC]++;

  uint16_t SSSS = registers & 0x0f;
  uint16_t DDDD = registers >> 4;

  bool carry = false;
  
  for (int i = 0; i < reg[SSSS]; i++)
  {
    if (reg[DDDD] & 0x8000 == 0x8000)
      carry = true;
    else
      carry = false;

    reg[DDDD] = reg[DDDD] << 1;
  }

  if (reg[DDDD] == 0)
  {
    psw = psw | 0x0001;
  }
  else
  {
    psw = psw & 0xfffe;
  }

  if (reg[DDDD] > 0x7fff)
  {
    psw = psw | 0x0008;
  }
  else
  {
    psw = psw & 0xfff7;
  }

  if (carry)
  {
    psw = psw | 0x0004;
  }
  else
  {
    psw = psw & 0xffffb;
  }
}
void  Emulator::shr(){
  uint16_t registers = memory[reg[PC]];
  reg[PC]++;

  uint16_t SSSS = registers & 0x0f;
  uint16_t DDDD = registers >> 4;

  bool carry = false;

  for (int i = 0; i < reg[SSSS]; i++)
    {
      if (reg[DDDD] & 0x0001 == 0x0001)
        carry = true;
      else
        carry = false;
      if (reg[DDDD] & 0x8000 == 0x8000)
      {
        reg[DDDD] = (reg[DDDD] >> 1) | 0x8000;
      }
      else
      {
        reg[DDDD] = reg[DDDD] >> 1;
      }
    }
    // setovanje flegova
    if (reg[DDDD] == 0)
    {
      psw = psw | 0x0001;
    }
    else
    {
      psw = psw & 0xfffe;
    }
    if (reg[DDDD] > 0x7fff)
    {
      psw = psw | 0x0008;
    }
    else
    {
      psw = psw & 0xfff7;
    }
    if (carry)
    {
      psw = psw | 0x0004;
    }
    else
    {
      psw = psw & 0xffffb;
    }
}





void Emulator::push(uint16_t r){
  reg[6]--;
  memory[reg[6]] = reg[r] >> 8;
  reg[6]--;
  memory[reg[6]] = reg[r] & 0xff;
}

uint16_t Emulator::pop(uint16_t r){
  uint16_t operand = memory[reg[6]];
  reg[6]++;
  operand = operand + (memory[reg[6]] << 8);
  reg[6]++;
  return operand;
}

void  Emulator::str(uint16_t r, uint16_t c){

  uint16_t registers = r;
  uint16_t DDDD = registers >> 4;
  uint16_t SSSS = registers & 0x0f;
  
  if (DDDD < 0 || DDDD>7){
    error= true;
    return;
  }

  uint16_t operand;
  uint16_t code = c;

  if (code == 1 && SSSS >= 0 && SSSS < 8)
  {
    reg[SSSS] = reg[DDDD];
  }
  else if (code == 2 && SSSS >= 0 && SSSS < 8)
  {
    memory[reg[SSSS]] = reg[DDDD] & 0xff;
    memory[reg[SSSS] + 1] = reg[DDDD] >> 8;
  }
  else if (code == 3 && SSSS >= 0 && SSSS < 8)
  {
    uint16_t address = reg[SSSS] + memory[reg[PC]];
    reg[PC]++;
    address = address + (memory[reg[PC]] << 8);
    reg[PC]++;
    memory[address] = reg[DDDD] & 0xff;
    memory[address + 1] = reg[DDDD] >> 8;
  }
  else if ( code == 4 && SSSS == 15 )
  {
    operand = memory[reg[7]];
    reg[PC]++;
    operand = operand + (memory[reg[PC]] << 8);
    reg[PC]++;
    memory[operand] = reg[DDDD] & 0xff;
    memory[operand + 1] = reg[DDDD] >> 8;
  }
  else  if (code == 18 && SSSS == 6)
  {
    push(DDDD);
  }
  else
  {
    error = true;
    cout<< "Error in str" << endl;
  }
}

void  Emulator::ldr(uint16_t r, uint16_t c){
  uint16_t registers = r;
  uint16_t DDDD = registers >> 4;
  uint16_t SSSS = registers & 0x0f;
  
  if (DDDD < 0 || DDDD>7){
    error= true;
    return;
  }

  uint16_t operand;
  uint16_t code = c;

  if (code == 0 && SSSS == 15)
  {
    operand = memory[reg[PC]];
    reg[PC]++;
    operand = operand + (memory[reg[PC]] << 8);
    reg[PC]++;
  }
  else if (code == 1 && SSSS >= 0 && SSSS < 8)
  {
    operand = reg[SSSS];
  }
  else if (code == 2 && SSSS >= 0 && SSSS < 8)
  {
    operand = memory[reg[SSSS]] + (memory[reg[SSSS] + 1] << 8);
  }
  else if (code == 3 && SSSS >= 0 && SSSS < 8)
  {
    uint16_t address = reg[SSSS] + memory[reg[PC]];
    reg[PC]++;
    address = address + (memory[reg[PC]] << 8);
    reg[PC]++;
    operand = memory[address] + (memory[address + 1] << 8);
  }
  else if (code == 4 && SSSS == 15 )
  {
    uint16_t address = memory[reg[PC]];
    reg[PC]++;
    address = address + (memory[reg[PC]] << 8);
    reg[PC]++;
    operand = memory[address] + (memory[address + 1] << 8);
  }
  else if (code == 5 && SSSS == 7  )
  {
    operand = reg[PC] + memory[reg[PC]];
    reg[PC]++;
    operand = operand + (memory[reg[PC]] << 8);
    reg[PC]++;
  }
  else if (code == 66 && SSSS == 6)
  {
    operand = Emulator::pop(DDDD);
  }
  else
  {
    error = true;
    cout<< "Error in ldr" << endl;
    return;
  }
   reg[DDDD] = operand;
}



