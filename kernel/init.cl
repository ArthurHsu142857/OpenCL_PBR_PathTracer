constant int width = 512;
constant int height = 512;

typedef struct ray_info {
	float4 ori;
	float4 dir;
	float4 color;
	float4 hitPoint;
	int coord;
} ray_info;

kernel void initial_kernel(global ray_info* rayBuffer, global float4* gpu_output, global long* streamTable, global int* atomicBuffer) {
	unsigned int work_item_id = get_global_id(0);

	atomic_inc(atomicBuffer);
	
	rayBuffer[work_item_id].dir = (float4)(0, 0, 0, 1);
	rayBuffer[work_item_id].ori = (float4)(0, 0, 0, -1);
	rayBuffer[work_item_id].color = (float4)(1, 1, 1, 1);
	rayBuffer[work_item_id].hitPoint = (float4)(0, 0, 0, 0);
	rayBuffer[work_item_id].coord = work_item_id;

	gpu_output[work_item_id] = (float4)(0.0, 0.0, 0.0, 0.0);
}