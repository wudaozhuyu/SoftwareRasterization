#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include "model.h"
#include "core/macro.h"
#include "core/darray.h"

mesh_t* build_mesh(vec3_t* positions, vec2_t* texcoords, vec3_t* normals, vec4_t* tangents,
	int* position_indices, int* texcoord_indices, int* normal_indices)
{
	printf("### model ###\n");
	printf("positions size:%d\n", darray_size(positions));
	printf("texcoords size:%d\n", darray_size(texcoords));
	printf("normals size:%d\n", darray_size(normals));
	printf("tangents size:%d\n", darray_size(tangents));
	printf("position_indices size:%d\n", darray_size(position_indices));
	printf("texcoord_indices size:%d\n", darray_size(texcoord_indices));
	printf("normal_indices size:%d\n\n", darray_size(normal_indices));

	mesh_t* mesh;
	vertex_t* vertices;
	vec3_t center;

	vec3_t bbox_min = vec3_new(+1e6, +1e6, +1e6);
	vec3_t bbox_max = vec3_new(-1e6, -1e6, -1e6);

	int num_indices = darray_size(position_indices);
	int num_faces = num_indices / 3;

	vertices = (vertex_t*)malloc(sizeof(vertex_t) * num_indices);

	for (int i = 0; i < num_indices; i++)
	{
		int position_idx = position_indices[i];
		int texcoord_idx = texcoord_indices[i];
		int normal_idx = normal_indices[i];

		assert(position_idx >= 0 && position_idx < darray_size(positions));
		assert(texcoord_idx >= 0 && texcoord_idx < darray_size(texcoords));
		assert(normal_idx >= 0 && normal_idx < darray_size(normals));

		vertices[i].position = positions[position_idx];
		vertices[i].texcoord = texcoords[texcoord_idx];
		vertices[i].normal = normals[normal_idx];

		if (tangents)
		{
			int tangent_idx = position_idx;
			vertices[i].tangent = tangents[tangent_idx];
		}else
		{ 
			vertices[i].tangent = vec4_new(1, 0, 0, 1);
		}

		bbox_min = vec3_min(bbox_min, vertices[i].position);
		bbox_max = vec3_max(bbox_max, vertices[i].position);
	}

	mesh = (mesh_t*)malloc(sizeof(mesh_t));
	mesh->face_num = num_faces;
	mesh->vertices = vertices;
	mesh->center = vec3_div(vec3_add(bbox_min, bbox_max), 2);

	return mesh;
}

mesh_t* load_mesh(const char *path)
{
	vec3_t* positions = NULL;
	vec2_t* texcoords = NULL;
	vec3_t* normals = NULL;
	vec4_t* tangents = NULL;

	int* position_indices = NULL;
	int* texcoord_indices = NULL;
	int* normal_indices = NULL;

	char line[LINE_SIZE];
	FILE* stream = NULL;
	errno_t error;

	error = fopen_s(&stream, path, "rb");
	assert(error == 0);

	while (1)
	{
		int items;
		if (fgets(line, LINE_SIZE, stream) == NULL)
			break;
		else if (strncmp(line, "v ", 2) == 0)
		{
			vec3_t position;
			items = sscanf_s(line, "v %f %f %f", &position.x, &position.y, &position.z);
			darray_push(positions, position);
		}
		else if (strncmp(line, "vt ", 2) == 0)
		{
			vec2_t texcoord;
			items = sscanf_s(line, "vt %f %f", &texcoord.x, &texcoord.y);
			darray_push(texcoords, texcoord);
		}
		else if (strncmp(line, "vn ", 2) == 0)
		{
			vec3_t normal;
			items = sscanf_s(line, "vn %f %f %f", &normal.x, &normal.y, &normal.z);
			darray_push(normals, normal);
		}
		else if (strncmp(line, "f ", 2) == 0)
		{
			int p_indices[3], uv_indices[3], n_indices[3];

			items = sscanf_s(line, "f %d/%d/%d %d/%d/%d %d/%d/%d",
				&p_indices[0], &uv_indices[0], &n_indices[0],
				&p_indices[1], &uv_indices[1], &n_indices[1],
				&p_indices[2], &uv_indices[2], &n_indices[2]);

			for (int i = 0; i < 3; i++)
			{
				darray_push(position_indices, p_indices[i] - 1);
				darray_push(texcoord_indices, uv_indices[i] - 1);
				darray_push(normal_indices, n_indices[i] - 1);
			}
		}
		else if (strncmp(line, "# ext.tangent ", 2) == 0)
		{
			vec4_t tangent;
			items = sscanf_s(line, "# ext.tangent %f %f %f", &tangent.x, &tangent.y, &tangent.z, &tangent.w);
			darray_push(tangents, tangent);
		}
	}

	fclose(stream);

	//构建数据
	return build_mesh(positions, texcoords, normals, tangents, position_indices, texcoord_indices, normal_indices);
}

void release_mesh(mesh_t *mesh)
{

}

