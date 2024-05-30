#pragma once
#include <cstdint>
#include <fstream>
#include <vector>


/* 
结构体内存对齐字节可以通过#pragma pack(n)的方式来指定
#pragma pack(push)：

英文单词push是“压入”的意思。编译器编译到此处时将保存对齐状态（保存的是push指令之前的对齐状态）。

#pragma pack(pop)：

英文单词pop是”弹出“的意思。编译器编译到此处时将恢复push指令前保存的对齐状态

当我们想要一个结构体按照4字节对齐时，可以使用#pragma   pack(4) ，
最后又想使用默认对齐方式时，可以使用#pragma pack() ；
*/
#pragma pack(push,1)
// 用来存储我们的文件头
struct TGAHeader {
	std::uint8_t idlength = 0;
	std::uint8_t colormaptype = 0;
	std::uint8_t datatypecode = 0;
	std::uint16_t colormaporigin = 0;
    std::uint16_t colormaplength = 0;
    std::uint8_t  colormapdepth = 0;
    std::uint16_t x_origin = 0;
    std::uint16_t y_origin = 0;
    std::uint16_t width = 0;
    std::uint16_t height = 0;
    //像素深度是指存储每个像素所用的位数，也用它来度量图像的分辨率
    std::uint8_t  bitsperpixel = 0;
    std::uint8_t  imagedescriptor = 0;
};

#pragma pack(pop)
// 用来存储文件信息
struct TGAColor {
    std::uint8_t bgra[4] = { 0,0,0,0 };
    std::uint8_t bytespp = 4;
    std::uint8_t& operator[](const int i) { return bgra[i]; }
};

struct TGAImage {
    enum Format { GRAYSCALE = 1, RGB = 3, RGBA = 4 };

    TGAImage() = default;
    TGAImage(const int w, const int h, const int bpp);
    bool read_tga_file(const std::string filename);
    bool write_tga_file(const std::string filename, const bool vflip = true, const bool rle = true)const;
    void flip_horizontally();// 水平翻转图形
    void flip_vertically();//实现翻转y坐标
    TGAColor get(const int x, const int y)const;
    void set(const int x, const int y, const TGAColor& c);
    int width()const;
    int height()const;
private:
    bool load_rle_data(std::ifstream& in);
    bool unload_rle_data(std::ofstream& out)const;

    int w = 0;
    int h = 0;
    /*
    *   bpp (bits per pixel)是指每个 像素 所占用的有效比特数 ( 忽略通道 )，bpp = 总有效比特数/总像素数目 = 总有效比特数/ (W*H). 
        bpp也用于压缩域的表示，即图片压缩后所占用的bit数/总像素数目。
    */
    std::uint8_t bpp = 0; // 像素景深
    std::vector<std::uint8_t> data = {};// data是用来控制整个图像的颜色值
                                        // 一般对于图像数据的读取，都使用 char* 类型
};