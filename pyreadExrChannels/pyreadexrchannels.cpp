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
#include <OpenEXR/ImfAttribute.h>
#include <OpenEXR/ImfStandardAttributes.h>

#include "isexr.h"

#define xstr(a) str(a)
#define str(a) #a

namespace py = pybind11;

class ImageAttributeInfos {

public :

	struct NameAttributePair {
		std::string& _name;
		Imf::Attribute*& _attribute;
	};

	struct ConstNameAttributePair {
		std::string const& _name;
		Imf::Attribute* const& _attribute;
	};

	ImageAttributeInfos (size_t size);
	ImageAttributeInfos (ImageAttributeInfos const& other);
	ImageAttributeInfos (ImageAttributeInfos && r_value_other);

	~ImageAttributeInfos();

	ImageAttributeInfos& operator= (ImageAttributeInfos const& other);
	ImageAttributeInfos& operator= (ImageAttributeInfos && r_value_other);

	NameAttributePair operator[] (size_t i);
	ConstNameAttributePair operator[] (size_t i) const;

	size_t size() const;

private :

	std::string * _names;
	Imf::Attribute** _data;
	size_t _size;
};

ImageAttributeInfos::ImageAttributeInfos (size_t size) :
	_size(size)
{
	if (_size == 0) {
		_names = nullptr;
		_data = nullptr;

		return;
	}

	_names = new std::string[_size];
	_data = new Imf::Attribute*[_size];

	std::fill(_data, _data+_size, nullptr);
}
ImageAttributeInfos::ImageAttributeInfos (ImageAttributeInfos const& other) :
	ImageAttributeInfos(other._size)
{
	for (size_t i = 0; i < _size; i++) {
		_names[i] = other._names[i];
		_data[i] = (other._data[i] != nullptr) ? other._data[i]->copy() : nullptr;
	}
}
ImageAttributeInfos::ImageAttributeInfos (ImageAttributeInfos && r_value_other) {

	_size = r_value_other._size;
	_names = r_value_other._names;
	_data = r_value_other._data;
	r_value_other._size = 0;
	r_value_other._names = nullptr;
	r_value_other._data = nullptr;

}

ImageAttributeInfos::~ImageAttributeInfos() {
	delete []  _names;

	for (size_t i = 0; i < _size; i++) {
		if (_data[i] == nullptr) {
			delete _data[i];
		}
	}

	delete [] _data;
}

ImageAttributeInfos& ImageAttributeInfos::operator= (ImageAttributeInfos const& other) {

	if (this == &other) {
		return *this;
	}

	delete []  _names;

	for (size_t i = 0; i < _size; i++) {
		if (_data[i] == nullptr) {
			delete _data[i];
		}
	}

	delete [] _data;

	_size = other._size;

	if (_size == 0) {
		_names = nullptr;
		_data = nullptr;

		return *this;
	}

	_names = new std::string[_size];
	_data = new Imf::Attribute*[_size];

	for (size_t i = 0; i < _size; i++) {
		_names[i] = other._names[i];
		_data[i] = (other._data[i] != nullptr) ? other._data[i]->copy() : nullptr;
	}

	return *this;

}
ImageAttributeInfos& ImageAttributeInfos::operator= (ImageAttributeInfos && r_value_other) {

	if (this == &r_value_other) {
		return *this;
	}

	delete []  _names;

	for (size_t i = 0; i < _size; i++) {
		if (_data[i] == nullptr) {
			delete _data[i];
		}
	}

	delete [] _data;

	_size = r_value_other._size;
	_names = r_value_other._names;
	_data = r_value_other._data;
	r_value_other._size = 0;
	r_value_other._names = nullptr;
	r_value_other._data = nullptr;

	return *this;

}

ImageAttributeInfos::NameAttributePair ImageAttributeInfos::operator[] (size_t i) {

	if (i >= _size) {
		throw std::out_of_range("Tried to acess out of bounds memory !");
	}

	return {_names[i], _data[i]};
}
ImageAttributeInfos::ConstNameAttributePair ImageAttributeInfos::operator[] (size_t i) const {

	if (i >= _size) {
		throw std::out_of_range("Tried to acess out of bounds memory !");
	}

	return {_names[i], _data[i]};
}

size_t ImageAttributeInfos::size() const
{
return _size;
}

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

void writeExrFile(std::string out_file, std::map<std::string, py::array_t<float> > channels, ImageAttributeInfos const& infos = ImageAttributeInfos(0)) {

	size_t height = channels.begin()->second.shape(0);
	size_t width = channels.begin()->second.shape(1);

	Imf::Header header (static_cast<int>(width), static_cast<int>(height));
	Imf::FrameBuffer frameBuffer;

	for (size_t i = 0; i < infos.size(); i++) {
		auto&& info = infos[i];
		header.insert(info._name, *info._attribute);
	}

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

ImageAttributeInfos getAttributesInfos(std::string file, std::vector<std::string> attributes) {

	const char * fileName = file.c_str();

	if (!isExrFile(fileName)) {
		throw std::invalid_argument("Given file is not a valid exr file, a valid exr file should be provided: " + file);
	}

	Imf::InputFile exr_file (fileName);

	std::vector<std::string> selectedAttributes;
	selectedAttributes.reserve(attributes.size());

	for (auto it = exr_file.header().begin(); it != exr_file.header().end(); ++it) {

		if (std::find(attributes.begin(), attributes.end(), it.name()) != attributes.end())  {
			selectedAttributes.push_back(it.name());
		}

	}

	ImageAttributeInfos infos(selectedAttributes.size());

	for (size_t i = 0; i < selectedAttributes.size(); i++) {
		auto&& info = infos[i];
		info._name = selectedAttributes[i];
		info._attribute = exr_file.header().find(selectedAttributes[i]).attribute().copy();
	}

	return infos;

}

PYBIND11_PLUGIN(LIB_NAME) {
	py::module m(xstr(LIB_NAME), "pybind11 read exr channels module");

	py::class_<ImageAttributeInfos>(m, "ImageAttributeInfos")
			.def("attrsNamesList", [] (const ImageAttributeInfos & infos) {
		std::vector<std::string> names(infos.size());

		for (size_t i = 0; i < infos.size(); i++) {
			names[i] = infos[i]._name;
		}

		return names;

	})
			.def("getAttributeValue", [] (const ImageAttributeInfos & infos, std::string const& attribute_name) -> py::object {

		Imf::Attribute* attr = nullptr;

		for (size_t i = 0; i < infos.size(); i++) {
			auto&& info = infos[i];

			if (info._name == attribute_name) {
				attr = info._attribute;
				break;
			}
		}

		if (attr == nullptr) {
			return py::none();
		}

		if (std::string(attr->typeName()) == "string") {
			Imf::StringAttribute att = Imf::StringAttribute::cast(*attr);
			return py::cast(att.value());
		}

		if (std::string(attr->typeName()) ==  "stringvector") {
			Imf::StringVectorAttribute att = Imf::StringVectorAttribute::cast(*attr);
			return py::cast(att.value());
		}

		if (std::string(attr->typeName()) == "float") {
			Imf::FloatAttribute att = Imf::FloatAttribute::cast(*attr);
			return py::cast(att.value());
		}

		return py::none();

	});

	m.def("readExrChannel", &readExrChannel, "Read a channel from a file", py::arg("in_file"), py::arg("channel"));
	m.def("readExrLayer", &readExrLayer, "Read a complete layer from a file", py::arg("in_file"), py::arg("layer"));
	m.def("writeExrFile", &writeExrFile, "write an exr file from a dict of channels", py::arg("out_file"), py::arg("channels"), py::arg("imageAttributesInfo") = ImageAttributeInfos(0));
	m.def("getAttributesInfos", &getAttributesInfos, "Get an object containing information about the attributes of an exr files.", py::arg("in_file"), py::arg("attributes"));

	return m.ptr();
}
