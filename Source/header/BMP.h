#ifndef BMP_h
#define BMP_h

#include <iostream> 
#include <fstream> //----> ifstream, ofstream, read, write
#include <cstring> //----> memcpy, strcmp. 

#define DIB_SIZE 40
#define HEADER_SIZE 14
#define COLOUR_PALETTES 1024

using namespace std;

#pragma pack(1)
struct BMPFileHeader
{
    int16_t Signature = 0x4D42; // Ký hiệu của file 0x4D42 là BM
    int32_t file_size = 0;      // Kích thước của file
    int32_t reserved = 0;       // Reserved
    int32_t data_offset = 0;    // Địa chỉ bắt đầu của dữ liệu điểm ảnh
};

struct BMPFileDIB
{
    uint32_t size = 0;  // Số byte của DIB
    int32_t width = 0;  // Chiều rộng của ảnh
    int32_t height = 0; // Chiều cao của ảnh

    uint16_t planes = 1;
    uint16_t bit_per_pixel = 0;   // Số bit mỗi pixel
    uint32_t compression = 0;     // Nếu 32bit thì compression = 3, 24bit/16bit/8bit thì compression = 1
    uint32_t image_size = 0;      // Kích thước ảnh
    uint32_t x_pixels_per_m = 0;  // x pixel mỗi m
    uint32_t y_pixels_per_m = 0;  // y pixel mỗi m
    uint32_t color_used = 0;      // màu sử dụng
    uint32_t important_color = 0; // màu quan trọng
};

// Dữ liệu mỗi điểm ảnh
struct Pixel
{
    uint8_t blue;  // màu xanh biển
    uint8_t green; // màu xanh lá
    uint8_t red;   // màu đỏ
    uint8_t alpha; // phần alpha dành riêng cho ảnh 32 bit

    uint8_t rgb; // sử dụng trong trường hợp ảnh 8bit
};

// Cấu trúc một file hình ảnh
struct Image
{
    BMPFileHeader header; // Phần header
    BMPFileDIB dib;       // Phần DIB
    char *data;           // Dữ liệu điểm ảnh
    char *unused;         // Phần dư
    char *colorPalette;   // Dữ liệu bảng màu
};

/*
    Hàm tự động lấy các ảnh test để demo chương
    trình convert 32/24bit sang ảnh 8bit.
*/
void AutoTest();

/*
    Hàm tự động lấy các ảnh test để demo chương
    trình zoom ảnh theo tỉ lệ S.
*/
void AutoTest(int);

/*
    Hàm để đọc dữ liệu từ file ảnh vào bộ nhớ ROM.
*/
void read(Image &img, const char *fileName);

/*
    Hàm để ghi dữ liệu từ bộ nhớ ROM vào file ảnh.
*/
void write(const Image img, const char *fileName);

/*
    Hàm để chuyển dữ liệu điểm ảnh sang mảng
    các điểm ảnh.
*/
Pixel **getPixel(const Image img);

/*
    Hàm để chuyển ảnh bình thường thành ảnh xám.
    Kết quả trả về là một bức ảnh mới.
*/
Image convert(const Image src);

/*
    Hàm tính trung bình giá trị màu của size số
    pixel của mảng pixel.
*/
Pixel avg(Pixel *pixel, int16_t size);

/*
    Hàm thu nhỏ ảnh theo một tỷ lệ S dương cho trước.
    Áp dụng đối với ảnh 32bit/24bit
*/
Image zoom(const Image src, int s);

/*
    Hàm thu nhỏ ảnh theo một tỷ lệ S dương cho trước.
    Áp dụng đối với ảnh 8bit
*/
Image zoom(const Image src, int s, int bpp);

/*
    Hàm giải phóng bộ nhớ của mảng 2 chiều pixel
    với chiều cao height
*/
void delPixel(Pixel **&pixel, int height);

/*
    Hàm giải phóng bộ nhớ của data của ảnh.
*/
void delImg(Image &img);

#endif