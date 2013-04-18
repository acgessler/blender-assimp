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

/** \file SceneOrientation.h
 *  \ingroup assimp
 */

#ifndef INCLUDED_SCENE_ORIENTATION_H
#define INCLUDED_SCENE_ORIENTATION_H

#include  "bassimp_shared.h"

namespace bassimp {

	// utility class to apply uniform scalings to the imported assimp scene.
	// All scaling is done in-place.
	class SceneScaler
	{
	private:

		aiScene* sc;
		bool deepScale;

	public:
		SceneScaler(aiScene* sc);
	
	public:

		// enabling deep scale means that the scaling operation will
		// be applied recursively to the entire scene and not just
		// embedded in the root node.
		void use_deep_scaling(bool doit);

		// apply a specific relative scaling to the scene. This function
		// can be invoked multiple times on a scene.
		void scale(float s);
	};


	// utility class to rotate imported assimp scenes. This is needed
	// to transform them to Blender's internal coordinate space.
	// All rotations are done in-place.
	class SceneRotator
	{
	private:

		aiScene* sc;
		bool deepRotate;

	public:

		SceneRotator(aiScene* sc);

	public:

		// enabling deep rotate means that the rotation operation will
		// be applied recursively to the entire scene and not just
		// embedded in the root node.
		void use_deep_rotation(bool doit);

		// rotate the scene by 90 degrees on the x-axis. This is 
		// hard-coded to this angle in order to avoid precision
		// issues.
		void rotate_90deg_xpositive();
	};
}

#endif

