#ifndef objLoader_h
#define objLoader_h
#include <map>
#include "Material.h"
#include "Triangle.h"
#include <stdio.h>

bool objLoader(const char *path, std::vector<Triangle>& out_triangle, std::map<std::string, Material> material);
#endif /* objLoader_h */
