#include <algorithm>
#include "bvh.h"

using namespace std;

void AABB::addPoint(vec3 p) {
	for (int i = 0; i < 3; i++) {
		_points[0][i] = min(_points[0][i], p[i]);
		_points[1][i] = max(_points[1][i], p[i]);
	}
}

int AABB::maxDimension() {
	vec3 delta = _points[1] - _points[0];
	int index = 0;
	for (int i = 1; i < 3; i++) {
		if (delta[i] > delta[index])
			index = i;
	}
	return index;
}

void BVHNode::buildAABB(std::vector<Triangle>& v) {
	Box = AABB();

	for (int i = L; i <= R; i++) {
		Box.addPoint(v[i].get_a());
		Box.addPoint(v[i].get_b());
		Box.addPoint(v[i].get_c());
	}

	setPD();

	switch (_principal_d) {
	case 0:
		sort(v.begin() + L, v.begin() + R + 1, compare_x);
		break;
	case 1:
		sort(v.begin() + L, v.begin() + R + 1, compare_y);
		break;
	case 2:
		sort(v.begin() + L, v.begin() + R + 1, compare_z);
		break;
	}
}

void BVHTree::createBVHNode(std::vector<Triangle>& v, int index, int L, int R) {
	if (L > R)
		return;

	int mid = (L + R) / 2;

	BVH[index].setBound(L, R);
	BVH[index].buildAABB(v);

	if (L == R) {
		BVH[index].setTriList(0, L);
		BVH[index].setTriList(1, -1);
		BVH[index].set_isleaf();
		return;
	}
	if (L + 1 == R) {
		BVH[index].setTriList(0, L);
		BVH[index].setTriList(1, R);
		BVH[index].set_isleaf();
		return;
	}

	createBVHNode(v, index * 2, L, mid);
	createBVHNode(v, index * 2 + 1, mid + 1, R);
}

void BVHTree::buildBVHTree(std::vector<Triangle>& v) {
	int LOG = 0;
	int TMP = v.size();
	while (TMP) {
		TMP /= 2;
		LOG++;
	}
	BVH = vector<BVHNode>(1 << (LOG));
	cout << "triangles : " << v.size() << endl;
	cout << "BVH size : " << BVH.size() << endl;
	createBVHNode(v, 1, 0, v.size() - 1);
}

bool compare_x(Triangle& lhs, Triangle& rhs) {
	return lhs.getCenter()[0] < rhs.getCenter()[0];
}

bool compare_y(Triangle& lhs, Triangle& rhs) {
	return lhs.getCenter()[1] < rhs.getCenter()[1];
}

bool compare_z(Triangle& lhs, Triangle& rhs) {
	return lhs.getCenter()[2] < rhs.getCenter()[2];
}
