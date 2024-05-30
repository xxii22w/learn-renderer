#include "tgaimage.h"
#include "model.h"
#include "geometry.h"
#include <iostream>
using namespace std;

// 定义颜色
const TGAColor white = { 255,255,255,255 };
const TGAColor red = { 255, 0, 0, 255 };
const TGAColor green = { 0, 255, 0, 255 };
Model* model = NULL;

// 定义宽度高度
const int width = 800;
const int height = 800;


// 画线算法(坐标1，坐标2，tga目标指针，特定颜色）
void line(int x0, int y0, int x1, int y1, TGAImage& image, TGAColor color) {
	//for (float t = 0.; t < 1.; t += .1) {
	//	// t为步长，每次增加t*(x1-x0)或者t*(y1-y0)
	//	int x = x0 * (1. - t) + x1 * t;
	//	int y = y0 * (1. - t) + y1 * t;
	//	image.set(x, y, color);
	//}

	// 斜率公式   k = (y1 - y2) / (x1 - x2)。
		bool steep = false; //标记当前斜率的绝对值是否大于1
		if (std::abs(x0 - x1) < std::abs(y0 - y1)) {
			//斜率绝对值>1了，此时将线段端点各自的x,y坐标对调。
			std::swap(x0, y0);
			std::swap(x1, y1);
			steep = true;
		}
		if (x0 > x1) {  //x0>x1时，对线段端点坐标进行对调
			std::swap(x0, x1);
			std::swap(y0, y1);
		}
		for (int x = x0; x <= x1; x++) {
			float t = (x - x0) / (float)(x1 - x0);
			int y = y0 * (1. - t) + y1 * t;
			if (steep) {
				//如果线段是斜率大于1的，那么线段上的点原本坐标应该是(y,x)
				image.set(y, x, color);
			}
			else {
				image.set(x, y, color);
			}
		}
}

// 计算质心坐标
// A B C P分别是三角形的3个顶点和三角形内的点P。我们可以求得S[0]和S[1]。
// 因为S[0]和S[1]是相互正交的，向量(u, v, 1)与它俩也是正交的，所以为了求得向量(u, v, 1)，
// 我们对S[0]和S[1]进行叉乘。
Vec3f barycentric(Vec3f A, Vec3f B, Vec3f C, Vec3f P) {
	Vec3f s[2];
	// 计算[AB,AC,PA]的x和y分量
	for (int i = 2; i--; ) {
		s[i][0] = C[i] - A[i];
		s[i][1] = B[i] - A[i];
		s[i][2] = A[i] - P[i];
	}
	// [u,v,1]和[AB,AC,PA] 对应的x和y向量都垂直，所有叉乘
	Vec3f u = cross(s[0], s[1]);
	// 三点共线时，会导致u[2]为0的数，表示点在三角形内部
	if (std::abs(u[2]) > 1e-2)
		// 若1-u-v,u,v全为大于0的数，表示点在三角形内部
		return Vec3f(1.f - (u.x + u.y) / u.z, u.y / u.z, u.x / u.z);
	return Vec3f(-1, 1, 1);
}

//绘制三角形(坐标数组，zbuffer指针，tga指针，颜色)
void triangle(Vec3f* pts, float* zbuffer, TGAImage& image, TGAColor color) {
	Vec2f bboxmin(std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
	Vec2f bboxmax(-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max());
	Vec2f clamp(image.width() - 1, image.height() - 1);
	// 确定三角形的边框
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 2; j++) {
			bboxmin[j] = std::max(0.f, std::min(bboxmin[j], pts[i][j]));
			bboxmax[j] = std::min(clamp[j], std::max(bboxmax[j], pts[i][j]));
		}
	}
	Vec3f P;
	// 遍历边框中的每个点
	for (P.x = bboxmin.x; P.x <= bboxmax.x; P.x++) {
		for (P.y = bboxmin.y; P.y <= bboxmax.y; P.y++) {
			// 计算质心
			if (P.x > 600 && P.y > 500)
			{
				P.x += 0.01;
			}
			// bc_screen就是质心坐标
			Vec3f bc_screen = barycentric(pts[0], pts[1], pts[2], P);
			// 质心坐标有一个负值，说明点在三角形外
			if (bc_screen.x < 0 || bc_screen.y < 0 || bc_screen.z < 0)continue;
			P.z = 0;
			// 计算zbuffer,并且每个顶点的z值呈上对应的质心坐标分量
			for (int i = 0; i < 3; i++)P.z += pts[i][2] * bc_screen[i];
			if (zbuffer[int(P.x + P.y * width)] < P.z) {
				zbuffer[int(P.x + P.y * width)] = P.z;
				image.set(P.x, P.y, color);
			}
		}
	}
}

//世界坐标转屏幕坐标
Vec3f world2screen(Vec3f v) {
	return Vec3f(int((v.x + 1.) * width / 2. + .5), int((v.y + 1.) * height / 2. + .5), v.z);
}


int main(int argc,char** argv) {
	// 命令行控制方式和代码构造model
	// 构造模型(obj文件模型)
	if (2 == argc) {
		model = new Model(argv[1]);
	}
	else {
		model = new Model("obj/african_head.obj");
	}

	// 创建zbuffer，大小为画布大小
	float* zbuffer = new float[width * height];
	// 初始化zbuffer，设定一个很小的值
	for (int i = width * height; i--; zbuffer[i] = -std::numeric_limits<float>::max());

	// 构造tga（宽，高，指定颜色空间）
	TGAImage image(width, height, TGAImage::RGB);
	//高洛德着色
	//指定光照方向
	Vec3f light_dir(0, 0, -1);
	// 模型的面作为循环控制变量
	for (int i = 0; i < model->nfaces(); i++) {
		// 创建face数组用于保存一个face的三个顶点坐标
		std::vector<int>face = model->face(i);
		Vec3f screen_coords[3];
		Vec3f world_coords[3]; // 新加入一个数组用于存放三个顶点的世界坐标
		for (int j = 0; j < 3; j++) {
			Vec3f v = model->vert(face[j]);
			//屏幕坐标    (-1,-1)映射为(0,0)  （1,1）映射为(width,height)
			screen_coords[j] = world2screen(model->vert(face[j]));
			//世界坐标    即模型坐标
			world_coords[j] = v;
		}
		//用世界坐标计算法向量
		Vec3f n = cross((world_coords[2] - world_coords[0]), (world_coords[1] - world_coords[0]));
		n.normalize();
		//强度=法向量*光照方向   即法向量和光照方向重合时，亮度最高
		float intensity = n * light_dir;
		//强度小于0，说明平面朝向为内  即背面裁剪
		if (intensity > 0) {
			TGAColor color = { intensity * 255, intensity * 255, intensity * 255, 255 };
			triangle(screen_coords, zbuffer, image, color);
		}
	}
	// tga默认原点在左上角，现需要指定为左下角，所有进行竖直翻转
	image.flip_vertically();
	image.write_tga_file("output.tga");
	delete model;
	return 0;
}