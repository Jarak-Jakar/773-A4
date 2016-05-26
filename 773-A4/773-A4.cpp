// 773-A4.cpp : Defines the entry point for the console application.
// This is a naive implementation, with no optimisations for multi-threading or using SIMD instructions
// Also, could be some bad memory caching misses with the way some for loops are structured

#include "stdafx.h"

using namespace std;


int main(int argc, char *argv[])
{
	fipImage leftPicOrig, rightPicOrig;
	
	if (!leftPicOrig.load(argv[1])) {
		return -1;
	}

	if (!rightPicOrig.load(argv[2])) {
		return -1;
	}

	// Build intensity of the right image, using formula i = 0.3R + 0.6G + 0.1B
	vector<vector<double>> rightPicInt;
	// Shape vector right size - from http://www.cplusplus.com/forum/beginner/12409/
	rightPicInt.resize(FreeImage_GetWidth(rightPicOrig), vector<double>(FreeImage_GetHeight(rightPicOrig), 0));

	//This segment is taken straight from the FreeImage documentation, p27
	// Get number of bytes per pixel
	int bytespp = FreeImage_GetLine(rightPicOrig) / FreeImage_GetWidth(rightPicOrig);

	for (unsigned y = 0; y < FreeImage_GetHeight(rightPicOrig); y++) {
		BYTE *bits = FreeImage_GetScanLine(rightPicOrig, y);

		for (unsigned x = 0; x < FreeImage_GetWidth(rightPicOrig); x++)
		{
			rightPicInt[x][y] = (0.3 * bits[FI_RGBA_RED]) + (0.6 * bits[FI_RGBA_GREEN]) + (0.1 * bits[FI_RGBA_BLUE]);

			bits += bytespp;
		}
	}

	// Now that the intensity for each pixel has been computed, build an array of the 5x5 window average intensity
	vector<vector<double>> rightPicWin;
	rightPicWin.resize(FreeImage_GetWidth(rightPicOrig), vector<double>(FreeImage_GetHeight(rightPicOrig), 0));

	for (unsigned y = 2; y < (FreeImage_GetHeight(rightPicOrig) - 2); y++) {
		BYTE *bits = FreeImage_GetScanLine(rightPicOrig, y);

		for (unsigned x = 2; x < (FreeImage_GetWidth(rightPicOrig) - 2); x++)
		{
			double intensitySum = 0.0;

			intensitySum += rightPicInt[x - 2][y - 2];
			intensitySum += rightPicInt[x - 2][y - 1];
			intensitySum += rightPicInt[x - 2][y];
			intensitySum += rightPicInt[x - 2][y + 1];
			intensitySum += rightPicInt[x - 2][y + 2];
			intensitySum += rightPicInt[x - 1][y - 2];
			intensitySum += rightPicInt[x - 1][y - 1];
			intensitySum += rightPicInt[x - 1][y];
			intensitySum += rightPicInt[x - 1][y + 1];
			intensitySum += rightPicInt[x - 1][y + 2];
			intensitySum += rightPicInt[x][y - 2];
			intensitySum += rightPicInt[x][y - 1];
			intensitySum += rightPicInt[x][y];
			intensitySum += rightPicInt[x][y + 1];
			intensitySum += rightPicInt[x][y + 2];
			intensitySum += rightPicInt[x + 1][y - 2];
			intensitySum += rightPicInt[x + 1][y - 1];
			intensitySum += rightPicInt[x + 1][y];
			intensitySum += rightPicInt[x + 1][y + 1];
			intensitySum += rightPicInt[x + 1][y + 2];
			intensitySum += rightPicInt[x + 2][y - 2];
			intensitySum += rightPicInt[x + 2][y - 1];
			intensitySum += rightPicInt[x + 2][y];
			intensitySum += rightPicInt[x + 2][y + 1];
			intensitySum += rightPicInt[x + 2][y + 2];

			rightPicWin[x][y] = intensitySum / 25.0;

			bits += bytespp;
		}
	}

	// Now that the window average array has been built, can get rid of individual intensities array
	rightPicInt.~vector();

	vector<vector<int>> leftPicMatch;
	leftPicMatch.resize(FreeImage_GetWidth(leftPicOrig), vector<int>(FreeImage_GetHeight(leftPicOrig), 0));



	// Clear objects from memory
	//FreeImage_Unload(leftPicOrig);
	//FreeImage_Unload(rightPicOrig);
	rightPicWin.~vector();
	leftPicMatch.~vector();
	//rightPicInt.~vector();

	return 0;
}

