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

/** \file collada.h
 *  \ingroup collada
 */

#ifndef INCLUDED_BASSIMP_H
#define INCLUDED_BASSIMP_H

struct bContext;
struct Scene;
struct ReportList;

#ifdef __cplusplus
extern "C" {
#endif

	/* list all import file extensions recognized by assimp.
	 * entries should follow the format '.aaa'
	 *  'out' array should be of size dim+1
	 */
	void bassimp_query_import_file_extensions(const char* out[], int dim);


	/* assimp import settings */
	typedef struct bassimp_import_settings
	{
		/* reportlist to write reports to or NULL for silent mode */
		ReportList* reports;

		/* enable (1) or disable (0) assimp's own logging.
		 *  This only works if a non-NULL ReportList is given */
		int enableAssimpLog;

		/* run assimp's triangulation postprocessor on imported meshes */
		int triangulate;

		/* drop line and point meshes - in many cases these are
		 *  artifacts and have no further use*/
		int nolines;

		/* scale input scene to fit into (-maximum_size,maximum_size) cube*/
		int unit_scaling;

		/* if unit_scaling is on (1), this specifies the size to which
		 * the scene is scaled to */
		float maximum_size;


		/* flags to specify which parts of the scene to import */
		int read_animations;
		int read_cameras;
		int read_lights;
		int read_armature;
		int read_materials;

	} bassimp_import_settings;


	/* obtain default settings for bassimp_import() */
	void bassimp_import_set_defaults(bassimp_import_settings* defaults_out);


	/* import a scene using assimp. settings are optional, bassimp_import_set_defaults
	 * will be used if NULL is specified.
	 * returns 1 on success, 0 on error
	 */
	int bassimp_import(bContext *C, const char *filepath, const bassimp_import_settings* settings);
	//int bassimp_export(Scene *sce, const char *filepath, int selected, int apply_modifiers);
#ifdef __cplusplus
}
#endif

#endif
