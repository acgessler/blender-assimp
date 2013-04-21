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
#include "MaterialImporter.h"
#include "ImageImporter.h"
#include "bassimp_internal.h"

extern "C" {
#	include "BKE_global.h"
#	include "BKE_main.h"
#	include "BKE_mesh.h"
#	include "BKE_displist.h"
#	include "BKE_library.h"
#	include "BKE_material.h"
#	include "BKE_texture.h"

#	include "MEM_guardedalloc.h"

#	include "BLI_listbase.h"
#	include "BLI_math.h"
#	include "BLI_string.h"
}

namespace bassimp {

MaterialImporter::MaterialImporter(const SceneImporter& scene_imp,  const aiMaterial* in_mat, const aiScene* in_scene, Scene *out_scene, unsigned int in_mat_index)
: in_mat(in_mat)
, out_scene(out_scene)
, mat()
, in_mat_index(in_mat_index)
, in_scene(in_scene)
, next_texture()
, scene_imp(scene_imp)
{
	assert(in_mat);
	assert(out_scene);
	assert(in_scene);
	assert(in_scene);
}


MaterialImporter::~MaterialImporter()
{
	if (mat) {
		BKE_libblock_free(&G.main->mat,mat);
	}
}


void MaterialImporter::error(const char* message)
{
	scene_imp.error((message + (" (mat: " + name + ")")).c_str());
}


void MaterialImporter::verbose(const char* message)
{
	scene_imp.verbose((message + (" (mat: " + name + ")")).c_str());
}


const UVTextureInfo* MaterialImporter::get_uv_texture(unsigned int index) const
{
	if (index >= uv_textures.size()) {
		return NULL;
	}
	return &uv_textures[index];
}


Material* MaterialImporter::get_material() const
{
	return mat;
}


Material* MaterialImporter::disown_material()
{
	Material* m = mat;
	mat = NULL;
	return m;
}


void MaterialImporter::convert()
{
	aiString s;
	if(aiGetMaterialString(in_mat,AI_MATKEY_NAME,&s) == AI_SUCCESS) {
		mat = BKE_material_add(&scene_imp.get_main(), s.C_Str());
	}
	else {
		std::stringstream ss;
		ss << "assimpmat-" << in_mat_index;
		mat = BKE_material_add(&scene_imp.get_main(), ss.str().c_str());
	}

	aiColor4D c;
	if (aiGetMaterialColor(in_mat,AI_MATKEY_COLOR_DIFFUSE,&c) == AI_SUCCESS) {
		mat->r = c.r;
		mat->g = c.g;
		mat->b = c.b;
	}

	if (aiGetMaterialColor(in_mat,AI_MATKEY_COLOR_SPECULAR,&c) == AI_SUCCESS) {
		mat->specr = c.r;
		mat->specg = c.g;
		mat->specb = c.b;
	}

	if (aiGetMaterialColor(in_mat,AI_MATKEY_COLOR_EMISSIVE,&c) == AI_SUCCESS) {
		// ok,  some information will be lost, just take over the luminance
		// of the emitted color value to keep the amount of energy emitted
		// by the surface roughly the same.
		mat->emit = luminance(c.r,c.g,c.b);
	}

	if (aiGetMaterialColor(in_mat,AI_MATKEY_COLOR_REFLECTIVE,&c) == AI_SUCCESS) {
		mat->mirr = c.r;
		mat->mirg = c.g;
		mat->mirb = c.b;
	}

	convert_textures();
	convert_shading();
}


void MaterialImporter::set_vertex_color_flag() const
{
	assert(mat != NULL);
	mat->mode |= MA_VERTEXCOLP;
}


float MaterialImporter::luminance(float r, float g, float b) const
{
	return 0.2126f *r + 0.7152f *g + 0.0722f * b;
}


void MaterialImporter::convert_textures()
{
	aiString path;
	aiTextureMapping mapping = aiTextureMapping_UV;
	unsigned int uv = 0;
	aiTextureOp op = aiTextureOp_Add;
	aiTextureMapMode mapMode[2] = {aiTextureMapMode_Wrap,aiTextureMapMode_Wrap};
	unsigned int flags = 0;
	float blend = 1.0f;

	const unsigned int lasttex = static_cast<unsigned int>(aiTextureType_UNKNOWN);
	for (unsigned int t = 0; t <= lasttex; ++t) {
		for (unsigned int i = 0; ; ++i) {

			const aiTextureType type = static_cast<aiTextureType>(t);
			if(aiGetMaterialTexture(in_mat,type,i,&path,&mapping,&uv,&blend,&op,mapMode,&flags) == AI_SUCCESS) {
				convert_texture(type,i,path,mapping,uv,op,mapMode,flags,blend);
			}
			else {
				break;
			}
		}
	}
}


void MaterialImporter::convert_texture(aiTextureType type, unsigned int index, const aiString& path, aiTextureMapping mapping,
	unsigned int uv,
	aiTextureOp op,
	aiTextureMapMode mapMode[2],
	unsigned int flags,
	float blend)
{
	MTex* const mtex = mat->mtex[next_texture++] = add_mtex();
	mtex->texco = TEXCO_UV;
	mtex->tex = add_texture(&scene_imp.get_main(), "Texture");
	mtex->tex->type = TEX_IMAGE;
	mtex->tex->imaflag &= ~TEX_USEALPHA;
	mtex->tex->ima = convert_image(path);

	std::string s = get_default_uv_channel_name(index);
	if (s.length() >= sizeof(mtex->uvname)) {
		error("UV channel name to long, ignoring");
	}
	strcpy(mtex->uvname,s.c_str());

	// XXX convert further material properties
}


Image* MaterialImporter::convert_image(const aiString& path) const
{
	ImageImporter impo(scene_imp,path,in_scene);
	impo.convert();
	return impo.extract_image();
}


void MaterialImporter::convert_shading()
{
	aiShadingMode mode;

	int m = 0;
	if(aiGetMaterialInteger(in_mat,AI_MATKEY_SHADING_MODEL,&m) == AI_SUCCESS) {
		mode = static_cast<aiShadingMode>(m);
	}
	else {
		mode = aiShadingMode_Phong;
	}

	float gloss = 0.0f;
	if (aiGetMaterialFloat(in_mat, AI_MATKEY_SHININESS, &gloss) == AI_SUCCESS) {
		mat->har = static_cast<short>(gloss);
	}
	else {
		// assimp materials puts emphasis on specular shading? but doesn't specify a gloss value? set reasonable defaults
		if (mode == aiShadingMode_Phong || mode == aiShadingMode_CookTorrance || mode == aiShadingMode_Blinn) {
			mat->har = 16;
		}
	}

	float spec = 0.0f;
	if (aiGetMaterialFloat(in_mat, AI_MATKEY_SHININESS_STRENGTH, &gloss) == AI_SUCCESS) {
		mat->spec = spec;
	}
	else {
		// assimp materials puts emphasis on specular shading but doesn't specify a specularity value? set reasonable defaults
		if (mode == aiShadingMode_Phong || mode == aiShadingMode_CookTorrance || mode == aiShadingMode_Blinn) {
			mat->spec = 0.5f;
		}
	}

	float opacity = 0.0f;
	if (aiGetMaterialFloat(in_mat, AI_MATKEY_OPACITY, &opacity) == AI_SUCCESS) {
		mat->alpha = opacity;
	}

	// while assimp has only a single shading model flag, blender
	// distinguishes (rightly) between specular and diffuse shaders.
	// good thing about all this: I vaguely remember to have copied the
	// list of basic shaders recognized by assimp from blender four
	// years ago, so this feels a bit like coming home ...
	switch(mode)
	{
		case aiShadingMode_Flat:
			mat->diff_shader = MA_DIFF_LAMBERT;
			mat->spec_shader = MA_SPEC_PHONG;
			// XXX: this needs to ne accompanied by hard per-face normals
			break;

		case aiShadingMode_Gouraud:
			mat->diff_shader = MA_DIFF_LAMBERT;
			mat->spec_shader = MA_SPEC_PHONG;

			// kill the phong shader - want pure lambertian shading
			mat->spec = 0.0f;
			break;

		case aiShadingMode_Phong:
			mat->spec_shader = MA_SPEC_PHONG;
			mat->diff_shader = MA_DIFF_LAMBERT;
			break;

		case aiShadingMode_Blinn:
			mat->spec_shader = MA_SPEC_BLINN;
			mat->diff_shader = MA_DIFF_LAMBERT;
			break;

		case aiShadingMode_Toon:
			mat->spec_shader = MA_SPEC_TOON;
			mat->diff_shader = MA_DIFF_TOON;
			break;

		case aiShadingMode_OrenNayar:
			mat->spec_shader = MA_SPEC_PHONG;
			mat->diff_shader = MA_DIFF_ORENNAYAR;
			// XXX: extract oren nayar parameters
 			break;

		case aiShadingMode_Minnaert:
			mat->spec_shader = MA_SPEC_PHONG;
			mat->diff_shader = MA_DIFF_MINNAERT;
			// XXX: extract minnaert parameters
			break;

		case aiShadingMode_CookTorrance:
			mat->spec_shader = MA_SPEC_COOKTORR;
			mat->diff_shader = MA_DIFF_LAMBERT;
			break;

		case aiShadingMode_NoShading:
			mat->spec_shader = MA_SPEC_PHONG;
			mat->diff_shader = MA_DIFF_LAMBERT;
			// XXX
			break;

		case aiShadingMode_Fresnel:
			mat->spec_shader = MA_SPEC_PHONG;
			mat->diff_shader = MA_DIFF_FRESNEL;
			break;
	}
}
}
