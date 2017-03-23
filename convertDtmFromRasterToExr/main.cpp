/*
 *  exr-tools, a collection of command line tools to work with exr images.
 *  Copyright (C) 2017 Laurent "Paragon" Jospin
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include <iostream>

#undef cimg_display
#define cimg_display 0
#include "CImg.h"

#include <OpenEXR/ImfOutputFile.h>
#include <OpenEXR/ImfIO.h>
#include <OpenEXR/ImfFrameBuffer.h>
#include <OpenEXR/ImathBox.h>
#include <OpenEXR/ImfChannelList.h>

using namespace std;
using namespace Imf;

void writeZ (const char fileName[],
			 const float *zPixels,
			 int width,
			 int height)
{
	Header header (width, height);
	header.channels().insert ("Z", Channel (FLOAT));

	OutputFile oFile(fileName, header);

	FrameBuffer frameBuffer;

	frameBuffer.insert ("Z", Slice(FLOAT, (char *) zPixels, sizeof (*zPixels) * 1, sizeof (*zPixels) * width));

	oFile.setFrameBuffer (frameBuffer);
	oFile.writePixels (height);
}

int main(int argc, char *argv[])
{

	if (argc != 4) {
		cerr << "Error, program needs 3 arguments. Usage:" << argv[0] << " pathToInputRasterFile scaleFactor pathToExrOutputFile" << endl;
		return 1;
	}

	const char* inputFileName = argv[1];
	double scale = atof(argv[2]);
	const char* outPutFileName = argv[3];

	cil::CImg<float> imageIn(inputFileName);
	imageIn.normalize(0, 1);
	imageIn *= scale;

	writeZ(outPutFileName, imageIn.data(), imageIn.width(), imageIn.height());

	return 0;
}
