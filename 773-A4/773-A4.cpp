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

	unsigned const imageWidth = FreeImage_GetWidth(rightPicOrig);
	unsigned const imageHeight = FreeImage_GetHeight(rightPicOrig);

	// Build intensity of the right image, using formula i = 0.3R + 0.6G + 0.1B
	vector<vector<double>> rightPicInt;
	// Shape vector right size - from http://www.cplusplus.com/forum/beginner/12409/
	rightPicInt.resize(imageWidth, vector<double>(imageHeight, 0));

	//This segment is taken straight from the FreeImage documentation, p27
	// Get number of bytes per pixel
	int bytespp = FreeImage_GetLine(rightPicOrig) / imageWidth;

	for (unsigned y = 0; y < imageHeight; y++) {
		BYTE *bits = FreeImage_GetScanLine(rightPicOrig, y);

		for (unsigned x = 0; x < imageWidth; x++)
		{
			rightPicInt[x][y] = (0.3 * bits[FI_RGBA_RED]) + (0.6 * bits[FI_RGBA_GREEN]) + (0.1 * bits[FI_RGBA_BLUE]);

			bits += bytespp;
		}
	}

	// Now that the intensity for each pixel has been computed, build an array of the 5x5 window average intensity
	vector<vector<double>> rightPicWin;
	rightPicWin.resize(imageWidth, vector<double>(imageHeight, 0));

	double intensitySum = 0.0;

	// Create intensity values for all pixels that are not on the border or next to the border - done this way to avoid a lot of unnecessary if statements
	for (unsigned x = 2; x < (imageWidth - 2); x++) {
		//BYTE *bits = FreeImage_GetScanLine(rightPicOrig, y);

		for (unsigned y = 2; y < (imageHeight - 2); y++)
		{
			intensitySum = 0.0;

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

			//bits += bytespp;
		}
	}

	// Now create intensity values for the border pixels - done separately to avoid a lot of unnecessary if statements, though it does bloat the code an awful lot
	// Pixels outside the image are assigned an intensity of 127 to be totally average

	// Go up the left side
	for (unsigned y = 0; y < imageHeight; y++)
	{
		// Do x = 0 line

		if (y == 0) {
			intensitySum = 0.0;
			intensitySum += 16.0 * 127.0;
			intensitySum += rightPicInt[0][0];
			intensitySum += rightPicInt[0][1];
			intensitySum += rightPicInt[0][2];
			intensitySum += rightPicInt[1][0];
			intensitySum += rightPicInt[1][1];
			intensitySum += rightPicInt[1][2];
			intensitySum += rightPicInt[2][0];
			intensitySum += rightPicInt[2][1];
			intensitySum += rightPicInt[2][2];
			rightPicWin[0][y] = intensitySum / 25.0;
		}
		else if (y == 1)
		{
			intensitySum = 0.0;
			intensitySum += 13.0 * 127.0;
			intensitySum += rightPicInt[0][0];
			intensitySum += rightPicInt[0][1];
			intensitySum += rightPicInt[0][2];
			intensitySum += rightPicInt[0][3];
			intensitySum += rightPicInt[1][0];
			intensitySum += rightPicInt[1][1];
			intensitySum += rightPicInt[1][2];
			intensitySum += rightPicInt[1][3];
			intensitySum += rightPicInt[2][0];
			intensitySum += rightPicInt[2][1];
			intensitySum += rightPicInt[2][2];
			intensitySum += rightPicInt[2][3];
			rightPicWin[0][y] = intensitySum / 25.0;
		}
		else if (y == imageHeight - 2)
		{
			intensitySum = 0.0;
			intensitySum += 13.0 * 127.0;
			intensitySum += rightPicInt[0][imageHeight - 4];
			intensitySum += rightPicInt[0][imageHeight - 3];
			intensitySum += rightPicInt[0][imageHeight - 2];
			intensitySum += rightPicInt[0][imageHeight - 1];
			intensitySum += rightPicInt[1][imageHeight - 4];
			intensitySum += rightPicInt[1][imageHeight - 3];
			intensitySum += rightPicInt[1][imageHeight - 2];
			intensitySum += rightPicInt[1][imageHeight - 1];
			intensitySum += rightPicInt[2][imageHeight - 4];
			intensitySum += rightPicInt[2][imageHeight - 3];
			intensitySum += rightPicInt[2][imageHeight - 2];
			intensitySum += rightPicInt[2][imageHeight - 1];
			rightPicWin[0][y] = intensitySum / 25.0;
		}
		else if (y == imageHeight - 1)
		{
			intensitySum = 0.0;
			intensitySum += 16.0 * 127.0;
			intensitySum += rightPicInt[0][imageHeight - 3];
			intensitySum += rightPicInt[0][imageHeight - 2];
			intensitySum += rightPicInt[0][imageHeight - 1];
			intensitySum += rightPicInt[1][imageHeight - 3];
			intensitySum += rightPicInt[1][imageHeight - 2];
			intensitySum += rightPicInt[1][imageHeight - 1];
			intensitySum += rightPicInt[2][imageHeight - 3];
			intensitySum += rightPicInt[2][imageHeight - 2];
			intensitySum += rightPicInt[2][imageHeight - 1];
			rightPicWin[0][y] = intensitySum / 25.0;
		}
		else
		{
			intensitySum = 0.0;
			intensitySum += 10.0 * 127.0;
			intensitySum += rightPicInt[0][y - 2];
			intensitySum += rightPicInt[0][y - 1];
			intensitySum += rightPicInt[0][y];
			intensitySum += rightPicInt[0][y + 1];
			intensitySum += rightPicInt[0][y + 2];
			intensitySum += rightPicInt[1][y - 2];
			intensitySum += rightPicInt[1][y - 1];
			intensitySum += rightPicInt[1][y];
			intensitySum += rightPicInt[1][y + 1];
			intensitySum += rightPicInt[1][y + 2];
			intensitySum += rightPicInt[2][y - 2];
			intensitySum += rightPicInt[2][y - 1];
			intensitySum += rightPicInt[2][y];
			intensitySum += rightPicInt[2][y + 1];
			intensitySum += rightPicInt[2][y + 2];
			rightPicWin[0][y] = intensitySum / 25.0;
		}
 	}

	// Another change!

	// Now that the window average array has been built, can get rid of individual intensities array
	rightPicInt.~vector();

	// Array to use to hold the best pixel location estimate - index is location of left pixel, value is location of right pixel on same line
	vector<vector<int>> leftPicMatch;
	leftPicMatch.resize(FreeImage_GetWidth(leftPicOrig), vector<int>(FreeImage_GetHeight(leftPicOrig), 0));



	// Clear objects from memory
	//FreeImage_Unload(leftPicOrig); // Commented out because they seem to cause an error at the return instruction...
	//FreeImage_Unload(rightPicOrig);
	rightPicWin.~vector();
	leftPicMatch.~vector();
	//rightPicInt.~vector();

	return 0;
}

