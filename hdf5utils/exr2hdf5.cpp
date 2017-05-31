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

#include <strings.h>

#include <OpenEXR/ImfArray.h>
#include <OpenEXR/ImfInputFile.h>
#include <OpenEXR/ImfIO.h>
#include <OpenEXR/ImfFrameBuffer.h>
#include <OpenEXR/ImathBox.h>
#include <OpenEXR/ImfChannelList.h>

#include "isexr.h"

#include <H5Cpp.h>

using namespace std;
using namespace Imf;
using namespace H5;

int convertFiles(string fileIn, string fileOut) {


	if (!isExrFile(fileIn.c_str())) {
		return 1;
	}

	InputFile file (fileIn.c_str());
	H5File outfile(fileOut, H5F_ACC_TRUNC);

	const ChannelList &channels = file.header().channels();

	Imath::Box2i dw = file.header().dataWindow();
	int width  = dw.max.x - dw.min.x + 1;
	int height = dw.max.y - dw.min.y + 1;

	hsize_t dimsf[2];
	dimsf[0] = height;
	dimsf[1] = width;
	DataSpace dataspace(2, dimsf);

	DataType datatype(H5::PredType::NATIVE_FLOAT);

	for (ChannelList::ConstIterator it = channels.begin(); it != channels.end(); ++it) {
		//for all channels.

		//Channel & c = it.channel();
		DataSet dataset = outfile.createDataSet(it.name(), datatype, dataspace);

		Array2D<float> arr(height, width);

		FrameBuffer frameBuffer;

		frameBuffer.insert (it.name(),                                  // name
							Slice (FLOAT,                         // type
								   (char *) (&arr[0][0] -     // base
											 dw.min.x -
											 dw.min.y * width),
								   sizeof (arr[0][0]) * 1,    // xStride
								   sizeof (arr[0][0]) * width,// yStride
								   1, 1,                          // x/y sampling
								   FLT_MAX));                     // fillValue

		file.setFrameBuffer (frameBuffer);
		file.readPixels (dw.min.y, dw.max.y);

		dataset.write(arr[0], datatype);
	}

	return 0;

}

int main (int argc, char** argv) {

	if (argc != 3) {
		cerr << argv[0] << " expect 2 arguments [fileIn (.exr) fileOut (.h5)]" << endl;
	}

	return convertFiles(argv[1], argv[2]);

}
