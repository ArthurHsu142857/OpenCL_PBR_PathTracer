constant int width = 512;
constant int height = 512;
constant float PI = 3.14159265359f;
constant float ONE_OVER_PI = 1 / 3.14159265359f;
constant int BOUNCE = 10;
constant float EPSILON = 0.00003f;


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
	float3 _center;
	float3 ambient;
	float3 diffuse;
	float3 specular;
	float3 emission;
	float3 ninsa;
} Triangle;

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


uint hash1(uint x) {
	x  = x * 0xdefaced + 7091357;
	x += x >> 10;
	x += x << 10;
	return x;
}

uint hash2(uint x) {
	x  = x * 0xdefaced + 7091401;
	x += x >> 10;
	x += x << 10;
	return x;
}

uint SetRandomSeed(uint seed){ 
	return hash1(hash1(seed)); 
}

float intBitsToFloat(int bits) {
	int s = ((bits >> 31) == 0) ? 1 : -1;
	int e = ((bits >> 23) & 0xff);
	int m = (e == 0) ?
		(bits & 0x7fffff) << 1 :
		(bits & 0x7fffff) | 0x800000;
	return s * m * pow(2.0f, e - 150);
}

float Random(uint* randomSeed) {
    const uint mantissaMask = 0x007FFFFFu;
    const uint one          = 0x3F800000u;
    
    uint h = hash1(*randomSeed);
	*randomSeed = h;
    h &= mantissaMask;
    h |= one;
    
    float  r = intBitsToFloat((int)h);
    return r - 1.0f;
}

static float get_random(float3 seed1, float* seed2) {
	float dt = dot(seed1 * *seed2, (float3)(1.9898f, 7.233f, 5.164f)) * 94.5453f;
	*seed2 = dt - floor(dt);
	return *seed2;
}


float NsToRoughness(const float ns) {
	return 1.0f / sqrt((ns + 2.0f) * 0.5f);
}

/*
void sample_diffuse(Ray* ray, Triangle hitTriangle, bool is_refract, float rand1, float rand2, float rand2s) {
	float3 hitpoint = ray->origin + ray->t * ray->dir;
	float3 hitpoint = ray->uv;
	float3 normal = (float3)(hitTriangle.n.x, hitTriangle.n.y, hitTriangle.n.z);
	float3 normal_facing = dot(normal, ray->dir) < 0.0f ? normal : normal * (-1.0f);

	float3 w = is_refract ? -normal_facing : normal_facing;
	float3 axis = fabs(w.x) > 0.1f ? (float3)(0.0f, 1.0f, 0.0f) : (float3)(1.0f, 0.0f, 0.0f);
	float3 u = normalize(cross(axis, w));
	float3 v = cross(w, u);

	float cosPhi = cos(rand1);
	float sinPhi = sin(rand1);
	float cosTheta = rand2;
	float sinTheta = sqrt(1.0 - rand2 * rand2);
	float3 newdir = normalize(u * cos(rand1)*rand2s + v*sin(rand1)*rand2s + w*sqrt(1.0f - rand2));

	float3 newdir = normalize(u * cosPhi * sinTheta + v * sinPhi * sinTheta + w * cosTheta);

	ray->origin = hitpoint + w * EPSILON;
	ray->dir = newdir;

}
*/

void sample_diffuse(Ray* ray, float3 normal, bool is_refract, float rand1, float rand2, float cosTheta) {
	float3 hitpoint = ray->uv;
	float phi      = rand1;
	float sinTheta = sqrt(1.0f - cosTheta * cosTheta);

	float3 wi = normalize((float3)(cos(phi) * sinTheta, sin(phi) * sinTheta, cosTheta));
	float3 up = fabs(normal.z) < 0.999f ? (float3)(0.0f, 0.0f, 1.0f) : (float3)(1.0f, 0.0f, 0.0f);
	float3 x = normalize(cross(up, normal));
	float3 y = normalize(cross(normal, x));

	float3 newdir = normalize(x * wi.x + y * wi.y + normal * wi.z);
	ray->origin = hitpoint + (is_refract ? -normal : normal) * EPSILON;
	
	if (is_refract) {
		ray->dir = -newdir;
	} else {
		ray->dir = newdir;
	}
}


void refraction (Ray* ray, float3 n, float eta, bool is_refract) {
	float3 hitpoint = ray->uv;
	float3 i = ray->dir;

	float k = 1.0 - eta * eta * (1.0 - dot(n, i) * dot(n, i));
	if (k < 0.0 || !is_refract) {
		float3 S = dot(-i, n) * n;
		float3 P = S + i;
		ray->dir = normalize(2 * P - i);
		ray->origin = hitpoint + n * EPSILON;
	} else {
		ray->dir = normalize(eta * i - (eta * dot(n, i) + sqrt(k)) * n);
		ray->origin = hitpoint - n * EPSILON;
	}
	
}

float3 ImportanceSampleGGX(Ray* ray, const float Roughness, const float3 n, uint* randomSeed) {
	const float r1 = Random(randomSeed);
	const float r2 = Random(randomSeed);
	
	const float a        = Roughness * Roughness;
	const float phi      = r1 * 2.0f * PI;
	const float cosTheta = sqrt((1.0f - r2) / (1.0f + (a * a - 1.0f) * r2));
	const float sinTheta = sqrt(1.0f - cosTheta * cosTheta);
	
	const float3  h  = normalize((float3)(sinTheta * cos(phi), sinTheta * sin(phi), cosTheta));
	const float3  up = fabs(n.z) < 0.99f ? (float3)(0.0f, 0.0f, 1.0f) : (float3)(1.0f, 0.0f, 0.0f);
	const float3  x  = normalize(cross(up, n));
	const float3  y  = normalize(cross(n, x));
	
	return normalize(x * h.x + y * h.y + n * h.z);
}

float4 EvaluateDiffuse(const float3 Kd, const float NoL) {
	/*
	Incident light = Color * NoL
	Microfacet diffuse = Kd / PI
	PDF = NoL / PI
	*/

	const float3  diffuseFactor = Kd * NoL * ONE_OVER_PI;
	const float diffusePDF = NoL * ONE_OVER_PI;
	
	/*if (isnan(diffuseFactor.x) || isnan(diffuseFactor.y) || isnan(diffuseFactor.z)) {
		printf("nan\n");
	}*/

	return (float4)(diffuseFactor, diffusePDF);
}

float3 F_None(const float3 Ks) { 
	return Ks; 
}

float3 F_Schlick(const float3 Ks, const float VoH) {
	const float f0 = pow(1.0f - VoH, 5.0f);
	
	return f0 + (1.0f - f0) * Ks;
}

/*float3 F_Fresnel(const float3 Ks, const float VoH) {
	float3 KsSqrt = sqrt(clamp(Ks, (float3)(0.0f), (float3)(0.99f)));
	float3 n = (1.0f + KsSqrt) / (1.0f - KsSqrt);
	float3 g = sqrt(n * n + VoH * VoH - 1.0f);
	return 0.5 * pow((g - VoH) / (g + VoH), (float3)(2.0f)) * (1.0f + pow(((g + VoH) * VoH - 1.0f) / ((g - VoH) * VoH + 1.0f), (float3)(2.0f)));
}*/


float D_GGX(const float Roughness, const float NoH) {
	const float a  = Roughness * Roughness;
	const float a2 = a * a;
	const float d  = ( NoH * a2 - NoH ) * NoH + 1.0f;
	return a2 / (PI * d * d);
}


float G_Implicit(const float NoL, const float NoV) {
	return NoL * NoV;
}

float G_Neumann(const float NoL, const float NoV) {
	return (NoL * NoV) / max(NoL, NoV);
}


float4 EvaluateSpecular(const float3 Ks, const float Roughness, const float NoV, const float NoL, const float NoH, const float VoH) {
	const float3 F = F_None(Ks);
	const float D = D_GGX(Roughness, NoH);
	const float G = G_Neumann(NoL, NoV);
	
	const float3 specularFactor = F * D * G / (4.0f * NoV);
	const float specularPDF = D * NoH / (4.0f * VoH);
	
	return (float4)(specularFactor, specularPDF);
}

bool BRDF(global Triangle* triangles, Ray* ray, float3* color, uint* randomSeed) {

	float3 c = *color;

	if (ray->hitTriID == -1) {
		*color = (float3)(0, 0, 0);
		return true;
	} else {
		Triangle hitTriangle = triangles[(int)(ray->hitTriID)];
		float3 lightSource = (float3)(hitTriangle.emission.x, hitTriangle.emission.y, hitTriangle.emission.z);
		
		if (lightSource.x != 0) {
			*color = c * lightSource;
			return true;
		}

		if (ray->depth == BOUNCE) {
			*color = (float3)(0, 0, 0);
			return true;
		}

		float3 kd = (float3)(hitTriangle.diffuse.x, hitTriangle.diffuse.y, hitTriangle.diffuse.z);
		float3 ks = (float3)(hitTriangle.specular.x, hitTriangle.specular.y, hitTriangle.specular.z);
		float ni = hitTriangle.ninsa.x;
		float ns = hitTriangle.ninsa.y;
		float a = hitTriangle.ninsa.z;
		const float Roughness = NsToRoughness(ns);
		bool is_refract = false;

		float3 v = ray->dir;
		float3 l = (float3)(0, 0, 0);
		float3 normal = normalize((float3)(hitTriangle.n.x, hitTriangle.n.y, hitTriangle.n.z));
		float cosNO = dot(normal, -v);
		normal = cosNO > 0.0f ? normal : -normal;
		float3 h = 0;
		float eta = cosNO > 0.0f ? 1.0f / ni : ni;

		float3 sRGB = (float3)(0.212671f, 0.71516f, 0.072169f);
		float mD = dot(kd, sRGB);
		float mG = dot(ks, sRGB);

		float one_over_m  = 1.0f / (mD + mG);
		float pd = mD * one_over_m;
		float ps = mG * one_over_m;

		if (Random(randomSeed) > a) { 
			is_refract = true; 
		}

		if (Random(randomSeed) < pd) {
			float rand1 = 2.0f * PI * Random(randomSeed);
			float rand2 = Random(randomSeed);
			float rand2s = sqrt(rand2);

			sample_diffuse(ray, normal, is_refract, rand1, rand2, rand2s);
			h = normalize(-v + ray->dir);
		} else {
			h = ImportanceSampleGGX(ray, Roughness, normal, randomSeed);
			refraction(ray, h, eta, is_refract);
		}

		float NoV = fabs(dot(normal, v));
		float NoL = fabs(dot(normal, ray->dir));
		float NoH = fabs(dot(normal, h));
		float VoH = fabs(dot(v, h));
		float4 intensity = (float4)(0, 0, 0, 0);

		if (pd > 0.0f) {
			intensity += pd * EvaluateDiffuse(kd, NoL);
		}

		if (ps > 0.0f) {
			intensity += ps * EvaluateSpecular(ks, Roughness, NoV, NoL, NoH, VoH);
		}

		intensity = (float4)(intensity.x / intensity.w, intensity.y / intensity.w, intensity.z / intensity.w, intensity.w);
		
		if (isnan(intensity.x) || isnan(intensity.y) || isnan(intensity.z)) {
			*color = c;
			return true;
		}

		c = c * (float3)(intensity.x, intensity.y, intensity.z);
		
		/*if (c.x < 0.005 && c.y < 0.005 && c.z < 0.005) {
			*color = (float3)(0, 0, 0);
			return true;
		}*/
		
		*color = c;
		return false;
	}

}

kernel void brdf_kernel(global Triangle* triangles, global ray_info* ray_buffer, global long* streamTable, global int* atomicBuffer, global float4* output, const uint rng, int NDcounter) {
	unsigned int work_item_id = get_global_id(0);
	unsigned int x_coord = work_item_id % width;	
	unsigned int y_coord = work_item_id / width;	
	
	uint randomSeed = SetRandomSeed(hash1(x_coord) ^ hash2(y_coord) ^ rng);

	if (work_item_id < NDcounter) {
		int ray_id = streamTable[work_item_id];

		float3 color = (float3)(ray_buffer[ray_id].color.x, ray_buffer[ray_id].color.y, ray_buffer[ray_id].color.z);
		
		Ray ray;
		ray.hitTriID = ray_buffer[ray_id].ori.w;
		ray.dir = (float3)(ray_buffer[ray_id].dir.x, ray_buffer[ray_id].dir.y, ray_buffer[ray_id].dir.z);
		ray.uv = (float3)(ray_buffer[ray_id].hitPoint.x, ray_buffer[ray_id].hitPoint.y, ray_buffer[ray_id].hitPoint.z);
		ray.depth = ray_buffer[ray_id].dir.w;
		
		bool terminate = BRDF(triangles, &ray, &color, &randomSeed);

		if (terminate) {
			/*printf("%f %f %f\n", color.x, color.y, color.z);*/
			output[ray_id] += (float4)(color, 1);
			/*output[ray_id] += (float4)(1, 1, 1, 1);*/
		} else {
			int range = atomic_add(atomicBuffer, 1);
			
			streamTable[range] = ray_id;

			ray.depth = ray.depth + 1;

			ray_buffer[ray_id].color = (float4)(color, 0);
			ray_buffer[ray_id].dir = (float4)(ray.dir, ray.depth);
			ray_buffer[ray_id].ori = (float4)(ray.origin, ray.hitTriID);
		}
	} 
}