// 3DS Controller Server

#define VERSION 0.6

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

#include "wireless.h"
#include "keys.h"
#include "general.h"
#include "joystick.h"
#include "settings.h"
#include "keyboard.h"

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR lpCmd, int nShow) {
	printf("3DS Controller Server %.1f\n", VERSION);
	
	DWORD screenWidth = GetSystemMetrics(SM_CXSCREEN);
	DWORD screenHeight = GetSystemMetrics(SM_CYSCREEN);
	
	double widthMultiplier = screenWidth / 320.0;
	double heightMultiplier = screenHeight / 240.0;
	
	bool vJoy = true;
	UINT iInterface = 1;
	
	iReport.wAxisX = JOY_MIDDLE;
	iReport.wAxisY = JOY_MIDDLE;
	iReport.wAxisZ = JOY_MIDDLE;
	iReport.wAxisXRot = JOY_MIDDLE;
	iReport.wAxisYRot = JOY_MIDDLE;
	iReport.wAxisZRot = JOY_MIDDLE;
	iReport.wSlider = JOY_MIDDLE;
	iReport.wDial = JOY_MIDDLE;
	iReport.lButtons = 0;
	iReport.bHats = -1;
	
	if(vJoy && !vJoyEnabled()) {
		printf("vJoy failed (1)! Buttons will still work, but joy stick won't work.\n");
		vJoy = false;
	}
	
	enum VjdStat status = GetVJDStatus(iInterface);
	if(vJoy && (status == VJD_STAT_OWN || (status == VJD_STAT_FREE && !AcquireVJD(iInterface)))) {
		printf("vJoy failed (2)! Buttons will still work, but joy stick won't work.\n");
		vJoy = false;
	}
	
	ContPovNumber = GetVJDContPovNumber(iInterface);
	//int DiscPovNumber = GetVJDDiscPovNumber(iInterface);
	
	if(vJoy && !updateJoystick()) {
		printf("vJoy failed (3)! Buttons will still work, but joystick won't work.\n");
		vJoy = false;
	}
	
	if(!readSettings()) {
		printf("Couldn't read settings file, using default key bindings.\n");
	}
	
	initNetwork();
	
	char nButtons = GetVJDButtonNumber(iInterface);
	if(nButtons <16) printf("Your vJoy has less than 16 buttons (8 by default), some may not work!\n");
	
	printf("Port: %d\n", settings.port);
	
	printf("Running on: %s\n", hostName);
	
	printf("Your local IP(s):\n");
	printIPs();
	
	printf("\n");
	
	startListening();
	
	while(1) {
		memset(&buffer, 0, sizeof(struct packet));
		
		while(receiveBuffer(sizeof(struct packet)) <= 0) {
			// Waiting
			
			Sleep(settings.throttle);
		}
		
		keyboardActive = buffer.keyboardActive;
		
		switch(buffer.command) {
			case CONNECT:
				lastKeys = 0;
				currentKeys = 0;
				circlePad.x = 0;
				circlePad.y = 0;
				lastTouch.x = 0;
				lastTouch.y = 0;
				currentTouch.x = 0;
				currentTouch.y = 0;
				cStick.x = 0;
				cStick.y = 0;
				
				buffer.command = CONNECT;
				printf("3DS Connected!\n");
				
				Sleep(50);
				sendBuffer(1);
				
				Sleep(50);
				sendBuffer(1);
				
				Sleep(50);
				sendBuffer(1);
				break;
			
			case KEYS:
				lastKeys = currentKeys;
				if(currentKeys & KEY_TOUCH) lastTouch = currentTouch;
				
				memcpy(&currentKeys, &buffer.keys, 4);
				memcpy(&circlePad, &buffer.circlePad, 4);
				memcpy(&currentTouch, &buffer.touch, 4);
				memcpy(&cStick, &buffer.cStick, 4);
				memcpy(&volume, &buffer.volume, 4);
				//printf("\rVolume is currently: %x ", volume); //test
				
				handleKey(KEY_A, settings.A);
				handleKey(KEY_B, settings.B);
				handleKey(KEY_SELECT, settings.Select);
				handleKey(KEY_START, settings.Start);
				if(!settings.isUsingPov) { //Handle normally if not using POV in settings.
					handleKey(KEY_DRIGHT, settings.Right);
					handleKey(KEY_DLEFT, settings.Left);
					handleKey(KEY_DUP, settings.Up);
					handleKey(KEY_DDOWN, settings.Down);
				}
				handleKey(KEY_R, settings.R);
				handleKey(KEY_L, settings.L);
				handleKey(KEY_ZR, settings.ZR);
				handleKey(KEY_ZL, settings.ZL);
				handleKey(KEY_X, settings.X);
				handleKey(KEY_Y, settings.Y);
				
				//handleKey(KEY_LID, 'I');
				
				if(newpress(KEY_TOUCH)) {
					lastTouch.x = currentTouch.x;
					lastTouch.y = currentTouch.y;
				}
				
				if((currentKeys & KEY_TOUCH)) {
					if(keyboardActive) {
						if(newpress(KEY_TOUCH)) {
							char letter = currentKeyboardKey();
							if(letter) {
								simulateKeyNewpress(letter);
								simulateKeyRelease(letter);
							}
						}
					}
					else if(settings.touch == mouse) {
						if(settings.mouseSpeed) {
							POINT p;
							GetCursorPos(&p);
							SetCursorPos(p.x + (currentTouch.x - lastTouch.x) * settings.mouseSpeed, p.y + (currentTouch.y - lastTouch.y) * settings.mouseSpeed);
						}
						else {
							SetCursorPos((int)((double)currentTouch.x * widthMultiplier), (int)((double)currentTouch.y * heightMultiplier));
						}
					}
					else if(settings.touch == joystick1) { //made a little bit more accurate to the screen size.
						joyX = (int)((float)(currentTouch.x) * 102.3f);
						joyY = (int)((float)(currentTouch.y) * 136.5f);
					}
					
					else if(settings.touch == joystick2) {
						joyRX = (int)((float)(currentTouch.x) * 102.3f);
						joyRY = (int)((float)(currentTouch.y) * 136.5f);
					}
					else {
						handleKey(KEY_TOUCH, settings.Tap);
					}
				} else { //If we are not touching, move to center (Like if you release the joystick on a normal controller).
					if(settings.touch == joystick1) {
						joyX = 16383; //Halfway between the x
						joyY = 16383; //Halfway between the y
					}
					
					else if(settings.touch == joystick2) {
						joyRX = 16383; //Halfway between the rx
						joyRY = 16383; //Halfway between the ry
					}
				}
				
				if(settings.circlePad == mouse) {
					if(abs(circlePad.x) < settings.mouseSpeed * 3) circlePad.x = 0;
					if(abs(circlePad.y) < settings.mouseSpeed * 3) circlePad.y = 0;
					
					POINT p;
					GetCursorPos(&p);
					SetCursorPos(p.x + (circlePad.x * settings.mouseSpeed) / 32, p.y - (circlePad.y * settings.mouseSpeed) / 32);
				}
				else if(settings.circlePad == joystick1) {
					joyX = (circlePad.x + 128) * 128;
					joyY = (128 - circlePad.y) * 128;
				}
				
				else if(settings.circlePad == joystick2) {
					joyRX = (circlePad.x + 128) * 128;
					joyRY = (128 - circlePad.y) * 128;
				}
				
				if(settings.cStick == mouse) {
					if(abs(cStick.x) < settings.mouseSpeed * 3) cStick.x = 0;
					if(abs(cStick.y) < settings.mouseSpeed * 3) cStick.y = 0;
					
					POINT p;
					GetCursorPos(&p);
					SetCursorPos(p.x + (cStick.x * settings.mouseSpeed) / 32, p.y - (cStick.y * settings.mouseSpeed) / 32);
				}
				
				else if(settings.cStick == joystick1) {
					joyX = (cStick.x + 128) * 128;
					joyY = (128 - cStick.y) * 128;
				}
				
				else if(settings.cStick == joystick2) {
					joyRX = (cStick.x + 128) * 128;
					joyRY = (128 - cStick.y) * 128;
				}
				
				
				if(settings.isUsingPov) {
					if((currentKeys & KEY_DUP) && !(currentKeys & KEY_DLEFT)) {
						if((currentKeys & KEY_DRIGHT)) {
							povHat = 4500;
						} else {
							povHat = 0;
						}
					} else if((currentKeys & KEY_DRIGHT)) {
						if((currentKeys & KEY_DDOWN)) {
							povHat = 13500;
						} else {
							povHat = 9000;
						}
					} else if((currentKeys & KEY_DDOWN)) {
						if((currentKeys & KEY_DLEFT)) {
							povHat = 22500;
						} else {
							povHat = 18000;
						}
					} else if((currentKeys & KEY_DLEFT)) {
						if ((currentKeys & KEY_DUP)) {
							povHat = 31500;
						} else {
							povHat = 27000;
						}
					}
					
					if(!((currentKeys & KEY_DUP) || (currentKeys & KEY_DRIGHT) || (currentKeys & KEY_DDOWN) || (currentKeys & KEY_DLEFT))) {
						//If none are pressed, reset the POV hat
						povHat = -1;
					}
					
				}
				
				joyVolume = volume * 512;
				
				break;
		}
		
		if(vJoy) updateJoystick();
	}
	
	error("accept()");
	return 0;
}