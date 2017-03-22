#ifndef GETVERSIONFIELD_H
#define GETVERSIONFIELD_H

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

#include <cstdint>

enum EXR_VERSION_FIELD_MASK{
	VERSION_MASK = 0xff,
	IS_SINGLE_PART_TILED = 0x200,
	LONG_NAME_BIT = 0x400,
	CONTAIN_DEEP_DATAS_BIT = 0x800,
	IS_MULTIPART_BIT = 0x1000
};

uint32_t getVersionField(const char* fileName);

#endif // GETVERSIONFIELD_H
