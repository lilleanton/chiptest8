#include "chip8.h"

// Chip-8 reference from "Chip-8 Design Specification" By Kling, Oliver, Taylor, Watkins. Columbia University (March 24, 2016)
Chip8::Interpreter::Interpreter(){
    // Initialize font
    int idx = 0;
    for (auto i :
	   {0xf999f, 0x72262, 0xF8F1F, 0xF1F1F,
	    0x99f11, 0xF1F8F, 0xF9F8F, 0x4421F,
	    0xF9F9F, 0xF1F9F, 0x99F9F, 0xE9E9E,
	    0xF888F, 0xE999E, 0xF8F8F, 0x88F8F}){
      for (int j=0; j<5; j++){
	this->RAM[idx] = ((i>>j*4)&0xF)*0x10;
	idx++;
      }
    }

    // Entry point for Chip-8 programs are at $0200, $0000 - $0200 is traditionally reserved for the interpreter
    PC = 0x200;
    lastop = 0;
    this->clearBuf();
  }

// Print interpreter status for debugging purposes
void Chip8::Interpreter::printStatus(){
    printf("PC - %x", PC);
    printf("\n");
    printf("OP - %x\n", lastop);
    printf("I - %x\n", I);
    printf("V 0->f - ");
    for (int i=0; i<16; i++) printf("%x, ", V[i]);
    printf("\nStack - ");
    for (int i=0; i<16; i++) printf("%x, ", Stack[i]);
    printf("SP=%i\n", SP);
    printf("DT - %i\n\n", this->delayTimer);
}

// Clear drawing surface
void Chip8::Interpreter::clearBuf(){
    for (int i=0; i<sHeight; i++)
      for (int j=0; j<sWidth; j++)
	screenBuf[i][j] = false;
}

// Execute one instruction, instructions are documented with their respective mnemonic.
// These are conventional Chip-8 mnemonics with their roots in assembly language, although
// not actual machine code.
void Chip8::Interpreter::exec(){
    unsigned opcode = RAM[PC]*0x100 + RAM[PC+1]; PC+=2; // Read two-byte opcode
    lastop = opcode;

    // Define opcode aliases
    const unsigned nnn = opcode & 0xFFF;
    const unsigned n   = opcode & 0xF;
    const unsigned u   = (opcode>>12)& 0xF;
    const unsigned x   = (opcode>>8) & 0xF;
    const unsigned y   = (opcode>>4) & 0xF;
    const unsigned kk  = opcode & 0xFF;

    if (opcode == 0x00E0){ this->clearBuf(); } // CLS
    else if (opcode == 0x00EE){ PC = Stack[SP]; SP--;} // RET
    else if (u == 0x1){ PC = nnn;} // JMP
    else if (u == 0x2){ SP++; Stack[SP] = PC; PC = nnn;} // CALL addr
    else if (u == 0x3){ if (V[x] == kk)   PC += 2;} //      SE Vx =  BYTE
    else if (u == 0x4){ if (V[x] != kk)   PC += 2;} //      SE Vx =! BYTE
    else if (u == 0x5){ if (V[x] == V[y]) PC += 2;} //      SE Vx = Vy
    else if (u == 0x6){     V[x] = kk;}  //                 LD Vx, BYTE
    else if (u == 0x7){ V[x] += kk;}//                      ADD Vx, BYTE
    else if (u == 0x8 && n == 0x0){ V[x] = V[y];}//         LD Vx, Vy
    else if (u == 0x8 && n == 0x1){ V[x] = V[x] | V[y];} // OR Vx, Vy
    else if (u == 0x8 && n == 0x2){ V[x] = V[x] & V[y];} // AND Vx, Vy
    else if (u == 0x8 && n == 0x3){ V[x] = V[x] ^ V[y];} // XOR Vx, Vy
    else if (u == 0x8 && n == 0x4){ int temp = int(V[x]);int res = int(V[x]) + int(V[y]); // ADD Vx, Vy
      if (res > 0xFF) V[0xF] = (byte)1;
      else V[0xF] = (byte)0;
      V[x] = res & 0xFF;}
    else if (u == 0x8 && n == 0x5){ // SUB Vx, Vy
	if ((int)V[x] > (int)V[y]) V[0xF] = (byte)1;
	else {V[0xF] = (byte)0;}
	V[x] -= V[y];}
    else if (u == 0x8 && n == 0x6){ // SHR Vx, Vy
	if (int(V[x] & 0x1) == 1) V[0xF] = (byte)1;
	else V[0xF] = (byte)0;
	V[x] = V[x]/2;}
    else if (u == 0x8 && n == 0x7){ // SUBN Vx, Vy
	if (V[y] > V[x]) V[0xF] = (byte)1;
	else V[0xF] = (byte)0;
	V[x] = V[y] - V[x];}
    else if (u == 0x8 && n == 0xE){ // SHL Vx, Vy
	if (V[x] >> 7) V[0xF] = (byte)1;
	else V[0xF] = (byte)0;
	V[x] *= 2;}
    else if ( u == 0x9 && n == 0){if (V[x] != V[y]) PC += 2;} // SNE Vx, Vy
    else if ( u == 0xA){I = nnn;} //                             LD I, Addr
    else if ( u == 0xB){PC = nnn + V[0];} //                     JP V0, Addr
    else if ( u == 0xC){V[x] = (rand() % 0xFF) & kk;} //         RND Vx, BYTE
    else if ( u == 0xD){ //                                      DRW, Vx, Vy
	const int spriteWidth = 8;
	const int spriteHeight = n;
	V[0xF] = 0;
	
	bool spriteData[spriteHeight][spriteWidth];
	for (int i=0; i<spriteHeight; i++)
	  for (int j=0; j<spriteWidth; j++)
	    spriteData[i][j] = (((long)RAM[I+i])<<j) & 0x80;

	for (int i=0; i<spriteHeight; i++){
	  for (int j=0; j<spriteWidth; j++){
	    bool old = screenBuf[(i+int(V[y]))%sHeight][(j+int(V[x]))%sWidth];
	    screenBuf[(i+int(V[y]))%sHeight][(j+int(V[x]))%sWidth] = old ^ spriteData[i][j];
	    if (old != screenBuf[(i+int(V[y]))%sHeight][(j+int(V[x]))%sWidth])
	      V[0xF] = (byte)1;
	  }
	}
    }
    else if ( u == 0xE && kk == 0x9E){}                        // SKP Vx
    else if ( u == 0xE && kk == 0xA1){ PC+=2;}                 // SKNP Vx
    else if ( u == 0xF && kk == 0x07){ V[x] = delayTimer;}     // LD Vx, DT
    else if ( u == 0xF && kk == 0x0A){ std::cin >> V[x];}      // LD Vx, K (Key)
    else if ( u == 0xF && kk == 0x15){ delayTimer = V[x];}     // LD DT, Vx
    else if ( u == 0xF && kk == 0x18){ soundTimer = V[x];}     // LD ST, Vx
    else if ( u == 0xF && kk == 0x1E){ I += V[x];}             // ADD I, Vx
    else if ( u == 0xF && kk == 0x29){ I = this->RAM[V[x]*5];} // LD F, Vx
    else if ( u == 0xF && kk == 0x33){                         // LD B, Vx
	int hundred = floor(V[x]/100);
	int tenth   = floor((V[x]-hundred*100)/10);
	int first   = V[x] - hundred*100 - tenth*10;
	this->RAM[I] = hundred; this->RAM[I+1] = tenth; this->RAM[I+2] = first;
      }
    else if (u == 0xF && kk == 0x55){for (int i=0; i<=x; i++) this->RAM[I+i] = V[i];} // LD [I], Vx
    else if (u == 0xF && kk == 0x65){for (int i=0; i<=x; i++) V[i] = this->RAM[I+i];} // LD Vx, [I] (Parenthesis implies memory location)
    else { printf("Unknown opcode %x\n", opcode); }
}

// Load ROM
void Chip8::Interpreter::loadROM(std::string path){
    int idx = 0x200;
    for(std::ifstream file(path, std::ios::binary);; ){
      RAM[idx & 0xFFF] = file.get();
      idx++;

      if (idx >= 0x1000)
	break;
    }
}
