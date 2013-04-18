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

/** \file blender/assimp/SceneImporter.cpp
 *  \ingroup assimp
 */

#include <iostream>
#include <string>
#include <cassert>

#include "SceneImporter.h"
#include "MeshImporter.h"
#include "MaterialImporter.h"
#include "SceneOrientation.h"
#include "ArmatureImporter.h"
#include "AnimationImporter.h"
#include "SkinImporter.h"

#include "bassimp_internal.h"

extern "C"
{
#include "BKE_global.h"
#include "BKE_main.h"
#include "BKE_object.h"
#include "BKE_camera.h"
#include "BKE_lamp.h"
#include "BKE_node.h"
#include "BKE_mesh.h"
#include "BKE_library.h"
#include "BKE_context.h"
#include "BKE_report.h"

}

#include "../../extern/assimp/include/assimp/postprocess.h"

namespace bassimp {

	using namespace Assimp;

SceneImporter::SceneImporter(const char* path, bContext& C, const bassimp_import_settings& settings)
: path(path)
, C(C)
, scene()
, out_scene(CTX_data_scene(&C))
, armature()
, settings(settings)
, log_pipe(!!settings.enableAssimpLog ? settings.reports : NULL)
, root_collapsed()
{
}


SceneImporter::~SceneImporter()
{
	for (size_t i = 0; i < materials.size(); ++i) {
		if (materials[i]) {
			// is the material actually used? if so, make sure the object is preserved
			if (materials_used[i]) {
				materials[i]->disown_material();
			}
			delete materials[i];
		}
	}
}


void SceneImporter::error(const char* what) const
{
	if(!settings.reports) {
		return;
	}
	BKE_reportf(settings.reports,RPT_ERROR,"bassimp error: %s",what);
}


void SceneImporter::verbose(const char* what) const
{
	if(!settings.reports) {
		return;
	}
	BKE_reportf(settings.reports,RPT_INFO,"bassimp verbose: %s",what);
}


Assimp::Importer& SceneImporter::get_importer()
{
	return importer;
}


const char* SceneImporter::get_file_path() const
{
	return path;
}


const bassimp_import_settings& SceneImporter::get_settings() const
{
	return settings;
}


void SceneImporter::configure_importer()
{
	if (settings.nolines) {
		importer.SetPropertyInteger(AI_CONFIG_PP_SBP_REMOVE,aiPrimitiveType_LINE | aiPrimitiveType_POINT);
	}

	// do not generate skeleton meshes, Blender's armature viz does the same job much better
	importer.SetPropertyInteger(AI_CONFIG_IMPORT_NO_SKELETON_MESHES,1);
}


unsigned int SceneImporter::get_assimp_flags() const
{
	unsigned int flags = aiProcess_JoinIdenticalVertices;

	// needed for AI_CONFIG_PP_SBP_REMOVE
	if (settings.nolines) {
		flags |= aiProcess_SortByPType;

	}

	if (settings.triangulate) {
		flags |= aiProcess_Triangulate;
	}

#ifdef _DEBUG
	flags |= aiProcess_ValidateDataStructure;
#endif

	return flags;
}


bool SceneImporter::import()
{
	const unsigned int postprocessing_flags = get_assimp_flags();
	configure_importer();

	if(!importer.ReadFile(path, postprocessing_flags)) {
		error(("failed to import file, assimp error message is\'" + std::string(importer.GetErrorString()) + "\'").c_str());

		scene =  NULL;
		return false;
	}

	verbose((std::string("assimp import successful: ") + get_file_path()).c_str());
	scene = importer.GetScene();
	return true;
}


bool SceneImporter::apply()
{
	if (!scene) {
		return false;
	}

	collapse_root_node();

	populate_node_name_map(scene->mRootNode);

	// generate the subgraph of all mesh-carrier nodes
	populate_mesh_node_graph(scene->mRootNode);

	handle_coordinate_space();
	handle_scale();

	if(settings.read_materials) {
		convert_materials();
	}

	// check if there are bones present because this changes
	// the way how the assimp node hierarchy is imported.
	has_bones = false;
	for (unsigned int i = 0; i < scene->mNumMeshes; ++i) {
		if (scene->mMeshes[i]->mNumBones > 0)	{
			has_bones = true;
			break;
		}
	}

	convert_node(*scene->mRootNode,NULL);

	if(settings.read_armature) {
		convert_armature();
		convert_skin();
	}

	if(settings.read_animations) {
		convert_animations();
	}

	verbose("conversion to blender Scene ok");
	return true;
}


void SceneImporter::collapse_root_node()
{
	root_collapsed = false;
	const aiNode* const nd = scene->mRootNode;

	// look for reasons why we couldn't get rid of the root node
	if(nd->mNumMeshes) {
		return;
	}

	if(!nd->mNumChildren) {
		return;
	}

	// lights assigned to root
	for (unsigned int j = 0; j < scene->mNumLights; ++j) {
		if (scene->mLights[j]->mName == nd->mName) {
			return;
		}
	}

	// cameras assigned to root
	for (unsigned int j = 0; j < scene->mNumCameras; ++j) {
		if (scene->mCameras[j]->mName == nd->mName) {
			return;
		}
	}

	// animations assigned to root
	for (unsigned int i = 0; i < scene->mNumAnimations; ++i) {
		const aiAnimation& anim = *scene->mAnimations[i];

		for (unsigned int j = 0; j < anim.mNumChannels; ++j) {
			if (anim.mChannels[j]->mNodeName == nd->mName) {
				return;
			}
		}
	}

	// non-identity root transform and animated children
	if (!nd->mTransformation.IsIdentity()) {
		for (unsigned int k = 0; k < nd->mNumChildren; ++k) {
			for (unsigned int i = 0; i < scene->mNumAnimations; ++i) {

				const aiAnimation& anim = *scene->mAnimations[i];
				for (unsigned int j = 0; j < anim.mNumChannels; ++j) {

					if (anim.mChannels[j]->mNodeName == nd->mChildren[k]->mName) {
						return;
					}
				}
			}
		}
	}

	// ok, drop the root node, apply its trafo to all children
	for (unsigned int k = 0; k < nd->mNumChildren; ++k) {
		nd->mChildren[k]->mTransformation = nd->mTransformation * nd->mChildren[k]->mTransformation;
	}

	// ignore const-correctness another time since this is not
	// dangerous either, but it may prevent nasty conversion errors.
	const_cast<aiNode*>(nd)->mTransformation = aiMatrix4x4();

	root_collapsed = true;

	verbose("collapse root node");
}


void SceneImporter::handle_scale()
{
	// XXX make this configurable
}


void SceneImporter::handle_coordinate_space()
{
	// note: the const_cast is definitely a hack to avoid creating a copy
	// of the assimp scene. Assimp does not allow manipulating its
	// data structure (for good reasons) - scaling and rotating, however, 
	// just changes a few float values and is thus totally safe.
	SceneRotator rot = SceneRotator(const_cast<aiScene*>(scene));

	// blender uses right-handed z-up, y-into-screen coordinate system,
	// assimp uses right-handled z-out-of-screen, z-up. All we need to
	// do is a 90 degrees rotation around x.
	rot.rotate_90deg_xpositive();
}

void SceneImporter::convert_armature() 
{
	if (!has_bones)	{
		return;
	}

	verbose("found bones, trying to import them as armature");

	ArmatureImporter imp(*this,scene,out_scene);
	imp.convert();

	armature = imp.get_armature();

	// preserve the armature
	imp.disown_armature();
}


void SceneImporter::convert_skin() 
{
	if (!has_bones || !armature)	{
		return;
	}

	SkinImporter imp(C,*armature);
	for (ObjectToMeshMap::const_iterator it = meshes_by_object.begin(), end = meshes_by_object.end(); it != end; ++it) {
		imp.link_armature(*(*it).first,(*it).second);
	}
}


void SceneImporter::convert_animations() 
{
	for (int i = 0; i < scene->mNumAnimations; ++i)
	{
		AnimationImporter animp(*this,*scene->mAnimations[i],*scene,*out_scene,i,armature);
		animp.convert();
	}
}


void SceneImporter::convert_materials() 
{
	materials.resize(scene->mNumMaterials);
	materials_used.resize(scene->mNumMaterials);

	for (unsigned int i = 0; i < scene->mNumMaterials; ++i) {
		materials[i] = new MaterialImporter(*this,scene->mMaterials[i],scene,out_scene,i);
		materials[i]->convert();
	}
}


unsigned int SceneImporter::resolve_matid(unsigned int src) const
{
	if (src >= materials.size()) {
		error("material index out of range, ignoring");
		// assimp does create a default material so we can rely on this
		return 0;
	}

	materials_used[src] = true;
	return src;
}


const MaterialImporter& SceneImporter::get_material(unsigned int idx) const
{
	const unsigned int i = resolve_matid(idx);
	return *materials[i];
}


void SceneImporter::populate_node_name_map(const aiNode* node)
{
	nodes_by_name[node->mName.C_Str()] = node;
	for (unsigned int i = 0; i < node->mNumChildren; ++i) {
		populate_node_name_map(node->mChildren[i]);
	}
}


const aiNode* SceneImporter::get_node_by_name(const std::string& node) const
{
	const NodeNameMap::const_iterator it = nodes_by_name.find(node);
	return it == nodes_by_name.end() ? NULL : (*it).second;
}


Object* SceneImporter::get_bobject_for_node(const aiNode& nd) const
{
	const NodeToObjectMap::const_iterator it = objects_by_node.find(&nd);
	return it == objects_by_node.end() ? NULL : (*it).second;
}


bool SceneImporter::has_mesh_descendants(const aiNode& in_node) const
{
	return nodes_with_mesh_descendants.find(&in_node) != nodes_with_mesh_descendants.end();
}


void SceneImporter::populate_mesh_node_graph(const aiNode* node)
{
	if (!node->mNumChildren)
	{
		// this is a leaf node, so traverse it upwards until we find a node
		// that carries a mesh. Starting with this node, insert all 
		// upwards nodes into the list of mesh nodes. 
		bool flag = false;
		do	{
			if (node->mNumMeshes > 0)	{
				flag = true;
			}

			if (nodes_with_mesh_descendants.find(node) != nodes_with_mesh_descendants.end()) {
				break;
			}

			if (flag)	{
				nodes_with_mesh_descendants.insert(node);
			}

			node = node->mParent;
		}
		while(node != NULL);
	}
	else {
		for (unsigned int i = 0; i < node->mNumChildren; ++i)	{
			populate_mesh_node_graph(node->mChildren[i]);
		}
	}
}


void SceneImporter::convert_node(const aiNode& in_node, Object* out_parent, bool is_root) 
{
	typedef std::vector<Object *> ObjectVector;
	ObjectVector objects_done;

	unsigned int meshes = 0, cameras = 0, lights = 0;
	verbose(("convert node: " + std::string(in_node.mName.C_Str())).c_str());

	const bool root_drop = is_root && root_collapsed;

	// attach meshes
	if (in_node.mNumMeshes) {

		// XXX join meshes on their name -- for each list of meshes we create a blender mesh
		std::vector<const aiMesh*> in_meshes;
		for (unsigned int i = 0, c = in_node.mNumMeshes; i < c; ++i) {
			in_meshes.clear();
			in_meshes.push_back(scene->mMeshes[in_node.mMeshes[i]]);
			Object* const obj = convert_mesh(in_meshes,in_node.mName.C_Str());

			if(obj != NULL) {
				objects_done.push_back(obj);
				++meshes;

				// store object->mesh mapping, we later need it to be able to assign skins
				meshes_by_object[obj] = in_meshes;
			}
		}
	}

	// attach lights (these are located in the global aiScene and referenced by name)
	if(scene->mNumLights && settings.read_lights) {
		for (unsigned int i = 0, c = scene->mNumLights; i < c; ++i) {
			const aiLight* const light = scene->mLights[i];

			if (light->mName == in_node.mName) {
				Object* const obj = convert_light(*light);

				if(obj != NULL) {
					objects_done.push_back(obj);
					++lights;
				}
			}
		}
	}

	// attach cameras (these are located in the global aiScene and referenced by name)
	if(scene->mNumCameras && settings.read_cameras) {
		for (unsigned int i = 0, c = scene->mNumCameras; i < c; ++i) {
			const aiCamera* const cam = scene->mCameras[i];

			if (cam->mName == in_node.mName) {
				Object* const obj = convert_camera(*cam);

				if(obj != NULL) {
					objects_done.push_back(obj);
					++cameras;
				}
			}
		}
	}

	// note:
	// empty nodes are usually bones and should thus not be represented by OB_EMPTY
	// objects - these would just spoil Blender's scene outline :-) An exception
	// are nodes that carry meshes in the subtree rooted at them. These must
	// be preserved to avoid loosing the trafo info.

	if (meshes + cameras + lights  == 0 &&  (!has_bones || has_mesh_descendants(in_node)) && !root_drop) {
		
		Object* const obj = util_add_object(out_scene, OB_EMPTY, in_node.mName.C_Str());
		objects_done.push_back(obj);
	}

	// note:
	// for animation purposes, we need a single anchor node to attach animation channels to.
	// therefore, if we got multiple objects this far, we need to add another OB_EMPTY
	// object into the hierarchy to serve as dummy parent. Since all objects need to
	// have unique names, the mesh, lamp and camera nodes already created need to be
	// renamed.

	else if (meshes + cameras + lights > 1)	{
		assert(objects_done.size() > 1);

		Object* const obj = util_add_object(out_scene, OB_EMPTY, in_node.mName.C_Str());
		objects_done.push_back(obj);

		const std::string name = in_node.mName.C_Str();

		ObjectVector::iterator it = objects_done.begin();
		for (unsigned int i = 0; i < meshes; ++i, ++it) {
			rename_id(&(*it)->id,(name+"-mesh").c_str());
		}

		for (unsigned int i = 0; i < lights; ++i, ++it) {
			rename_id(&(*it)->id,(name+"-lamp").c_str());
		}

		for (unsigned int i = 0; i < cameras; ++i, ++it) {
			rename_id(&(*it)->id,(name+"-camera").c_str());
		}

		assert(it+1 == objects_done.end());
	}


	if (objects_done.empty() && !root_drop) {
		return;
	}

	Object* anchor = NULL; 

	if(!is_root || !root_collapsed) {

		anchor = objects_done.back();
		convert_node_transform(in_node, *anchor); 

		objects_by_node[&in_node] = anchor;
		if(out_parent) {
			util_set_parent(anchor, out_parent ,&C, true);
		}

		for (ObjectVector::iterator it = objects_done.begin(), end = objects_done.end() - 1; it != end; ++it) {
			Object& obj = **it;

			set_identity_transform(obj);
			util_set_parent(&obj,anchor,&C,true);
		}
	}

	// recursively convert child nodes
	for (unsigned int i = 0, c = in_node.mNumChildren; i < c; ++i) {
		convert_node(*in_node.mChildren[i],anchor, false);
	}
}


void SceneImporter::set_identity_transform(Object& out_obj) const
{
	copy_ai_matrix(out_obj.obmat,aiMatrix4x4());
	BKE_object_apply_mat4(&out_obj, out_obj.obmat, 0, 0);
}


void SceneImporter::convert_node_transform(const aiNode& node_in, Object& out_obj) const
{
	const aiMatrix4x4& m = node_in.mTransformation;
	
	copy_ai_matrix(out_obj.obmat,m);
	BKE_object_apply_mat4(&out_obj, out_obj.obmat, 0, 0);
}


Object* SceneImporter::convert_light(const aiLight& light) const
{
	Object* const ob = util_add_object(out_scene, OB_LAMP, NULL);
	verbose(("convert light: " + std::string(light.mName.C_Str())).c_str());

	Lamp* const lamp = static_cast<Lamp*>(BKE_lamp_add(light.mName.C_Str()));
	if (!lamp) {
		error("Cannot create lamp");
		return NULL;
	}

	lamp->r = light.mColorDiffuse.r;
	lamp->g = light.mColorDiffuse.g;
	lamp->b = light.mColorDiffuse.b;

	if(light.mColorSpecular.r < 1e-4f && light.mColorSpecular.g < 1e-4f && light.mColorSpecular.b < 1e-4f)
	{
		lamp->mode |= LA_NO_SPEC;
	}

	switch(light.mType) 
	{
		case aiLightSource_UNDEFINED:

		case aiLightSource_DIRECTIONAL:
			lamp->type = LA_SUN;
			break;

		case aiLightSource_POINT:
			lamp->type = LA_LOCAL;
			break;

		case aiLightSource_SPOT:
			lamp->type = LA_SPOT;
			
			break;
	}

	// XXX should also set Att1,Att2?
	if (light.mAttenuationQuadratic > 0.0f)
	{
		lamp->falloff_type = LA_FALLOFF_INVSQUARE;
	}
	else if (light.mAttenuationLinear > 0.0f)
	{
		lamp->falloff_type = LA_FALLOFF_INVLINEAR;
	}
	else {
		lamp->falloff_type = LA_FALLOFF_CONSTANT;
	}

	Lamp* const old_lamp = static_cast<Lamp*>(ob->data);
	ob->data = lamp;
	if (--old_lamp->id.us == 0) {
		BKE_libblock_free(&G.main->lamp, old_lamp);
	}

	return ob;
}


Object* SceneImporter::convert_camera(const aiCamera& camera) const
{
	Object* const ob = util_add_object(out_scene, OB_CAMERA, NULL);
	verbose(("convert camera: " + std::string(camera.mName.C_Str())).c_str());

	Camera* const cam = static_cast<Camera*>(BKE_camera_add(camera.mName.C_Str()));
	if (!cam) {
		error("Cannot create camera");
		return NULL;
	}

	cam->clipend = camera.mClipPlaneFar;
	cam->clipsta = camera.mClipPlaneNear;
	cam->type = CAM_PERSP;
	cam->dof_ob = 0;

	Camera* const old_cam = static_cast<Camera*>(ob->data);

	ob->data = cam;
	if (--old_cam->id.us == 0) {
		BKE_libblock_free(&G.main->camera, old_cam);
	}

	return ob;
}


Object* SceneImporter::convert_mesh(const std::vector<const aiMesh*>& in_meshes, const char* name) const
{
	MeshImporter imp(*this,in_meshes,out_scene,name);
	imp.convert();
	imp.makebmesh();
	return imp.create_object(name);
}

}