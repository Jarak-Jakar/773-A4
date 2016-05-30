// ConsoleApplication1.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

using namespace std;
using namespace arma;


int main(int argc, char *argv[])
{
	fipImage leftPic, rightPic;

	if (!leftPic.load(argv[1]))
	{
		return -1;
	}

	if (!rightPic.load(argv[2]))
	{
		return -1;
	}

	ifstream leftPicCalibrationParamsFile;
	ifstream rightPicCalibrationParamsFile;

	leftPicCalibrationParamsFile.open(argv[3]);
	Mat<double> leftCameraMatrix(3, 3);
	Mat<double> leftRotationMatrix(3, 4);

	if (leftPicCalibrationParamsFile.is_open()) {
		string line;
		for (int i = 0; i < 3; i++) {
			getline(leftPicCalibrationParamsFile, line);

		}
	}

    return 0;
}

