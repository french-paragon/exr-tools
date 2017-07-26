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
#include <sstream>
#include <vector>

#include <gdal_priv.h>
#include <cpl_conv.h> // for CPLMalloc()

#include <OpenEXR/ImfInputFile.h>
#include <OpenEXR/ImfOutputFile.h>
#include <OpenEXR/ImfIO.h>
#include <OpenEXR/ImfFrameBuffer.h>
#include <OpenEXR/ImathBox.h>
#include <OpenEXR/ImfChannelList.h>

using namespace Imf;

int main(int argc, char** argv) {

	if (argc < 3) {
		std::cerr << "Error: " << argv[0] << " expect 2 arguments, inputFile and outPutFile !" << std::endl;
		return 1;
	}

	GDALDataset  *bilDataset;

	GDALAllRegister();
	//GDALRegister_EHdr(); //register EHdr driver

	bilDataset = (GDALDataset *) GDALOpen( argv[1], GA_ReadOnly );

	if (bilDataset == nullptr) {
		std::cerr << "Impossible to open file " << argv[1] << " !" << std::endl;
		return 1;
	}

	int w = bilDataset->GetRasterXSize();
	int h = bilDataset->GetRasterYSize();
	int n_band = bilDataset->GetRasterCount();

	Header header (w, h);
	std::vector<int> selectedChannels;
	std::vector<int> selectedChannelsType;
	selectedChannels.reserve(n_band);
	selectedChannelsType.reserve(n_band);

	std::vector<uint32_t*> intBuffers;
	std::vector<float*> floatBuffers;

	int countIntBuffers = 0;
	int countFloatBuffers = 0;

	for (int i = 1; i <= n_band; i++) {

		std::stringstream ss;
		ss << "B." << i;
		std::string name = ss.str();

		GDALRasterBand* band = bilDataset->GetRasterBand(i);

		GDALDataType type = band->GetRasterDataType();

		int ExrDataType;

		switch (type) {
		case GDT_UInt32:
		case GDT_UInt16:
			ExrDataType = UINT; //uint16 and uint32 will be stored as uint32.
			break;
		case GDT_Float32:
			ExrDataType = FLOAT; //float32 will be stored as float32.
		default:
			ExrDataType = NUM_PIXELTYPES; //other type will be ignored.
			break;
		}

		if (ExrDataType == NUM_PIXELTYPES) {
			continue;
		}

		header.channels().insert (name.c_str(), Channel ((PixelType) ExrDataType));
		selectedChannels.push_back(i);
		selectedChannelsType.push_back(ExrDataType);

		if (ExrDataType == FLOAT) {

			floatBuffers.push_back((float *) CPLMalloc(sizeof(float)*w*h));
			CPLErr errCode = band->RasterIO(GF_Read,
											0, 0, w, h,
											floatBuffers.back(),
											w, h,
											GDT_Float32,
											0,
											0);

			if (errCode != CE_None) {
				std::cerr << "IO error, aborting !" << std::endl;

				for (float* f : floatBuffers) {
					CPLFree(f);
				}

				for (uint32_t* i : intBuffers) {
					CPLFree(i);
				}

				return 1;
			}

		} else if (ExrDataType == UINT) {
			intBuffers.push_back((uint32_t *) CPLMalloc(sizeof(uint32_t)*w*h));
			CPLErr errCode = band->RasterIO(GF_Read,
						   0, 0, w, h,
						   intBuffers.back(),
						   w, h,
						   GDT_UInt32,
						   0,
						   0);

			if (errCode != CE_None) {
				std::cerr << "IO error, aborting !" << std::endl;

				for (float* f : floatBuffers) {
					CPLFree(f);
				}

				for (uint32_t* i : intBuffers) {
					CPLFree(i);
				}

				return 1;
			}
		}

	}

	OutputFile oFile(argv[2], header);
	FrameBuffer frameBuffer;

	for (int i = 0; i < selectedChannels.size(); i++) {

		int n_band = selectedChannels[i];
		int ExrDataType = selectedChannelsType[i];

		std::stringstream ss;
		ss << "B." << n_band;
		std::string name = ss.str();

		char * data;
		int so;
		int sw;

		if (ExrDataType == FLOAT) {
			data = (char*) floatBuffers[countFloatBuffers++];
			so = sizeof(float);
			sw = sizeof(float)*w;
		} else {
			data = (char*) intBuffers[countIntBuffers++];
			so = sizeof(uint32_t);
			sw = sizeof(uint32_t)*w;
		}

		frameBuffer.insert(name.c_str(), Slice( (PixelType) ExrDataType, data, so, sw));

	}

	oFile.setFrameBuffer (frameBuffer);
	oFile.writePixels (h);

	for (float* f : floatBuffers) {
		CPLFree(f);
	}

	for (uint32_t* i : intBuffers) {
		CPLFree(i);
	}

	return 0;

}
