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

/** \file bassimp_internal.h
 *  \ingroup assimp
 */

#ifndef INCLUDED_BASSIMP_INTERNAL_H
#define INCLUDED_BASSIMP_INTERNAL_H

#include <string>
#include <vector>
#include <map>

extern "C" {

#include "DNA_armature_types.h"
#include "DNA_material_types.h"
#include "DNA_object_types.h"
#include "DNA_scene_types.h"
#include "BLI_math.h"
#include "DNA_object_types.h"
#include "DNA_customdata_types.h"
#include "DNA_texture_types.h"
#include "BKE_context.h"
#include "DNA_scene_types.h"

}

#include "../../extern/assimp/include/assimp/types.h"

namespace bassimp
{
	int util_set_parent(Object *ob, Object *par, bContext *C, bool is_parent_space);
	Object* util_add_object(Scene *scene, int type, const char *name);

	std::string get_default_uv_channel_name(unsigned int index);
	std::string get_default_vc_channel_name(unsigned int index);

	void copy_ai_matrix(float outf[4][4], const aiMatrix4x4& m);
}


#endif /* INCLUDED_BASSIMP_INTERNAL_H */
