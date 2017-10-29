/* Tells the compiler not to add padding for these structs. This may
   be useful when reading/writing to binary files.
   http://stackoverflow.com/questions/3318410/pragma-pack-effect
*/
#pragma pack(1)

typedef struct
{
    unsigned char b, g, r;
} Pixel;

typedef struct
{
    short i, j;
    unsigned char r, g, b;
} pixelPos; // compressed image pixel data

typedef struct bmp_fileheader
{
    unsigned char fileMarker1;    /* 'B' */
    unsigned char fileMarker2;    /* 'M' */
    unsigned int bfSize;          /* File's size */
    unsigned short unused1;       /* Aplication specific */
    unsigned short unused2;       /* Aplication specific */
    unsigned int imageDataOffset; /* Offset to the start of image data */
} fHeader;

typedef struct bmp_infoheader
{
    unsigned int biSize; /* Size of the info header - 40 bytes */
    signed int width;    /* Width of the image */
    signed int height;   /* Height of the image */
    unsigned short planes;
    unsigned short bitPix;      /* Number of bits per pixel */
    unsigned int biCompression; /* Type of compression */
    unsigned int biSizeImage;   /* Size of the image data */
    int biXPelsPerMeter;
    int biYPelsPerMeter;
    unsigned int biClrUsed;
    unsigned int biClrImportant;
} iHeader;

#pragma pack()
