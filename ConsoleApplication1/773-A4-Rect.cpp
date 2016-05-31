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

			string token = line.substr(0, line.find(","));
			leftCameraMatrix(i, 1) = stod(token);
			line.erase(0, (line.find(",") + 1));

			string token = line.substr(0, line.find(","));
			leftCameraMatrix(i, 2) = stod(token);
			line.erase(0, (line.find(",") + 1));

			string token = line.substr(0, line.find(","));
			leftRotationTranslationMatrix(i, 0) = stod(token);
			line.erase(0, (line.find(",") + 1));

			string token = line.substr(0, line.find(","));
			leftRotationTranslationMatrix(i, 1) = stod(token);
			line.erase(0, (line.find(",") + 1));

			string token = line.substr(0, line.find(","));
			leftRotationTranslationMatrix(i, 2) = stod(token);
			line.erase(0, (line.find(",") + 1));

			string token = line.substr(0, line.find(","));
			leftRotationTranslationMatrix(i, 3) = stod(token);
		}

		getline(calibrationParamsFile, line);

		// Do the right camera's stuff
		for (int i = 0; i < 3; i++) {
			getline(calibrationParamsFile, line);

			string token = line.substr(0, line.find(","));
			rightCameraMatrix(i, 0) = stod(token);
			line.erase(0, (line.find(",") + 1));

			string token = line.substr(0, line.find(","));
			rightCameraMatrix(i, 1) = stod(token);
			line.erase(0, (line.find(",") + 1));

			string token = line.substr(0, line.find(","));
			rightCameraMatrix(i, 2) = stod(token);
			line.erase(0, (line.find(",") + 1));

			string token = line.substr(0, line.find(","));
			rightRotationTranslationMatrix(i, 0) = stod(token);
			line.erase(0, (line.find(",") + 1));

			string token = line.substr(0, line.find(","));
			rightRotationTranslationMatrix(i, 1) = stod(token);
			line.erase(0, (line.find(",") + 1));

			string token = line.substr(0, line.find(","));
			rightRotationTranslationMatrix(i, 2) = stod(token);
			line.erase(0, (line.find(",") + 1));

			string token = line.substr(0, line.find(","));
			rightRotationTranslationMatrix(i, 3) = stod(token);
		}
	}

	calibrationParamsFile.close();

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

	normalise(v1);
	normalise(v2);
	normalise(v3);




    return 0;
}

