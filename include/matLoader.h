#ifndef matLoader_h
#define matLoader_h

#include <map>
#include "Material.h"

#include <stdio.h>

std::map<std::string, Material> matLoader(const char * path);

void initMtl();
#endif /* matLoader_h */
