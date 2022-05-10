#include "BMP.h"

void AutoTest(int s = 1)
{
    ifstream iname("Input/list.txt");
    ifstream oname("Output/list.txt");

    char nameIn[1000], nameOut[1000];
    Image in, out;

    while (!iname.eof())
    {
        iname >> nameIn;
        oname >> nameOut;

        cout << "Input: " << nameIn << endl;
        cout << "Output: " << nameOut << endl
             << endl;

        read(in, nameIn);

        out = zoom(in, s, in.dib.bit_per_pixel);
        write(out, nameOut);
    }

    cout << "All done!" << endl;
}

void AutoTest()
{
    ifstream iname("Input/list.txt");
    ifstream oname("Output/list.txt");

    char nameIn[1000], nameOut[1000];
    Image in, out;

    while (!iname.eof())
    {
        iname >> nameIn;
        oname >> nameOut;

        cout << "Input: " << nameIn << endl;
        cout << "Output: " << nameOut << endl
             << endl;

        read(in, nameIn);

        out = convert(in);
        write(out, nameOut);
    }

    cout << "All done!" << endl;
}

void read(Image &img, const char *fileName)
{
    // Mở file để đọc
    ifstream ifs(fileName, ios::binary);

    // Kiểm tra file có đang mở hay không
    if (ifs.is_open())
    {
        // Đọc header của file
        ifs.read((char *)&img.header, HEADER_SIZE);

        // Đọc DIB của file
        ifs.read((char *)&img.dib, DIB_SIZE);

        // Nếu DIB của file > DIB_SIZE thì đọc và lưu vào
        // biến unused
        if (img.dib.size > DIB_SIZE)
        {
            ifs.seekg(HEADER_SIZE + DIB_SIZE, ios::beg);

            uint32_t size = img.dib.size - DIB_SIZE;
            // Cấp phát vùng nhớ cho unused
            img.unused = new char[size];
            ifs.read(img.unused, size);
        }

        // Cấp phát vùng nhớ cho data
        img.data = new char[img.dib.image_size];

        // Đặt con trỏ file đến phần data_offset
        ifs.seekg(img.header.data_offset, ios::beg);

        // Tiến hành đọc dữ liệu điểm ảnh
        ifs.read(img.data, img.dib.image_size);

        ifs.close();
    }
    else
    {
        throw runtime_error("Khong the mo file!");
    }
}

void write(Image img, const char *fileName)
{
    // Mở file để ghi
    ofstream ofs(fileName, ios::binary | ios::ate);

    // Kiểm tra file có đang mở hay không
    if (ofs.is_open())
    {
        // Ghi header vào file
        ofs.write((char *)&img.header, HEADER_SIZE);

        // Ghi DIB vào file
        ofs.write((char *)&img.dib, DIB_SIZE);

        // Nếu là ảnh 8bit thì ghi bảng màu
        if (img.dib.bit_per_pixel == 8)
            ofs.write(img.colorPalette, COLOUR_PALETTES);

        // Nếu phần DIB > DIB_SIZE thì ghi phần thừa
        // của DIB vào file
        if (img.dib.size > DIB_SIZE)
        {
            ofs.seekp(HEADER_SIZE + DIB_SIZE, ios::beg);
            ofs.write(img.unused, img.dib.size - DIB_SIZE);
        }

        // Ghi dữ liệu điểm ảnh
        ofs.seekp(img.header.data_offset, ios::beg);
        ofs.write(img.data, img.dib.image_size);
        ofs.close();
    }
    else
    {
        throw runtime_error("Khong the mo file!");
    }
}

Pixel **getPixel(Image img)
{
    int16_t height = img.dib.height;
    int16_t width = img.dib.width;
    int16_t padding = (4 - (width * (img.dib.bit_per_pixel / 8) % 4)) % 4;

    // Cấp phát vùng nhớ cho mảng 2 chiều các điểm ảnh
    Pixel **pixel = new Pixel *[height];
    for (int i = 0; i < height; i++)
        pixel[i] = new Pixel[width];

    if (img.dib.bit_per_pixel != 8)
    {
        // Duyệt từng điểm ảnh
        for (int x = 0; x < height; x++)
        {
            for (int y = 0; y < width; y++)
            {
                if (img.dib.bit_per_pixel == 32)
                    pixel[x][y].alpha = *(img.data++);
                else
                    pixel[x][y].alpha = 0;

                pixel[x][y].blue = *(img.data++);
                pixel[x][y].green = *(img.data++);
                pixel[x][y].red = *(img.data++);
            }

            // Nếu có padding byte thì bỏ qua padding
            img.data += padding;
        }
    }
    else
    {
        // Duyệt từng điểm ảnh
        for (int x = 0; x < height; x++)
        {
            for (int y = 0; y < width; y++)
            {
                pixel[x][y].rgb = *(img.data++);
            }

            // Nếu có padding byte thì bỏ qua padding
            img.data += padding;
        }
    }

    return pixel;
}

Image convert(const Image src)
{
    /*--------------- Khởi tạo thông tin ảnh đích --------------*/
    Image newImg;

    // Header
    newImg.header.Signature = 0x4d42;
    newImg.header.file_size = COLOUR_PALETTES + DIB_SIZE + HEADER_SIZE;
    newImg.header.data_offset = COLOUR_PALETTES + DIB_SIZE + HEADER_SIZE;
    newImg.header.reserved = 0;

    // DIB
    newImg.dib = src.dib;
    newImg.dib.bit_per_pixel = 8;
    newImg.dib.compression = 0;
    newImg.dib.size = DIB_SIZE;
    newImg.dib.color_used = 256;

    // image size
    int16_t padding = (4 - (newImg.dib.width % 4)) % 4;
    newImg.dib.image_size = newImg.dib.width * newImg.dib.height * 1 + padding * newImg.dib.height;
    newImg.data = new char[newImg.dib.image_size];

    // update file size
    newImg.header.file_size += newImg.dib.image_size;

    // Phần thừa của DIB
    if (newImg.dib.size > DIB_SIZE)
    {
        uint32_t size = newImg.dib.size - DIB_SIZE;
        // Cấp phát vùng nhớ cho unused
        newImg.unused = new char[size];
        memcpy(newImg.unused, src.unused, size);
    }

    // Lấy thông tin bảng màu
    newImg.colorPalette = new char[COLOUR_PALETTES];

    ifstream ifs("ColourPalettes.bin", ios::binary);
    if (ifs.is_open())
    {
        ifs.read(newImg.colorPalette, COLOUR_PALETTES);
    }
    else
        throw runtime_error("Khong the mo file!");
    ifs.close();
    //----- Khởi tạo xong

    /*--------------- Khởi tạo và cấp phát các biến mảng ------------*/
    // Lấy data từ ảnh gốc và chuyển vào ma trận pixel
    Pixel **pixel;
    pixel = getPixel(src);
    // Khai báo các biến trung gian để thuận tiện
    // Cho việc tính toán
    char *temp = newImg.data;
    int16_t value = 0;
    //----- Kết thúc khởi tạo

    /*--------------------- Xử lý data gốc --------------------------*/
    // Tính toán lại giá trị các điểm ảnh
    for (int x = 0; x < src.dib.height; x++)
    {
        for (int y = 0; y < src.dib.width; y++)
        {
			// R + G + B / 3
            value = (pixel[x][y].blue + pixel[x][y].green + pixel[x][y].red) / 3;
            *(temp++) = value;
        }

        // Thêm padding byte nếu có
        for (int i = 0; i < padding; i++)
            *(temp++) = 0;
    }
    //-----Xử lý xong

    // Giải phóng vùng nhớ của pixel
    delPixel(pixel, src.dib.height);

    return newImg;
}

Pixel avg(Pixel *pixel, int16_t count, int16_t bpp)
{
    // Khởi tạo pixel kết quả sau khi tính trung bình
    Pixel ans = {0};
    int32_t sumA = 0, sumB = 0, sumG = 0, sumR = 0, sumRGB = 0;

    if (bpp != 8)
    {
        // Tính tổng các giá trị màu
        for (int x = 0; x < count; x++)
        {
            sumA += pixel[x].alpha;
            sumB += pixel[x].blue;
            sumG += pixel[x].green;
            sumR += pixel[x].red;
        }

        // Tính trung bình giá trị màu
        ans.alpha = sumA / count;
        ans.blue = sumB / count;
        ans.green = sumG / count;
        ans.red = sumR / count;
    }
    else
    {
        for (int x = 0; x < count; x++)
        {
            sumRGB += pixel[x].rgb;
        }

        ans.rgb = sumRGB / count;
    }
    // Kết quả trả về là một điểm ảnh
    return ans;
}

Image zoom(const Image src, int s)
{
    /*--------------- Khởi tạo thông tin ảnh đích --------------*/
    Image newImg;

    // Chép phần header và DIB của source Image
    newImg.header = src.header;
    newImg.dib = src.dib;

    // Tăng kích thước ảnh gốc sao cho vừa chia hết cho s
    while (newImg.dib.width % s != 0)
        newImg.dib.width++;
    while (newImg.dib.height % s != 0)
        newImg.dib.height++;

    // Chỉnh sửa thông số của DIB
    newImg.dib.height /= s;
    newImg.dib.width /= s;

    newImg.dib.x_pixels_per_m = src.dib.x_pixels_per_m / s;
    newImg.dib.y_pixels_per_m = src.dib.y_pixels_per_m / s;

    // Tính toán lại padding và Image size
    int16_t padding = (4 - (newImg.dib.width * (newImg.dib.bit_per_pixel / 8)) % 4) % 4;
    newImg.dib.image_size = newImg.dib.width * newImg.dib.height * (newImg.dib.bit_per_pixel / 8) + padding * newImg.dib.height;

    // Tính toán lại file size
    newImg.header.file_size = newImg.dib.image_size + 54;

    // Nếu có phần unused thì chép vào
    if (src.dib.size > DIB_SIZE)
    {
        // Kích thước phần unused
        int16_t size = src.header.data_offset - HEADER_SIZE - DIB_SIZE;

        // Cấp phát bộ nhớ cho unused và chép thông tin
        // của unused gốc.
        newImg.unused = new char[size];
        memcpy(newImg.unused, src.unused, size);

        // Tính toán lại file size
        newImg.header.file_size += size;
    }

    // Cấp phát bộ nhớ cho data sau khi thu nhỏ ảnh
    newImg.data = new char[newImg.dib.image_size];
    //----- Khởi tạo xong

    /*--------------- Khởi tạo và cấp phát các biến mảng ------------*/
    // Lấy data từ ảnh gốc và chuyển vào ma trận pixel
    Pixel **srcPix;
    srcPix = getPixel(src);

    // Khởi tạo và cấp phát bộ nhớ cho mảng pixel 2 chiều
    Pixel **dstPix;
    dstPix = new Pixel *[newImg.dib.height];
    for (int i = 0; i < newImg.dib.height; i++)
        dstPix[i] = new Pixel[newImg.dib.width];

    // Khởi tạo và cấp phát biến temp
    Pixel *temp;
    temp = new Pixel[s * s];
    //---- Kết thúc khởi tạo

    /*--------------------- Xử lý data gốc --------------------------*/
    // Duyệt trên ma trận gốc
    for (int x = 0; x < src.dib.height; x += s)
    {
        for (int y = 0; y < src.dib.width; y += s)
        {
            int32_t count = 0;
            // Lấy giá trị các ma trận con SxS
            for (int i = 0; i < s; i++)
            {
                for (int j = 0; j < s; j++)
                {
                    // Nếu index của pixel không bị out of range
                    if ((x + i) < src.dib.height && (y + j) < src.dib.width)
                    {
                        temp[count] = srcPix[x + i][y + j];
                        count++;
                    }
                }
            }

            // Tính điểm ảnh trung bình
            dstPix[x / s][y / s] = avg(temp, count, newImg.dib.bit_per_pixel);
        }
    }
    //-----Xử lý xong

    /*---------- Chuyển dữ liệu từ mảng pixel sang mảng 1 chiều ----------*/
    char *pData = newImg.data;
    for (int x = 0; x < newImg.dib.height; x++)
    {
        for (int y = 0; y < newImg.dib.width; y++)
        {
            if (newImg.dib.bit_per_pixel == 32)
                *(pData++) = dstPix[x][y].alpha;

            *(pData++) = dstPix[x][y].blue;
            *(pData++) = dstPix[x][y].green;
            *(pData++) = dstPix[x][y].red;
        }

        // Thêm padding byte vào
        for (int i = 0; i < padding; i++)
            *(pData++) = 0;
    }
    //----- Chuyển xong

    // Giải phóng vùng nhớ của pixel
    delPixel(srcPix, src.dib.height);
    delPixel(dstPix, newImg.dib.height);
    delete[] temp;
    return newImg;
}

Image zoom(const Image src, int s, int bpp)
{
    // Nếu bit per pixel là 24 hoặc 3
    if (bpp != 8)
    {
        Image newImg;
        newImg = zoom(src, s);
        return newImg;
    }
    /*--------------- Khởi tạo thông tin ảnh đích --------------*/
    Image newImg;

    // HEADER
    newImg.header = src.header;
    newImg.header.data_offset = COLOUR_PALETTES + DIB_SIZE + HEADER_SIZE;

    // DIB
    newImg.dib = src.dib;

    // Tăng kích thước ảnh gốc sao cho vừa chia hết cho s
    while (newImg.dib.width % s != 0)
        newImg.dib.width++;
    while (newImg.dib.height % s != 0)
        newImg.dib.height++;

    // Chỉnh sửa thông số của DIB
    newImg.dib.height /= s;
    newImg.dib.width /= s;

    newImg.dib.x_pixels_per_m = src.dib.x_pixels_per_m / s;
    newImg.dib.y_pixels_per_m = src.dib.y_pixels_per_m / s;

    // Tính toán lại padding và Image size
    int16_t padding = (4 - (newImg.dib.width % 4)) % 4;
    newImg.dib.image_size = newImg.dib.width * newImg.dib.height + padding * newImg.dib.height;

    // Tính toán lại
    newImg.header.file_size = newImg.dib.image_size + DIB_SIZE + HEADER_SIZE + COLOUR_PALETTES;

    // Cấp phát bộ nhớ cho data sau khi thu nhỏ ảnh
    newImg.data = new char[newImg.dib.image_size];

    // Lấy dữ liệu bảng màu từ file
    newImg.colorPalette = new char[COLOUR_PALETTES];

    ifstream ifs("ColourPalettes.bin", ios::binary);
    if (ifs.is_open())
    {
        ifs.read(newImg.colorPalette, COLOUR_PALETTES);
    }
    else
        throw runtime_error("Khong the mo file!");

    ifs.close();
    //----- Khởi tạo xong

    /*--------------- Khởi tạo và cấp phát các biến mảng ------------*/
    // Lấy data từ ảnh gốc và chuyển vào ma trận pixel
    Pixel **srcPix;
    srcPix = getPixel(src);

    // Khởi tạo và cấp phát bộ nhớ cho mảng pixel 2 chiều
    Pixel **dstPix;
    dstPix = new Pixel *[newImg.dib.height];
    for (int i = 0; i < newImg.dib.height; i++)
        dstPix[i] = new Pixel[newImg.dib.width];

    // Khởi tạo và cấp phát biến temp
    Pixel *temp;
    temp = new Pixel[s * s];
    //----- Kết thúc khởi tạo

    /*--------------------- Xử lý data gốc --------------------------*/
    // Duyệt trên ma trận gốc
    for (int x = 0; x < src.dib.height; x += s)
    {
        for (int y = 0; y < src.dib.width; y += s)
        {
            int32_t count = 0;
            // Lấy giá trị các ma trận con SxS
            for (int i = 0; i < s; i++)
            {
                for (int j = 0; j < s; j++)
                {
                    // Nếu index của pixel không bị out of range
                    if ((x + i) < src.dib.height && (y + j) < src.dib.width)
                    {
                        temp[count] = srcPix[x + i][y + j];
                        count++;
                    }
                }
            }

            // Tính điểm ảnh trung bình
            dstPix[x / s][y / s].rgb = avg(temp, count, newImg.dib.bit_per_pixel).rgb;
        }
    }
    //-----Xử lý xong

    /*---------- Chuyển dữ liệu từ mảng pixel sang mảng 1 chiều ----------*/
    char *pData = newImg.data;
    for (int x = 0; x < newImg.dib.height; x++)
    {
        for (int y = 0; y < newImg.dib.width; y++)
        {
            *(pData++) = dstPix[x][y].rgb;
        }

        // Thêm padding byte vào
        for (int i = 0; i < padding; i++)
            *(pData++) = 0;
    }
    //----- Chuyển xong

    // Giải phóng vùng nhớ của pixel
    delPixel(srcPix, src.dib.height);
    delPixel(dstPix, newImg.dib.height);
    delete[] temp;
    return newImg;
}

void delPixel(Pixel **&pixel, int height)
{
    // Kiểm tra rỗng
    if (pixel == NULL)
        throw runtime_error("NULL pointer detected!");

    for (int i = 0; i < height; i++)
        delete[] pixel[i];
    delete[] pixel;
}

void delImg(Image &img)
{
    // Kiểm tra rỗng
    if (img.data == NULL || img.unused == NULL)
        throw runtime_error("NULL pointer detected!");

    delete[] img.data;
    delete[] img.unused;
}