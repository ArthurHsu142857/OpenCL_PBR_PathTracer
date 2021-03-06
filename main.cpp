// based on smallpt by Kevin Beason 
// based on simple sphere path tracer by Sam Lapere, 2016

#include <glad\glad.h> 
#include <GLFW\glfw3.h>

#include <CL\cl.hpp>

#include <iostream>
#include <fstream>
#include <vector>
#include <math.h>
#include <random>
#include <windows.h>
#include <time.h>
#include <atomic>

#include "algebra3.h"
#include "Triangle.h"
#include "Material.h"
#include "Shader.h"

#include "objLoader.h"
#include "matLoader.h"
#include "bvh.h"


using namespace cl;

const int image_width = 512;
const int image_height = 512;
const int bounces = 15;
int SAMPLE = 0;

cl_float4* gpu_output;
cl_float3 vbo_output[image_width * image_height];
cl_float4* output;

CommandQueue queue;
Device device;
Kernel kernel;
Kernel initial, generation, traversal, rendering, initAtomic;
Context context;
Program program;
Buffer cl_output, streamTable, ray_buffer;
Buffer atomicBuffer;
int* streamCounter = new int;

int deb[263144] = { 0 };
int debC = 0;

HGLRC g_hRC = NULL;
HDC   g_hDC = NULL;
HWND  g_hWnd = NULL;


// 2
//const int BVH_size = 1024;
//const int triangle_size = 1015;

// 4
//const int BVH_size = 64;
//const int triangle_size = 38;

// 0
//const int triangle_size = 46;
//const int BVH_size = 64;

// 5
//const int BVH_size = 32;
//const int triangle_size = 22;

// 6
//const int BVH_size = 1024;
//const int triangle_size = 1023;
//float3 uni_eye = (float3)(0.0f, 0.2f, -8.4f);


// 7
//const int BVH_size = 1024;
//const int triangle_size = 1013;
//float3 uni_eye = (float3)(0.0f, 0.2f, 5.4f);

// box3
//const int BVH_size = 1024;
//const int triangle_size = 994;
//float3 uni_eye = (float3)(0.0f, 2.0f, 5.2f);

// box4
//const int BVH_size = 4096;
//const int triangle_size = 3945;
//float3 uni_eye = (float3)(0.0f, 2.0f, 5.2f);

// box5
//const int BVH_size = 4096;
//const int triangle_size = 3943;
//float3 uni_eye = (float3)(0.0f, 2.0f, 5.2f);

// bathroom
const int BVH_size = 2097152;
const int triangle_size = 1415864;
//float3 uni_view = normalize((float3)(0, 0, 1));
//float3 uni_eye = (float3)(0.0, 0.68, -2.48);


// dummy variables are required for memory alignment
// float3 is considered as float4 by OpenCL
struct ray_info
{
	cl_float4 ori;
	cl_float4 dir;
	cl_float4 color;
	cl_float4 hitPoint;
	__declspec(align(16))cl_int coord;
};

struct cl_BVH {
	__declspec(align(16))cl_float3 points[2];
	__declspec(align(16))cl_int triangleList[2] = { -1, -1 };
};

struct cl_Triangle {
	cl_float _a[3];
	cl_float _b[3];
	cl_float _c[3];
	cl_float _n[3];
	cl_float _center[3];
	cl_float _ambient[3];
	cl_float _diffuse[3];
	cl_float _specular[3];
	cl_float _emission[3];
	cl_float _ninsa[3];
};

cl_BVH* cl_BVHTree;
cl_Triangle* cl_triangles;
int count = 0;


void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
	glViewport(0, 0, width, height);
}

void processInput(GLFWwindow* window) {
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
}

void pickPlatform(Platform& platform, const std::vector<Platform>& platforms) {

	if (platforms.size() == 1) platform = platforms[0];
	else {
		int input = 1;
		std::cout << "\nChoose an OpenCL platform: ";
		//std::cin >> input;

		// handle incorrect user input
		while (input < 1 || input > platforms.size()) {
			std::cin.clear(); //clear errors/bad flags on cin
			std::cin.ignore(std::cin.rdbuf()->in_avail(), '\n'); // ignores exact number of chars in cin buffer
			std::cout << "No such option. Choose an OpenCL platform: ";
			std::cin >> input;
		}
		platform = platforms[input - 1];
	}
}

void pickDevice(Device& device, const std::vector<Device>& devices) {

	if (devices.size() == 1) device = devices[0];
	else {
		int input = 1;
		std::cout << "\nChoose an OpenCL device: ";
		//std::cin >> input;

		// handle incorrect user input
		while (input < 1 || input > devices.size()) {
			std::cin.clear(); //clear errors/bad flags on cin
			std::cin.ignore(std::cin.rdbuf()->in_avail(), '\n'); // ignores exact number of chars in cin buffer
			std::cout << "No such option. Choose an OpenCL device: ";
			std::cin >> input;
		}
		device = devices[input - 1];
	}

	cl_device_svm_capabilities caps;

	cl_int err = clGetDeviceInfo(
		device(),
		CL_DEVICE_SVM_CAPABILITIES,
		sizeof(cl_device_svm_capabilities),
		&caps,
		0
	);

	if (err == CL_SUCCESS && (caps & CL_DEVICE_SVM_COARSE_GRAIN_BUFFER)) {
		printf("Coarse-grained buffer : true");
	}
	else {
		printf("Coarse-grained buffer : false");
	}


}

void printErrorLog(const Program& program, const Device& device) {

	// Get the error log and print to console
	std::string buildlog = program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(device);
	std::cerr << "Build log:" << std::endl << buildlog << std::endl;

	// Print the error log to a file
	FILE* log = fopen("errorlog.txt", "w");
	fprintf(log, "%s\n", buildlog);
	std::cout << "Error log saved in 'errorlog.txt'" << std::endl;
	system("PAUSE");
	exit(1);
}

void initOpenCL() {
	// Get all available OpenCL platforms (e.g. AMD OpenCL, Nvidia CUDA, Intel OpenCL)
	std::vector<cl::Platform> platforms;
	Platform::get(&platforms);
	std::cout << "Available OpenCL platforms : " << std::endl << std::endl;
	for (int i = 0; i < platforms.size(); i++)
		std::cout << "\t" << i + 1 << ": " << platforms[i].getInfo<CL_PLATFORM_NAME>() << std::endl;

	//std::cout << platforms[1].getInfo<CL_PLATFORM_VERSION>() << std::endl;

	// Pick one platform
	Platform platform;
	pickPlatform(platform, platforms);
	std::cout << "\nUsing OpenCL platform: \t" << platform.getInfo<CL_PLATFORM_NAME>() << std::endl;

	// Get available OpenCL devices on platform
	std::vector<cl::Device> devices;
	platform.getDevices(CL_DEVICE_TYPE_ALL, &devices);

	std::cout << "Available OpenCL devices on this platform: " << std::endl << std::endl;
	for (int i = 0; i < devices.size(); i++) {
		std::cout << "\t" << i + 1 << ": " << devices[i].getInfo<CL_DEVICE_NAME>() << std::endl;
		std::cout << "\t\tMax compute units: " << devices[i].getInfo<CL_DEVICE_MAX_COMPUTE_UNITS>() << std::endl;
		std::cout << "\t\tMax work group size: " << devices[i].getInfo<CL_DEVICE_MAX_WORK_GROUP_SIZE>() << std::endl;
		std::vector<::size_t> maxWorkItems = devices[i].getInfo<CL_DEVICE_MAX_WORK_ITEM_SIZES>();
		std::cout << "\t\tMax work item size: " << maxWorkItems[0] << std::endl;
		std::cout << "\t\tMax work item size: " << maxWorkItems[1] << std::endl;
		std::cout << "\t\tMax work item size: " << maxWorkItems[2] << std::endl;
		std::cout << "\t\tVersion:" << devices[i].getInfo<CL_DEVICE_OPENCL_C_VERSION>() << std::endl << std::endl;
	}

	// Pick one device
	pickDevice(device, devices);
	std::cout << "\nUsing OpenCL device: \t" << device.getInfo<CL_DEVICE_NAME>() << std::endl;
	std::cout << "\t\t\tMax compute units: " << device.getInfo<CL_DEVICE_MAX_COMPUTE_UNITS>() << std::endl;
	std::cout << "\t\t\tMax work group size: " << device.getInfo<CL_DEVICE_MAX_WORK_GROUP_SIZE>() << std::endl;


	// Create an OpenCL context on that device.
	// Windows specific OpenCL-OpenGL interop

	//g_hRC = wglCreateContext(g_hDC);
	//g_hDC = GetDC(g_hWnd);

	/*cl_context_properties properties[] =
	{
		CL_CONTEXT_PLATFORM, (cl_context_properties)platform(),
		CL_GL_CONTEXT_KHR,   (cl_context_properties)g_hRC,
		CL_WGL_HDC_KHR,      (cl_context_properties)g_hDC,
		0
	};*/

	cl_context_properties properties[] = {
		CL_GL_CONTEXT_KHR, (cl_context_properties)wglGetCurrentContext(),
		CL_WGL_HDC_KHR, (cl_context_properties)wglGetCurrentDC(),
		CL_CONTEXT_PLATFORM, (cl_context_properties)platform(),
		0
	};

	cl_int error;
	context = Context(device, properties, NULL, NULL, &error);

	// Create a command queue
	queue = CommandQueue(context, device);

	// Convert the OpenCL source code to a string
	std::string source;
	std::ifstream init("./kernel/init.cl");
	std::ifstream gen("./kernel/ray_generation.cl");
	std::ifstream traverse("./kernel/traversal.cl");
	std::ifstream render("./kernel/sampleBybrdf.cl");
	std::ifstream cleanAtomic("./kernel/initAtomic.cl");

	if (!init && !gen && !traverse && !render) {
		std::cout << "\nNo OpenCL file found!" << std::endl << "Exiting..." << std::endl;
		system("PAUSE");
		exit(1);
	}

	while (!init.eof()) {
		char line[256];
		init.getline(line, 255);
		source += line;
	}

	const char* kernel_source = source.c_str();

	// Create an OpenCL program by performing runtime source compilation for the chosen device
	program = Program(context, kernel_source);
	cl_int result = program.build({ device }, "-cl-std=CL2.0");
	if (result) std::cout << "Error during compilation OpenCL code!!!\n (" << result << ")" << std::endl;
	if (result == CL_BUILD_PROGRAM_FAILURE) printErrorLog(program, device);

	// Create a kernel (entry point in the OpenCL source program)
	initial = Kernel(program, "initial_kernel");

	source = "";

	while (!gen.eof()) {
		char line[256];
		gen.getline(line, 255);
		source += line;
	}

	kernel_source = source.c_str();

	// Create an OpenCL program by performing runtime source compilation for the chosen device
	program = Program(context, kernel_source);
	result = program.build({ device }, "-cl-std=CL2.0");
	if (result) std::cout << "Error during compilation OpenCL code!!!\n (" << result << ")" << std::endl;
	if (result == CL_BUILD_PROGRAM_FAILURE) printErrorLog(program, device);

	// Create a kernel (entry point in the OpenCL source program)
	generation = Kernel(program, "generation_kernel");

	source = "";

	while (!traverse.eof()) {
		char line[256];
		traverse.getline(line, 255);
		source += line;
	}

	kernel_source = source.c_str();

	// Create an OpenCL program by performing runtime source compilation for the chosen device
	program = Program(context, kernel_source);
	result = program.build({ device }, "-cl-std=CL2.0");
	if (result) std::cout << "Error during compilation OpenCL code!!!\n (" << result << ")" << std::endl;
	if (result == CL_BUILD_PROGRAM_FAILURE) printErrorLog(program, device);

	// Create a kernel (entry point in the OpenCL source program)
	traversal = Kernel(program, "traversal_kernel");

	source = "";

	while (!render.eof()) {
		char line[256];
		render.getline(line, 255);
		source += line;
	}

	kernel_source = source.c_str();

	// Create an OpenCL program by performing runtime source compilation for the chosen device
	program = Program(context, kernel_source);
	result = program.build({ device }, "-cl-std=CL2.0");
	if (result) std::cout << "Error during compilation OpenCL code!!!\n (" << result << ")" << std::endl;
	if (result == CL_BUILD_PROGRAM_FAILURE) printErrorLog(program, device);

	// Create a kernel (entry point in the OpenCL source program)
	rendering = Kernel(program, "brdf_kernel");

	source = "";

	while (!cleanAtomic.eof()) {
		char line[256];
		cleanAtomic.getline(line, 255);
		source += line;
	}

	kernel_source = source.c_str();

	// Create an OpenCL program by performing runtime source compilation for the chosen device
	program = Program(context, kernel_source);
	result = program.build({ device }, "-cl-std=CL2.0");
	if (result) std::cout << "Error during compilation OpenCL code!!!\n (" << result << ")" << std::endl;
	if (result == CL_BUILD_PROGRAM_FAILURE) printErrorLog(program, device);

	// Create a kernel (entry point in the OpenCL source program)
	initAtomic = Kernel(program, "initAtomic_kernel");
}

void initTriSturct(std::vector<Triangle> triangles) {

	cl_Triangle* data = cl_triangles;
	cl_Triangle* temp = data;

	// transport data
	for (int i = 0; i < triangle_size; i++) {
		vec3 vec;
		vec = triangles[i].get_a();
		temp->_a[0] = vec[0];
		temp->_a[1] = vec[1];
		temp->_a[2] = vec[2];
		vec = triangles[i].get_b();
		temp->_b[0] = vec[0];
		temp->_b[1] = vec[1];
		temp->_b[2] = vec[2];
		vec = triangles[i].get_c();
		temp->_c[0] = vec[0];
		temp->_c[1] = vec[1];
		temp->_c[2] = vec[2];
		vec = triangles[i].get_n();
		temp->_n[0] = vec[0];
		temp->_n[1] = vec[1];
		temp->_n[2] = vec[2];
		vec = triangles[i].getCenter();
		temp->_center[0] = vec[0];
		temp->_center[1] = vec[1];
		temp->_center[2] = vec[2];

		Material mat = triangles[i].get_m();
		vec = mat.getKa();
		temp->_ambient[0] = vec[0];
		temp->_ambient[1] = vec[1];
		temp->_ambient[2] = vec[2];
		vec = mat.getKd();
		temp->_diffuse[0] = vec[0];
		temp->_diffuse[1] = vec[1];
		temp->_diffuse[2] = vec[2];
		vec = mat.getKs();
		temp->_specular[0] = vec[0];
		temp->_specular[1] = vec[1];
		temp->_specular[2] = vec[2];
		vec = mat.getKe();
		temp->_emission[0] = vec[0];
		temp->_emission[1] = vec[1];
		temp->_emission[2] = vec[2];
		float ni = mat.getNi();
		temp->_ninsa[0] = ni;
		float ns = mat.getNs();
		temp->_ninsa[1] = ns;
		float a = mat.getIa();
		temp->_ninsa[2] = a;

		temp += 1;
	}

}

void initBVHstruct(BVHTree bt) {

	cl_BVH* data = cl_BVHTree;
	cl_BVH* temp = data;

	temp->points[1].x = -1;
	temp->points[1].y = -1;
	temp->points[1].z = -1;

	temp->points[0].x = -1;
	temp->points[0].y = -1;
	temp->points[0].z = -1;

	temp->triangleList[0] = -1;
	temp->triangleList[1] = -1;

	for (int i = 1; i < bt.getSize(); i++) {

		temp += 1;

		BVHNode node = bt.getBVHNode(i);
		AABB box = bt.getBVHNode(i).getAABB();
		vec3 vec;

		vec = box.get_pointMax();
		temp->points[1].x = vec[0];
		temp->points[1].y = vec[1];
		temp->points[1].z = vec[2];

		vec = box.get_pointMin();
		temp->points[0].x = vec[0];
		temp->points[0].y = vec[1];
		temp->points[0].z = vec[2];

		if (node.get_isleaf() == 1.0) {
			for (int j = 0; j < 2; j++) {
				int tri = bt.getBVHNode(i).getTriList(j);
				if (tri != -1) {
					temp->triangleList[j] = tri;
				}
				else {
					temp->triangleList[j] = -1;
				}
			}
		}
		else {
			temp->triangleList[0] = -1;
		}
	}

}

inline float clamp(float x) { return x < 0.0f ? 0.0f : x > 1.0f ? 1.0f : x; }

// convert RGB float in range [0,1] to int in range [0, 255] and perform gamma correction
inline int toInt(float x) { return int(clamp(x) * 255 + .5); }

void saveImage() {
	// write image to PPM file, a very simple image file format
	// PPM files can be opened with IrfanView (download at www.irfanview.com) or GIMP
	FILE* f = fopen("opencl_raytracer.ppm", "w");
	fprintf(f, "P3\n%d %d\n%d\n", image_width, image_height, 255);

	cl_int err = CL_SUCCESS;

	// loop over all pixels, write RGB values
	for (int i = 0; i < image_width * image_height; i++) {
		//std::cout << "save : " << output[i].s[0] << std::endl;
		fprintf(f, "%d %d %d ",
			toInt(output[i].s[0] / output[i].s[3]),
			toInt(output[i].s[1] / output[i].s[3]),
			toInt(output[i].s[2] / output[i].s[3]));
	}
}

#define float3(x, y, z) {{x, y, z}}  // macro to replace ugly initializer braces

void main() {

	// initialise OpenGL
	std::cout << "initialise OpenGL...\n" << std::endl;

	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(image_width, image_height, "LearnOpenGL", NULL, NULL);

	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
	}

	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	// glad: load all OpenGL function pointers
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
	}

	Shader shader("./shader/texture.vs", "./shader/texture.fs");

	float vertices[] = {
		// positions          // colors           // texture coords
		 0.8f,  0.8f, 0.0f,   1.0f, 0.0f, 0.0f,   1.0f, 0.0f, // top right
		 0.8f, -0.8f, 0.0f,   0.0f, 1.0f, 0.0f,   1.0f, 1.0f, // bottom right
		-0.8f, -0.8f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 1.0f, // bottom left
		-0.8f,  0.8f, 0.0f,   1.0f, 1.0f, 0.0f,   0.0f, 0.0f  // top left 
	};
	unsigned int indices[] = {
		0, 1, 3, // first triangle
		1, 2, 3  // second triangle
	};
	unsigned int VBO, VAO, EBO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	// position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	// color attribute
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	// texture coord attribute
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);

	unsigned int texture;
	//generate the texture ID
	glGenTextures(1, &texture);
	//binnding the texture
	glBindTexture(GL_TEXTURE_2D, texture);
	//regular sampler params
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	//need to set GL_NEAREST
	//(not GL_NEAREST_MIPMAP_* which would cause CL_INVALID_GL_OBJECT later)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	//specify texture dimensions, format etc
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, image_width, image_height, 0, GL_RGBA, GL_FLOAT, 0);

	// initialise OpenCL
	std::cout << "initialise OpenCL...\n" << std::endl;

	initOpenCL();
	cl_int err = CL_SUCCESS;

	// Create buffers on the OpenCL device for the image and the scene

	output = new cl_float4[image_width * image_height];

	streamTable = Buffer(context, CL_MEM_READ_WRITE, image_width * image_height * sizeof(cl_long));
	ray_buffer = Buffer(context, CL_MEM_READ_WRITE, image_width * image_height * sizeof(ray_info));

	cl_triangles = (cl_Triangle*)clSVMAlloc(
		context(),                // the context where this memory is supposed to be used
		CL_MEM_READ_ONLY,
		triangle_size * sizeof(cl_Triangle),   // amount of memory to allocate (in bytes)
		0                       // alignment in bytes (0 means default)
	);

	gpu_output = (cl_float3*)clSVMAlloc(
		context(),                // the context where this memory is supposed to be used
		CL_MEM_WRITE_ONLY,
		image_width * image_height * sizeof(cl_float4),     // amount of memory to allocate (in bytes)
		0                       // alignment in bytes (0 means default)
	);

	// initialise scene

	std::map<std::string, Material> material;
	std::cout << "Loading mtl" << std::endl;
	//material = matLoader("./scene/box_4.mtl");
	material = matLoader("./scene/bathroom.mtl");
	//material = matLoader("untitled7.mtl");

	std::cout << std::endl;

	std::vector<Triangle> triangles;
	std::cout << "Loading obj" << std::endl;
	//bool res = objLoader("./scene/box_4.obj", triangles, material);
	bool res = objLoader("./scene/bathroom.obj", triangles, material);
	//bool res = objLoader("untitled7.obj", triangles, material);

	BVHTree bt;
	bt.buildBVHTree(triangles);

	cl_BVHTree = (cl_BVH*)clSVMAlloc(
		context(),                // the context where this memory is supposed to be used
		CL_MEM_READ_ONLY,
		BVH_size * sizeof(cl_BVH),   // amount of memory to allocate (in bytes)
		0                       // alignment in bytes (0 means default)
	);

	err = clEnqueueSVMMap(
		queue(),
		CL_TRUE,       // blocking map
		CL_MAP_WRITE,
		cl_BVHTree,
		sizeof(cl_BVH) * BVH_size,
		0, 0, 0
	);

	initBVHstruct(bt);

	err = clEnqueueSVMUnmap(
		queue(),
		cl_BVHTree,
		0, 0, 0
	);

	err = clEnqueueSVMMap(
		queue(),
		CL_TRUE,       // blocking map
		CL_MAP_WRITE,
		cl_triangles,
		sizeof(cl_Triangle) * triangle_size,
		0, 0, 0
	);

	initTriSturct(triangles);

	err = clEnqueueSVMUnmap(
		queue(),
		cl_triangles,
		0, 0, 0
	);

	atomicBuffer = Buffer(context, CL_MEM_READ_WRITE, sizeof(cl_int));

	// every pixel in the image has its own thread or "work item",
	// so the total amount of work items equals the number of pixels
	std::size_t global_work_size = image_width * image_height;

	std::size_t local_work_size = 64;
	std::cout << "Kernel work group size: " << local_work_size << std::endl;

	if (global_work_size % local_work_size != 0)
		global_work_size = (global_work_size / local_work_size + 1) * local_work_size;

	// Ensure the global work size is a multiple of local work size

	std::cout << "Rendering started..." << std::endl;

	double START, END;
	START = clock();
	double tt;

	while (!glfwWindowShouldClose(window)) {

		// ray initial-------------------------------

		initial.setArg(0, ray_buffer);
		clSetKernelArgSVMPointer(initial(), 1, gpu_output);
		initial.setArg(2, streamTable);
		initial.setArg(3, atomicBuffer);

		queue.enqueueNDRangeKernel(initial, NULL, image_width * image_height, local_work_size, NULL, NULL);

		queue.enqueueReadBuffer(atomicBuffer, CL_TRUE, 0, sizeof(cl_int), streamCounter);

		while (true) {

			processInput(window);

			// ray generation-------------------------------

			std::random_device rd; 
			std::default_random_engine generator(rd()); 
			std::uniform_real_distribution<cl_float> random_float(0.0001f, 0.001f); 
			std::uniform_int_distribution<cl_uint> random_uint(0, CL_UINT_MAX);

			float rngx = random_float(generator) - 0.5;
			float rngy = random_float(generator) - 0.5;

			generation.setArg(0, ray_buffer);
			generation.setArg(1, streamTable);
			generation.setArg(2, atomicBuffer);
			generation.setArg(3, rngx);
			generation.setArg(4, rngy);

			queue.enqueueNDRangeKernel(generation, NULL, image_width * image_height, local_work_size, NULL, NULL);

			queue.enqueueReadBuffer(atomicBuffer, CL_TRUE, 0, sizeof(cl_int), streamCounter);

			int NDCounter = 0;
			int modND = 0;


			for (int i = 0; i < bounces; i++) {

				queue.enqueueReadBuffer(atomicBuffer, CL_TRUE, 0, sizeof(cl_int), streamCounter);

				//queue.finish();

				NDCounter = *streamCounter;
				modND = *streamCounter;


				if (modND % local_work_size != 0)
					modND = (modND / local_work_size + 1) * local_work_size;


				// clean atomic-------------------------------

				initAtomic.setArg(0, atomicBuffer);

				queue.enqueueNDRangeKernel(initAtomic, NULL, 1, 1, NULL, NULL);

				//queue.finish();

				// ray traversal-------------------------------

				//glFinish();

				clSetKernelArgSVMPointer(traversal(), 0, cl_triangles);
				clSetKernelArgSVMPointer(traversal(), 1, cl_BVHTree);
				traversal.setArg(2, streamTable);
				traversal.setArg(3, NDCounter);
				traversal.setArg(4, ray_buffer);

				queue.enqueueNDRangeKernel(traversal, NULL, modND, local_work_size, NULL, NULL);

				//queue.finish();

				// rendering-------------------------------

				unsigned int rng = random_uint(generator);
				std::uniform_real_distribution<cl_float> random_float_01(0.0f, 1.0f);

				clSetKernelArgSVMPointer(rendering(), 0, cl_triangles);
				rendering.setArg(1, ray_buffer);
				rendering.setArg(2, streamTable);
				rendering.setArg(3, atomicBuffer);
				clSetKernelArgSVMPointer(rendering(), 4, gpu_output);
				rendering.setArg(5, rng);
				rendering.setArg(6, NDCounter);

				queue.enqueueNDRangeKernel(rendering, NULL, modND, local_work_size, NULL, NULL);

				//queue.finish();

				queue.enqueueReadBuffer(atomicBuffer, CL_TRUE, 0, sizeof(cl_int), streamCounter);

				//queue.finish();

			}

			err = clEnqueueSVMMap(
				queue(),
				CL_TRUE,       // blocking map
				CL_MAP_READ,
				gpu_output,
				sizeof(cl_float4) * image_width * image_height,
				0, 0, 0
			);

			//std::cout << "Rendering done! \nCopying output from device to host" << std::endl;

			glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT);

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, texture);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, image_width, image_height, 0, GL_RGBA, GL_FLOAT, gpu_output);

			err = clEnqueueSVMUnmap(
				queue(),
				gpu_output,
				0, 0, 0
			);

			shader.use();
			glBindVertexArray(VAO); 
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

			glfwSwapBuffers(window);
			glfwPollEvents();

			SAMPLE++;
			
			if (SAMPLE == 3000) {
				break;
			}
			
		}

		END = clock();
		tt = (END - START);

		printf("%f\n", tt);

		err = clEnqueueSVMMap(
			queue(),
			CL_TRUE,       // blocking map
			CL_MAP_READ,
			gpu_output,
			sizeof(cl_float4) * image_width * image_height,
			0, 0, 0
		);

		for (int j = 0; j < image_width * image_height; j++) {
			output[j] = { gpu_output[j].s[0], gpu_output[j].s[1], gpu_output[j].s[2], gpu_output[j].s[3] };
		}

		err = clEnqueueSVMUnmap(
			queue(),
			gpu_output,
			0, 0, 0
		);

		std::cout << "loop time " << SAMPLE << std::endl;
		
		break;
	}

	glfwTerminate();

	// save image
	saveImage();
	std::cout << "Saved image to 'opencl_raytracer.ppm'" << std::endl;

	system("PAUSE");

}
