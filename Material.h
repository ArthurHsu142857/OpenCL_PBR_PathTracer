#ifndef Material_h
#define Material_h
#include "algebra3.h"

#include <stdio.h>

class Material
{
public:
    vec3 _ka = vec3(0, 0, 0), _kd = vec3(0, 0, 0), _ks = vec3(0, 0, 0), _ke = vec3(0, 0, 0);
    double _ia = 0.5, _ns = 0, _reflect = 0, _refract = 0, _ni = 0;
    bool isReflection = false, isRefraction = false;
    int illum;
    Material();

    vec3 getKa() { return _ka; }
    vec3 getKd() { return _kd; }
    vec3 getKs() { return _ks; }
    vec3 getKe() { return _ke; }
    double getNs() { return _ns; }
    double getNi() { return _ni; }
    double getIa() { return _ia; }
    bool isReflect() { return isReflection; }
    double getReflect();
    double getRefract() { return _refract; }

    void setKa(vec3 ka) { _ka = ka; }
    void setKd(vec3 kd) { _kd = kd; }
    void setKs(vec3 ks) { _ks = ks; }
    void setKe(vec3 ke) { _ke = ke; }
    void setNs(double ns) { _ns = ns; }
    void setNi(double nr) { _ni = nr; }
    void setIa(double ia) { _ia = ia; }
    void setIllum(int I) { illum = I; }
    void setReflection(bool d) { isReflection = d; }
    void setRefraction(bool d) { isRefraction = d; }
};
#endif /* Material_h */
