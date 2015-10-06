#include <stdlib.h>
#include <string.h>
#include <malloc.h>

#include <3ds.h>

#include "drawing.h"

bool usingPro = false;
bool askedForPro = false;
bool hasIr = false;

int frame = 0; // if the framerate is 60, every time we hit 60, 1 second has passed. 

u32 *irmemloc;

Result MASTER;

void initIr() {
	irmemloc = (u32*) malloc(0x1000);
	MASTER = IRU_Initialize(irmemloc, 0x1000);
	if(MASTER != 0) {
        free(irmemloc);
        irmemloc = NULL;
	}
	hasIr = MASTER == 0;
}

void closeIr() {
    if(hasIr) {
        IRU_Shutdown();
        if(irmemloc != NULL) {
            free(irmemloc);
            irmemloc = NULL;
        }
        hasIr = false;
    }
}

void printIr() {
	if(frame % 60 == 0) {
		IRU_SetIRLEDState(1); //send 1 time per 60 fps. Should (should) turn on CPP.
		drawString(10, 20, "SENT: %x", MASTER);
		frame++;
	} else {
		IRU_SetIRLEDState(0);
		drawString(10, 20, "GOT: %x", MASTER);
		frame++;
	}
	
}

void askForCirclePadPro() {
	while(!askedForPro) {
		hidScanInput();
		clearScreen();
		drawString(10, 10, "Do you want to use the Circle Pad Pro?");
		
		drawString(10, 30, "(Using the CPP may cause unintentional");
		drawString(10, 40, "side effects!) DO NOT USE ON THE n3DS!");
		
		drawString(10, 60, "(A) - Turn OFF the CPP (Recommended)");
		drawString(10, 70, "(L + R + Y) - Turn on the CPP");	
		
		
		drawString(10, 100, "Stop this message in the .ini file.");		
		
		u32 kHeld = hidKeysHeld();
		if((kHeld & KEY_A)) {
			askedForPro = true;
			usingPro = false;
		}
		if((kHeld & KEY_L) && (kHeld & KEY_R) && (kHeld & KEY_Y)) {
			initIr();
			if(hasIr) {
				askedForPro = true;
				usingPro = true;
			} else {
				bool err = true;
				while(err) {
					hidScanInput();
					clearScreen();
					drawString(10, 10, "Something happened...");
					drawString(10, 20, "Error: %x", MASTER);
					drawString(10, 60, "(A) - Turn OFF the CPP (Recommended)");
					kHeld = hidKeysHeld();
					if((kHeld & KEY_A)) {
						askedForPro = true;
						usingPro = false;
						err = false;
					}
					gfxFlushBuffers();
					gspWaitForVBlank();
					gfxSwapBuffers();
				}
			}
		}
		
		
		gfxFlushBuffers();
		gspWaitForVBlank();
		gfxSwapBuffers();
	}
}