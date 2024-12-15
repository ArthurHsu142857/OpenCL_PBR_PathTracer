#include "algebra3.h"
#include <vector>
#include "Material.h"
#include <stdio.h>
#include <string>
#include <cstring>
#include <iostream>
#include <map>
#include "matLoader.h"

std::map<std::string, bool> check;
Material m = Material();
std::map<std::string, Material> material;

std::map<std::string, Material> matLoader(const char *path) {
    printf("Loading material from %s \n", path);

    FILE *file = fopen(path, "r");
    std::string s;

    while(true){

        char line[100];
        char mtl[100];
        double x, y, z, sp;
        int res = fscanf(file, "%s", line);

        if (res == EOF)
            break; // EOF = End Of File. Quit the loop.
        if (strcmp(line, "newmtl") == 0) {
            m = Material();
            fscanf(file, "%s", mtl);
            s = std::string(mtl);
            initMtl();
        } else if(strcmp(line, "Ns") == 0) {
            fscanf(file, "%lf\n", &sp);
            m.setNs(sp);
        } else if(strcmp(line, "Ka") == 0) {
            fscanf(file, "%lf %lf %lf\n", &x, &y, &z);
            m.setKa(vec3(x, y, z));
        } else if(strcmp(line, "Kd") == 0) {
            fscanf(file, "%lf %lf %lf\n", &x, &y, &z);
            m.setKd(vec3(x, y, z));
        } else if(strcmp(line, "Ks") == 0) {
            fscanf(file, "%lf %lf %lf\n", &x, &y, &z);
            m.setKs(vec3(x, y, z));
        } else if(strcmp(line, "Ni") == 0) {
            fscanf(file, "%lf", &sp);
            m.setNi(sp);
        } else if(strcmp(line, "Ke") == 0) {
            fscanf(file, "%lf %lf %lf\n", &x, &y, &z);
            m.setKe(vec3(x, y, z));
        }
        else if (strcmp(line, "d") == 0) {
            fscanf(file, "%lf", &sp);
            m.setIa(sp);
        }
        material[s] = m;
    }
    fclose(file);
    return material;

}

void initMtl() {
    m = Material();
}
