#ifndef _emulator_h_
#define _emulator_h_

#include <vector>
#include <string>
#include <iostream>
#include <algorithm>
#include <unordered_map>

using namespace std;

class Emulator{
  public:
    static ofstream*   outputFile;
    static vector<uint16_t> memory;
    static vector<uint16_t> reg;
    static uint16_t psw;

    static bool finished;
    static bool error;
    static int programSize;




    static void emulator_start(vector<uint16_t> data);
    static void emulator_processing();

    static void print();
    static void  halt();
    static void  intreg();
    static void  iret();
    static void  ret();
    static void  call();
    static void  jmp();
    static void  jeq();
    static void  jne();
    static void  jgt();
    static void  xchg();
    static void  notins();
    static void  add();
    static void  sub();
    static void  mul();
    static void  div();
    static void  cmp();
    static void  andins();
    static void  orins();
    static void  xorins();
    static void  test();
    static void  shl();
    static void  shr();
    static void  ldr(uint16_t r, uint16_t c);
    static void  str(uint16_t r, uint16_t c);
    static void push(uint16_t );
    static uint16_t pop(uint16_t );
};

#endif