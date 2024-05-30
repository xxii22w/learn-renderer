#include "tgaimage.h"
#include "model.h"
#include "geometry.h"
#include <iostream>
using namespace std;

// 定义颜色
const TGAColor white = { 255,255,255,255 };
const TGAColor red = { 255, 0, 0, 255 };
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
	// 模型的面作为循环控制变量
	for (int i = 0; i < model->nfaces(); i++) {
		// 创建face数组用于保存一个face的三个顶点坐标
		std::vector<int>face = model->face(i);
		for (int j = 0; j < 3; j++) {
			// 顶点v0
			Vec3f v0 = model->vert(face[j]);
			// 顶点v1
			Vec3f v1 = model->vert(face[(j + 1) % 3]);
			// 根据顶点v0和v1画线
			// 先要进行模型坐标到屏幕坐标的转换
			// (-1,-1)对于(0,0) (1,1)对于(width,height)
			int x0 = (v0.x + 1.) * width / 2.;
			int y0 = (v0.y + 1.) * height / 2.;
			int x1 = (v1.x + 1.) * width / 2.;
			int y1 = (v1.y + 1.) * height / 2.;
			// 画线
			line(x0, y0, x1, y1, image, white);
		}
	}
	// tga默认原点在左上角，现需要指定为左下角，所有进行竖直翻转
	image.flip_vertically();
	image.write_tga_file("output.tga");
	delete model;
	return 0;
}