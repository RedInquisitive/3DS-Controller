#pragma once

#include <stdbool.h>

bool usingPro;
bool askedForPro;

u32 *irmemloc;

void initIr();

void closeIr();

void printIr(); //Used to display IR stuff.

void askForCirclePadPro(); //Beginning of program.