#include <stdlib.h>
#include <string.h>
#include <malloc.h>

#include <3ds.h>

#include "drawing.h"

bool usingPro = false;
bool askedForPro = false;

u32 *irmemloc;

u8 *buf;

u32 *transfercount;

void initIr() {
	irmemloc = malloc(sizeof(u32));
	buf = malloc(sizeof(u8));
	transfercount = malloc(sizeof(u32));
	IRU_Initialize(irmemloc, sizeof(irmemloc));
}

void closeIr() {
	IRU_Shutdown();
	free(transfercount);
	free(buf);
	free(irmemloc);
}

void printIr() {

	u32 ir_memloc = *irmemloc;
	u8 bu_f = *buf;
	u32 ir_ansfercount = *transfercount;
	
	bu_f = 0xAA;
	
	IRU_SetIRLEDState(0x1);
	//IRU_RecvData(buf, (u32)sizeof(irmemloc), (u8)0, transfercount, (u32)0x00000010);
	
	drawString(10, 30, "irmemloc: %x", ir_memloc);
	drawString(10, 40, "buf: %x", bu_f);
	drawString(10, 50, "trans: %x", ir_ansfercount);
	
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
			askedForPro = true;
			usingPro = true;
			initIr();
		}
		
		
		gfxFlushBuffers();
		gspWaitForVBlank();
		gfxSwapBuffers();
	}
}