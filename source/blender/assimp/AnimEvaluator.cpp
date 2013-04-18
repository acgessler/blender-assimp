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
 * Contributor(s): Thomas Schulze, Alexander Gessler
 *
 * ***** END GPL LICENSE BLOCK *****
 */

/** \file blender/assimp/AnimEvaluator.cpp
 *  \ingroup assimp
 *
 *  Note: this is a slightly modified version of assimpview's 
 *  animation code (assimp_repos/tools/assimp_view/AnimEvaluator.cpp)
 */


#include "AnimEvaluator.h"

namespace bassimp {

// ------------------------------------------------------------------------------------------------
// Constructor on a given animation. 
AnimEvaluator::AnimEvaluator( const aiAnimation* pAnim)
{
	mAnim = pAnim;
	mLastTime = 0.0;
	mLastPositions.resize( pAnim->mNumChannels,PositionTuple());
}

// ------------------------------------------------------------------------------------------------
// Evaluates the animation tracks for a given time stamp. 
void AnimEvaluator::EvaluateSingle(double time, unsigned int channel_index)
{
	// note: unlike the original AnimEvaluator from assimp, this one thinks in ticks, not seconds.
	if( mTransforms.size() != mAnim->mNumChannels) {
		mTransforms.resize( mAnim->mNumChannels);
	}

	const aiNodeAnim* const channel = mAnim->mChannels[channel_index];

	// ******** Position *****
	aiVector3D presentPosition( 0, 0, 0);
	if( channel->mNumPositionKeys > 0)
	{
		// Look for present frame number. Search from last position if time is after the last time, else from beginning
		// Should be much quicker than always looking from start for the average use case.
		unsigned int frame = (time >= mLastTime) ? mLastPositions[channel_index].i0 : 0;
		while( frame < channel->mNumPositionKeys - 1)	{
			if( time < channel->mPositionKeys[frame+1].mTime) {
				break;
			}
			frame++;
		}

		// interpolate between this frame's value and next frame's value
		unsigned int nextFrame = (frame + 1) % channel->mNumPositionKeys;
		const aiVectorKey& key = channel->mPositionKeys[frame];
		const aiVectorKey& nextKey = channel->mPositionKeys[nextFrame];
		double diffTime = nextKey.mTime - key.mTime;
		if( diffTime < 0.0) {
			diffTime += mAnim->mDuration;
		}

		if( diffTime > 0)	{
			const float factor = float( (time - key.mTime) / diffTime);
			presentPosition = key.mValue + (nextKey.mValue - key.mValue) * factor;
		} 
		else	{
			presentPosition = key.mValue;
		}

		mLastPositions[channel_index].i0 = frame;
	}

	// ******** Rotation *********
	aiQuaternion presentRotation( 1, 0, 0, 0);
	if( channel->mNumRotationKeys > 0)	{
		unsigned int frame = (time >= mLastTime) ? mLastPositions[channel_index].i1 : 0;
		while( frame < channel->mNumRotationKeys - 1)	{

			if( time < channel->mRotationKeys[frame+1].mTime) {
				break;
			}
			frame++;
		}

		// interpolate between this frame's value and next frame's value
		unsigned int nextFrame = (frame + 1) % channel->mNumRotationKeys;
		const aiQuatKey& key = channel->mRotationKeys[frame];
		const aiQuatKey& nextKey = channel->mRotationKeys[nextFrame];
		double diffTime = nextKey.mTime - key.mTime;
		if( diffTime < 0.0) {
			diffTime += mAnim->mDuration;
		}

		if( diffTime > 0)	{
			const float factor = float( (time - key.mTime) / diffTime);
			aiQuaternion::Interpolate( presentRotation, key.mValue, nextKey.mValue, factor);
		} 
		else {
			presentRotation = key.mValue;
		}

		mLastPositions[channel_index].i1 = frame;
	}

	// ******** Scaling **********
	aiVector3D presentScaling( 1, 1, 1);
	if( channel->mNumScalingKeys > 0) {
		unsigned int frame = (time >= mLastTime) ? mLastPositions[channel_index].i2 : 0;
		while( frame < channel->mNumScalingKeys - 1)	{

			if( time < channel->mScalingKeys[frame+1].mTime) {
				break;
			}
			frame++;
		}

		// TODO: (thom) interpolation maybe? This time maybe even logarithmic, not linear
		presentScaling = channel->mScalingKeys[frame].mValue;
		mLastPositions[channel_index].i2 = frame;
	}

	// build a transformation matrix from it
	aiMatrix4x4& mat = mTransforms[channel_index];
	mat = aiMatrix4x4( presentRotation.GetMatrix());
	mat.a1 *= presentScaling.x; mat.b1 *= presentScaling.x; mat.c1 *= presentScaling.x;
	mat.a2 *= presentScaling.y; mat.b2 *= presentScaling.y; mat.c2 *= presentScaling.y;
	mat.a3 *= presentScaling.z; mat.b3 *= presentScaling.z; mat.c3 *= presentScaling.z;
	mat.a4 = presentPosition.x; mat.b4 = presentPosition.y; mat.c4 = presentPosition.z;

	mLastTime = time;
}


// ------------------------------------------------------------------------------------------------
// Evaluates the animation tracks for a given time stamp. 
void AnimEvaluator::Evaluate( double pTime)
{
	// note: unlike the original AnimEvaluator from assimp, this one thinks in ticks, not seconds.

	// calculate the transformations for each animation channel
	for( unsigned int a = 0; a < mAnim->mNumChannels; a++)	{
		EvaluateSingle(pTime, a);		
	}
}

}

