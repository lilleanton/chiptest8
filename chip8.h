#include <stdio.h>
#include <cstdlib>
#include <ctime>
#include <initializer_list>
#include <cmath>
#include <iostream>
#include <fstream>
#include <chrono>

// Chip-8 reference from "Chip-8 Design Specification" By Kling, Oliver, Taylor, Watkins. Columbia University (March 24, 2016)

namespace Chip8 {
  // Define specifications for the Chip-8 interpreter
  typedef unsigned char byte;
  const int sWidth = 64;
  const int sHeight = 32;
  const int pixelx = 6;
  const int pixely = 8;

  // Interpreter
  struct Interpreter{
    // Registers
    byte RAM[0x1000] = {0};
    byte V[16] = {0}, delayTimer=0, soundTimer=0, keys[16];
    unsigned int I = 0;
    unsigned short PC = 0;
    unsigned short SP = 0;
    unsigned short Stack[16] = {0};
    bool screenBuf[sHeight][sWidth];
    unsigned lastop;

    Interpreter();

    // Entry point for Chip-8 programs are at $0200, $0000 - $0200 is traditionally reserved for the interpreter

    // Print interpreter status for debugging purposes
    void printStatus();

    // Clear drawing surface
    void clearBuf();

    // Execute one instruction, instructions are documented with their respective mnemonic.
    // These are conventional Chip-8 mnemonics with their roots in assembly language, although
    // not actual machine code.
    void exec();

    // Load ROM
    void loadROM(std::string path);
  };
  
}
