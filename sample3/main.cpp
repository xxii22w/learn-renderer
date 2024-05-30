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

// 绘制三角形（坐标1，坐标2，坐标3，tga指针，颜色）
void triangle(Vec2i t0, Vec2i t1, Vec2i t2, TGAImage& image, TGAColor color) {
	// 三角形面积为0
	if (t0.y == t1.y && t0.y == t2.y)return;
	// 根据y的大小对坐标进行排序
	if (t0.y > t1.y)std::swap(t0, t1);
	if (t0.y > t2.y)std::swap(t0, t2);
	if (t1.y > t2.y)std::swap(t1, t2);
	int total_height = t2.y - t0.y;
	// 以高度差作为循环控制变量，此时不需要考虑斜率，因为着色后每行都会被填充
	for (int i = 0; i < total_height; i++) {
		// 根据t1将三角形分割为上下两部分
		bool second_half = i > t1.y - t0.y || t1.y == t0.y;
		int segment_height = second_half ? t2.y - t1.y : t1.y - t0.y;
		float alpha = (float)i / total_height;
		float beta = (float)(i - (second_half ? t1.y - t0.y : 0))/ segment_height;
		// 计算A,B两点的坐标
		Vec2i A = t0 + (t2 - t0) * alpha;
		Vec2i B = second_half ? t1 + (t2 - t1) * beta : t0 + (t1 - t0) * beta;
		if (A.x > B.x)std::swap(A,B);
		// 根据A,B和当前高度对tga着色
		for (int j = A.x; j <= B.x; j++) {
			image.set(j, t0.y + i, color);
		}
	}
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
	// 构造tga（宽，高，指定颜色空间）
	TGAImage image(width, height, TGAImage::RGB);
	//高洛德着色
	//指定光照方向
	Vec3f light_dir(0, 0, -1);
	// 模型的面作为循环控制变量
	for (int i = 0; i < model->nfaces(); i++) {
		// 创建face数组用于保存一个face的三个顶点坐标
		std::vector<int>face = model->face(i);
		Vec2i screen_coords[3];
		Vec3f world_coords[3]; // 新加入一个数组用于存放三个顶点的世界坐标
		for (int j = 0; j < 3; j++) {
			Vec3f v = model->vert(face[j]);
			//屏幕坐标    (-1,-1)映射为(0,0)  （1,1）映射为(width,height)
			screen_coords[j] = Vec2i((v.x + 1.) * width / 2., (v.y + 1.) * height / 2.);
			//世界坐标    即模型坐标
			world_coords[j] = v;
		}
		//用世界坐标计算法向量
		Vec3f n = (world_coords[2] - world_coords[0]) ^ (world_coords[1] - world_coords[0]);
		n.normalize();
		//强度=法向量*光照方向   即法向量和光照方向重合时，亮度最高
		float intensity = n * light_dir;
		//强度小于0，说明平面朝向为内  即背面裁剪
		if (intensity > 0) {
			TGAColor color = { intensity * 255, intensity * 255, intensity * 255, 255 };
			triangle(screen_coords[0], screen_coords[1], screen_coords[2], image, color);
		}
	}
	// tga默认原点在左上角，现需要指定为左下角，所有进行竖直翻转
	image.flip_vertically();
	image.write_tga_file("output.tga");
	delete model;
	return 0;
}