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

#ifndef INCLUDED_ARMATUREIMPORTER_H
#define INCLUDED_ARMATUREIMPORTER_H

#include <string>
#include <map>

#include "bassimp_shared.h"

struct EditBone;

namespace bassimp {

class ArmatureImporter 
{
private:

	Object* ob_armature;
	bArmature* armature;
	Scene* const out_scene;
	const aiScene* const in_scene;

	const SceneImporter& scene_imp;

	std::map<const aiNode*,aiMatrix4x4> node_to_world_matrix;

private:

	void error(const char* message);
	void verbose(const char* message);

	void setup_leaf_bone(EditBone *bone);

	void compute_world_matrix(const aiNode* node, const aiMatrix4x4& parentWorld);
	void get_world_matrix(const aiNode* node, float out[4][4]);

	void convert_node(const aiNode& node, EditBone* parent, const aiNode* node_parent, unsigned int totchild);

public:

	ArmatureImporter(const SceneImporter& scene_imp, const aiScene* in_scene, Scene* out_scene);
	~ArmatureImporter();

	// run conversion
	void convert();

	// get converted armature, but ownership of the object stays here
	Object* get_armature() const;

	// disown the converted armature (i.e. prevent deletion in the destructor)
	Object* disown_armature();
};

}

#endif
