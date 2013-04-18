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
#include "ImageImporter.h"
#include "bassimp_internal.h"

extern "C" {
#	include "BKE_global.h"
#	include "BKE_main.h"
#	include "BKE_mesh.h"
#	include "BKE_image.h"
#	include "BKE_library.h"
#	include "BKE_packedFile.h"
#	include "DNA_packedFile_types.h"
#	include "../imbuf/IMB_imbuf.h"
#	include "../imbuf/IMB_imbuf_types.h"

#	include "MEM_guardedalloc.h"

#	include "BLI_listbase.h"
#	include "BLI_math.h"
#	include "BLI_string.h"
}

namespace bassimp {

ImageImporter::ImageImporter(const SceneImporter& scene_imp,const aiString& path, const aiScene* in_scene)
: path(path)
, ima()
, in_scene(in_scene)
, scene_imp(scene_imp)
{
	assert(in_scene);
}


ImageImporter::~ImageImporter()
{
	if (ima) {
		BKE_libblock_free(&G.main->image,ima);
	}
}


void ImageImporter::error(const char* message)
{
	std::string m(message);
	scene_imp.error((m + " (" + std::string(path.C_Str())+ ")").c_str());
}


void ImageImporter::verbose(const char* message)
{
	std::string m(message);
	scene_imp.verbose((m + " (" + std::string(path.C_Str())+ ")").c_str());
}


Image* ImageImporter::extract_image()
{
	Image* m = ima;
	ima = NULL;
	return m;
}


bool ImageImporter::convert()
{
	if (!path.length) {
		scene_imp.error("failed to convert image, no path given");
		return false;
	}

	if (path.data[0] == '*') {
		return convert_embedded();
	}

	return convert_file();
}

#define FILE_MAX 512

bool ImageImporter::convert_file()
{
	const char *image_filename = path.C_Str();
	char dir[FILE_MAX];
	char full_path[FILE_MAX];

	BLI_split_dir_part(scene_imp.get_file_path(), dir, sizeof(dir));
	BLI_join_dirfile(full_path, sizeof(full_path), dir, image_filename);
	ima = BKE_image_load_exists(full_path);
	if (!ima) {
		error("failed to convert image, image load fails");
		return false;
	}

	return true;
}


bool ImageImporter::convert_embedded()
{
	const unsigned int index = static_cast<unsigned int>(atoi(path.data+1));
	if (index >= in_scene->mNumTextures)	{
		error("failed to convert image, embedded texture index is out of range");
	}

	const aiTexture* const tex = in_scene->mTextures[index];
	
	// raw image data
	if (tex->mHeight) {
		if (!tex->mWidth) {
			error("failed to convert image, embedded texture has empty width");
		}

		ImBuf* const buff = IMB_allocImBuf(tex->mWidth,tex->mWidth,1,IB_rect);
		unsigned char* rect = reinterpret_cast<unsigned char*>(buff->rect);

		aiTexel* t = tex->pcData;
		for (unsigned int y  = 0; y < tex->mHeight; ++y) {
			for (unsigned int x  = 0; x < tex->mWidth; ++x, rect += 4, ++t) {
				rect[0] = t->r;
				rect[1] = t->g;
				rect[2] = t->b;
				rect[3] = t->a;
			}
		}
	}
	// compressed (i.e. png) image data
	else {

		std::stringstream name;
		name << "assimp-embedded-" << index;
		ima = BKE_image_load_exists(name.str().c_str());

		// copy the texture data to a PackedFile and assign it to the image, this will make Blender load it therefrom
		PackedFile *pf = static_cast<PackedFile*>( MEM_callocN(sizeof(*pf), "PackedFile") );

		void* buff = MEM_callocN(tex->mWidth,"EmbeddedTex");
		memcpy(buff,tex->pcData,tex->mWidth);

		pf->data = buff;
		pf->size = tex->mWidth;
		ima->packedfile = pf;
		
		ima->source = IMA_SRC_FILE;
		ima->type = IMA_TYPE_IMAGE;

		ima->packedfile = pf;
	}

	return true;
}

}
