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

/** \file blender/assimp/MeshImporter.cpp
 *  \ingroup assimp
 */

#include <cassert>
#include <set>

#include "SkinImporter.h"
#include "bassimp_internal.h"

extern "C" {
#	include "BKE_object.h"
#	include "DNA_armature_types.h"
#	include "DNA_modifier_types.h"
#	include "ED_mesh.h"
#	include "ED_object.h"
#	include "BKE_action.h"
#	include "BLI_listbase.h"
#	include "BLI_math.h"
}

namespace bassimp {

SkinImporter::SkinImporter(bContext& C, const Object& ob_armature)
: C(C)
, bmain(*CTX_data_main(&C))
, scene(*CTX_data_scene(&C))
, ob_armature(ob_armature)
, armature(*static_cast<bArmature*>(ob_armature.data))
{
	
}


SkinImporter::~SkinImporter()
{

}


void SkinImporter::link_armature(Object& ob, const std::vector<const aiMesh*>& meshes)
{
	ModifierData* const md = ED_object_modifier_add(NULL, &bmain, &scene, &ob, NULL, eModifierType_Armature);
	ArmatureModifierData *amd = (ArmatureModifierData *)md;
	amd->object = const_cast<Object*>(&ob_armature);

	//copy_m4_m4(ob->obmat, bind_shape_matrix);
	//
	//BKE_object_apply_mat4(ob, ob->obmat, 0, 0);


	util_set_parent(&ob, const_cast<Object*>(&ob_armature), &C, true);
	amd->deformflag = ARM_DEF_VGROUP;

	// create all vertex groups that we need
	std::vector<const aiBone*> groups;
	std::set<std::string> groups_set;
	unsigned int count = 0;

	for (std::vector<const aiMesh*>::const_iterator it = meshes.begin(); it != meshes.end(); ++it) {
		const aiMesh& m = **it;
		for (unsigned int i = 0; i < m.mNumBones; ++i) {
			const aiBone& b = *m.mBones[i];

			std::set<std::string>::const_iterator it = groups_set.find(b.mName.C_Str());
			if (it == groups_set.end()) {
				groups.push_back(&b);
			}
		}
	}

	for (std::vector<const aiBone*>::const_iterator it = groups.begin(); it != groups.end(); ++it) {
		ED_vgroup_add_name(&ob, (*it)->mName.C_Str());
	}

	unsigned int vert_offset = 0;
	for (std::vector<const aiMesh*>::const_iterator it = meshes.begin(); it != meshes.end(); ++it) {
		const aiMesh& m = **it;
		for (unsigned int i = 0; i < m.mNumBones; ++i) {
			const aiBone& b = *m.mBones[i];

			unsigned int index = groups.size();
			for (std::vector<const aiBone*>::const_iterator it = groups.begin(); it != groups.end(); ++it) {
				if (*it == &b) {
					index = std::distance<std::vector<const aiBone*>::const_iterator>(groups.begin(),it);
					break;
				}
			}

			assert(index <= groups.size());
			bDeformGroup& def = *static_cast<bDeformGroup*>(BLI_findlink(&ob.defbase, index));

			for (unsigned int j = 0; j < b.mNumWeights; ++j) {
				const aiVertexWeight& v = b.mWeights[j];

				ED_vgroup_vert_add(&ob, &def, v.mVertexId + vert_offset, v.mWeight, WEIGHT_REPLACE);
			}
		}

		vert_offset += m.mNumVertices;
	}
}

}

