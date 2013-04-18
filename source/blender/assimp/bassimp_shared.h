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

/** \file bassimp_shared.h
 *  \ingroup assimp
 */

#ifndef INCLUDED_BASSIMP_SHARED_H
#define INCLUDED_BASSIMP_SHARED_H

#include <vector>

#include "../../extern/assimp/include/assimp/Importer.hpp"
#include "../../extern/assimp/include/assimp/scene.h"
#include "../../extern/assimp/include/assimp/camera.h"
#include "../../extern/assimp/include/assimp/light.h"
#include "../../extern/assimp/include/assimp/mesh.h"


extern "C" {
#include "DNA_scene_types.h"
#include "DNA_object_types.h"
#include "DNA_group_types.h"
#include "DNA_meshdata_types.h"
#include "DNA_mesh_types.h"
#include "DNA_image_types.h"
#include "DNA_lamp_types.h"
#include "DNA_camera_types.h"
#include "DNA_material_types.h"
#include "DNA_texture_types.h"
#include "DNA_anim_types.h"
#include "DNA_action_types.h"
#include "DNA_curve_types.h"
#include "DNA_armature_types.h"
#include "DNA_modifier_types.h"
#include "DNA_userdef_types.h"

#include "BKE_DerivedMesh.h"
#include "BKE_fcurve.h"
#include "BKE_animsys.h"
#include "BLI_path_util.h"
#include "BLI_fileops.h"
#include "ED_keyframing.h"
}

#endif
