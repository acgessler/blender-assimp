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

/** \file SceneImporter.h
 *  \ingroup assimp
 */

#ifndef INCLUDED_SCENE_IMPORTER_H
#define INCLUDED_SCENE_IMPORTER_H

#include "bassimp_shared.h"
#include "bassimp.h"
#include "LogPipe.h"

#include <set>
#include <map>

namespace bassimp {

	class MaterialImporter;

class SceneImporter
{
private:

	const char* path;
	bContext& C;
	Scene* out_scene;

	Assimp::Importer importer;
	const aiScene* scene;

	std::vector<MaterialImporter*> materials;
	mutable std::vector<bool> materials_used;

	Object* armature;
	bool has_bones;

	// subgraph of the node graph containing all nodes that
	// carry meshes, directly or indirectly.
	typedef std::set<const aiNode*> NodeSet;
	NodeSet nodes_with_mesh_descendants;

	// nodes indexed by their unique name
	typedef std::map<std::string, const aiNode*> NodeNameMap;
	NodeNameMap nodes_by_name;

	// reverse mapping from assimp nodes to Blender objects
	typedef std::map<const aiNode*, Object*> NodeToObjectMap;
	NodeToObjectMap objects_by_node;

	typedef std::vector<const aiMesh*> MeshVector;
	typedef std::map<Object*, MeshVector> ObjectToMeshMap;
	ObjectToMeshMap meshes_by_object;

	const bassimp_import_settings settings;
	LogPipe log_pipe;

	bool root_collapsed;

private:

	void configure_importer();
	unsigned int get_assimp_flags() const;

	void convert_materials();
	void convert_animations();
	void convert_armature();
	void convert_skin();

	void convert_node(const aiNode& in_node, Object* out_parent, bool is_root = true);
	void convert_node_transform(const aiNode& node_in, Object& out_obj) const;
	void set_identity_transform(Object& out_obj) const;

	Object* convert_light(const aiLight& light) const;
	Object* convert_camera(const aiCamera& cam) const;
	Object* convert_mesh(const std::vector<const aiMesh*>& in_meshes, const char* name) const;

	void handle_scale();
	void handle_coordinate_space();
	void populate_mesh_node_graph(const aiNode* node);
	void populate_node_name_map(const aiNode* node);
	void collapse_root_node();

public:

	SceneImporter(const char* path, bContext& C, const bassimp_import_settings& settings);
	~SceneImporter();

	Assimp::Importer& get_importer();
	
	bool import();
	bool apply();

	// poor man's logging
	void error(const char* what) const;
	void verbose(const char* what) const;

	const bassimp_import_settings& get_settings() const;
	const char* get_file_path() const;

	const MaterialImporter& get_material(unsigned int idx) const;
	unsigned int resolve_matid(unsigned int src) const;

	// check if a node has any descendant nodes which carry meshes.
	bool has_mesh_descendants(const aiNode& in_node) const;

	// get a node by its name
	const aiNode* get_node_by_name(const std::string& node) const;

	// get blender object for a particular node, only works after the
	// nodegraph has been converted.
	Object* get_bobject_for_node(const aiNode& nd) const;
	bContext& get_context() const;
	Main& get_main() const;
};

}

#endif
