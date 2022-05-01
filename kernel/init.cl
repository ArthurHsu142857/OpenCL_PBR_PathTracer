constant int width = 512;
constant int height = 512;

__global int counter = 0;

typedef struct ray_info {
	float4 ori;
	float4 dir;
	float4 color;
	float4 hitPoint;
	int coord;
} ray_info;

kernel void initial_kernel(__global ray_info* ray_buffer, __global float4* gpu_output, __global long* streamTable, __global int* atomicBuffer) {
	unsigned int work_item_id = get_global_id(0);	/* the unique global id of the work item for the current pixel */
	
	atomic_inc(atomicBuffer);

	ray_buffer[work_item_id].dir = (float4)(0, 0, 0, 1);
	ray_buffer[work_item_id].ori = (float4)(0, 0, 0, -1);
	ray_buffer[work_item_id].color = (float4)(1, 1, 1, 1);
	ray_buffer[work_item_id].hitPoint = (float4)(0, 0, 0, 0);
	ray_buffer[work_item_id].coord = work_item_id;

	gpu_output[work_item_id] = (float4)(0.0, 0.0, 0.0, 0.0);
}