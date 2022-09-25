constant int width = 512;
constant int height = 512;
constant float inf = 1e20f;
constant float EPSILON = 0.00003f;
constant float PI = 3.14159265359f;


typedef struct ray_info {
	float4 ori;
	float4 dir;
	float4 color;
	float4 hitPoint;
	int coord;
} ray_info;

typedef struct Triangle {
	float3 a;
	float3 b;
	float3 c;
	float3 n;
	float3 center;
	float3 ambient;
	float3 diffuse;
	float3 specular;
	float3 emission;
	float3 ninsa;
} Triangle;

typedef struct BVH {
	float3 points[2];
	int triList[2];
} BVH;

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

typedef struct Stack {
	float content[32][2];
	int pointer;
} Stack;


float intersect_BVHtriangle(Triangle triangles, Ray* ray) {
	float t = 0.0f;

	float3 v0 = (float3)(triangles.a.x, triangles.a.y, triangles.a.z);
	float3 v1 = (float3)(triangles.b.x, triangles.b.y, triangles.b.z);
	float3 v2 = (float3)(triangles.c.x, triangles.c.y, triangles.c.z);

	float3 e1 = v1 - v0;
	float3 e2 = v2 - v0;
	float3 x = ray->origin - v0;
	float3 p = cross(ray->dir, e2);
	float det = dot(p, e1);
	t = dot(cross(x, e1), e2) / det;
	float u = dot(cross(ray->dir, e2), x) / det;
	float v = dot(cross(x, e1), ray->dir) / det;

	if ((t > 0) && (u >= 0) && (u <= 1) && (v >= 0) && (u + v <= 1)) {
		return t;
	}

	return inf;
}

float intersect_AABB(global BVH* BVHtriangles, int index, const Ray* r) {
	int id  = index;
	float3 points[2] = { BVHtriangles[id].points[0] - r->origin, BVHtriangles[id].points[1] - r->origin };
	float3 p1_ = points[0] * r->inv_dir;
	float3 p2_ = points[1] * r->inv_dir;
	float3 p1  = min(p1_, p2_);
	float3 p2  = max(p1_, p2_);
	   
	float t_low  = max(max(0.0f, p1.x),max(p1.y, p1.z));
	float t_high = min(p2.x, min(p2.y, p2.z));
				
	return t_low <= t_high ? t_low : -1.0f;
}

bool intersect_BVHscene(global Triangle* triangles, global BVH* BVHtriangles, Ray* ray) {
	bool hit = false;
	ray->t = inf;

	if (intersect_AABB(BVHtriangles, 1, ray) >= 0.0f) {
		Stack stack;
		stack.pointer = -1;

		int index = 1;
		bool checking = false;

		while (!checking || stack.pointer >= 0) {
			if (checking) {
				index = index / 2;
				if (stack.content[stack.pointer][1] < ray->t) {
					index = index * 2 + stack.content[stack.pointer][0];
					stack.content[stack.pointer][0] = 1 - stack.content[stack.pointer][0];
					stack.content[stack.pointer][1] = inf;
					checking = false;
				}
				else {
					stack.pointer = stack.pointer - 1;
				}
			}
			else {
				if (BVHtriangles[index].triList[0] >= 0) {
					int triIndex[2] = { BVHtriangles[index].triList[0], BVHtriangles[index].triList[1] };
					int tricount = 2;
					float hitdistance = -1;

					if (triIndex[1] < 0) {
						tricount = 1;
					}

					for (int i = 0; i < tricount; i++) {
						int triListIndex =  triIndex[i];
						hitdistance = intersect_BVHtriangle(triangles[triListIndex], ray);
						
						if (hitdistance != inf && hitdistance < ray->t) {
							ray->t = hitdistance;
							ray->hitTriID = (float)(triIndex[i]);
							
							hit = true;
						}
					}

					checking = true;
				}
				else {
					float t1 = intersect_AABB(BVHtriangles, index * 2, ray);
					float t2 = intersect_AABB(BVHtriangles, index * 2 + 1, ray);

					if ((t1 < 0.0f && t2 < 0.0f) || (t1 > ray->t && t2 > ray->t)) {
						checking = true;
					}
					else if (t1 >= 0.0f && t2 < 0.0f) {
						if (ray->t > t1) {
							stack.pointer = stack.pointer + 1;
							stack.content[stack.pointer][0] = 1;
							stack.content[stack.pointer][1] = inf;
							index = index * 2;
						}
						else {
							checking = true;
						}
					}
					else if (t1 < 0.0f && t2 >= 0.0f) {
						if (ray->t > t2) {
							stack.pointer = stack.pointer + 1;
							stack.content[stack.pointer][0] = 0;
							stack.content[stack.pointer][1] = inf;
							index = index * 2 + 1;
						}
						else {
							checking = true;
						}
					}
					else if (t1 < t2) {
						stack.pointer = stack.pointer + 1;
						stack.content[stack.pointer][0] = 1;
						stack.content[stack.pointer][1] = t2;
						index = index * 2;
					}
					else {
						stack.pointer = stack.pointer + 1;
						stack.content[stack.pointer][0] = 0;
						stack.content[stack.pointer][1] = t1;
						index = index * 2 + 1;
					}
				}
			}
		}
	}

	return hit; 
}

void BVHtrace(global Triangle* triangles, global BVH* BVHtriangles, Ray* ray) {	
	bool hit = intersect_BVHscene(triangles, BVHtriangles, ray);
	
	if (hit) {
		ray->uv = ray->origin + ray->dir * ray->t;
	}
}

kernel void traversal_kernel(global Triangle* triangles, global BVH* BVHtriangles, global long* streamTable, int NDcounter, global ray_info* ray_buffer) {
	unsigned int work_item_id = get_global_id(0);
	unsigned int x_coord = work_item_id % width;
	unsigned int y_coord = work_item_id / width;

	if (work_item_id < NDcounter) {
		int ray_id = streamTable[work_item_id];

		Ray ray;
		ray.origin = (float3)(ray_buffer[ray_id].ori.xyz);
		ray.dir = (float3)(ray_buffer[ray_id].dir.xyz);
		ray.inv_dir = (float3)(1 / ray.dir.x, 1 / ray.dir.y, 1 / ray.dir.z);
		ray.si0 = ray.inv_dir.x < 0;
		ray.si1 = ray.inv_dir.y < 0;
		ray.si2 = ray.inv_dir.z < 0;
		ray.hitTriID = -1.0f;
		ray.depth = ray_buffer[ray_id].dir.w;
		
		BVHtrace(triangles, BVHtriangles, &ray);
		
		ray_buffer[ray_id].ori = (float4)(ray.origin, ray.hitTriID);
		ray_buffer[ray_id].dir = (float4)(ray.dir, ray.depth);
		ray_buffer[ray_id].hitPoint = (float4)(ray.uv, 0);
	}
}