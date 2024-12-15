#include "Triangle.h"

Triangle::Triangle() {
    _a = vec3(0, 0, 0);
    _b = vec3(0, 0, 0);
    _c = vec3(0, 0, 0);
    _n = vec3(0, 0, 0);
    _m = Material();
}

void Triangle::setCenter()
{
	_center = _a + _b + _c;
	_center /= 3;
}

