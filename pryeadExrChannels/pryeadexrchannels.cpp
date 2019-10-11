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

#include <string>

#include <OpenEXR/ImfArray.h>
#include <OpenEXR/ImfInputFile.h>
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


PYBIND11_PLUGIN(LIB_NAME) {
	py::module m(xstr(LIB_NAME), "pybind11 read exr channels module");

	m.def("readExrChannel", &readExrChannel, "Read a channel from a file");

	return m.ptr();
}
