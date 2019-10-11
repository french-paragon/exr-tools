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
#include <vector>

#undef cimg_display
#define cimg_display 0
#include "CImg.h"

#include <OpenEXR/ImfArray.h>
#include <OpenEXR/ImfInputFile.h>
#include <OpenEXR/ImfIO.h>
#include <OpenEXR/ImfFrameBuffer.h>
#include <OpenEXR/ImathBox.h>
#include <OpenEXR/ImfChannelList.h>

#include "isexr.h"

using namespace std;
using namespace Imf;

bool readZ (const char fileName[], Array2D<float>* zPixels, int &width, int &height)
{
	if (!isExrFile(fileName)) {
		return false;
	}

	InputFile file (fileName);

	const ChannelList &channels = file.header().channels();
	const Channel *channelPtr = channels.findChannel("Z");

	std::string cName = "Z";

	if (channelPtr == nullptr) {
		//try alternatives
		std::vector<std::string> alts = {"Z.Z", "Depth.Z", "Composite.Z", "A", "Colors.A", "Composite.A"};

		for (std::string alt : alts) {
			channelPtr = channels.findChannel(alt);

			if (channelPtr != nullptr) {
				cName = alt;
				goto prog_suite;
			}
		}

		return false;
	}

	prog_suite:

	Imath::Box2i dw = file.header().dataWindow();
	width  = dw.max.x - dw.min.x + 1;
	height = dw.max.y - dw.min.y + 1;

	zPixels->resizeErase(height, width);

	FrameBuffer frameBuffer;

	frameBuffer.insert (cName,                                  // name
						Slice (FLOAT,                         // type
                               static_cast<char *>(static_cast<void *>(&(*zPixels)[0][0] -     // base
										 dw.min.x -
                                         dw.min.y * width)),
							   sizeof ((*zPixels)[0][0]) * 1,    // xStride
                               sizeof ((*zPixels)[0][0]) * static_cast<unsigned long>(width),// yStride
							   1, 1,                          // x/y sampling
							   FLT_MAX));                     // fillValue

	file.setFrameBuffer (frameBuffer);
	file.readPixels (dw.min.y, dw.max.y);

	return true;
}

int main(int argc, char *argv[])
{
	if (argc != 3) {
		cerr << "Error, program needs 2 arguments. Usage:" << argv[0] << " pathToInputExrFile pathToOutputRasterFile" << endl;
		return 1;
	}

	const char* inputFileName = argv[1];
	const char* outPutFileName = argv[2];

	Array2D<float> datas;
	int width;
	int height;

	bool sucess = readZ(inputFileName, &datas, width, height);

	if (!sucess) {
		cerr << "Impossible to read file " << inputFileName << endl;
		return 1;
	}

    cil::CImg<float> fImg(static_cast<unsigned int>(width), static_cast<unsigned int>(height));
	fImg._data = datas[0];

	fImg.normalize(0, 255);

	cil::CImg<uint8_t> img = fImg; //convert from float to int,

	img.save(outPutFileName);

	fImg._data = nullptr; //reset datas to nullptr, or we will have an error.

	return 0;
}
