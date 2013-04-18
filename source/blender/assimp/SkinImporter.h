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
 * Contributor(s): Alexander Gessler.
 *
 * ***** END GPL LICENSE BLOCK *****
 */

/** \file SkinImporter.h
 *  \ingroup assimp
 */

#ifndef INCLUDED_SKIN_IMPORTER_H
#define INCLUDED_SKIN_IMPORTER_H

#include  "bassimp_shared.h"


namespace bassimp {

class SkinImporter
{
private:

	bContext& C;
	Main& bmain;
	Scene& scene;

	const Object& ob_armature;
	const bArmature& armature;

public:

	SkinImporter(bContext& C, const Object& ob_armature);
	~SkinImporter();

public:

	void link_armature(Object& obj, const std::vector<const aiMesh*>& meshes);
};

}

#endif 
