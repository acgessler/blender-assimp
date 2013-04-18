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

/** \file blender/assimp/AnimEvaluator.h
 *  \ingroup assimp
 *
 *  Note: this is a slightly modified version of assimpview's 
 *  animation code (assimp_repos/tools/assimp_view/AnimEvaluator.h)
 */


/** Calculates a pose for a given time of an animation */

#ifndef INCLUDED_ANIM_EVALUATOR_H
#define INCLUDED_ANIM_EVALUATOR_H

#include <vector>
#include "bassimp_shared.h"

namespace bassimp
{

/** Calculates transformations for a given timestamp from a set of animation tracks. Not directly useful,
 * better use the AnimPlayer class.
 */
class AnimEvaluator
{
public:
	/** Constructor on a given animation. The animation is fixed throughout the lifetime of
	 * the object.
	 * @param pAnim The animation to calculate poses for. Ownership of the animation object stays
	 *   at the caller, the evaluator just keeps a reference to it as long as it persists.
	 */
	AnimEvaluator( const aiAnimation* pAnim);

	/** Evaluates the animation tracks for a given time stamp. The calculated pose can be retrieved as a
	 * array of transformation matrices afterwards by calling GetTransformations().
	 * @param pTime The time for which you want to evaluate the animation, in TICKS. Must be in-range.
	 */
	void Evaluate( double pTicks);

	/** Same as @Evaluate, but only evaluates a single node animation channel */
	void EvaluateSingle( double pTicks, unsigned int channel);

	/** Returns the transform matrices calculated at the last Evaluate() call. The array matches the mChannels array of
	 * the aiAnimation. */
	const std::vector<aiMatrix4x4>& GetTransformations() const { return mTransforms; }

protected:

	// original AnimEvaluator used boost::tuple<unsigned int, unsigned int, unsigned int>
	struct PositionTuple 
	{
		PositionTuple()
			: i0()
			, i1()
			, i2()
		{
			
		}

		unsigned int i0, i1, i2;
	};

	/** The animation we're working on */
	const aiAnimation* mAnim;

	/** At which frame the last evaluation happened for each channel. 
	 * Useful to quickly find the corresponding frame for slightly increased time stamps
	 */
	double mLastTime;
	std::vector<PositionTuple > mLastPositions;

	/** The array to store the transformations results of the evaluation */
	std::vector<aiMatrix4x4> mTransforms;
};

} // end bassimp

#endif // INCLUDED_ANIM_EVALUATOR_H
