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
