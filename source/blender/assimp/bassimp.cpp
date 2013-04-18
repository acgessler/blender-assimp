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

/** \file blender/assimp/bassimp.cpp
 *  \ingroup assimp
 */

#include <cassert>
#include "SceneImporter.h"

namespace {
#define MAX_ASSIMP_EXT_TOTAL_LENGTH 512
	char cached_extensions[MAX_ASSIMP_EXT_TOTAL_LENGTH] = {0};
}

extern "C"
{
#include "BKE_scene.h"
#include "BKE_context.h"

/* make dummy file */
#include "BLI_fileops.h"
#include "BLI_path_util.h"

	void bassimp_query_import_file_extensions(const char* out[], int dim)
	{
		assert(out);
		assert(dim > 0);

		// XXX: this is a very inconvenient solution (and its not re-entrant either).
		// the problem is that we should return static strings to Blender, but 
		// we can't take them directly from assimp.

		Assimp::Importer importer;

		aiString ext;
		importer.GetExtensionList(ext);

		char* buff_cursor = cached_extensions, *buff_end = cached_extensions + MAX_ASSIMP_EXT_TOTAL_LENGTH - 1;

		int cnt = 0;
		bool done = false;
		for(const char* sz = ext.data, *last = sz+1 /* skip the '*' */; !done; ++sz)
		{
			done = !(*sz);
			if (*sz == ';' || done) {
				const size_t len = static_cast<size_t>(sz-last);
				if (buff_cursor + len > buff_end) {
					return;
				}

				memcpy(buff_cursor,last,len);
				buff_cursor[len] = '\0';

				out[cnt++] = buff_cursor;

				buff_cursor += len+1;
				last = sz+2;

				if(cnt == dim) {
					return;
				}
			}
		}
	}


	void bassimp_import_set_defaults(bassimp_import_settings* defaults_out)
	{
		assert(defaults_out);

		defaults_out->enableAssimpLog = 0;
		defaults_out->reports = NULL;
		defaults_out->nolines = 0;
		defaults_out->triangulate = 0;

		defaults_out->read_animations = 1;
		defaults_out->read_armature = 1;
		defaults_out->read_cameras = 1;
		defaults_out->read_lights = 1;
		defaults_out->read_materials = 1;
	}


	int bassimp_import(bContext *C, const char *filepath, const bassimp_import_settings* settings)
	{
		assert(C);
		assert(filepath);

		bassimp_import_settings defaults;
		if (!settings) {
			bassimp_import_set_defaults(&defaults);
			settings = &defaults;
		}

		bassimp::SceneImporter imp(filepath,*C,*settings);
		return imp.import() != 0 && imp.apply() != 0;
	}

	/*
    // export via assimp not currently implemented
	int bassimp_export(Scene *sce, const char *filepath, int selected, int apply_modifiers)
	{
		
		return 0;
	} */
}
