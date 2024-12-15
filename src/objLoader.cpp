#include "algebra3.h"
#include <vector>
#include "Material.h"
#include <stdio.h>
#include <string>
#include <cstring>
#include <iostream>
#include <map>
#include "Triangle.h"
#include "objLoader.h"

bool objLoader(const char* path, std::vector<Triangle>& out_triangle, std::map<std::string, Material> material) {

	printf("Loading obj from %s \n", path);
	std::vector<vec3> out_vertices;
	std::vector<vec3> out_normals;
	std::vector<int> vertexIndices, uvIndices, normalIndices;
	std::vector<vec3> temp_vertices;
	std::vector<vec3> temp_normals;
	std::vector<std::string> temp_mtls;

	FILE* file = fopen(path, "r");
	std::string s;

	while (true) {

		char line[256];

		// read the first word of the line
		int res = fscanf(file, "%s", line);
		if (res == EOF)
			break; // EOF = End Of File. Quit the loop.

		// else : parse lineHeader
		if (strcmp(line, "usemtl") == 0) {
			char usemtl[100];
			fscanf(file, "%s\n", usemtl);
			s = std::string(usemtl);
		}
		if (strcmp(line, "v") == 0) {
			vec3 vertex;
			fscanf(file, "%f %f %f\n", &vertex[0], &vertex[1], &vertex[2]);
			temp_vertices.push_back(vertex);
		}
		else if (strcmp(line, "vn") == 0) {
			vec3 normal;
			fscanf(file, "%f %f %f\n", &normal[0], &normal[1], &normal[2]);
			temp_normals.push_back(normal);
		}
		else if (strcmp(line, "f") == 0) {
			std::string vertex1, vertex2, vertex3;
			int vertexIndex[3], uvIndex[3], normalIndex[3];
			int matches = fscanf(file, "%d/%d/%d %d/%d/%d %d/%d/%d\n", &vertexIndex[0], &uvIndex[0], &normalIndex[0], &vertexIndex[1], &uvIndex[1], &normalIndex[1], &vertexIndex[2], &uvIndex[2], &normalIndex[2]);
			//int matches = fscanf(file, "%d//%d %d//%d %d//%d\n", &vertexIndex[0], &normalIndex[0], &vertexIndex[1], &normalIndex[1], &vertexIndex[2], &normalIndex[2]);

			if (matches != 9) {
				printf("File can't be read by our simple parser :-( Try exporting with other options\n");
			}

			vertexIndices.push_back(vertexIndex[0]);
			vertexIndices.push_back(vertexIndex[1]);
			vertexIndices.push_back(vertexIndex[2]);

			normalIndices.push_back(normalIndex[0]);
			normalIndices.push_back(normalIndex[1]);
			normalIndices.push_back(normalIndex[2]);
			temp_mtls.push_back(s);
		}

	}

	// For each vertex of each triangle
	for (int i = 0; i < vertexIndices.size(); i++) {

		// Get the indices of its attributes
		int vertexIndex = vertexIndices[i];
		int normalIndex = normalIndices[i];

		// Get the attributes thanks to the index
		vec3 vertex = temp_vertices[vertexIndex - 1];
		vec3 normal = temp_normals[normalIndex - 1];

		// Put the attributes in buffers
		out_vertices.push_back(vertex);
		out_normals.push_back(normal);

	}

	for (int i = 0; i < out_vertices.size(); i += 3) {
		Triangle temp;
		temp.set_a(out_vertices[i]);
		temp.set_b(out_vertices[i + 1]);
		temp.set_c(out_vertices[i + 2]);
		temp.setCenter();
		vec3 nor = ((out_normals[i] + out_normals[i + 1] + out_normals[i + 2]) / 3).normalize();
		temp.set_n(nor);
		std::string mt = temp_mtls[i / 3];
		temp.set_m(material[mt]);
		out_triangle.push_back(temp);
	}

	fclose(file);

	return true;
}

