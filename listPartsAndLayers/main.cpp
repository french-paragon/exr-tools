#include <iostream>
#include <set>

#include <OpenEXR/ImfInputFile.h>
#include <OpenEXR/ImfTiledInputFile.h>
#include <OpenEXR/ImfIO.h>
#include <OpenEXR/ImfFrameBuffer.h>
#include <OpenEXR/ImathBox.h>
#include <OpenEXR/ImfChannelList.h>

#include "isexr.h"
#include "getversionfield.h"

using namespace Imf;
using namespace std;

void treatFileScanLines(const char* fileName) {

	cout << "File: " << fileName << endl;
	InputFile file (fileName);

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
		for (int i = 1; i < argc; i++) {
			if (isExrFile(argv[i])) {
				uint32_t v_number = getVersionField(argv[i]);

				if (v_number & IS_MULTIPART_BIT) {
					cerr << "File: " << argv[i] << " is multipart, skipping it for the moment" << endl;
				} else if (v_number & IS_SINGLE_PART_TILED) {
					cerr << "File: " << argv[i] << " is tiled" << endl;
					treatFileScanLines(argv[i]);
				} else {
					treatFileScanLines(argv[i]);
				}
			}
		}
	}

	return 0;

}
