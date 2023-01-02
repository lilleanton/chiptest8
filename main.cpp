#include "chip8.h"
#include "include/SDL.h"

int main(int argc, char** argv){ // Enable command-line arguments

  // Get seed for RND instruction
  srand(time(NULL));

  // Create interpreter and load ROM from command-line
  Chip8::Interpreter chiptest;
  chiptest.loadROM(argv[1]);

  // Initialize graphics, using OpenGL wrapper SDL for the user interface
  SDL_Init(SDL_INIT_VIDEO);
  SDL_Window* win = SDL_CreateWindow(argv[1], SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, Chip8::sWidth*Chip8::pixelx, Chip8::sHeight*Chip8::pixely, SDL_WINDOW_SHOWN);
  SDL_Renderer* ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

  // Get UI 
  SDL_Event e;
  bool quit = false;
  SDL_Rect paintbrush; // SDL_Rect which the programs moves around and draws pixels with
  paintbrush.x = 0;
  paintbrush.y = 0;
  paintbrush.w = Chip8::pixelx;
  paintbrush.h = Chip8::pixely;

  // Enter main loop
  int counter = 0;
  while (!quit){

    // Handle user events
    while (SDL_PollEvent(&e)){
        if (e.type == SDL_QUIT){
            quit = true;
        }
    }
    
    if ((counter % 50000) == 0){
      SDL_SetRenderDrawColor(ren, 0, 0, 0, 255);
      SDL_RenderClear(ren);
      SDL_SetRenderDrawColor(ren, 255, 255, 255, 255);

      for (int i=0; i<Chip8::sHeight; i++){
	for (int j=0; j<Chip8::sWidth; j++){
	  if(chiptest.screenBuf[i][j]){
	    paintbrush.x = j*Chip8::pixelx;
	    paintbrush.y = i*Chip8::pixely;
	    SDL_RenderFillRect(ren, &paintbrush);
	}
      }
      if (chiptest.delayTimer>0) chiptest.delayTimer-=1; // Decrement timer register for instruction execution
      
    }
      SDL_RenderPresent(ren);
    }

    chiptest.exec(); // Execute one instruction
    counter++;
  }

  // Exit program
  SDL_DestroyWindow(win);
  return 0;
}
