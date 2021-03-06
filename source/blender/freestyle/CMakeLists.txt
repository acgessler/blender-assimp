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
# Contributor(s): Jacques Beaurain.
#
# ***** END GPL LICENSE BLOCK *****

file(GLOB_RECURSE SRC *.cpp *.h)

set(INC 
  ../blenkernel ../blenloader ../blenlib ../imbuf ../makesdna ../makesrna
  ../python ../python/intern ../render/extern/include ../render/intern/include
  ../../../extern/glew/include ../../../intern/guardedalloc ../freestyle
)

set(INC_SYS
  ${PYTHON_INCLUDE_DIRS}
  ${PNG_INC}
)

add_definitions(-DWITH_FREESTYLE)

if(WIN32)
  set(INC_SYS ${INC_SYS} ${PTHREADS_INC})
endif(WIN32)

blender_add_lib(bf_freestyle "${SRC}" "${INC}" "${INC_SYS}")
