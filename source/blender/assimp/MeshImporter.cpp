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
#include "MeshImporter.h"
#include "MaterialImporter.h"
#include "bassimp_internal.h"

extern "C" {
#	include "BKE_global.h"
#	include "BKE_main.h"
#	include "BKE_mesh.h"
#	include "BKE_displist.h"
#	include "BKE_library.h"
#	include "BKE_material.h"

#	include "MEM_guardedalloc.h"

#	include "BLI_listbase.h"
#	include "BLI_math.h"
#	include "BLI_string.h"
}

namespace bassimp {

MeshImporter::MeshImporter(const SceneImporter& scene_imp, const std::vector<const aiMesh*>& in_meshes, Scene *out_scene, const char* name)
: scene_imp(scene_imp)
, in_meshes(in_meshes)
, out_scene(out_scene)
, mesh(BKE_mesh_add(CTX_data_main(&scene_imp.get_context()), name))
, name(name)
{
	assert(in_meshes.size());
	assert(out_scene);
	assert(mesh);

	// XXX copypaste from collada
	mesh->id.us--; // is already 1 here, but will be set later in set_mesh

	if(scene_imp.get_settings().read_materials) {
		reverseMatIDs.reserve(16);
	}
}


MeshImporter::~MeshImporter()
{
	if (mesh) {
		BKE_libblock_free(&G.main->mesh,mesh);
	}
}



void MeshImporter::error(const char* message)
{
	scene_imp.error((message + (" (mesh: " + name + ")")).c_str());
}


void MeshImporter::verbose(const char* message)
{
	scene_imp.verbose((message + (" (mesh: " + name + ")")).c_str());
}


// convert current conversion result to bmesh
void MeshImporter::makebmesh()
{
	BKE_mesh_convert_mfaces_to_mpolys(mesh);
	BKE_mesh_tessface_clear(mesh);

	BKE_mesh_calc_normals_mapping(mesh->mvert, mesh->totvert, mesh->mloop, mesh->mpoly, mesh->totloop, mesh->totpoly, NULL, NULL, 0, NULL, NULL);
}


// wrap the current conversion result in an Object and return it
Object* MeshImporter::create_object(const char* name)
{
	assert(name);

	if (!mesh) {
		return NULL;
	}

	Object* const ob = util_add_object(&scene_imp.get_main(), out_scene, OB_MESH, name);
	Mesh* const old_mesh = static_cast<Mesh*>(ob->data);

	ob->data = mesh;
	id_us_plus((ID *)mesh);

	if (--old_mesh->id.us == 0) {
		BKE_libblock_free(&G.main->mesh, old_mesh);
	}

	// give up ownership of the mesh object
	mesh = NULL;

	if(scene_imp.get_settings().read_materials) {
		// assign materials
		for (size_t i = 0; i < reverseMatIDs.size(); ++i) {
			assign_material(ob,scene_imp.get_material(reverseMatIDs[i]).get_material(),ob->totcol + 1, BKE_MAT_ASSIGN_OBDATA);
		}
	}

	return ob;
}


// count extra tris added by polygon triangulation
unsigned int MeshImporter::count_new_tris()
{
	unsigned int extra = 0;

	std::vector<unsigned int> tri;
	for (std::vector<const aiMesh*>::const_iterator it = in_meshes.begin(), end = in_meshes.end(); it != end; ++it) {
		const aiMesh& m = **it;

		for (unsigned int i = 0; i < m.mNumFaces; ++i) {
		
			const aiFace& f = m.mFaces[i];
			if (f.mNumIndices >= 4) {
				tri.clear();

				// XXX: this seems utterly redundant, we do the same thing later on again
				extra += triangulate_poly(f.mIndices, f.mNumIndices, mesh->mvert, tri);
			}
		}
	}
						
	return extra;
}


// taken from collada/meshimporter.cpp
bool MeshImporter::flat_face(const unsigned int *nind, unsigned int count, const aiVector3D* normals)
{
	float a[3], b[3];

	a[0] = normals[nind[0]].x;
	a[1] = normals[nind[0]].y;
	a[2] = normals[nind[0]].z;
	normalize_v3(a);

	for (int i = 1; i < count; i++) {
		b[0] = normals[nind[i]].x;
		b[1] = normals[nind[i]].y;
		b[2] = normals[nind[i]].z;
		normalize_v3(b);

		float dp = dot_v3v3(a, b);

		if (dp < 0.99999f || dp > 1.00001f) {
			return false;
		}
	}

	return true;
}


// allocate mtfaces for the given + 'extra' faces
void MeshImporter::allocate_face_data(unsigned int extra)
{
	// count TRI and POLY faces over all meshes
	unsigned int faces = extra;
	for (std::vector<const aiMesh*>::const_iterator it = in_meshes.begin(), end = in_meshes.end(); it != end; ++it) {
			const aiMesh& m = **it;
			for (unsigned int i = 0; i < m.mNumFaces; ++i) {
				faces += m.mFaces[i].mNumIndices >= 3 ? 1 : 0;
			}
	}

	if (faces) {
		mesh->totface = faces;
		mesh->mface   = (MFace*)CustomData_add_layer(&mesh->fdata, CD_MFACE, CD_CALLOC, NULL, mesh->totface);
	}
}


// copy face indices to a MFace
void MeshImporter::set_face_indices(MFace *mface, const unsigned int *indices, bool quad) const
{
	mface->v1 = indices[0];
	mface->v2 = indices[1];
	mface->v3 = indices[2];
	mface->v4 = quad ? indices[3] : 0;
}


void MeshImporter::convert_vertices()
{
	// count first
	unsigned int count = 0;
	for (std::vector<const aiMesh*>::const_iterator it = in_meshes.begin(), end = in_meshes.end(); it != end; ++it) {
		const aiMesh& m = **it;
		count += m.mNumVertices;
	}
	
	mesh->totvert = count;
	mesh->mvert = static_cast<MVert*>(CustomData_add_layer(&mesh->vdata, CD_MVERT, CD_CALLOC, NULL, mesh->totvert));

	MVert* mvert = mesh->mvert;
	for (std::vector<const aiMesh*>::const_iterator it = in_meshes.begin(), end = in_meshes.end(); it != end; ++it) {
		const aiMesh& m = **it;

		for (unsigned int i = 0, e = m.mNumVertices; i < e; ++i, ++mvert) {
			const aiVector3D& v = m.mVertices[i];
			mvert->co[0] = v.x;
			mvert->co[1] = v.y;
			mvert->co[2] = v.z;
		}
	}
}


// polygon triangulation
int MeshImporter::triangulate_poly(const unsigned int *indices, int totvert, MVert *verts, std::vector<unsigned int>& tri)
{
	// note: this is 100% copypaste from bf_collada
	ListBase dispbase;
	DispList *dl;
	float *vert;
	int i = 0;

	dispbase.first = dispbase.last = NULL;

	dl = (DispList*)MEM_callocN(sizeof(DispList), "poly disp");
	dl->nr = totvert;
	dl->type = DL_POLY;
	dl->parts = 1;
	dl->verts = vert = (float*)MEM_callocN(totvert * 3 * sizeof(float), "poly verts");
	dl->index = (int*)MEM_callocN(sizeof(int) * 3 * totvert, "dl index");

	BLI_addtail(&dispbase, dl);

	for (i = 0; i < totvert; i++) {
		copy_v3_v3(vert, verts[indices[i]].co);
		vert += 3;
	}

	BKE_displist_fill(&dispbase, &dispbase, 0);

	int tottri = 0;
	dl= (DispList*)dispbase.first;

	if (dl->type == DL_INDEX3) {
		tottri = dl->parts;

		int *index = dl->index;
		for (i= 0; i < tottri; i++) {
			int t[3]= {*index, *(index + 1), *(index + 2)};

			std::sort(t, t + 3);

			tri.push_back(t[0]);
			tri.push_back(t[1]);
			tri.push_back(t[2]);

			index += 3;
		}
	}

	BKE_displist_free(&dispbase);
	return tottri;
}


void MeshImporter::convert_faces()
{
	allocate_face_data(count_new_tris());

	// see how many UV channels we have. This needs to be the same number across all meshes
	unsigned int uv_count = static_cast<unsigned int>(-1);
	unsigned int vc_count = static_cast<unsigned int>(-1);

	for (std::vector<const aiMesh*>::const_iterator it = in_meshes.begin(), end = in_meshes.end(); it != end; ++it) {
		const aiMesh& m = **it;
		if (uv_count == static_cast<unsigned int>(-1)) {
			uv_count = m.GetNumUVChannels();
		}
		else {
			if (m.GetNumUVChannels() != uv_count) {
				error("unexpected number of UV channels, ignoring");
				return;
			}
		}

		if (vc_count == static_cast<unsigned int>(-1)) {
			vc_count = m.GetNumColorChannels();
		}
		else {
			if (m.GetNumColorChannels() != vc_count) {
				error("unexpected number of vertex color channels, ignoring");
				return;
			}
		}
	}	


	for (unsigned int i = 0; i < uv_count; i++) {		
		const std::string& s = get_default_uv_channel_name(i);
		CustomData_add_layer_named(&mesh->fdata, CD_MTFACE, CD_CALLOC, NULL, mesh->totface, s.c_str());
	}

	for (unsigned int i = 0; i < vc_count; i++) {		
		const std::string& s = get_default_vc_channel_name(i);
		CustomData_add_layer_named(&mesh->fdata, CD_MCOL, CD_CALLOC, NULL, mesh->totface, s.c_str());
	}


	// activate the first uv and vertex color layers, respectively
	if (uv_count) {
		mesh->mtface = static_cast<MTFace*>(CustomData_get_layer_n(&mesh->fdata, CD_MTFACE, 0));	
	}

	if(vc_count) {
		mesh->mcol = static_cast<MCol*>(CustomData_get_layer_n(&mesh->fdata, CD_MCOL, 0));	
	}

	MFace *mface = mesh->mface;
	unsigned int face_index = 0;

	std::vector<unsigned int> tri;

	for (std::vector<const aiMesh*>::const_iterator it = in_meshes.begin(), end = in_meshes.end(); it != end; ++it) {
		const aiMesh& m = **it;

		const bool has_normals = m.mNormals != NULL;

		MFace* const start = mface;
		unsigned int mesh_face_index_start = face_index;

		for (unsigned int i = 0, e = m.mNumFaces; i < e; ++i) {
			const aiFace& f = m.mFaces[i];

			// skip over POINT and LINE primitives at this stage
			if (f.mNumIndices < 3) {
				continue;
			}

			if (f.mNumIndices == 3 || f.mNumIndices == 4) {
				const bool is_quad = f.mNumIndices == 4;
				set_face_indices(mface, f.mIndices, is_quad);
				
				for (unsigned int k = 0; k < uv_count; k++) {
					// get mtface by face index and uv set index
					MTFace *mtface = static_cast<MTFace*>(CustomData_get_layer_n(&mesh->fdata, CD_MTFACE, k));
					for (int j = 0; j < f.mNumIndices; ++j){
						const aiVector3D& v = m.mTextureCoords[k][f.mIndices[j]];
						mtface[face_index].uv[j][0] = v.x;
						mtface[face_index].uv[j][1] = v.y;
					}				
				}

				for (unsigned int k = 0; k < vc_count; k++) {
					MCol *mcol = static_cast<MCol*>(CustomData_get_layer_n(&mesh->fdata, CD_MCOL, k));
					for (int j = 0; j < f.mNumIndices; ++j){
						const aiColor4D& c = m.mColors[k][f.mIndices[j]];
						MCol& co = mcol[face_index*4+j];
						co.r = c.r;
						co.g = c.g;
						co.b = c.b;
						co.a = c.a;
					}				
				}

				test_index_face(mface, &mesh->fdata, face_index, f.mNumIndices);
				
				if (has_normals) {
					if (!flat_face(f.mIndices,f.mNumIndices,m.mNormals)) {
						mface->flag |= ME_SMOOTH;
					}
				} 

				mface++;
				face_index++;				
			}
			else {
				tri.clear();

				const unsigned int* const indices = f.mIndices;
				triangulate_poly(indices, f.mNumIndices, mesh->mvert, tri);

				for (unsigned int k = 0, e = tri.size() / 3; k < e; ++k) {
					int v = k * 3;
					const unsigned int tri_indices[3] = {
						indices[tri[v]],
						indices[tri[v + 1]],
						indices[tri[v + 2]]
					};

					set_face_indices(mface, tri_indices, false);

					for (unsigned int k = 0; k < uv_count; k++) {
						// get mtface by face index and uv set index
						MTFace *mtface = static_cast<MTFace*>(CustomData_get_layer_n(&mesh->fdata, CD_MTFACE, k));
						for (int j = 0; j < 3; ++j){
							const aiVector3D& v = m.mTextureCoords[k][tri_indices[j]];
							mtface[face_index].uv[j][0] = v.x;
							mtface[face_index].uv[j][1] = v.y;
						}				
					}

					for (unsigned int k = 0; k < vc_count; k++) {
						MCol *mcol = static_cast<MCol*>(CustomData_get_layer_n(&mesh->fdata, CD_MCOL, k));
						for (int j = 0; j < f.mNumIndices; ++j){
							const aiColor4D& c = m.mColors[k][tri_indices[j]];
							MCol& co = mcol[face_index*4+j];
							co.r = c.r;
							co.g = c.g;
							co.b = c.b;
							co.a = c.a;
						}				
					}

					test_index_face(mface, &mesh->fdata, face_index, 3);
			
					if (has_normals) {
						
						if (!flat_face(tri_indices, 3, m.mNormals)) {
							mface->flag |= ME_SMOOTH;
						}
					} 

					mface++;
					face_index++;
				}
			}
		}

		if(!scene_imp.get_settings().read_materials) {
			continue;
		}

		if (matIDs.find(m.mMaterialIndex) == matIDs.end()) {
			matIDs[m.mMaterialIndex] = reverseMatIDs.size();
			reverseMatIDs.push_back(m.mMaterialIndex);

			if (vc_count > 0)
			{
				scene_imp.get_material(m.mMaterialIndex).set_vertex_color_flag();
			}
		}

		const short mat = static_cast<short>( matIDs[m.mMaterialIndex] );

		// assign material indices to each MFace
		for (MFace* p = start; p != mface; ++p) {
			p->mat_nr = mat;
		}

		// assign textures to each MTFace
		for (unsigned int k = 0; k < uv_count; k++) {
			MTFace *mtface = static_cast<MTFace*>(CustomData_get_layer_n(&mesh->fdata, CD_MTFACE, k)) + mesh_face_index_start;

			for (const MFace* p = start; p != mface; ++p, ++mtface) {
				assign_textures_to_face(*p,*mtface,m.mMaterialIndex,k);
			}
		}
	}
}

void MeshImporter::assign_textures_to_face(const MFace& fac, MTFace& tfac, unsigned int assimp_mat_id, unsigned int uv_index)
{
	// bind texture images to faces
	const UVTextureInfo* uvtex = scene_imp.get_material(assimp_mat_id).get_uv_texture(uv_index);
	if(uvtex) {
		tfac.tpage = uvtex->texture->ima;
	}
}


unsigned int MeshImporter::count_lines()
{
	unsigned int count = 0;
	for (std::vector<const aiMesh*>::const_iterator it = in_meshes.begin(), end = in_meshes.end(); it != end; ++it) {
		const aiMesh& m = **it;

		for (unsigned int i = 0; i < m.mNumFaces; ++i) {

			const aiFace& f = m.mFaces[i];
			if (f.mNumIndices == 2)	{
				++count;
			}
		}
	}

	return count;
}

void MeshImporter::mesh_add_edges(unsigned int len)
{
	if (!len) {
		return;
	}

	CustomData edata;
	MEdge *medge;
	int i, totedge;

	totedge = mesh->totedge + len;

	/* update customdata  */
	CustomData_copy(&mesh->edata, &edata, CD_MASK_MESH, CD_DEFAULT, totedge);
	CustomData_copy_data(&mesh->edata, &edata, 0, 0, mesh->totedge);

	if (!CustomData_has_layer(&edata, CD_MEDGE)) {
		CustomData_add_layer(&edata, CD_MEDGE, CD_CALLOC, NULL, totedge);
	}

	CustomData_free(&mesh->edata, mesh->totedge);
	mesh->edata = edata;
	//mesh_update_customdata_pointers(mesh, FALSE); /* new edges don't change tessellation */

	/* set default flags */
	medge = &mesh->medge[mesh->totedge];
	for (i = 0; i < len; i++, medge++) {
		medge->flag = ME_EDGEDRAW | ME_EDGERENDER | SELECT;
	}
	mesh->totedge = totedge;
}


void MeshImporter::convert_lines()
{
	const unsigned int loose_edge_count = count_lines();
	if (loose_edge_count > 0) {

		const unsigned int face_edge_count  = mesh->totedge;
		const unsigned int total_edge_count = loose_edge_count + face_edge_count;

		mesh_add_edges(loose_edge_count);
		MEdge *med = mesh->medge + face_edge_count;

		for (std::vector<const aiMesh*>::const_iterator it = in_meshes.begin(), end = in_meshes.end(); it != end; ++it) {
			const aiMesh& m = **it;

			for (unsigned int i = 0; i < m.mNumFaces; ++i) {
				const aiFace& f = m.mFaces[i];
				if (f.mNumIndices == 2)	{
					med->bweight = 0;
					med->crease  = 0;
					med->flag    = 0;
					med->v1      = f.mIndices[0];
					med->v2      = f.mIndices[1];

					++med;
				}
			}
		}
	}
}


// initiate the conversion from aiMesh to BlenMesh.
void MeshImporter::convert()
{
	convert_vertices();
	convert_faces();

	BKE_mesh_make_edges(mesh, 0);

	// so far we ignored all lines, now add them to the final mesh
	convert_lines();
}

}
