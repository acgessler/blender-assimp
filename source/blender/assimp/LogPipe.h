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

/** \file LogPipe.h
 *  \ingroup assimp
 */

#ifndef INCLUDED_LOG_PIPE_H
#define INCLUDED_LOG_PIPE_H

struct ReportList;

namespace bassimp {

/** utility class to pipe assimp's logging into Blender. Uses RAII to make
 *  sure the pipe gets disabled again. This is needed because logging in 
 *  assimp is (unfortunately) global.
 *
 * note: only a single log pipe can be active at a time.  */
class LogPipe {


public:
	
	/** setup a assimp log pipe that reports to the given ReportList.
	  * reports may be NULL, in this case nothing happens. */
	LogPipe(ReportList* reports);
	~LogPipe();

public:

	void kill();
};

}

#endif
