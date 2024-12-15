#ifndef RAYTRACEACCELERATOR_H_INCLUDED
#define RAYTRACEACCELERATOR_H_INCLUDED

#include <iostream>
#include <cstdio>
#include <vector>
#include <cmath>
#include <stack>
#include "Triangle.h"
#include "algebra3.h"

typedef struct Ray {
	vec3 origin;
	vec3 dir;
	float t;
}Ray;


class AABB
{
private:
	vec3 _points[2];

public:
	AABB()
	{
		_points[0] = vec3(1e9, 1e9, 1e9);    // min
		_points[1] = vec3(-1e9, -1e9, -1e9); // max
	}

	void addPoint(vec3 p);
	int maxDimension();
	float intersection(Ray& r);
	vec3 get_pointMin() { return _points[0]; };
	vec3 get_pointMax() { return _points[1]; };
};

class BVHNode
{
private:
	int L, R; // index of vector
	int _principal_d;
	AABB Box;
	int triList[2] = { -1, -1 };
	Triangle* t1 = NULL, * t2 = NULL;
	float isleaf = 0.0;
public:
	void setBound(int l, int r) { L = l; R = r; }
	AABB& getAABB() { return Box; }
	void setPD() { _principal_d = Box.maxDimension(); }
	void set_isleaf() { isleaf = 1.0; }
	void buildAABB(std::vector<Triangle>& v);
	void setTriList(int a, int b) { triList[a] = b; }
	void setTriangle(Triangle* _t1, Triangle* _t2) { t1 = _t1; t2 = _t2; }
	int getL() { return L; }
	int getR() { return R; }
	int getTriList(int a) { return triList[a]; }
	Triangle* getTriangle(int i) { return (i == 0 ? t1 : t2); }
	float get_isleaf() { return isleaf;}
};

class BVHTree
{
private:
	std::vector<BVHNode> BVH;
	void createBVHNode(std::vector<Triangle>& v, int index, int L, int R);

public:
	void buildBVHTree(std::vector<Triangle>& v);
	float findIntersection(Ray r, std::stack<std::pair<int, float>>& s, int index, int _valid_index, int& ans);
	BVHNode getBVHNode(int index) { return BVH[index]; }
	int getSize() { return BVH.size(); }
	void Print();
};

bool compare_x(Triangle& lhs, Triangle& rhs);
bool compare_y(Triangle& lhs, Triangle& rhs);
bool compare_z(Triangle& lhs, Triangle& rhs);

#endif /* bvh_h */
