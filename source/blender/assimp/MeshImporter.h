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

/** \file MeshImporter.h
 *  \ingroup assimp
 */

#ifndef INCLUDED_MESHIMPORTER_H
#define INCLUDED_MESHIMPORTER_H

#include <map>
#include "bassimp_shared.h"

namespace bassimp {

	class SceneImporter;
	class MaterialImporter;
	struct UVTextureInfo;

class MeshImporter 
{
private:

	const SceneImporter& scene_imp;

	Mesh* mesh;
	Scene* out_scene;
	std::vector<const aiMesh*> in_meshes;

	
	std::string name;

	// assimp-to-mesh local IDs
	std::map<unsigned int, unsigned int> matIDs;

	// mesh-local-to-assimp IDs
	std::vector<unsigned int> reverseMatIDs;

private:

	void error(const char* verbose);
	void verbose(const char* message);

	void convert_vertices();
	void convert_faces();

	void convert_lines();
	void mesh_add_edges(unsigned int len);

	// count line primitives contained in the mesh(es)
	unsigned int count_lines();

	// count extra tris added by polygon triangulation
	unsigned int count_new_tris();

	// allocate mtfaces for the given + 'extra' faces
	void allocate_face_data(unsigned int extra);

	// copy face indices to a MFace
	void set_face_indices(MFace *mface, const unsigned int *indices, bool quad) const;

	// polygon triangulation
	int triangulate_poly(const unsigned int* indices, int totvert, MVert* verts, std::vector<unsigned int>& tri);

	void assign_textures_to_face(const MFace& fac, MTFace& tfac,unsigned int assimp_mat_id, unsigned int uv_index);

	bool flat_face(const unsigned int *nind, unsigned int count, const aiVector3D* normals);
	
public:

	MeshImporter(const SceneImporter& scene_imp, const std::vector<const aiMesh*>& in_meshes, Scene* out_scene, const char* name);
	~MeshImporter ();

	// initiate the conversion from aiMesh to BlenMesh.
	// this is needed for makebmesh and create_object to work properly
	void convert();

	// convert current conversion result to bmesh
	void makebmesh();

	// wrap the current conversion result in an Object and return it
	// (this transfers ownership of the Mesh object to the caller)
	Object* create_object(const char* name);

};

}

#endif
