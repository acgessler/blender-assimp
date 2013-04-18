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

/** \file blender/assimp/SceneImporter.cpp
 *  \ingroup assimp
 */

#include <cassert>
#include <math.h>

#include "SceneOrientation.h"


namespace bassimp {

	using namespace Assimp;


SceneScaler::SceneScaler(aiScene* sc)
	: sc(sc)
	, deepScale()
{

}


void SceneScaler::use_deep_scaling(bool doit)
{
	deepScale = doit;
}


void SceneScaler::scale(float s)
{
	assert(fabs(s) > 1e-7f && "zero scaling specified");
	assert(s > 0.0f && "negative scaling specified");
	assert(!deepScale && "deepScale not yet implemented");

	aiMatrix4x4& m = sc->mRootNode->mTransformation;
	m.a1 *= s;
	m.a2 *= s;
	m.a3 *= s;

	m.b1 *= s;
	m.b2 *= s;
	m.b3 *= s;

	m.c1 *= s;
	m.c2 *= s;
	m.c3 *= s;

	m.d1 *= s;
	m.d2 *= s;
	m.d3 *= s;
}


SceneRotator::SceneRotator(aiScene* sc)
: sc(sc)
, deepRotate()
{

}


void SceneRotator::use_deep_rotation(bool doit)
{
	deepRotate = doit;
}


void SceneRotator::rotate_90deg_xpositive()
{
	assert(!deepRotate && "deepRotate not yet implemented");

	// rotating root transform by 90 deg is only ok if there are no animations attached 
	// to the root node in this case we need a new root node :-)
	for (unsigned int i = 0; i < sc->mNumAnimations; ++i) {
		const aiAnimation& anim = *sc->mAnimations[i];

		for (unsigned int n = 0; n < anim.mNumChannels; ++n) {
			const aiNodeAnim& na = *anim.mChannels[n];

			if(na.mNodeName == sc->mRootNode->mName) {
				aiNode* newRoot = new aiNode("$AssimpDummyRoot$");

				newRoot->mChildren = new aiNode*[1];
				newRoot->mChildren[0] = sc->mRootNode;
				sc->mRootNode->mParent = newRoot;
				sc->mRootNode = newRoot;
				
				i = sc->mNumAnimations;
				break;
			}
		}
	}

	aiMatrix4x4 rot;
	aiMatrix4x4::RotationX(AI_MATH_HALF_PI,rot);
	sc->mRootNode->mTransformation = sc->mRootNode->mTransformation * rot;

	// now the same transformation needs to be applied to all bone offset matrices
	// for this we need the inverse which happens to be the transpose.
	rot.Transpose();
	for (unsigned int i = 0; i < sc->mNumMeshes; ++i) {
		aiMesh& mesh = *sc->mMeshes[i];

		for (unsigned int n = 0; n < mesh.mNumBones; ++n) {
			aiBone& b = *mesh.mBones[n];
			b.mOffsetMatrix = rot * b.mOffsetMatrix;
		}
	}
}


}
