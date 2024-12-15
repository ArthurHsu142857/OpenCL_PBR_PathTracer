#include "Material.h"
#include "algebra3.h"


Material::Material()
{
    _ka = vec3(0, 0, 0);
    _kd = vec3(0, 0, 0);
    _ks = vec3(0, 0, 0);
    _ia = 0.5;
    _ns = 100;
    _reflect = 0;
    _refract = 0;
    _ni = 0;
    _ke = (0, 0, 0);
    isReflection = false;
}
