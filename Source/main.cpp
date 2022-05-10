#include "header/BMP.h"

int main(int argc, char *argv[])
{
    Image src, dst;

    read(src, argv[2]);

    if (strcmp(argv[1], "-conv") == 0)
    {
        dst = convert(src);
        write(dst, argv[3]);
    }
    else if (strcmp(argv[1], "-zoom") == 0)
    {
        dst = zoom(src, atoi(argv[4]));
        write(dst, argv[3]);
    }
    else
        throw runtime_error("Khong the nhan dang duoc yeu cau!");

    cout << "All done!" << endl;

    delImg(src);
    delImg(dst);

    return 0;
}
