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
#include <sstream>

#include "SceneImporter.h"
#include "ArmatureImporter.h"
#include "bassimp_internal.h"
#include "ED_armature.h"

extern "C" {
#	include "BKE_global.h"
#	include "BKE_main.h"
#	include "BKE_mesh.h"
#	include "BKE_displist.h"
#	include "BKE_library.h"
#	include "BKE_material.h"
#	include "BKE_texture.h"
#	include "BKE_armature.h"
#	include "BKE_depsgraph.h"

#	include "MEM_guardedalloc.h"

#	include "BLI_listbase.h"
#	include "BLI_math.h"
#	include "BLI_string.h"
}

namespace bassimp {

ArmatureImporter::ArmatureImporter(const SceneImporter& scene_imp,  const aiScene* in_scene, Scene *out_scene)
: out_scene(out_scene)
, armature()
, in_scene(in_scene)
, scene_imp(scene_imp)
{
	assert(out_scene);
	assert(in_scene);
}


ArmatureImporter::~ArmatureImporter()
{
	if (ob_armature) {
		BKE_libblock_free(&G.main->armature,armature);
	}
}


void ArmatureImporter::error(const char* message)
{
	scene_imp.error((message + std::string(" (armature)")).c_str());
}


void ArmatureImporter::verbose(const char* message)
{
	scene_imp.verbose((message + std::string(" (armature)")).c_str());
}

Object* ArmatureImporter::get_armature() const
{
	return ob_armature;
}


Object* ArmatureImporter::disown_armature()
{
	Object* m = ob_armature;
	ob_armature = NULL;
	return m;
}


void ArmatureImporter::compute_world_matrix(const aiNode* node, const aiMatrix4x4& parentWorld)
{
	// "the limit of my matrix means the limit of my world" or something like that.
	const aiMatrix4x4 myWorld = parentWorld * node->mTransformation;
	node_to_world_matrix[node] = myWorld;
	
	for (unsigned int i = 0; i < node->mNumChildren; ++i) {
		compute_world_matrix(node->mChildren[i],myWorld);
	}
}


void ArmatureImporter::get_world_matrix(const aiNode* node, float out[4][4])
{
	assert(node_to_world_matrix.find(node) != node_to_world_matrix.end());
	copy_ai_matrix(out, node_to_world_matrix[node] );
}


void ArmatureImporter::convert_node(const aiNode& node, EditBone* parent, const aiNode* parentNode, unsigned int totchild)
{
	verbose(("convert bone: " + std::string(node.mName.C_Str())).c_str());

	EditBone* const bone = ED_armature_edit_bone_add(armature, (char *)node.mName.data);
	if(parent != NULL)
	{
		bone->parent = parent;
	}
	else {
		// make this the active bone for the armature
		armature->act_edbone = bone;
	}

	// set bone name
	BLI_strncpy(bone->name,node.mName.C_Str(),sizeof(bone->name));

	float obmat[4][4];
	if(parentNode != NULL)
	{
		get_world_matrix(parentNode,obmat);
	}
	else {
		for (unsigned int y = 0; y < 4; ++y) {
			for (unsigned int x = 0; x < 4; ++x)	{
				obmat[x][y] = (x == y ? 1.0f : 0.0f);
			}
		}
	}

	float loc[3], size[3], rot[3][3], angle;
	mat4_to_loc_rot_size(loc, rot, size, obmat);
	mat3_to_vec_roll(rot, NULL, &angle);
	bone->roll = angle;

	// set head
	copy_v3_v3(bone->head, obmat[3]);

	// set tail, don't set it to head because 0-length bones are not allowed
	float vec[3] = {0.0f, 0.5f, 0.0f};
	add_v3_v3v3(bone->tail, bone->head, vec);

	// set parent tail
	if (parent && totchild == 1) {
		copy_v3_v3(parent->tail, bone->head);

		// XXX the code below taken from collada - I'm not sure if it
		// applies to Assimp, though.

		// XXX increase this to prevent "very" small bones?
		const float epsilon = 0.000001f;

		// derive leaf bone length
		float length = len_v3v3(parent->head, parent->tail);

		// treat zero-sized bone like a leaf bone
		if (length <= epsilon) {
			setup_leaf_bone(parent);
		} 
	}

	for(unsigned int i = 0; i < node.mNumChildren; ++i) {
		convert_node(*node.mChildren[i],bone,&node,node.mNumChildren);
	}

	// in second case it's not a leaf bone, but we handle it the same way
	if (node.mNumChildren != 1) {
		setup_leaf_bone(bone);
	} 
}

void ArmatureImporter::setup_leaf_bone(EditBone *bone)
{
	// pointing up
	float vec[3] = {0.0f, 0.0f, 0.1f};

	// if parent: take parent length and direction
	if (bone->parent) {
		sub_v3_v3v3(vec, bone->parent->tail, bone->parent->head);
	}

	copy_v3_v3(bone->tail, bone->head);
	add_v3_v3v3(bone->tail, bone->head, vec);
}


void ArmatureImporter::convert()
{
	// first cache world matrices for all nodes
	compute_world_matrix(in_scene->mRootNode,aiMatrix4x4());

	// create a new object and attach a also new armature
	ob_armature = util_add_object(&scene_imp.get_main(), out_scene, OB_ARMATURE, NULL);

	armature = BKE_armature_add(&scene_imp.get_main(), "armature");
	verbose("converting armature");

	bArmature* const old_armature = static_cast<bArmature*>(ob_armature->data);
	ob_armature->data = armature;
	if (--old_armature->id.us == 0) {
		BKE_libblock_free(&G.main->armature, old_armature);
	}

	// enter armature edit mode
	ED_armature_to_edit(ob_armature);

	convert_node(*in_scene->mRootNode,NULL,NULL,1);

	// exit armature edit mode
	ED_armature_from_edit(ob_armature);
	ED_armature_edit_free(ob_armature);
	DAG_id_tag_update(&ob_armature->id, OB_RECALC_OB | OB_RECALC_DATA);
}

}
