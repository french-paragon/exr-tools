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
#include <string>
#include <map>
#include <fstream>
#include <math.h>

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

int main (int argc, char** argv) {

	if (argc != 4) {
		cerr << "usage: " << argv[0] << " inputLookupTable inputCatRaster outputMapExr" << endl;
		return 1;
	}

	string line;
	ifstream ftable(argv[1]);

	if ( !getline(ftable, line) ) {
		cerr << "input lookupt table empty, abort !" << endl;
		return 1;
	}

	vector<int> category;

	{
		stringstream ss(line);

		int c;
		while (ss >> c) {
			category.push_back(c);
		}
	}

	map<string, vector<float> > dict;
	vector<string> channels;

	while ( getline(ftable, line) ) {

		stringstream ss(line);

		string channel;
		ss >> channel;
		channels.push_back(channel);

		vector<float> vals(255, nanf(""));

		for (int i = 0; i < category.size(); i++) {
			float v;
			ss >> v;

			if ( !ss.good() ) {
				cerr << "error while reading file " << argv[1] << " aborting!" << endl;
				return 1;
			}

			vals[category[i]] = v;
		}

		dict.emplace(channel, vals);
	}

	cil::CImg<uint8_t> imageIn(argv[2]);

	vector<float*> pixels(channels.size());

	for (int i = 0; i < channels.size(); i++) {
		try {
			pixels[i] = new float[imageIn.width() * imageIn.height()];
		} catch (bad_alloc & ba) {

			cerr << "memory error: " << ba.what() << " abort!" << endl;

			for (int j = i-1; j >= 0; j--) {
				delete [] pixels[j];
			}

			return 1;
		}

		for (int j = 0; j < imageIn.width() * imageIn.height(); j++) {
			pixels[i][j] = dict[channels[i]][imageIn.data()[j]];
		}
	}

	Header header (imageIn.width(), imageIn.height());

	for (string c : channels) {
		header.channels().insert (c.c_str(), Channel (FLOAT));
	}

	OutputFile oFile(argv[3], header);

	FrameBuffer frameBuffer;

	for (int i = 0; i < channels.size(); i++) {

		frameBuffer.insert (channels[i], Slice(FLOAT,
											(char *) pixels[i],
											sizeof (*(pixels[i])) * 1,
											sizeof (*(pixels[i])) * imageIn.width()));

	}

	oFile.setFrameBuffer (frameBuffer);
	oFile.writePixels (imageIn.height());

	for (int i = 0; i < pixels.size(); i++) {
		delete [] pixels[i];
	}

	return true;

	return 0;

}
