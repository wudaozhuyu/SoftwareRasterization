#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "core/graphics.h"
#include "core/platform.h"
#include "core/image.h"
#include "core/darray.h"
#include "core/camera.h"
#include "core/macro.h"
#include "core/maths.h"
#include "core/texture.h"
#include "art/model.h"

static const char* WINDOW_TITLE = "";
static const int WINDOW_WIDTH = 800;
static const int WINDOW_HEIGHT = 600;

static const vec3_t CAMERA_POSITION = { 0, 0, 1.5 };
static const vec3_t CAMERA_TARGET = { 0, 0, 0 };

static const float LIGHT_THETA = TO_RADIANS(45);
static const float LIGHT_PHI = TO_RADIANS(45);

const char* texture_path = "asset/ponycar/body_basecolor.tga";
const char* model_path = "asset/ponycar/body.obj";

typedef struct
{
	vec3_t position;
	vec2_t texcoord;
	vec3_t normal;
	vec4_t tangent;
} attribute_t;

typedef struct
{
	vec2_t texcoord;
	vec3_t world_position;
	vec3_t world_normal;
} varying_t;

typedef struct
{
	mat4_t model_matrix;
	mat3_t normal_matrix;
	vec3_t camera_pos;
	mat4_t camera_view_matrix;
	mat4_t camera_proj_matrix;
	vec3_t light_dir;
	mat4_t light_view_matrix;
	mat4_t light_proj_matrix;
	texture_t* base_texture;

} uniform_t;

vec4_t vertex_func(void* _attribs, void* _varyings, void* _uniforms)
{
	attribute_t* attribs = (attribute_t*)_attribs;
	varying_t* varyings = (varying_t*)_varyings;
	uniform_t* uniform = (uniform_t*)_uniforms;

	mat4_t model_matrix = uniform->model_matrix;
	mat4_t camera_vp_matrix = mat4_mul_mat4(uniform->camera_view_matrix, uniform->camera_proj_matrix);
	mat3_t normal_matrix = uniform->normal_matrix;

	vec4_t input_position = vec4_from_vec3(attribs->position, 1.0);
	vec4_t world_position = mat4_mul_vec4(model_matrix, input_position);
	vec4_t clip_position = mat4_mul_vec4(camera_vp_matrix, world_position);

	vec3_t input_normal = attribs->normal;
	vec3_t world_normal = mat3_mul_vec3(normal_matrix, input_normal);

	varyings->texcoord = attribs->texcoord;
	varyings->world_position = vec3_from_vec4(world_position);
	varyings->world_normal = vec3_normalize(world_normal);

	return clip_position;
}

vec4_t fragment_func(void* varyings_, void* uniforms_, int* discard_, int backface_)
{
	varying_t* varyings = (varying_t*)varyings_;
	uniform_t* uniform = (uniform_t*)uniforms_;
	vec2_t texcoord = varyings->texcoord;
	vec4_t color = texture_sample(uniform->base_texture, texcoord);
	return color;
}

uniform_t* setupUniform(program_t* program, camera_t* camera)
{
	float theta = LIGHT_THETA;
	float phi = LIGHT_PHI;
	float x = (float)sin(phi) * (float)sin(theta);
	float y = (float)cos(phi);
	float z = (float)sin(phi) * (float)cos(theta);
	vec3_t light_dir = vec3_new(-x, -y, -z);

	vec3_t light_pos = vec3_negate(light_dir);
	vec3_t light_target = vec3_new(0, 0, 0);
	vec3_t light_up = vec3_new(0, 1, 0);
	mat4_t light_view_matrix = mat4_lookat(light_pos, light_target, light_up);
	mat4_t light_proj_matrix = mat4_orthographic(1, 1, 0, 2);

	mat4_t model_matrix = mat4_identity();

	uniform_t* uniform = (uniform_t*)program_get_uniforms(program);
	uniform->camera_pos = camera_get_position(camera);
	uniform->camera_view_matrix = camera_get_view_matrix(camera);
	uniform->camera_proj_matrix = camera_get_proj_matrix(camera);
	uniform->light_dir = vec3_normalize(light_dir);
	uniform->light_view_matrix = light_view_matrix;
	uniform->light_proj_matrix = light_proj_matrix;
	uniform->model_matrix = model_matrix;
	uniform->normal_matrix = mat3_inverse_transpose(mat3_from_mat4(model_matrix));
	uniform->base_texture = texture_from_file(texture_path);
}

int main(int argc, char* argv[])
{
	camera_t* camera = camera_create(CAMERA_POSITION, CAMERA_TARGET, (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT);

	framebuffer_t* frameBuffer = (framebuffer_t*)malloc(sizeof(frameBuffer));
	vec4_t default_color = {0, 0, 0, 0};
	float default_depth = 1.0;
	int pix_num = WINDOW_WIDTH * WINDOW_HEIGHT;
	frameBuffer->width = WINDOW_WIDTH;
	frameBuffer->height = WINDOW_HEIGHT;
	frameBuffer->colorbuffer = (vec4_t*)malloc(sizeof(vec4_t) * pix_num);
	frameBuffer->depthbuffer = (float*)malloc(sizeof(float) * pix_num);

	mesh_t* mesh = load_mesh(model_path);

	vertex_shader_t* vertex_shader = vertex_func;
	fragment_shader_t* fragment_shader = fragment_func;

	int sizeof_attribs = sizeof(attribute_t);
	int sizeof_varyings = sizeof(varying_t);
	int sizeof_uniforms = sizeof(uniform_t);

	program_t* program = program_create(vertex_shader, fragment_shader, sizeof_attribs, sizeof_varyings, sizeof_uniforms, 0, 0);

	setupUniform(program, camera);

	window_t* window = window_create(WINDOW_TITLE, WINDOW_WIDTH, WINDOW_HEIGHT);
	while (!window_should_close(window))
	{
		for (int i = 0; i < pix_num; i++)
		{
			frameBuffer->colorbuffer[i] = default_color;
			frameBuffer->depthbuffer[i] = default_depth;
		}

		{
			int face_num = mesh->face_num;
			for (int i = 0; i < face_num; i++)
			{
				for (int j = 0; j < 3; j++)
				{
					attribute_t * att = (attribute_t*)program_get_attribs(program, j);
					vertex_t vertex = mesh->vertices[i * 3 + j];
					att->position = vertex.position;
					att->texcoord = vertex.texcoord;
					att->normal = vertex.normal;
					att->tangent = vertex.tangent;
				}
				graphics_draw_triangle(frameBuffer, program);
			}
		}
		input_poll_events();
	}

	return 0;
}

