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
 * Contributor(s): Chingiz Dyussenov, Arystanbek Dyussenov.
 *
 * ***** END GPL LICENSE BLOCK *****
 */

/** \file blender/assimp/bassimp_internal.cpp
 *  \ingroup assimp
 */


#include "bassimp_internal.h"
#include "sstream" 

#include "BKE_object.h"
#include "BKE_scene.h"

#include "BKE_depsgraph.h"
#include "BKE_object.h"

#include "WM_api.h" // XXX hrm, see if we can do without this
#include "WM_types.h"


namespace bassimp {


std::string get_default_uv_channel_name(unsigned int index)
{
	std::stringstream ss; 
	ss << "UVChannel-" << index;
	return ss.str();
}


std::string get_default_vc_channel_name(unsigned int index)
{
	std::stringstream ss; 
	ss << "VertexColorChannel-" << index;
	return ss.str();
}


void copy_ai_matrix(float outf[4][4], const aiMatrix4x4& m)
{
	outf[0][0] = m.a1;
	outf[1][0] = m.a2;
	outf[2][0] = m.a3;
	outf[3][0] = m.a4;
	outf[0][1] = m.b1;
	outf[1][1] = m.b2;
	outf[2][1] = m.b3;
	outf[3][1] = m.b4;
	outf[0][2] = m.c1;
	outf[1][2] = m.c2;
	outf[2][2] = m.c3;
	outf[3][2] = m.c4;
	outf[0][3] = m.d1;
	outf[1][3] = m.d2;
	outf[2][3] = m.d3;
	outf[3][3] = m.d4;
}


// copied from /editors/object/object_relations.c
int util_test_parent_loop(const Object *par, const Object *ob)
{
	/* test if 'ob' is a parent somewhere in par's parents */

	if (par == NULL) return 0;
	if (ob == par) return 1;

	return util_test_parent_loop(par->parent, ob);
}


// note: these are currently copy&paste from collada:


// a shortened version of parent_set_exec()
// if is_parent_space is true then ob->obmat will be multiplied by par->obmat before parenting
int util_set_parent(Object *ob, Object *par, bContext *C, bool is_parent_space)
{
	Object workob;
	Main *bmain = CTX_data_main(C);
	Scene *sce = CTX_data_scene(C);

	if (!par || util_test_parent_loop(par, ob)) {
		return false;
	}

	ob->parent = par;
	ob->partype = PAROBJECT;

	ob->parsubstr[0] = 0;

	if (is_parent_space) {
		float mat[4][4];
		// calc par->obmat
		BKE_object_where_is_calc(sce, par);

		// move child obmat into world space
		mult_m4_m4m4(mat, par->obmat, ob->obmat);
		copy_m4_m4(ob->obmat, mat);
	}

	// apply child obmat (i.e. decompose it into rot/loc/size)
	BKE_object_apply_mat4(ob, ob->obmat, 0, 0);

	// compute parentinv
	BKE_object_workob_calc_parent(sce, ob, &workob);
	invert_m4_m4(ob->parentinv, workob.obmat);

	ob->recalc |= OB_RECALC_OB | OB_RECALC_DATA;
	par->recalc |= OB_RECALC_OB;

	//DAG_scene_sort(bmain, sce);
	//DAG_ids_flush_update(bmain, 0);
	WM_event_add_notifier(C, NC_OBJECT|ND_TRANSFORM, NULL);

	return true;
}


Object* util_add_object(Main* bmain, Scene *scene, int type, const char *name)
{
	Object *ob = BKE_object_add_only_object(bmain, type, name);

	ob->data= BKE_object_obdata_add_from_type(type);
	ob->lay= scene->lay;
	ob->recalc |= OB_RECALC_OB|OB_RECALC_DATA|OB_RECALC_TIME;

	BKE_scene_base_select(scene, BKE_scene_base_add(scene, ob));

	return ob;
}



}

