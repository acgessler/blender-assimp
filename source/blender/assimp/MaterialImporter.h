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

#ifndef INCLUDED_MATERIALIMPORTER_H
#define INCLUDED_MATERIALIMPORTER_H

#include <string>
#include "bassimp_shared.h"

namespace bassimp {


struct UVTextureInfo
{
	std::string name;
	Tex* texture;
};


class MaterialImporter 
{
private:

	Material* mat;
	Scene* out_scene;
	const aiMaterial* in_mat;
	unsigned int in_mat_index;
	const aiScene* in_scene;

	unsigned int next_texture;

	const SceneImporter& scene_imp;
	std::string name;

	std::vector<UVTextureInfo> uv_textures;

private:

	void error(const char* message);
	void verbose(const char* message);

	void convert_textures();
	void convert_texture(aiTextureType type, unsigned int index, const aiString& path, aiTextureMapping mapping,
		unsigned int uv,
		aiTextureOp op,
		aiTextureMapMode mapMode[2],
		unsigned int flags,
		float blend);

	void convert_shading();

	Image* convert_image(const aiString& path) const;
	float luminance(float r, float g, float b) const;
	
public:

	MaterialImporter(const SceneImporter& scene_imp, const aiMaterial* in_mat, const aiScene* in_scene, Scene* out_scene, unsigned int in_mat_index);
	~MaterialImporter ();

	// run conversion
	void convert();

	const UVTextureInfo* get_uv_texture(unsigned int index) const;

	// get converted material, but possesion of the Material* stays here
	Material* get_material() const;

	// disown the converted material (i.e. prevent deletion in the destructor)
	Material* disown_material();

	// set MA_VERTCOLP flag - this is valid only after the conversion has been run
	void set_vertex_color_flag() const;
};

}

#endif
