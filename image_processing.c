// Boroghina Gabriel
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bmp_header.h"
#define mem_block 5
#define header_size 54 // total headers' size

typedef struct
{
    short height, width;
    Pixel **pixel;
} Bitmap;

typedef struct
{
    fHeader fileHeader;
    iHeader infoHeader;
    char *imageName;
} Metadata; // headers and image filename

typedef struct
{
    short first, second;
} pair;

Bitmap newBitmap(short height, short width) /* create a Bitmap struct and
                                            allocate memory for pixel matrix*/
{
    short i;
    Bitmap bmp;
    bmp.height = height;
    bmp.width = width;

    bmp.pixel = malloc(height * sizeof(Pixel *));
    for (i = 0; i < height; i++)
        bmp.pixel[i] = malloc(width * sizeof(Pixel));

    return bmp;
}

char *appendName(char *begin, char *end) // create output filename
{
    // delete begin's ".bmp" and append end
    char *outFile = malloc((strlen(begin) + strlen(end) - 3) * sizeof(char));

    strcpy(outFile, begin);
    strcpy(outFile + strlen(begin) - 4, end);

    return outFile;
}

void free_mem(Bitmap *bmp) // free image memory
{
    short i;
    for (i = 0; i < bmp->height; i++)
        free(bmp->pixel[i]);
    free(bmp->pixel);
}

void initialize(Bitmap *bmp,
                Metadata *meta,
                int *threshold,
                char **compressedImage)
{
    short i;
    int paddingSize, fileNameLength;
    FILE *input;
    fpos_t pos; // position in stream

    input = fopen("input.txt", "r");

    fgetpos(input, &pos);
    fscanf(input, "%*[^\n]%n", &fileNameLength); // determine filename's size
    fsetpos(input, &pos);

    // name of input image
    meta->imageName = malloc((fileNameLength + 1) * sizeof(char));
    fscanf(input, "%[^\n]%i\n", meta->imageName, threshold);

    fgetpos(input, &pos);
    fscanf(input, "%*[^\n]%n", &fileNameLength); // determine filename's size
    fsetpos(input, &pos);

    // name of compressed image
    *compressedImage = malloc((fileNameLength + 1) * sizeof(char));
    fscanf(input, "%[^\n]", *compressedImage); // read to end of line or file

    fclose(input);
    input = fopen(meta->imageName, "rb");

    // read from image file
    fread(&meta->fileHeader, sizeof(fHeader), 1, input);
    fread(&meta->infoHeader, sizeof(iHeader), 1, input);

    bmp->height = meta->infoHeader.height;
    bmp->width = meta->infoHeader.width;

    paddingSize = 3 - ((bmp->width - 1) % 4); // padding size

    // skip offset
    fseek(input, meta->fileHeader.imageDataOffset - header_size, SEEK_CUR);

    // read pixel matrix
    bmp->pixel = malloc(bmp->height * sizeof(Pixel *));
    for (i = 0; i < bmp->height; i++)
    {
        bmp->pixel[i] = malloc(bmp->width * sizeof(Pixel));
        fread(bmp->pixel[i], sizeof(Pixel), bmp->width, input);
        fseek(input, paddingSize, SEEK_CUR); // skip padding
    }
    fclose(input);
}

void printImage(Bitmap *bmp, Metadata meta)
{
    short i;
    int paddingSize;
    FILE *out = fopen(meta.imageName, "wb");

    // offset
    paddingSize = meta.fileHeader.imageDataOffset - header_size;
    char *padding = calloc(paddingSize, sizeof(char));

    // print headers
    fwrite(&meta.fileHeader, sizeof(fHeader), 1, out);
    fwrite(&meta.infoHeader, sizeof(iHeader), 1, out);

    fwrite(padding, sizeof(char), paddingSize, out); // print offset
    free(padding);

    // padding
    paddingSize = 3 - ((bmp->width - 1) % 4);
    padding = calloc(paddingSize, sizeof(char));

    // print pixel matrix
    for (i = 0; i < bmp->height; i++)
    {
        fwrite(bmp->pixel[i], sizeof(Pixel), bmp->width, out);
        fwrite(padding, sizeof(char), paddingSize, out); // write padding
    }

    free(padding);
    fclose(out);
}

Bitmap blackWhite(Bitmap bmp)
{
    short i, j, new;
    Bitmap black_white;
    Pixel old;
    black_white = newBitmap(bmp.height, bmp.width);

    for (i = 0; i < black_white.height; i++)
        for (j = 0; j < black_white.width; j++)
        {
            old = bmp.pixel[i][j];
            new = (old.r + old.g + old.b) / 3; // create new pixel
            black_white.pixel[i][j] = (Pixel){new, new, new};
        }

    return black_white;
}

short correctVal(int n) // correct pixel's component value
{
    if (n < 0)
        return 0;
    if (n > 255)
        return 255;
    return n;
}

void edgeDetection(Bitmap *bmp, Metadata meta)
{
    short i, j, k, l, c;
    short r, g, b, filterVal;
    char fileEnd[7] = "_f?.bmp"; // filename's end pattern
    short F[3][9] = {{-1, -1, -1, -1, 8, -1, -1, -1, -1},
                     {0, 1, 0, 1, -4, 1, 0, 1, 0},
                     {-1, 0, 1, 0, 0, 0, 1, 0, -1}}; // filters
    Bitmap img;

    img = newBitmap(bmp->height, bmp->width);

    for (k = 0; k < 3; k++) // apply filters F[0], F[1] and F[2]
    {
        for (i = bmp->height - 1; i >= 0; i--)
            for (j = 0; j < bmp->width; j++)
            {
                r = g = b = 0;
                for (l = i - 1; l <= i + 1; l++)
                    for (c = j - 1; c <= j + 1; c++)
                        if (l >= 0 && l < bmp->height &&
                            c >= 0 && c < bmp->width)
                        /* the pixels outside the matrix are (0,0,0),
                            so they don't count */
                        {
                            // get filterVal from F[k] array
                            filterVal = F[k][3 * (l - i + 1) + (c - j + 1)];
                            r += (short)bmp->pixel[l][c].r * filterVal;
                            g += (short)bmp->pixel[l][c].g * filterVal;
                            b += (short)bmp->pixel[l][c].b * filterVal;
                        }
                // change value if < 0 or > 255
                r = correctVal(r);
                g = correctVal(g);
                b = correctVal(b);
                img.pixel[i][j] = (Pixel){b, g, r};
            }

        fileEnd[2] = k + 1 + '0'; // replace '?' in "_f?.bmp"
        char *oFile = appendName(meta.imageName, fileEnd);
        printImage(&img, (Metadata){meta.fileHeader, meta.infoHeader, oFile});
        free(oFile);
    }

    free_mem(&img);
}

int checkSimilarity(Pixel a, Pixel b, int threshold)
{
    return (abs(a.r - b.r) + abs(a.g - b.g) + abs(a.b - b.b) <= threshold);
}

void fill(short i,
          short j,
          Bitmap *bmp,
          Pixel pix,
          unsigned char **sw,
          int threshold)
{
    // iterative fill algorithm

    short k, I, J, inserted;
    int nr = 0, stackSize = mem_block;
    short dx[] = {1, 0, 0, -1}; // relative movements
    short dy[] = {0, -1, 1, 0};
    pair *stack; // dinamically allocated stack

    stack = malloc(mem_block * sizeof(pair));
    stack[0] = (pair){i, j}; // start pixel

    while (nr >= 0)
    {
        i = stack[nr].first; // stack's top
        j = stack[nr].second;

        bmp->pixel[i][j] = pix; // fill current pixel
        sw[i][j] = 1;
        inserted = 0;

        for (k = 0; k < 4; k++)
        {
            I = i + dx[k]; // new position
            J = j + dy[k];
            if (I >= 0 && J >= 0 && I < bmp->height && J < bmp->width &&
                !sw[I][J] &&
                checkSimilarity(pix, bmp->pixel[I][J], threshold))
            // found a similar neighbor
            {
                if (nr + 1 == stackSize) // stack is full
                {
                    stackSize += mem_block; // increase stack's size
                    stack = realloc(stack, stackSize * sizeof(pair));
                }
                stack[++nr] = (pair){I, J}; // push element into stack
                inserted = 1;               // found a next move
            }
        }
        if (!inserted)
            nr--; // remove first element from stack
    }
    free(stack);
}

int isBorder(Bitmap *bmp, short i, short j)
{
    short k;
    short dx[] = {1, 0, 0, -1}; // relative movements
    short dy[] = {0, -1, 1, 0};
    Pixel current, next;

    if (i == 0 || j == 0 || i == bmp->height - 1 || j == bmp->width - 1)
        return 1; // image border

    current = bmp->pixel[i][j];
    for (k = 0; k < 4; k++)
    {
        // search for different neighbors
        next = bmp->pixel[i + dx[k]][j + dy[k]];
        if (current.r != next.r || current.g != next.g || current.b != next.b)
            return 1; // zone border
    }
    return 0;
}

void compressImage(Bitmap *bmp, int threshold, Metadata meta)
{
    short i, j;
    unsigned char **sw;
    pixelPos line;

    // sw[i][j] retains if (i,j) was visited
    sw = malloc(bmp->height * sizeof(unsigned char *));
    for (i = 0; i < bmp->height; i++)
        sw[i] = calloc(bmp->width, sizeof(unsigned char));

    for (i = bmp->height - 1; i >= 0; i--)
        for (j = 0; j < bmp->width; j++)
            if (sw[i][j] == 0) // new zone
                fill(i, j, bmp, bmp->pixel[i][j], sw, threshold);

    // print image
    int paddingSize = meta.fileHeader.imageDataOffset - header_size;
    FILE *out = fopen("compressed.bin", "wb");
    char *padding = calloc(paddingSize, sizeof(char));

    fwrite(&meta.fileHeader, sizeof(fHeader), 1, out);
    fwrite(&meta.infoHeader, sizeof(iHeader), 1, out);
    fwrite(padding, sizeof(char), paddingSize, out); // offset

    for (i = bmp->height - 1; i >= 0; i--)
        for (j = 0; j < bmp->width; j++)
            if (isBorder(bmp, i, j))
            {
                // construct output line
                line.i = bmp->height - i;
                line.j = j + 1;

                line.r = bmp->pixel[i][j].r;
                line.g = bmp->pixel[i][j].g;
                line.b = bmp->pixel[i][j].b;

                fwrite(&line, sizeof(line), 1, out);
            }

    // free memory
    for (i = 0; i < bmp->height; i++)
        free(sw[i]);
    free(sw);
    free(padding);
    fclose(out);
}

void decompressImage(char *name)
{
    short i, j, line;
    int curr_pos, end_pos;
    Bitmap compressed; // compressed image
    Metadata meta;
    pixelPos pix;
    unsigned char **sw; // tells if a pixel is on a zone border

    FILE *input = fopen(name, "rb");
    fread(&meta.fileHeader, sizeof(fHeader), 1, input);
    fread(&meta.infoHeader, sizeof(iHeader), 1, input);

    compressed = newBitmap(meta.infoHeader.height, meta.infoHeader.width);

    sw = malloc(compressed.height * sizeof(unsigned char *));
    for (i = 0; i < compressed.height; i++)
        sw[i] = calloc(compressed.width, sizeof(unsigned char));

    // skip offset
    fseek(input, meta.fileHeader.imageDataOffset - header_size, SEEK_CUR);

    curr_pos = ftell(input);
    fseek(input, 0, SEEK_END);
    end_pos = ftell(input); // get file's end position
    fseek(input, curr_pos - end_pos, SEEK_CUR);

    while (curr_pos != end_pos) // read file to the end
    {
        fread(&pix, sizeof(pixelPos), 1, input);

        line = compressed.height - pix.i;
        compressed.pixel[line][pix.j - 1] = (Pixel){pix.b, pix.g, pix.r};
        sw[line][pix.j - 1] = 1; // colored pixel

        curr_pos = ftell(input);
    }
    fclose(input);

    for (i = 0; i < compressed.height; i++)
        for (j = 0; j < compressed.width; j++)
            if (!sw[i][j]) // it's not on a zone border, so it's inside a zone
                           // same zone as the left neighbor
                compressed.pixel[i][j] = compressed.pixel[i][j - 1];

    meta.imageName = "decompressed.bmp";
    printImage(&compressed, meta);

    //free memory
    free_mem(&compressed);
    for (i = 0; i < compressed.height; i++)
        free(sw[i]);
    free(sw);
}

int main()
{
    Bitmap bmp, black_white;
    Metadata meta;
    int threshold;
    char *compressedImage = NULL;

    meta.imageName = NULL;
    initialize(&bmp, &meta, &threshold, &compressedImage);

    // TASK 1
    black_white = blackWhite(bmp); // create the black-white image
    char *outFile = appendName(meta.imageName, "_black_white.bmp");
    // print black-white image
    printImage(&black_white,
               (Metadata){meta.fileHeader, meta.infoHeader, outFile});
    free(outFile);

    // TASK 2
    edgeDetection(&black_white, meta);
    // TASK 3
    compressImage(&bmp, threshold, meta);
    // TASK 4
    decompressImage(compressedImage);

    // free memory
    free_mem(&bmp);
    free_mem(&black_white);
    free(meta.imageName);
    free(compressedImage);
    return 0;
}
