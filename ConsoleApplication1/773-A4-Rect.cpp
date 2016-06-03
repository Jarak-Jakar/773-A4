// ConsoleApplication1.cpp : Defines the entry point for the console application.
// Note that this is a naive implemenation, without any performance optimisations
//

#include "stdafx.h"

using namespace std;
using namespace arma;

Col<double> extractColour(fipImage const &pic, unsigned x, unsigned y) {
	BYTE red, green, blue;
	RGBQUAD Colour;
	Col<double> returnColour(3);
	if (pic.getPixelColor(x, y, &Colour)) {
		red = Colour.rgbRed;
		green = Colour.rgbGreen;
		blue = Colour.rgbBlue;
		returnColour(0) = red;
		returnColour(1) = green;
		returnColour(2) = blue;
	} else {
		returnColour(0) = 0.0;
		returnColour(1) = 0.0;
		returnColour(2) = 0.0;
	}

	return returnColour;
}

// Based off the code supplied by Trevor Gee, in 'Rectification', CS 773 Course handout
void bilinearInterpolate(fipImage const &pic, Col<double> currentCoord, BYTE *Colours) {
	Col<double> colour1(3), colour2(3), colour3(3), colour4(3);

	unsigned x1 = (unsigned)floor(currentCoord(0));
	unsigned y1 = (unsigned)floor(currentCoord(1));
	unsigned x2 = x1 + 1;
	unsigned y2 = y1 + 1;

	//bool dontcare = pic.getPixelColor(x1, y1, &colour1);
	colour1 = extractColour(pic, x1, y1) * (x2 - currentCoord(0)) * (y2 - currentCoord(1));
	//dontcare = pic.getPixelColor(x2, y1, &colour2);
	colour2 = extractColour(pic, x2, y1) * (currentCoord(0) - x1) * (y2 - currentCoord(1));
	//dontcare = pic.getPixelColor(x1, y2, &colour3);
	colour3 = extractColour(pic, x1, y2) * (x2 - currentCoord(0)) * (currentCoord(1) - y1);
	//dontcare = pic.getPixelColor(x2, y2, &colour4);
	colour4 = extractColour(pic, x2, y2) * (currentCoord(0) - x1) * (currentCoord(1) - y1);

	Col<double> finalColour = colour1 + colour2 + colour3 + colour4;
	Colours[0] = (BYTE)round(finalColour(0));
	Colours[1] = (BYTE)round(finalColour(1));
	Colours[2] = (BYTE)round(finalColour(2));
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
	double KappaLeft = 0.0;
	double KappaRight = 0.0;
	double pixelSize = 0.0;

	// Read in calibration parameters
	if (calibrationParamsFile.is_open()) {
		string line, token;

		//Do the left camera's stuff
		for (int i = 0; i < 3; i++) {
			getline(calibrationParamsFile, line);

			token = line.substr(0, line.find(","));
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
		token = line.substr(0, line.find(","));
		KappaLeft = stod(token);
		line.erase(0, (line.find(",") + 1));
		token = line.substr(0, line.find(","));
		KappaRight = stod(token);
		line.erase(0, (line.find(",") + 1));
		token = line.substr(0, line.find(","));
		pixelSize = stod(token);

		// Do the right camera's stuff
		for (int i = 0; i < 3; i++) {
			getline(calibrationParamsFile, line);

			token = line.substr(0, line.find(","));
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
	//fipImage leftUndistortImage(FIT_BITMAP, imageWidth, imageHeight, 24);
	//fipImage rightUndistortImage(rightPic);
	fipImage rightUndistortImage(FIT_BITMAP, imageWidth, imageHeight, 24);
	Col<double> DistColours(3);
	RGBQUAD PixelColour;
	double rhosq = 0;
	int xu = 0;
	int yu = 0;

	// xu = xd * (1 + (K * p2))
	// yu = yd * (1 + (K * p2))

	//cout << "KappaLeft = " << KappaLeft << endl;

	for (unsigned x = 0; x < imageWidth; x++)
	{
		for (unsigned y = 0; y < imageHeight; y++)
		{
			rhosq = (int)round(pow(x, 2) + pow(y, 2));
			xu = (int)round(x * (1 + (KappaLeft * rhosq)));
			yu = (int)round(y * (1 + (KappaLeft * rhosq)));

			//cout << "xu = " << xu << ", yu = " << yu << ", rhosq = " << rhosq << endl;

			DistColours = extractColour(leftPic, x, y);
			PixelColour.rgbRed = (BYTE)round(DistColours(0));
			PixelColour.rgbGreen = (BYTE)round(DistColours(1));
			PixelColour.rgbBlue = (BYTE)round(DistColours(2));

			leftUndistortImage.setPixelColor(xu, yu, &PixelColour);
		}
	}

	for (unsigned x = 0; x < imageWidth; x++)
	{
		for (unsigned y = 0; y < imageHeight; y++)
		{
			rhosq = pow((x * pixelSize), 2) + pow((y * pixelSize), 2);
			//rhosq = pow((x), 2) + pow((y), 2);
			xu = (int)round(x * (1 + (KappaRight * rhosq * pixelSize)));
			yu = (int)round(y * (1 + (KappaRight * rhosq * pixelSize)));
			//xu = (int)round(x * (1 + (KappaRight * rhosq)));
			//yu = (int)round(y * (1 + (KappaRight * rhosq)));

			//cout << "xu = " << xu << ", yu = " << yu << ", rhosq = " << rhosq << ", KappaRight = " << KappaRight << endl;

			DistColours = extractColour(rightPic, x, y);
			PixelColour.rgbRed = (BYTE)round(DistColours(0));
			PixelColour.rgbGreen = (BYTE)round(DistColours(1));
			PixelColour.rgbBlue = (BYTE)round(DistColours(2));

			rightUndistortImage.setPixelColor(xu, yu, &PixelColour);
		}
	}

	leftUndistortImage.save(argv[4]);

	rightUndistortImage.save(argv[5]);

	cout << "Finished undistorting the images!" << endl;

	// Rectify the images

	// Find optical centres for the cameras

	Mat<double> leftProjectionMatrix = leftCameraMatrix * leftRotationTranslationMatrix;
	Mat<double> rightProjectionMatrix = rightCameraMatrix * rightRotationTranslationMatrix;
	Col<double> leftOpticalCentre = -1.0 * ((leftProjectionMatrix.cols(0, 2)).i()) * leftProjectionMatrix.col(3);
	Col<double> rightOpticalCentre = -1.0 * ((rightProjectionMatrix.cols(0, 2)).i()) * rightProjectionMatrix.col(3);

	Col<double> v1 = leftOpticalCentre - rightOpticalCentre;
	Mat<double> leftRotationMatrix = leftRotationTranslationMatrix.head_cols(3);
	Row<double> R3 = leftRotationMatrix.row(2);
	Col<double> R3T = R3.t();
	//Col<double> v2 = cross((((leftRotationTranslationMatrix.head_cols(3)).row(2)).t()), v1);
	Col<double> v2 = cross(R3T, v1);
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

	/*Mat<double> leftRect = v4;
	leftRect.insert_cols(1, v5);
	leftRect.insert_cols(2, v6);*/

	cout << "Finished building leftRect" << endl;

	Mat<double> cameraRect = (leftCameraMatrix + rightCameraMatrix) / 2.0;
	Mat<double> rightRect = mat(leftRect);
	leftRect.insert_cols(3, ((leftRect * -1.0) * leftOpticalCentre));
	rightRect.insert_cols(3, ((rightRect * -1.0) * rightOpticalCentre));

	cout << "Finished building rightRect" << endl;

	Mat<double> idealProjectionLeft = cameraRect * leftRect;
	Mat<double> idealProjectionRight = cameraRect * rightRect;

	cout << "Finished calculating idealProjections" << endl;

	Mat<double> homographyLeft = idealProjectionLeft.cols(0,2) * (leftProjectionMatrix.cols(0,2)).i(); // Following the Fusiello paper here, and just working with the first three columns
	Mat<double> homographyRight = idealProjectionRight.cols(0,2) * (rightProjectionMatrix.cols(0,2)).i();

	cout << "Finished calculating the homographies!" << endl;

	Mat<double> invHomographyLeft = homographyLeft.i();
	Mat<double> invHomographyRight = homographyRight.i();

	cout << "Finished calculating the inverse homographies!" << endl;

	// Destruct the now unnecessary objects, and hopefully clear up some memory space

	leftPic.~fipImage();
	rightPic.~fipImage();
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

	int bytesppr = FreeImage_GetLine(leftRectifyImage) / imageWidth;
	BYTE *bitsr;
	BYTE Colours[3];

	Col<double> currentCoordinateU(3);
	Col<double> currentCoordinateR(3);

	for (unsigned y = 0; y < imageHeight; y++) {
		bitsr = FreeImage_GetScanLine(leftRectifyImage, y);

		for (unsigned x = 0; x < imageWidth; x++)
		{
			currentCoordinateR(0) = x;
			currentCoordinateR(1) = y;
			currentCoordinateR(2) = 1;
			currentCoordinateU = invHomographyLeft * currentCoordinateR;

			bilinearInterpolate(leftUndistortImage, currentCoordinateU, Colours);

			bitsr[FI_RGBA_RED] = Colours[0];
			bitsr[FI_RGBA_GREEN] = Colours[1];
			bitsr[FI_RGBA_BLUE] = Colours[2];

			bitsr += bytesppr;
		}
	}

	leftRectifyImage.save(argv[6]);

	cout << "Finished creating the left rectified image!" << endl;
	leftUndistortImage.~fipImage();
	leftRectifyImage.~fipImage();

	bytesppr = FreeImage_GetLine(rightRectifyImage) / imageWidth;

	for (unsigned y = 0; y < imageHeight; y++) {
		bitsr = FreeImage_GetScanLine(rightRectifyImage, y);

		for (unsigned x = 0; x < imageWidth; x++)
		{

			currentCoordinateR(0) = x;
			currentCoordinateR(1) = y;
			currentCoordinateR(2) = 1;
			currentCoordinateU = invHomographyRight * currentCoordinateR;

			bilinearInterpolate(rightUndistortImage, currentCoordinateU, Colours);

			bitsr[FI_RGBA_RED] = Colours[0];
			bitsr[FI_RGBA_GREEN] = Colours[1];
			bitsr[FI_RGBA_BLUE] = Colours[2];

			bitsr += bytesppr;
		}
	}

	rightRectifyImage.save(argv[7]);

	cout << "Finished creating the right rectified image!" << endl;

    return 0;
}

