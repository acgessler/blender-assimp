/*
 * Copyright 2011, Blender Foundation.
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
 */

#ifndef __KERNEL_TYPES_H__
#define __KERNEL_TYPES_H__

#include "kernel_math.h"
#include "svm/svm_types.h"

#ifndef __KERNEL_GPU__
#define __KERNEL_CPU__
#endif

CCL_NAMESPACE_BEGIN

/* constants */
#define OBJECT_SIZE 		11
#define OBJECT_VECTOR_SIZE	6
#define LIGHT_SIZE			4
#define FILTER_TABLE_SIZE	256
#define RAMP_TABLE_SIZE		256
#define PARTICLE_SIZE 		5
#define TIME_INVALID		FLT_MAX

#define BSSRDF_RADIUS_TABLE_SIZE	1024
#define BSSRDF_REFL_TABLE_SIZE		256
#define BSSRDF_PDF_TABLE_OFFSET		(BSSRDF_RADIUS_TABLE_SIZE*BSSRDF_REFL_TABLE_SIZE)
#define BSSRDF_LOOKUP_TABLE_SIZE	(BSSRDF_RADIUS_TABLE_SIZE*BSSRDF_REFL_TABLE_SIZE*2)
#define BSSRDF_MIN_RADIUS			1e-8f
#define BSSRDF_MAX_ATTEMPTS			8

#define TEX_NUM_FLOAT_IMAGES	5

/* device capabilities */
#ifdef __KERNEL_CPU__
#define __KERNEL_SHADING__
#define __KERNEL_ADV_SHADING__
#define __NON_PROGRESSIVE__
#define __HAIR__
#ifdef WITH_OSL
#define __OSL__
#endif
#define __SUBSURFACE__
#endif

#ifdef __KERNEL_CUDA__
#define __KERNEL_SHADING__
#if __CUDA_ARCH__ >= 200
#define __KERNEL_ADV_SHADING__
#endif
#endif

#ifdef __KERNEL_OPENCL__

#ifdef __KERNEL_OPENCL_NVIDIA__
#define __KERNEL_SHADING__
#define __MULTI_CLOSURE__
#endif

#ifdef __KERNEL_OPENCL_APPLE__
//#define __SVM__
//#define __EMISSION__
//#define __IMAGE_TEXTURES__
//#define __HOLDOUT__
//#define __PROCEDURAL_TEXTURES__
//#define __EXTRA_NODES__
#endif

#ifdef __KERNEL_OPENCL_AMD__
#define __SVM__
#define __EMISSION__
#define __IMAGE_TEXTURES__
#define __HOLDOUT__
#define __PROCEDURAL_TEXTURES__
#define __EXTRA_NODES__
#endif

#endif

/* kernel features */
#define __SOBOL__
#define __INSTANCING__
#define __DPDU__
#define __UV__
#define __BACKGROUND__
#define __CAUSTICS_TRICKS__
#define __VISIBILITY_FLAG__
#define __RAY_DIFFERENTIALS__
#define __CAMERA_CLIPPING__
#define __INTERSECTION_REFINE__
#define __CLAMP_SAMPLE__

#ifdef __KERNEL_SHADING__
#define __SVM__
#define __EMISSION__
#define __PROCEDURAL_TEXTURES__
#define __IMAGE_TEXTURES__
#define __EXTRA_NODES__
#define __HOLDOUT__
#define __NORMAL_MAP__
#endif

#ifdef __KERNEL_ADV_SHADING__
#define __MULTI_CLOSURE__
#define __TRANSPARENT_SHADOWS__
#define __PASSES__
#define __BACKGROUND_MIS__
#define __LAMP_MIS__
#define __AO__
#define __ANISOTROPIC__
#define __CAMERA_MOTION__
#define __OBJECT_MOTION__
#endif
//#define __SOBOL_FULL_SCREEN__

/* Shader Evaluation */

enum ShaderEvalType {
	SHADER_EVAL_DISPLACE,
	SHADER_EVAL_BACKGROUND
};

/* Path Tracing
 * note we need to keep the u/v pairs at even values */

enum PathTraceDimension {
	PRNG_FILTER_U = 0,
	PRNG_FILTER_V = 1,
	PRNG_LENS_U = 2,
	PRNG_LENS_V = 3,
#ifdef __CAMERA_MOTION__
	PRNG_TIME = 4,
	PRNG_UNUSED = 5,
	PRNG_BASE_NUM = 6,
#else
	PRNG_BASE_NUM = 4,
#endif

	PRNG_BSDF_U = 0,
	PRNG_BSDF_V = 1,
	PRNG_BSDF = 2,
	PRNG_LIGHT = 3,
	PRNG_LIGHT_U = 4,
	PRNG_LIGHT_V = 5,
	PRNG_LIGHT_F = 6,
	PRNG_TERMINATE = 7,
	PRNG_BOUNCE_NUM = 8
};

/* these flags values correspond to raytypes in osl.cpp, so keep them in sync!
 *
 * for ray visibility tests in BVH traversal, the upper 20 bits are used for
 * layer visibility tests. */

enum PathRayFlag {
	PATH_RAY_CAMERA = 1,
	PATH_RAY_REFLECT = 2,
	PATH_RAY_TRANSMIT = 4,
	PATH_RAY_DIFFUSE = 8,
	PATH_RAY_GLOSSY = 16,
	PATH_RAY_SINGULAR = 32,
	PATH_RAY_TRANSPARENT = 64,

	PATH_RAY_SHADOW_OPAQUE = 128,
	PATH_RAY_SHADOW_TRANSPARENT = 256,
	PATH_RAY_SHADOW = (PATH_RAY_SHADOW_OPAQUE|PATH_RAY_SHADOW_TRANSPARENT),

	PATH_RAY_MIS_SKIP = 512,

	PATH_RAY_ALL = (1|2|4|8|16|32|64|128|256|512),

	/* this gives collisions with localview bits
	 * see: CYCLES_LOCAL_LAYER_HACK(), grr - Campbell */
	PATH_RAY_LAYER_SHIFT = (32-20)
};

/* Closure Label */

typedef enum ClosureLabel {
	LABEL_NONE = 0,
	LABEL_CAMERA = 1,
	LABEL_LIGHT = 2,
	LABEL_BACKGROUND = 4,
	LABEL_TRANSMIT = 8,
	LABEL_REFLECT = 16,
	LABEL_VOLUME = 32,
	LABEL_OBJECT = 64,
	LABEL_DIFFUSE = 128,
	LABEL_GLOSSY = 256,
	LABEL_SINGULAR = 512,
	LABEL_TRANSPARENT = 1024,
	LABEL_STOP = 2048
} ClosureLabel;

/* Render Passes */

typedef enum PassType {
	PASS_NONE = 0,
	PASS_COMBINED = 1,
	PASS_DEPTH = 2,
	PASS_NORMAL = 4,
	PASS_UV = 8,
	PASS_OBJECT_ID = 16,
	PASS_MATERIAL_ID = 32,
	PASS_DIFFUSE_COLOR = 64,
	PASS_GLOSSY_COLOR = 128,
	PASS_TRANSMISSION_COLOR = 256,
	PASS_DIFFUSE_INDIRECT = 512,
	PASS_GLOSSY_INDIRECT = 1024,
	PASS_TRANSMISSION_INDIRECT = 2048,
	PASS_DIFFUSE_DIRECT = 4096,
	PASS_GLOSSY_DIRECT = 8192,
	PASS_TRANSMISSION_DIRECT = 16384,
	PASS_EMISSION = 32768,
	PASS_BACKGROUND = 65536,
	PASS_AO = 131072,
	PASS_SHADOW = 262144,
	PASS_MOTION = 524288,
	PASS_MOTION_WEIGHT = 1048576
} PassType;

#define PASS_ALL (~0)

#ifdef __PASSES__

typedef float3 PathThroughput;

typedef struct PathRadiance {
	int use_light_pass;

	float3 emission;
	float3 background;
	float3 ao;

	float3 indirect;
	float3 direct_throughput;
	float3 direct_emission;

	float3 color_diffuse;
	float3 color_glossy;
	float3 color_transmission;

	float3 direct_diffuse;
	float3 direct_glossy;
	float3 direct_transmission;

	float3 indirect_diffuse;
	float3 indirect_glossy;
	float3 indirect_transmission;

	float3 path_diffuse;
	float3 path_glossy;
	float3 path_transmission;

	float4 shadow;
} PathRadiance;

typedef struct BsdfEval {
	int use_light_pass;

	float3 diffuse;
	float3 glossy;
	float3 transmission;
	float3 transparent;
} BsdfEval;

#else

typedef float3 PathThroughput;
typedef float3 PathRadiance;
typedef float3 BsdfEval;

#endif

/* Shader Flag */

typedef enum ShaderFlag {
	SHADER_SMOOTH_NORMAL = (1 << 31),
	SHADER_CAST_SHADOW = (1 << 30),
	SHADER_AREA_LIGHT = (1 << 29),
	SHADER_USE_MIS = (1 << 28),

	SHADER_MASK = ~(SHADER_SMOOTH_NORMAL|SHADER_CAST_SHADOW|SHADER_AREA_LIGHT|SHADER_USE_MIS)
} ShaderFlag;

/* Light Type */

typedef enum LightType {
	LIGHT_POINT,
	LIGHT_DISTANT,
	LIGHT_BACKGROUND,
	LIGHT_AREA,
	LIGHT_AO,
	LIGHT_SPOT,
	LIGHT_TRIANGLE,
	LIGHT_STRAND
} LightType;

/* Camera Type */

enum CameraType {
	CAMERA_PERSPECTIVE,
	CAMERA_ORTHOGRAPHIC,
	CAMERA_PANORAMA
};

/* Panorama Type */

enum PanoramaType {
	PANORAMA_EQUIRECTANGULAR,
	PANORAMA_FISHEYE_EQUIDISTANT,
	PANORAMA_FISHEYE_EQUISOLID
};

/* Differential */

typedef struct differential3 {
	float3 dx;
	float3 dy;
} differential3;

typedef struct differential {
	float dx;
	float dy;
} differential;

/* Ray */

typedef struct Ray {
	float3 P;
	float3 D;
	float t;
	float time;

#ifdef __RAY_DIFFERENTIALS__
	differential3 dP;
	differential3 dD;
#endif
} Ray;

/* Intersection */

typedef struct Intersection {
	float t, u, v;
	int prim;
	int object;
	int segment;
} Intersection;

/* Attributes */

#define ATTR_PRIM_TYPES		2
#define ATTR_PRIM_CURVE		1

typedef enum AttributeElement {
	ATTR_ELEMENT_NONE,
	ATTR_ELEMENT_VALUE,
	ATTR_ELEMENT_FACE,
	ATTR_ELEMENT_VERTEX,
	ATTR_ELEMENT_CORNER,
	ATTR_ELEMENT_CURVE,
	ATTR_ELEMENT_CURVE_KEY
} AttributeElement;

typedef enum AttributeStandard {
	ATTR_STD_NONE = 0,
	ATTR_STD_VERTEX_NORMAL,
	ATTR_STD_FACE_NORMAL,
	ATTR_STD_UV,
	ATTR_STD_UV_TANGENT,
	ATTR_STD_UV_TANGENT_SIGN,
	ATTR_STD_GENERATED,
	ATTR_STD_POSITION_UNDEFORMED,
	ATTR_STD_POSITION_UNDISPLACED,
	ATTR_STD_MOTION_PRE,
	ATTR_STD_MOTION_POST,
	ATTR_STD_PARTICLE,
	ATTR_STD_CURVE_TANGENT,
	ATTR_STD_CURVE_INTERCEPT,
	ATTR_STD_NUM,

	ATTR_STD_NOT_FOUND = ~0
} AttributeStandard;

/* Closure data */

#define MAX_CLOSURE 16

typedef struct ShaderClosure {
	ClosureType type;
	float3 weight;

#ifdef __MULTI_CLOSURE__
	float sample_weight;
#endif

	float data0;
	float data1;

	float3 N;
#ifdef __ANISOTROPIC__
	float3 T;
#endif

#ifdef __OSL__
	void *prim;
#endif
} ShaderClosure;

/* Shader Context
 *
 * For OSL we recycle a fixed number of contexts for speed */

typedef enum ShaderContext {
	SHADER_CONTEXT_MAIN = 0,
	SHADER_CONTEXT_INDIRECT = 1,
	SHADER_CONTEXT_EMISSION = 2,
	SHADER_CONTEXT_SHADOW = 3,
	SHADER_CONTEXT_SSS = 4,
	SHADER_CONTEXT_NUM = 5
} ShaderContext;

/* Shader Data
 *
 * Main shader state at a point on the surface or in a volume. All coordinates
 * are in world space. */

enum ShaderDataFlag {
	/* runtime flags */
	SD_BACKFACING = 1,		/* backside of surface? */
	SD_EMISSION = 2,		/* have emissive closure? */
	SD_BSDF = 4,			/* have bsdf closure? */
	SD_BSDF_HAS_EVAL = 8,	/* have non-singular bsdf closure? */
	SD_BSDF_GLOSSY = 16,	/* have glossy bsdf */
	SD_BSSRDF = 32,			/* have bssrdf */
	SD_HOLDOUT = 64,		/* have holdout closure? */
	SD_VOLUME = 128,		/* have volume closure? */
	SD_AO = 256,			/* have ao closure? */

	SD_CLOSURE_FLAGS = (SD_EMISSION|SD_BSDF|SD_BSDF_HAS_EVAL|SD_BSDF_GLOSSY|SD_BSSRDF|SD_HOLDOUT|SD_VOLUME|SD_AO),

	/* shader flags */
	SD_SAMPLE_AS_LIGHT = 512,			/* direct light sample */
	SD_HAS_SURFACE_TRANSPARENT = 1024,	/* has surface transparency */
	SD_HAS_VOLUME = 2048,				/* has volume shader */
	SD_HOMOGENEOUS_VOLUME = 4096,		/* has homogeneous volume */

	/* object flags */
	SD_HOLDOUT_MASK = 8192,				/* holdout for camera rays */
	SD_OBJECT_MOTION = 16384,			/* has object motion blur */
	SD_TRANSFORM_APPLIED = 32768 		/* vertices have transform applied */
};

typedef struct ShaderData {
	/* position */
	float3 P;
	/* smooth normal for shading */
	float3 N;
	/* true geometric normal */
	float3 Ng;
	/* view/incoming direction */
	float3 I;
	/* shader id */
	int shader;
	/* booleans describing shader, see ShaderDataFlag */
	int flag;

	/* primitive id if there is one, ~0 otherwise */
	int prim;

#ifdef __HAIR__
	/* for curves, segment number in curve, ~0 for triangles */
	int segment;
#endif
	/* parametric coordinates
	 * - barycentric weights for triangles */
	float u, v;
	/* object id if there is one, ~0 otherwise */
	int object;

	/* motion blur sample time */
	float time;
	
	/* length of the ray being shaded */
	float ray_length;

#ifdef __RAY_DIFFERENTIALS__
	/* differential of P. these are orthogonal to Ng, not N */
	differential3 dP;
	/* differential of I */
	differential3 dI;
	/* differential of u, v */
	differential du;
	differential dv;
#endif
#ifdef __DPDU__
	/* differential of P w.r.t. parametric coordinates. note that dPdu is
	 * not readily suitable as a tangent for shading on triangles. */
	float3 dPdu, dPdv;
#endif

#ifdef __OBJECT_MOTION__
	/* object <-> world space transformations, cached to avoid
	 * re-interpolating them constantly for shading */
	Transform ob_tfm;
	Transform ob_itfm;
#endif

#ifdef __MULTI_CLOSURE__
	/* Closure data, we store a fixed array of closures */
	ShaderClosure closure[MAX_CLOSURE];
	int num_closure;
	float randb_closure;
#else
	/* Closure data, with a single sampled closure for low memory usage */
	ShaderClosure closure;
#endif
} ShaderData;

/* Constrant Kernel Data
 *
 * These structs are passed from CPU to various devices, and the struct layout
 * must match exactly. Structs are padded to ensure 16 byte alignment, and we
 * do not use float3 because its size may not be the same on all devices. */

typedef struct KernelCamera {
	/* type */
	int type;

	/* panorama */
	int panorama_type;
	float fisheye_fov;
	float fisheye_lens;

	/* matrices */
	Transform cameratoworld;
	Transform rastertocamera;

	/* differentials */
	float4 dx;
	float4 dy;

	/* depth of field */
	float aperturesize;
	float blades;
	float bladesrotation;
	float focaldistance;

	/* motion blur */
	float shuttertime;
	int have_motion;

	/* clipping */
	float nearclip;
	float cliplength;

	/* sensor size */
	float sensorwidth;
	float sensorheight;

	/* render size */
	float width, height;

	/* more matrices */
	Transform screentoworld;
	Transform rastertoworld;
	/* work around cuda sm 2.0 crash, this seems to
	 * cross some limit in combination with motion 
	 * Transform ndctoworld; */
	Transform worldtoscreen;
	Transform worldtoraster;
	Transform worldtondc;
	Transform worldtocamera;

	MotionTransform motion;
} KernelCamera;

typedef struct KernelFilm {
	float exposure;
	int pass_flag;
	int pass_stride;
	int use_light_pass;

	int pass_combined;
	int pass_depth;
	int pass_normal;
	int pass_motion;

	int pass_motion_weight;
	int pass_uv;
	int pass_object_id;
	int pass_material_id;

	int pass_diffuse_color;
	int pass_glossy_color;
	int pass_transmission_color;
	int pass_diffuse_indirect;

	int pass_glossy_indirect;
	int pass_transmission_indirect;
	int pass_diffuse_direct;
	int pass_glossy_direct;

	int pass_transmission_direct;
	int pass_emission;
	int pass_background;
	int pass_ao;

	int pass_shadow;
	float pass_shadow_scale;

	int filter_table_offset;
	int filter_pad;
} KernelFilm;

typedef struct KernelBackground {
	/* only shader index */
	int shader;
	int transparent;

	/* ambient occlusion */
	float ao_factor;
	float ao_distance;
} KernelBackground;

typedef struct KernelSunSky {
	/* sun direction in spherical and cartesian */
	float theta, phi, pad3, pad4;

	/* perez function parameters */
	float zenith_Y, zenith_x, zenith_y, pad2;
	float perez_Y[5], perez_x[5], perez_y[5];
	float pad5;
} KernelSunSky;

typedef struct KernelIntegrator {
	/* emission */
	int use_direct_light;
	int use_ambient_occlusion;
	int num_distribution;
	int num_all_lights;
	float pdf_triangles;
	float pdf_lights;
	float inv_pdf_lights;
	int pdf_background_res;

	/* bounces */
	int min_bounce;
	int max_bounce;

	int max_diffuse_bounce;
	int max_glossy_bounce;
	int max_transmission_bounce;

	/* transparent */
	int transparent_min_bounce;
	int transparent_max_bounce;
	int transparent_shadows;

	/* caustics */
	int no_caustics;
	float filter_glossy;

	/* seed */
	int seed;

	/* render layer */
	int layer_flag;

	/* clamp */
	float sample_clamp;

	/* non-progressive */
	int progressive;
	int diffuse_samples;
	int glossy_samples;
	int transmission_samples;
	int ao_samples;
	int mesh_light_samples;
	int use_lamp_mis;
	int subsurface_samples;

	int pad1, pad2, pad3;
} KernelIntegrator;

typedef struct KernelBVH {
	/* root node */
	int root;
	int attributes_map_stride;
	int have_motion;
	int pad2;
} KernelBVH;

typedef enum CurveFlag {
	/* runtime flags */
	CURVE_KN_BACKFACING = 1,				/* backside of cylinder? */
	CURVE_KN_ENCLOSEFILTER = 2,				/* don't consider strands surrounding start point? */
	CURVE_KN_CURVEDATA = 4,				/* curve data available? */
	CURVE_KN_INTERPOLATE = 8,				/* render as a curve? - not supported yet */
	CURVE_KN_ACCURATE = 16,				/* use accurate intersections test? */
	CURVE_KN_INTERSECTCORRECTION = 32,		/* correct for width after determing closest midpoint? */
	CURVE_KN_POSTINTERSECTCORRECTION = 64,	/* correct for width after intersect? */
	CURVE_KN_NORMALCORRECTION = 128,		/* correct tangent normal for slope? */
	CURVE_KN_TRUETANGENTGNORMAL = 256,		/* use tangent normal for geometry? */
	CURVE_KN_TANGENTGNORMAL = 512,			/* use tangent normal for shader? */
	CURVE_KN_RIBBONS = 1024,				/* use flat curve ribbons */
} CurveFlag;

typedef struct KernelCurves {
	/* strand intersect and normal parameters - many can be changed to flags*/
	float normalmix;
	float encasing_ratio;
	int curveflags;
	int subdivisions;
} KernelCurves;

typedef struct KernelBSSRDF {
	int table_offset;
	int num_attempts;
	int pad1, pad2;
} KernelBSSRDF;

typedef struct KernelData {
	KernelCamera cam;
	KernelFilm film;
	KernelBackground background;
	KernelSunSky sunsky;
	KernelIntegrator integrator;
	KernelBVH bvh;
	KernelCurves curve_kernel_data;
	KernelBSSRDF bssrdf;
} KernelData;

CCL_NAMESPACE_END

#endif /*  __KERNEL_TYPES_H__ */

