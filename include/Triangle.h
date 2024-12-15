#ifndef Triangle_h
#define Triangle_h

#include "algebra3.h"
#include "Material.h"

#include <stdio.h>


class Triangle {
private:
    vec3 _a;
    vec3 _b;
    vec3 _c;
    vec3 _n;
	vec3 _center;

public:
    Material _m;
    Triangle();
    void set_a(vec3 a) {_a = a;};
    void set_b(vec3 b) {_b = b;};
    void set_c(vec3 c) {_c = c;};
    void set_n(vec3 n) {_n = n;};
    void set_m(Material m) {_m = m;};
	void setCenter();
    vec3 get_a() {return _a;};
    vec3 get_b() {return _b;};
    vec3 get_c() {return _c;};
    vec3 get_n() {return _n;};
    Material get_m() {return _m;};
	vec3 getCenter() {return _center;};
};

#endif /* Triangle_h */
