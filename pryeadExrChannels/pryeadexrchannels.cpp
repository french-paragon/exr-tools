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

#include "pybind11/pybind11.h"
#include "pybind11/numpy.h"
#include "pybind11/stl.h"

#include <string>
#include <iterator>

#include <OpenEXR/ImfArray.h>
#include <OpenEXR/ImfInputFile.h>
#include <OpenEXR/ImfOutputFile.h>
#include <OpenEXR/ImfIO.h>
#include <OpenEXR/ImfFrameBuffer.h>
#include <OpenEXR/ImathBox.h>
#include <OpenEXR/ImfChannelList.h>

#include "isexr.h"

#define xstr(a) str(a)
#define str(a) #a

namespace py = pybind11;

py::array_t<float> readExrChannel(std::string file, std::string channel) {

	const char * fileName = file.c_str();

	if (!isExrFile(fileName)) {
		return py::array_t<double>();
	}

	Imf::InputFile exr_file (fileName);

	const Imf::ChannelList &channels = exr_file.header().channels();
	const Imf::Channel *channelPtr = channels.findChannel(channel.c_str());


	if (channelPtr == nullptr) {
		return py::array_t<double>();
	}

	Imath::Box2i dw = exr_file.header().dataWindow();
	int width  = dw.max.x - dw.min.x + 1;
	int height = dw.max.y - dw.min.y + 1;

	Imf::Array2D<float> pixels;
	pixels.resizeErase(height, width);

    py::array_t<float> r(std::vector<size_t>({static_cast<size_t>(height), static_cast<size_t>(width)}));

	Imf::FrameBuffer frameBuffer;

	frameBuffer.insert (channel.c_str(),                                  // name
						Imf::Slice (Imf::FLOAT,                         // type
                                    static_cast<char*>(static_cast<void*>(r.mutable_data())),
									sizeof (float) * 1,    // xStride
                                    sizeof (float) * static_cast<unsigned long>(width),// yStride
									1, 1,                          // x/y sampling
									FLT_MAX));                     // fillValue

	exr_file.setFrameBuffer (frameBuffer);
	exr_file.readPixels (dw.min.y, dw.max.y);

	return r;
}


py::array_t<float> readExrLayer(std::string file, std::string layer) {

    const char * fileName = file.c_str();

    if (!isExrFile(fileName)) {
        return py::array_t<double>();
    }

    Imf::InputFile exr_file (fileName);

    const Imf::ChannelList &channels = exr_file.header().channels();

    Imf::ChannelList::ConstIterator layerBegin, layerEnd;
    channels.channelsInLayer (layer, layerBegin, layerEnd);

    Imath::Box2i dw = exr_file.header().dataWindow();
    int width  = dw.max.x - dw.min.x + 1;
    int height = dw.max.y - dw.min.y + 1;

    int nChannel = 0;
    for (Imf::ChannelList::ConstIterator j = layerBegin; j != layerEnd; ++j) {
        nChannel++;
    }

    if (nChannel <= 0) {
        return py::array_t<double>();
    }

    py::array_t<float> r(std::vector<size_t>({static_cast<size_t>(height), static_cast<size_t>(width), static_cast<size_t>(nChannel)}),
                         std::vector<size_t>({sizeof(float) * static_cast<size_t>(width), sizeof(float) * 1, sizeof(float) * static_cast<size_t>(height*width)}));

    Imf::FrameBuffer frameBuffer;

    int n_chan = 0;
    for (Imf::ChannelList::ConstIterator j = layerBegin; j != layerEnd; ++j, n_chan++) {

        frameBuffer.insert (j.name(),                                  // name
                            Imf::Slice (Imf::FLOAT,                         // type
                                        static_cast<char*>(static_cast<void*>(r.mutable_data(0, 0, n_chan))),
                                        sizeof (float) * 1,    // xStride
                                        sizeof (float) * static_cast<unsigned long>(width),// yStride
                                        1, 1,                          // x/y sampling
                                        FLT_MAX));

    }

    exr_file.setFrameBuffer (frameBuffer);
    exr_file.readPixels (dw.min.y, dw.max.y);

    return r;

}


void writeExrFile(std::string out_file, std::map<std::string, py::array_t<float> > channels) {

	size_t height = channels.begin()->second.shape(0);
	size_t width = channels.begin()->second.shape(1);

	Imf::Header header (static_cast<int>(width), static_cast<int>(height));
	Imf::FrameBuffer frameBuffer;

	for (auto keyVal : channels) {

		if (keyVal.second.ndim() != 2) {
			throw std::invalid_argument("The dict containing the channels should only contain 2-dimensional images.");
		}

		if (keyVal.second.shape(0) != height || keyVal.second.shape(1) != width) {
			throw std::invalid_argument("All channels must have the same size !");
		}

		header.channels().insert (keyVal.first, Imf::Channel (Imf::FLOAT));

		frameBuffer.insert (keyVal.first, Imf::Slice(Imf::FLOAT,
													 static_cast<char*>(const_cast<void*>(static_cast<const void*>(keyVal.second.data()))) ,
													 keyVal.second.strides(1),
													 keyVal.second.strides(0)));


	}

	Imf::OutputFile oFile(out_file.c_str(), header);

	oFile.setFrameBuffer (frameBuffer);
	oFile.writePixels (static_cast<int>(height));

}

PYBIND11_PLUGIN(LIB_NAME) {
	py::module m(xstr(LIB_NAME), "pybind11 read exr channels module");

	m.def("readExrChannel", &readExrChannel, "Read a channel from a file");
	m.def("readExrLayer", &readExrLayer, "Read a complete layer from a file");
	m.def("writeExrFile", &writeExrFile, "write an exr file from a dict of channels");

	return m.ptr();
}
