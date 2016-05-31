// ConsoleApplication1.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

using namespace std;
using namespace arma;

// Based off the code supplied by Trevor Gee, in 'Rectification', CS 773 Course handout
void bilinearInterpolate(fipImage pic, Col<double> currentCoord, BYTE *Colours) {
	RGBQUAD colour1, colour2, colour3, colour4;

	unsigned x1 = floor(currentCoord(0));
	unsigned y1 = floor(currentCoord(1));
	unsigned x2 = x1 + 1;
	unsigned y2 = y1 + 1;

	bool dontcare = pic.getPixelColor(x1, y1, &colour1);
	colour1 = colour1 * (x2 - currentCoord(0)) * (y2 - currentCoord(1));
	dontcare = pic.getPixelColor(x2, y1, &colour2);
	colour2 = colour2 * (currentCoord(0) - x1) * (y2 - currentCoord(1));
}


int main(int argc, char *argv[])
{
	fipImage leftPic, rightPic;

	if (!leftPic.load(argv[1])) {
		return -1;
	}

	if (!rightPic.load(argv[2])) {
		return -1;
	}

	unsigned imageWidth = leftPic.getWidth();
	unsigned imageHeight = leftPic.getHeight();

	ifstream calibrationParamsFile;

	calibrationParamsFile.open(argv[3]);
	Mat<double> leftCameraMatrix(3, 3, fill::zeros);
	Mat<double> leftRotationTranslationMatrix(3, 4, fill::zeros);
	Mat<double> rightCameraMatrix(3, 3, fill::zeros);
	Mat<double> rightRotationTranslationMatrix(3, 4, fill::zeros);

	// Read in calibration parameters
	if (calibrationParamsFile.is_open()) {
		string line;

		//Do the left camera's stuff
		for (int i = 0; i < 3; i++) {
			getline(calibrationParamsFile, line);

			string token = line.substr(0, line.find(","));
			leftCameraMatrix(i, 0) = stod(token);
			line.erase(0, (line.find(",") + 1));

			token = line.substr(0, line.find(","));
			leftCameraMatrix(i, 1) = stod(token);
			line.erase(0, (line.find(",") + 1));

			token = line.substr(0, line.find(","));
			leftCameraMatrix(i, 2) = stod(token);
			line.erase(0, (line.find(",") + 1));

			token = line.substr(0, line.find(","));
			leftRotationTranslationMatrix(i, 0) = stod(token);
			line.erase(0, (line.find(",") + 1));

			token = line.substr(0, line.find(","));
			leftRotationTranslationMatrix(i, 1) = stod(token);
			line.erase(0, (line.find(",") + 1));

			token = line.substr(0, line.find(","));
			leftRotationTranslationMatrix(i, 2) = stod(token);
			line.erase(0, (line.find(",") + 1));

			token = line.substr(0, line.find(","));
			leftRotationTranslationMatrix(i, 3) = stod(token);
		}

		getline(calibrationParamsFile, line);

		// Do the right camera's stuff
		for (int i = 0; i < 3; i++) {
			getline(calibrationParamsFile, line);

			string token = line.substr(0, line.find(","));
			rightCameraMatrix(i, 0) = stod(token);
			line.erase(0, (line.find(",") + 1));

			token = line.substr(0, line.find(","));
			rightCameraMatrix(i, 1) = stod(token);
			line.erase(0, (line.find(",") + 1));

			token = line.substr(0, line.find(","));
			rightCameraMatrix(i, 2) = stod(token);
			line.erase(0, (line.find(",") + 1));

			token = line.substr(0, line.find(","));
			rightRotationTranslationMatrix(i, 0) = stod(token);
			line.erase(0, (line.find(",") + 1));

			token = line.substr(0, line.find(","));
			rightRotationTranslationMatrix(i, 1) = stod(token);
			line.erase(0, (line.find(",") + 1));

			token = line.substr(0, line.find(","));
			rightRotationTranslationMatrix(i, 2) = stod(token);
			line.erase(0, (line.find(",") + 1));

			token = line.substr(0, line.find(","));
			rightRotationTranslationMatrix(i, 3) = stod(token);
		}
	}

	calibrationParamsFile.close();

	// Undistort the images

	fipImage leftUndistortImage(leftPic);
	fipImage rightUndistortImage(rightPic);

	// Rectify the images

	// Find optical centres for the cameras

	Mat<double> leftProjectionMatrix = leftCameraMatrix * leftRotationTranslationMatrix;
	Mat<double> rightProjectionMatrix = rightCameraMatrix * rightRotationTranslationMatrix;
	Col<double> leftOpticalCentre = (leftProjectionMatrix.cols(0, 2)).i() * leftProjectionMatrix.col(3);
	Col<double> rightOpticalCentre = (rightProjectionMatrix.cols(0, 2)).i() * rightProjectionMatrix.col(3);

	Col<double> v1 = leftOpticalCentre - rightOpticalCentre;
	//Mat<double> leftRotationMatrix = leftRotationTranslationMatrix.head_cols(3);
	//Row<double> R3 = leftRotationMatrix.row(2);
	//Col<double> R3T = R3.t();
	Col<double> v2 = cross((((leftRotationTranslationMatrix.head_cols(3)).row(2)).t()), v1);
	Col<double> v3 = cross(v1, v2);

	Col<double> v4 = normalise(v1);
	Col<double> v5 = normalise(v2);
	Col<double> v6 = normalise(v3);

	v1.~Col();
	v2.~Col();
	v3.~Col();

	Mat<double> leftRect = v4.t();
	leftRect.insert_rows(1, v5.t());
	leftRect.insert_rows(2, v6.t());

	Mat<double> cameraRect = (leftCameraMatrix + rightCameraMatrix) / 2.0;
	Mat<double> rightRect = mat(leftRect);
	leftRect.insert_cols(3, ((leftRect * -1) * leftOpticalCentre));
	rightRect.insert_cols(3, ((rightRect * -1) * rightOpticalCentre));
	Mat<double> idealProjectionLeft = cameraRect * leftRect;
	Mat<double> idealProjectionRight = cameraRect * rightRect;

	Mat<double> homographyLeft = idealProjectionLeft.cols(0,3) * (leftProjectionMatrix.cols(0,3)).i(); // Following the Fusiello paper here, and just working with the first three columns
	Mat<double> homographyRight = idealProjectionRight.cols(0,3) * (rightProjectionMatrix.cols(0,3)).i();

	Mat<double> invHomographyLeft = homographyLeft.i();
	Mat<double> invHomographyRight = homographyRight.i();

	// Destruct the now unnecessary objects, and hopefully clear up some memory space

	v4.~Col();
	v5.~Col();
	v6.~Col();

	leftCameraMatrix.~Mat();
	rightCameraMatrix.~Mat();
	leftRotationTranslationMatrix.~Mat();
	rightRotationTranslationMatrix.~Mat();
	leftProjectionMatrix.~Mat();
	rightProjectionMatrix.~Mat();
	leftOpticalCentre.~Col();
	rightOpticalCentre.~Col();
	cameraRect.~Mat();
	leftRect.~Mat();
	rightRect.~Mat();
	idealProjectionLeft.~Mat();
	idealProjectionRight.~Mat();

	// Rectify left image

	fipImage leftRectifyImage(leftUndistortImage);
	fipImage rightRectifyImage(rightUndistortImage);

	int bytesppu = FreeImage_GetLine(leftUndistortImage) / imageWidth;
	int bytesppr = FreeImage_GetLine(leftRectifyImage) / imageWidth;

	Col<double> currentCoordinateU(3);
	Col<double> currentCoordinateR(3);

	for (unsigned y = 0; y < imageHeight; y++) {
		BYTE *bitsu = FreeImage_GetScanLine(leftUndistortImage, y);
		BYTE *bitsr = FreeImage_GetScanLine(leftRectifyImage, y);

		for (unsigned x = 0; x < imageWidth; x++)
		{
			//rightPicInt[x][y] = (0.3 * bits[FI_RGBA_RED]) + (0.6 * bits[FI_RGBA_GREEN]) + (0.1 * bits[FI_RGBA_BLUE]);

			currentCoordinateR(0) = x;
			currentCoordinateR(1) = y;
			currentCoordinateR(2) = 1;
			currentCoordinateU = invHomographyLeft * currentCoordinateR;

			BYTE Colours[3];

			bilinearInterpolate(leftUndistortImage, currentCoordinateU, Colours);

			bitsu += bytesppu;
			bitsr += bytesppr;
		}
	}

    return 0;
}

