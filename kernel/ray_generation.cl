constant int width = 512;
constant int height = 512;
constant float inf = 1e20f;
constant float EPSILON = 0.00003f;


typedef struct ray_info {
	float4 ori;
	float4 dir;
	float4 color;
	float4 hitPoint;
	int coord;
} ray_info;

typedef struct Ray {
	float3 origin;
	float3 dir;
	float3 inv_dir;
	float3 uv;
	float si0;
	float si1;
	float si2;
	float t;
	float hitTriID;
	float depth;
} Ray;

void createCamRay(Ray* ray, const int x_coord, const int y_coord, const float rngx, const float rngy) {

	float3 uni_view = normalize((float3)(0, 0, 1));
	float3 uni_eye = (float3)(0.0, 0.68, -2.48);
	float3 dx = normalize(cross(uni_view, (float3)(0.0, 1.0, 0.0)));
	float3 dy = normalize(cross(uni_view, dx));
	float3 center = uni_eye + uni_view * (float)(width) * 0.5f / tan(60 * 0.00872665274f);
	float x = (float)(x_coord * 2 - width) * 0.5f + rngx * 1;
	float y = (float)(y_coord * 2 - height) * 0.5f + rngy * 1;
	float3 dir = normalize(center + x * dx + y * dy - uni_eye);

	/* create camera ray*/
	ray->origin = uni_eye;
	ray->dir = dir;
}

kernel void generation_kernel(global ray_info* ray_buffer, global long* streamTable, global int* atomicBuffer, const float rngx, const float rngy) {
	unsigned int work_item_id = get_global_id(0);	/* the unique global id of the work item for the current pixel */
	unsigned int x_coord = work_item_id % width;	 /*x-coordinate of the pixel */
	unsigned int y_coord = work_item_id / width;	 /*y-coordinate of the pixel */

	atomic_store_explicit((global atomic_int*)atomicBuffer, width * height, memory_order_seq_cst);
	streamTable[work_item_id] = work_item_id;

	Ray camray;
	createCamRay(&camray, x_coord, y_coord, rngx, rngy);

	ray_buffer[work_item_id].dir = (float4)(camray.dir, 1);
	ray_buffer[work_item_id].ori = (float4)(camray.origin, -1);
	ray_buffer[work_item_id].color = (float4)(1, 1, 1, 0);
	ray_buffer[work_item_id].hitPoint = (float4)(0, 0, 0, 0);

}