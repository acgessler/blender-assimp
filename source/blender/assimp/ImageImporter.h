/*
 * ***** BEGIN GPL LICENSE BLOCK *****
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * Contributor(s): Alexander Gessler
 *
 * ***** END GPL LICENSE BLOCK *****
 */

/** \file MaterialImporter.h
 *  \ingroup assimp
 */

#ifndef INCLUDED_IMAGEIMPORTER_H
#define INCLUDED_IMAGEIMPORTER_H

#include "bassimp_shared.h"

namespace bassimp {

	class SceneImporter;

class ImageImporter 
{
private:

	aiString path;
	Image* ima;
	const aiScene* in_scene;

	// injected SceneImporter
	const SceneImporter& scene_imp;

private:

	bool convert_file();
	bool convert_embedded();

	void error(const char* message);
	void verbose(const char* message);
	
public:

	ImageImporter( const SceneImporter& scene_imp, const aiString& path, const aiScene* in_scene);
	~ImageImporter ();

	// run conversion
	bool convert();

	// get converted material, this resets the cached conversion
	// result and transfers ownership of it to the caller.
	Image* extract_image();
};

}

#endif
