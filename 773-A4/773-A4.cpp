// 773-A4.cpp : Defines the entry point for the console application.
// This is a naive implementation, with no optimisations for multi-threading or using SIMD instructions

#include "stdafx.h"
#include "FreeImagePlus.h"


int main(int argc, char *argv[])
{
	fipImage leftPic, rightPic;
	
	if (!leftPic.load(argv[1])) {
		return -1;
	}

	if (!rightPic.load(argv[2])) {
		return -1;
	}

	// Build averaging of the right image


	return 0;
}

