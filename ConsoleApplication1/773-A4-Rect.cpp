// ConsoleApplication1.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"


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



    return 0;
}

