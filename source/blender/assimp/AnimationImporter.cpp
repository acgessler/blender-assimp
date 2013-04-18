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
#include "AnimationImporter.h"
#include "bassimp_internal.h"

#include "AnimEvaluator.h"

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
#	include "BKE_action.h"

#	include "MEM_guardedalloc.h"

#	include "BLI_listbase.h"
#	include "BLI_math.h"
#	include "BLI_string.h"
}

namespace bassimp {

AnimationImporter::AnimationImporter(const SceneImporter& scene_imp, const aiAnimation& in_anim, 
	const aiScene& in_scene, 
	Scene& out_scene, 
	unsigned int in_anim_index, 
	const Object* ob_armature)

: ob_armature(ob_armature)
, armature(ob_armature ? static_cast<bArmature*>(ob_armature->data) : NULL) 
, out_scene(out_scene)
, in_anim(in_anim)
, in_anim_index(in_anim_index)
, in_scene(in_scene)
, scene_imp(scene_imp)
, evaluator(ob_armature ?  new AnimEvaluator(&in_anim) : NULL)
{
	assert(!ob_armature || ob_armature->type == OB_ARMATURE);
	assert(!!ob_armature == !!armature);

	std::stringstream ss;
	ss << "animation-" << in_anim_index;
	logname = ss.str();
}


AnimationImporter::~AnimationImporter()
{
	delete evaluator;
}


void AnimationImporter::error(const char* message)
{
	scene_imp.error((message + logname).c_str());
}


void AnimationImporter::verbose(const char* message)
{
	scene_imp.verbose((message + logname).c_str());
}


void AnimationImporter::convert()
{
	for (unsigned int i = 0; i < in_anim.mNumChannels; ++i) {
		convert_node_anim(*in_anim.mChannels[i]);
	}
}


void AnimationImporter::get_rna_path_for_joint(char *joint_path, size_t count, const char* name)
{
	BLI_snprintf(joint_path, count, "pose.bones[\"%s\"]", name);
}


const aiNode* AnimationImporter::node_for_node_anim(const aiNodeAnim& anim)
{
	return scene_imp.get_node_by_name(anim.mNodeName.C_Str());
}


void AnimationImporter::convert_node_anim(const aiNodeAnim& anim)
{
	// first find out if this node is a Bone (i.e. part of armature) or a node or both
	const aiNode* nd = node_for_node_anim(anim);
	assert(nd);

	if(scene_imp.has_mesh_descendants(*nd)) {
		Object* const obj = scene_imp.get_bobject_for_node(*nd);
		assert(obj);

		convert_node_anim_for_bobject(anim, *nd, *obj);
	}

	if(armature) {
		// XXX why does BKE_armature_find_bone_name not take const bArmature*?
		Bone* const bone = BKE_armature_find_bone_name(const_cast<bArmature*>(armature), anim.mNodeName.C_Str());
		if(bone) {
			convert_node_anim_for_bone(anim,*bone);
		}
	}
}


void AnimationImporter::convert_node_anim_for_bobject(const aiNodeAnim& anim, const aiNode& nd, Object& ob)
{
	verbose(("convert node animation (target is object): " + std::string(anim.mNodeName.C_Str())).c_str());

	// 10 curves: rotation_quaternion, location and scale
	FCurve *newcu[10] = {0};

	setup_empty_fcurves(newcu,anim,NULL,false);
	populate_fcurves(newcu,anim,1.0);

	// setup correct rotation mode
	ob.rotmode = ROT_MODE_QUAT;

	ListBase* curves = NULL;

	// add curves to object
	for (int i = 0; i < 10; i++) {
		if (!newcu[i]){
			continue;
		}

		calchandles_fcurve(newcu[i]);

		// only add adt if needed - in many cases all fcurves will be NULL
		// since assimp always writes node anim channels, even if no
		// animation exists (i.e. it writes constant values taken from
		// the corr. node trafo).
		if(!curves) {
			verify_adt_action(&ob.id, 1);
			curves = &ob.adt->action->curves;
		}

		BLI_addtail(curves, newcu[i]);
	}
}


void AnimationImporter::convert_node_anim_for_bone(const aiNodeAnim& anim, Bone& bone)
{
	assert(ob_armature);
	assert(armature);

	verbose(("convert node animation (target is bone): " + std::string(anim.mNodeName.C_Str())).c_str());

	// 10 curves: rotation_quaternion, location and scale
	FCurve *newcu[10] = {0};

	char joint_path[200];
	get_rna_path_for_joint(joint_path,sizeof(joint_path),anim.mNodeName.C_Str());

	setup_empty_fcurves(newcu,anim,joint_path,true);
	populate_fcurves_bones(newcu,anim, 1.0 / in_anim.mTicksPerSecond);

	// setup correct rotation mode for pose
	bPoseChannel* const chan = BKE_pose_channel_find_name(ob_armature->pose, anim.mNodeName.C_Str());
	chan->rotmode = ROT_MODE_QUAT;

	verify_adt_action((ID *)&ob_armature->id, 1);

	// add curves to bone
	for (int i = 0; i < 10; i++) {
		if (!newcu[i]){
			continue;
		}

		calchandles_fcurve(newcu[i]);
		add_bone_fcurve(anim.mNodeName.C_Str(), newcu[i]);
	}
}


void AnimationImporter::setup_empty_fcurves(FCurve* curves_out[10], const aiNodeAnim& anim, const char* rna_base_path, bool always_create)
{
	for (int i = 0; i < 10; i++) {
		int axis = i;

		int vcount = 0;
		const char* rna_postfix;

		if (i < 4) {

			if (anim.mNumRotationKeys <= 1 && !always_create) {
				curves_out[i] = NULL;
				continue;
			}

			rna_postfix = "rotation_quaternion";
			axis = i;

			vcount = anim.mNumRotationKeys;
		}
		else if (i < 7) {
			if (anim.mNumPositionKeys <= 1 && !always_create) {
				curves_out[i] = NULL;
				continue;
			}

			rna_postfix = "location";
			axis = i - 4;

			vcount = anim.mNumPositionKeys;
		}
		else {
			if (anim.mNumScalingKeys <= 1 && !always_create) {
				curves_out[i] = NULL;
				continue;
			}

			rna_postfix = "scale";
			axis = i - 7;

			vcount = anim.mNumScalingKeys;
		}

		char rna_path[200];

		if(rna_base_path) {
			BLI_snprintf(rna_path, sizeof(rna_path), "%s.%s", rna_base_path, rna_postfix);		
		}
		else {
			BLI_strncpy(rna_path, rna_postfix, sizeof(rna_path));
		}

		curves_out[i] = create_fcurve(axis, rna_path);
		curves_out[i]->totvert = vcount;
	}
}


void AnimationImporter::populate_fcurves(FCurve* const curves_out[10], const aiNodeAnim& anim, double time_scale)
{
	if (anim.mNumRotationKeys > 1)	{
		for (unsigned int i = 0; i < anim.mNumRotationKeys; ++i)	{
			const aiQuatKey& q = anim.mRotationKeys[i];

			const float time = static_cast<float>(q.mTime * time_scale);

			add_bezt(curves_out[0], time, q.mValue.w);
			add_bezt(curves_out[1], time, q.mValue.x);
			add_bezt(curves_out[2], time, q.mValue.y);
			add_bezt(curves_out[3], time, q.mValue.z);
		}
	}

	if (anim.mNumPositionKeys > 1)	{
		for (unsigned int i = 0; i < anim.mNumPositionKeys; ++i)	{
			const aiVectorKey& v = anim.mPositionKeys[i];

			const float time = static_cast<float>(v.mTime * time_scale);

			add_bezt(curves_out[4], time, v.mValue.x);
			add_bezt(curves_out[5], time, v.mValue.y);
			add_bezt(curves_out[6], time, v.mValue.z);
		}
	}

	if (anim.mNumScalingKeys > 1)	{
		for (unsigned int i = 0; i < anim.mNumScalingKeys; ++i)	{
			const aiVectorKey& v = anim.mScalingKeys[i];

			const float time = static_cast<float>(v.mTime * time_scale);

			add_bezt(curves_out[7], time, v.mValue.x);
			add_bezt(curves_out[8], time, v.mValue.y);
			add_bezt(curves_out[9], time, v.mValue.z);
		}
	}
}


void AnimationImporter::get_anchestor_list(const aiNodeAnim& anim, AnchestorVector& anchestors)
{
	anchestors.clear();

	const aiNode* nd = node_for_node_anim(anim);
	assert(nd);

	do {
		unsigned int i = 0; 
		for (;i < in_anim.mNumChannels; ++i) {

			const aiNodeAnim* chan = in_anim.mChannels[i];
			if (chan->mNodeName == nd->mName) {
				anchestors.push_back(Anchestor(chan,nd));
				break;
			}
		}
		// its actually possible for animated nodes to have un-animated parents -
		// usually they do have dummy aiNodeAnim's with a single keyframe, but
		// this is not enforced by assimp.
		if(i == in_anim.mNumChannels) {
			anchestors.push_back(Anchestor(NULL,nd));
		}
		nd = nd->mParent;
	}
	while(nd != NULL);
	assert(anchestors.size() >= 1);
}


void AnimationImporter::get_merged_keypos_list(KeyTimeVector& keys, const AnchestorVector& anchestors, const aiNodeAnim& anim) const
{
	keys.clear();

	// merge all keyframe position in the upwards node hierarchy - this gives us the list
	// of time positions at which the evaluated animation has a keyframe, respectively.

	// try to reserve enough space upfront
	// XXX use a more conservative guess??
	size_t size_guess = 0;
	for (unsigned int i = 0; i < anchestors.size(); ++i) {
		const aiNodeAnim* const chan = anchestors[i].first;
		if(chan) {
			size_guess += std::max(std::max(chan->mNumPositionKeys, chan->mNumRotationKeys),chan->mNumScalingKeys);
		}
	}

	std::vector<unsigned int> next_pos;
	next_pos.resize(anchestors.size() * 3,0);

	keys.reserve(size_guess);

	float cur_tick = -1e10f;
	while(true) {

		float min_tick = 1e10f;

		for (unsigned int i = 0; i < anchestors.size(); ++i) {
			const aiNodeAnim* const chan = anchestors[i].first;
			if(!chan) {
				continue;
			}

			if (chan->mPositionKeys) {
				const unsigned int next_pos_key = next_pos[i*3+0];
				if (chan->mNumPositionKeys > next_pos_key && chan->mPositionKeys[next_pos_key].mTime < min_tick) {
					min_tick = chan->mPositionKeys[next_pos_key].mTime;
				}
			}

			if (chan->mRotationKeys) {
				const unsigned int next_rot_key = next_pos[i*3+1];
				if (chan->mNumRotationKeys > next_rot_key && chan->mRotationKeys[next_rot_key].mTime < min_tick) {
					min_tick = chan->mRotationKeys[next_rot_key].mTime;
				}
			}

			if (chan->mScalingKeys) {
				const unsigned int next_scl_key = next_pos[i*3+2];
				if (chan->mNumScalingKeys > next_scl_key && chan->mScalingKeys[next_scl_key].mTime < min_tick) {
					min_tick = chan->mScalingKeys[next_scl_key].mTime;
				}
			}
		}

		if (min_tick > 1e9f) {
			break;
		}

		keys.push_back(min_tick);

		const float time_epsilon = 1e-4f;

		for (unsigned int i = 0; i < anchestors.size(); ++i) {
			const aiNodeAnim* const chan = anchestors[i].first;
			if(!chan) {
				continue;
			}

			if (chan->mPositionKeys) {
				unsigned int& next_pos_key = next_pos[i*3+0];
				while (chan->mNumPositionKeys > next_pos_key && fabs(chan->mPositionKeys[next_pos_key].mTime - min_tick) < time_epsilon) {
					++next_pos_key;
				}
			}

			if (chan->mRotationKeys) {
				unsigned int& next_rot_key = next_pos[i*3+1];
				while (chan->mNumRotationKeys > next_rot_key && fabs(chan->mRotationKeys[next_rot_key].mTime - min_tick) < time_epsilon) {
					++next_rot_key;
				}
			}

			if (chan->mScalingKeys) {
				unsigned int& next_scl_key = next_pos[i*3+2];
				while (chan->mNumScalingKeys > next_scl_key && fabs(chan->mScalingKeys[next_scl_key].mTime - min_tick) < time_epsilon) {
					++next_scl_key;
				}
			}
		}
	}
}


unsigned int AnimationImporter::get_index_for_anim(const aiNodeAnim& anim) const
{
	for(unsigned int i = 0; i < in_anim.mNumChannels; ++i) {
		if (&anim == in_anim.mChannels[i]) {
			return i;
		}
	}

	assert(false);
	return 0u;
}


void AnimationImporter::populate_fcurves_bones(FCurve* const curves_out[10], const aiNodeAnim& anim, double time_scale)
{
	assert(evaluator);

	AnchestorVector anchestors;
	get_anchestor_list(anim,anchestors);

	//
	KeyTimeVector keys;
	get_merged_keypos_list(keys,anchestors,anim);


	typedef std::vector<unsigned int> AnchestorIndexVector;
	AnchestorIndexVector anchestor_indices;

	// the code below grows a bit complicated because the case
	// that there is no node animation for an element of the
	// anchestor list needs to be handled.
	const unsigned int extra_count_start = in_anim.mNumChannels;

	anchestor_indices.reserve(anchestors.size());
	for (AnchestorVector::const_iterator it = anchestors.begin(), end = anchestors.end(); it != end; ++it) {
		const Anchestor& anchestor = *it;
		if (anchestor.first) {
			anchestor_indices.push_back(get_index_for_anim(*anchestor.first));
		}
		else {
			anchestor_indices.push_back(extra_count_start + std::distance<AnchestorVector::const_iterator>(anchestors.begin(),it));
		}
	}

	float rest[4][4], irest[4][4];
	Bone* const bone = BKE_armature_find_bone_name(const_cast<bArmature*>(armature), anim.mNodeName.C_Str());
	assert(bone);

	unit_m4(rest);
	copy_m4_m4(rest, bone->arm_mat);
	invert_m4_m4(irest, rest);


	float irest_ai[4][4];

	// XXX: precompute bind matrices / take them from the aiBones
	bool found = false;
	for (unsigned int i = 0; i < in_scene.mNumMeshes; ++i) {
		const aiMesh& m = *in_scene.mMeshes[i];

		for (unsigned int j = 0; j <m.mNumBones; ++j) {
			const aiBone& b = *m.mBones[j];

			if (b.mName == anim.mNodeName) {
				found = true;
				copy_ai_matrix(irest_ai, b.mOffsetMatrix);
				break;
			}
		}
		if(found) {
			break;
		}
	} 

	if (!found) {
		error("didn't find assimp bone for bone animation, taking inverse world as bind matrix");

		aiMatrix4x4 m;
		const aiNode* nd = (*anchestors.begin()).second;
		do	{
			m = nd->mTransformation * m;
			nd = nd->mParent;
		}
		while(nd != NULL);
		copy_ai_matrix(irest_ai, m);
		invert_m4(irest_ai);
	}

	// resample - for this new values need to be generated for every input key position
	for(KeyTimeVector::const_iterator it = keys.begin(), end = keys.end(); it != end; ++it) {
		const float frame = *it;

		// only evaluate affected channels (i.e. anchestor channels)
		for (AnchestorIndexVector::const_iterator iit = anchestor_indices.begin(), iend = anchestor_indices.end(); iit != iend; ++iit) {
			const unsigned int idx = *iit;
			if (idx < extra_count_start) {
				evaluator->EvaluateSingle(frame, idx);
			}
		}

		const std::vector<aiMatrix4x4>& matrices = evaluator->GetTransformations();

		// obtain world transformation for this frame and channel
		const unsigned int last_anchestor = *anchestor_indices.rbegin();
		aiMatrix4x4 trafo =  last_anchestor < extra_count_start 
			? matrices[last_anchestor] 
			: anchestors[last_anchestor - extra_count_start].second->mTransformation;

		for (AnchestorIndexVector::const_reverse_iterator iit = anchestor_indices.rbegin() + 1, iend = anchestor_indices.rend(); iit != iend; ++iit) {
			const unsigned int idx = *iit;
			if (idx >= extra_count_start) {
				trafo *= anchestors[idx - extra_count_start].second->mTransformation;
			}
			else {
				trafo *= matrices[idx];
			}
		} 

		float mat[4][4];
		float matfra[4][4];
		copy_ai_matrix(matfra, trafo);

		// following conversion code based on collada. Matrices as follows:
		//
		// inverse blender rest matrix
		//  current world transformation (evaluated animation)
		//  inverse assimp world bind matrix
		// blender rest matrix

		// calc special matrix
		mul_serie_m4(mat, irest, matfra, irest_ai,  rest, NULL, NULL, NULL, NULL);

		float rot[4], loc[3], scale[3];

		mat4_to_quat(rot, mat);
		copy_v3_v3(loc, mat[3]);
		mat4_to_size(scale, mat);

		// add keys
		for (int i = 0; i < 10; i++) {
			if (i < 4) {
				add_bezt(curves_out[i], frame, rot[i]);
			}
			else if (i < 7) {
				add_bezt(curves_out[i], frame, loc[i - 4]);
			}
			else {
				add_bezt(curves_out[i], frame, scale[i - 7]);
			}
		}
	}
}


void AnimationImporter::add_bone_fcurve(const char* bone_name, FCurve *fcu)
{
	assert(ob_armature);
	bAction *act = ob_armature->adt->action;

	/* try to find group */
	bActionGroup *grp = BKE_action_group_find_name(act, bone_name);

	/* no matching groups, so add one */
	if (grp == NULL) {
		/* Add a new group, and make it active */
		grp = (bActionGroup *)MEM_callocN(sizeof(bActionGroup), "bActionGroup");

		grp->flag = AGRP_SELECTED;
		BLI_strncpy(grp->name, bone_name, sizeof(grp->name));

		BLI_addtail(&act->groups, grp);
		BLI_uniquename(&act->groups, grp, "Group", '.', offsetof(bActionGroup, name), 64);
	}

	/* add F-Curve to group */
	action_groups_add_channel(act, grp, fcu);
}


void AnimationImporter::add_bezt(FCurve *fcu, float fra, float value)
{
	BezTriple bez = {0};
	
	bez.vec[1][0] = fra;
	bez.vec[1][1] = value;
	bez.ipo = BEZT_IPO_LIN; /* use default interpolation mode here... */
	bez.f1 = bez.f2 = bez.f3 = SELECT;
	bez.h1 = bez.h2 = HD_AUTO;
	insert_bezt_fcurve(fcu, &bez, 0);
}


FCurve* AnimationImporter::create_fcurve(int array_index, const char *rna_path)
{
	FCurve* const fcu = (FCurve *)MEM_callocN(sizeof(FCurve), "FCurve");
	fcu->flag = (FCURVE_VISIBLE | FCURVE_AUTO_HANDLES | FCURVE_SELECTED);
	fcu->rna_path = BLI_strdupn(rna_path, strlen(rna_path));
	fcu->array_index = array_index;
	return fcu;
}



}
