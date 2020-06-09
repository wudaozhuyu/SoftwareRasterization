#include "core/maths.h"

//∂•µ„ Ù–‘
typedef struct
{
	vec3_t position;
	vec2_t texcoord;
	vec3_t normal;
	vec4_t tangent;
} vertex_t;

//Õ¯∏Ò
typedef struct
{
	int face_num;
	vertex_t* vertices;
	vec3_t center;
} mesh_t;

mesh_t* load_mesh(const char *path);
void release_mesh(mesh_t *mesh);