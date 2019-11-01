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
#include <set>
#include <vector>

#include <OpenEXR/ImfInputFile.h>
#include <OpenEXR/ImfTiledInputFile.h>
#include <OpenEXR/ImfIO.h>
#include <OpenEXR/ImfFrameBuffer.h>
#include <OpenEXR/ImathBox.h>
#include <OpenEXR/ImfChannelList.h>
#include <OpenEXR/ImfAttribute.h>
#include <OpenEXR/ImfStandardAttributes.h>

#include "isexr.h"
#include "getversionfield.h"

using namespace Imf;
using namespace std;

void treatFileScanLines(const char* fileName, bool list_attributes) {

	cout << "File: " << fileName << endl;
	InputFile file (fileName);

	if (list_attributes) {
		cout << endl << "Image attributes :" << endl;

		for(auto it = file.header().begin(); it != file.header().end(); ++it) {
			cout << endl << it.name() << "(" << it.attribute().typeName() << ")";

			if (std::string(it.attribute().typeName()) == "string") {
				StringAttribute att = StringAttribute::cast(it.attribute());
				cout << ": " << endl << att.value();
			}

			if (std::string(it.attribute().typeName()) == "stringvector") {
				StringVectorAttribute att = StringVectorAttribute::cast(it.attribute());
				cout << ": ";
				for (string line : att.value()) {
					cout << endl << line;
				}
				cout << endl;
			}

			if (std::string(it.attribute().typeName()) == "float") {
				FloatAttribute att = FloatAttribute::cast(it.attribute());
				cout << ": " << endl << att.value();
			}

			cout << endl;
		}

		cout << endl;
	}

	const ChannelList &channels = file.header().channels();

	set<string> layerNames;
	channels.layers (layerNames);

	if (layerNames.size() ) {

		for (set<string>::const_iterator i = layerNames.begin(); i != layerNames.end(); ++i) {
			cout << "\t" << "Layer " << *i << endl;

			ChannelList::ConstIterator layerBegin, layerEnd;
			channels.channelsInLayer (*i, layerBegin, layerEnd);

			for (ChannelList::ConstIterator j = layerBegin; j != layerEnd; ++j) {
				cout << "\t\t" << "channel " << j.name() << endl;
			}
		}

	} else {
		for (ChannelList::ConstIterator it = channels.begin(); it != channels.end(); ++it) {
			cout << "\t" << "channel " << it.name() << endl;
		}
	}

	cout << endl;
}

int main (int argc, char** argv) {

	if (argc > 1) {

		bool display_attr = false;
		int argv_offset = 1;

		if (argc > 2) {
			if (std::string(argv[1]) == "-d") {
				argv_offset = 2;
				display_attr = true;
			}
		}

		for (int i = argv_offset; i < argc; i++) {
			if (isExrFile(argv[i])) {
				uint32_t v_number = getVersionField(argv[i]);

				if (v_number & IS_MULTIPART_BIT) {
					cerr << "File: " << argv[i] << " is multipart, skipping it for the moment" << endl;
				} else if (v_number & IS_SINGLE_PART_TILED) {
					cerr << "File: " << argv[i] << " is tiled" << endl;
					treatFileScanLines(argv[i], display_attr);
				} else {
					treatFileScanLines(argv[i], display_attr);
				}
			}
		}
	}

	return 0;

}
