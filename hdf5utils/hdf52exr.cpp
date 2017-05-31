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
#include <string>
#include <vector>

#include <OpenEXR/ImfInputFile.h>
#include <OpenEXR/ImfOutputFile.h>
#include <OpenEXR/ImfIO.h>
#include <OpenEXR/ImfFrameBuffer.h>
#include <OpenEXR/ImathBox.h>
#include <OpenEXR/ImfChannelList.h>

#include <H5Cpp.h>

using namespace std;
using namespace Imf;
using namespace H5;

herr_t group_info(hid_t loc_id,
				 const char *name,
				 const H5L_info_t *linfo,
				 void *opdata) {

	(void) loc_id;
	(void) linfo;

	vector<string>* targetString = reinterpret_cast<vector<string>*>(opdata);

	targetString->push_back(string(name));

	return 0;

}

int convertFiles(string fileIn, string fileOut) {

	// Open HDF5 file handle, read only
	H5File file(fileIn.c_str(), H5F_ACC_RDONLY);

	Group *rootGr = new Group (file.openGroup("/"));

	vector<string> groupsName;
	vector<string> selectedGroupsName;

	herr_t idx = H5Literate(rootGr->getId(), H5_INDEX_NAME, H5_ITER_INC, NULL, group_info, reinterpret_cast<void*>(&groupsName));

	vector<float *> datas;
	int w = -1;
	int h = -1;

	for (string str : groupsName) {

		DataSet dataset = file.openDataSet( str );

		DataType datatype = dataset.getDataType();

		if (datatype == H5::PredType::NATIVE_FLOAT) {

			DataSpace dataspace = dataset.getSpace();

			int ndims = dataspace.getSimpleExtentNdims();

			if (ndims != 2) {
				cerr << "error, dataset contain a non 2d float array !, skip !" << endl;
				continue;
			}

			hsize_t dims_out[2];
			dataspace.getSimpleExtentDims( dims_out, nullptr);

			if (w <= -1) {
				w = dims_out[0];
			}

			if (h <= -1) {
				h = dims_out[1];
			}

			if (w != dims_out[0] || h != dims_out[1]) {
				cerr << "error, dataset " << str << " has unexpected size, skipping it !" << endl;
				continue;
			}

			dataspace.selectAll();

			DataSpace memspace(2, dims_out);
			memspace.selectAll();

			float * ptr = new float[w*h];
			datas.push_back(ptr);
			selectedGroupsName.push_back(str);

			dataset.read( ptr, datatype, memspace, dataspace );
		}
	}
	Header header (w, h);

	for (string str : selectedGroupsName) {
		header.channels().insert (str.c_str(), Channel (FLOAT));
	}

	OutputFile oFile(fileOut.c_str(), header);

	FrameBuffer frameBuffer;

	for (int i = 0; i < selectedGroupsName.size(); i++) {
		frameBuffer.insert (selectedGroupsName[i].c_str(), Slice(FLOAT, (char *) datas[i], sizeof (*datas[i]) * 1, sizeof (*datas[i]) * w));
	}

	oFile.setFrameBuffer (frameBuffer);
	oFile.writePixels (h);

	for (int i = 0; i < datas.size(); i++) {
		delete [] datas[i];
	}

	return 0;
}

int main (int argc, char** argv) {

	if (argc != 3) {
		cerr << argv[0] << " expect 2 arguments [fileIn (.h5) fileOut (.exr)]" << endl;
	}

	return convertFiles(argv[1], argv[2]);

}
