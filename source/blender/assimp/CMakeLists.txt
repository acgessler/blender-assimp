# ***** BEGIN GPL LICENSE BLOCK *****
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software Foundation,
# Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
#
# The Original Code is Copyright (C) 2006, Blender Foundation
# All rights reserved.
#
# The Original Code is: all of this file.
#
# Contributor(s): Alexander Gessler.
#
# ***** END GPL LICENSE BLOCK *****

remove_strict_flags()

set(INC
	.
	../blenkernel
	../blenlib
	../blenloader
	../editors/include
	../makesdna
	../makesrna
	../windowmanager
	../../../intern/guardedalloc
)


set(INC_SYS
)

set(SRC

	bassimp.cpp
	bassimp.h
	bassimp_internal.h
	bassimp_internal.cpp
	bassimp_shared.h

	SceneImporter.cpp
	SceneImporter.h

	MeshImporter.cpp
	MeshImporter.h

	MaterialImporter.cpp
	MaterialImporter.h

	ImageImporter.cpp
	ImageImporter.h

	SceneOrientation.cpp
	SceneOrientation.h

	AnimationImporter.cpp
	AnimationImporter.h

	ArmatureImporter.cpp
	ArmatureImporter.h

	AnimEvaluator.cpp
	AnimEvaluator.h

	SkinImporter.cpp
	SkinImporter.h
	
	LogPipe.cpp
	LogPipe.h
)

if(WITH_BUILDINFO)
	add_definitions(-DWITH_BUILDINFO)
endif()

blender_add_lib(bf_assimp "${SRC}" "${INC}" "${INC_SYS}")
