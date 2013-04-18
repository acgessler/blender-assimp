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

#ifndef INCLUDED_ANIMATIONIMPORTER_H
#define INCLUDED_ANIMATIONIMPORTER_H

#include <string>

#include "bassimp_shared.h"
#include "AnimEvaluator.h"

namespace bassimp {



class AnimationImporter 
{
private:

	typedef std::vector<float> KeyTimeVector;
	typedef std::pair<const aiNodeAnim*, const aiNode*> Anchestor;
	typedef std::vector<Anchestor> AnchestorVector;

private:

	// armature is optional, anims need not be coupled with skinning info
	const Object* ob_armature;
	const bArmature* armature;

	Scene& out_scene;
	const aiAnimation& in_anim;
	const unsigned int in_anim_index;
	const aiScene& in_scene;

	const SceneImporter& scene_imp;

	std::string logname;
	AnimEvaluator* evaluator;

private:

	void error(const char* message);
	void verbose(const char* message);

	void get_rna_path_for_joint(char *joint_path, size_t count, const char* name);
	void add_bone_fcurve(const char* bone_name, FCurve *fcu);
	FCurve* create_fcurve(int array_index, const char *rna_path);
	void add_bezt(FCurve *fcu, float fra, float value);
	const aiNode* node_for_node_anim(const aiNodeAnim& anim);
	void get_merged_keypos_list(KeyTimeVector& keys, 
		const AnchestorVector& anchestors, 
		const aiNodeAnim& anim) const;

	unsigned int get_index_for_anim(const aiNodeAnim& anim) const;
	void get_anchestor_list(const aiNodeAnim& anim, AnchestorVector& anchestors);

	void convert_node_anim(const aiNodeAnim& anim);
	void convert_node_anim_for_bobject(const aiNodeAnim& anim, const aiNode& nd, Object& ob);
	void convert_node_anim_for_bone(const aiNodeAnim& anim, Bone& bone);

	void setup_empty_fcurves(FCurve* curves_out[10], const aiNodeAnim& anim, const char* rna_path, bool always_create);
	void populate_fcurves(FCurve* const curves_out[10], const aiNodeAnim& anim, double time_scale);
	void populate_fcurves_bones(FCurve* const curves_out[10], const aiNodeAnim& anim, double time_scale);

public:

	AnimationImporter(const SceneImporter& scene_imp, const aiAnimation& in_anim, 
		const aiScene& in_scene, 
		Scene& out_scene, 
		unsigned int in_anim_index, 
		const Object* ob_armature);

	~AnimationImporter();

	// run conversion
	void convert();
};

}

#endif
