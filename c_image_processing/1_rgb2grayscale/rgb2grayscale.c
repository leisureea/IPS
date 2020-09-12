#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define GRAY_LEVELS          255

struct bmpfileheader{
    unsigned short  filetype;
    unsigned long   filesize;
    short  reserved1;
    short  reserved2;
    unsigned long   bitmapoffset;
};

struct bitmapheader{
    unsigned long   size;
    long   width;
    long   height;
    unsigned short  planes;
    unsigned short  bitsperpixel;
    unsigned long   compression;
    unsigned long   sizeofbitmap;
    unsigned long   horzres;
    unsigned long   vertres;
    unsigned long   colorsused;
    unsigned long   colorsimp;
};

struct ctstruct{
    unsigned char blue;
    unsigned char green;
    unsigned char red;
};

union colortable_union{
    struct colortable
    {
        unsigned char blue;
        unsigned char green;
        unsigned char red;
        unsigned char reserved;
    }colortable;
    unsigned long l_num;
};

union short_char_union{
    short s_num;
    char  s_alpha[2];
};

union int_char_union{
    int  i_num;
    char i_alpha[2];
};

union long_char_union{
    long  l_num;
    char  l_alpha[4];
};

union float_char_union{
    float f_num;
    char  f_alpha[4];
};

union ushort_char_union{
    short s_num;
    char  s_alpha[2];
};

union uint_char_union{
    int  i_num;
    char i_alpha[2];
};

union ulong_char_union{
    long  l_num;
    char  l_alpha[4];
};

union rgb565_union{
    struct rgb565_struct
    {
        unsigned red : 5;
        unsigned green : 6;
        unsigned blue : 5;
    }rgb565_struct;
    // short pixel;
    char exchange[2];
};

union rgb555_union{
    struct rgb555_struct
    {
        unsigned red : 5;
        unsigned green : 5;
        unsigned blue : 5;
        unsigned reserved : 1;
    }rgb555_struct;
    // short pixel;
    char exchange[2];
};

union bpp24_union{
    struct bpp24_struct
    {
        unsigned red : 8;
        unsigned green : 8;
        unsigned blue : 8;
    }bpp24_struct;
    char exchange[3];
};

union bpp32_union{
    struct bpp32_struct
    {
        unsigned red : 8;
        unsigned green : 8;
        unsigned blue : 8;
        unsigned reserved : 8;
    }bpp32_struct;
    char exchange[4];
};

int get_image_size(const char* file_name, long* rows, long* cols);
void extract_long_from_buffer(char buffer[], int lsb, int start, long* number);
void extract_ulong_from_buffer(char buffer[], int lsb, int start,unsigned long* number);
void extract_short_from_buffer(char buffer[], int lsb, int start, short* number);
void extract_ushort_from_buffer(char buffer[], int lsb, int start, unsigned short* number);
void insert_short_into_buffer(char buffer[], int start, short number);
void insert_ushort_into_buffer(char buffer[], int start, unsigned short number);
void insert_long_into_buffer(char buffer[], int start, long number);
void insert_ulong_into_buffer(char buffer[], int start, unsigned long number);
short **allocate_image_array(long length, long width);
int free_image_array(short **the_array, long length);
void create_allocate_bmp_file(const char* file_name, struct bmpfileheader* file_header, struct bitmapheader* bmheader);
void read_bmp_file_header(const char* file_name, struct bmpfileheader* file_header);
void read_bm_header(const char* file_name, struct bitmapheader* bmheader);
int calculate_pad(long width);
void write_bmp_image(const char* file_name, short **array);
int does_not_exist(const char* file_name);
union colortable_union* read_allocate_colortable(const char* file_name, struct bitmapheader* bmheader);
void free_colortable(union colortable_union* colortable);
void bpp82grayscale(long height, long width, FILE* fp, short** the_image, int pad, 
                    const char* file_name, struct bitmapheader* bmheader);
void rgb5552grayscale(long height, long width, FILE* fp, short** the_image, int pad);
void rgb5652grayscale(long height, long width, FILE* fp, short** the_image, int pad);
void bpp242grayscale(long height, long width, FILE* fp, short** the_image, int pad);
void bpp322grayscale(long height, long width, FILE* fp, short** the_image, int pad);
void rgb2grayscale_array(const char* file_name, short** the_image);
void flip_image_array(short** the_image, long rows, long cols);
// void half_tone(short** in_image, short** out_image, short threshold, 
//                short one, short zero, long rows, long cols);

// .\rgb5652grayscale.exe a.bmp test.bmp

int main(int argc, char const *argv[])
{
    char*   cc;
    long    l, w;
    short** grayscale_array;
    struct  bmpfileheader      bmp_file_header;
    struct  bitmapheader       bmheader;

    if(argc < 3){
        printf("\nusage: rgb5652bmp rgb565-file-name grayscale-file-name\n");
        exit(-1);
    }
    if(does_not_exist(argv[1])){
        printf("\nERROR input file %s does not exist", argv[1]);
        exit(0);
    }
    cc = strstr(argv[1], ".bmp");
    if(cc == NULL){  
        printf("\nERROR %s must be a bmp file", argv[1]);
        exit(0);
    }
    cc = strstr(argv[2], ".bmp");
    if(cc == NULL){  /* create a bmp */
        printf("\nERROR %s must be a bmp file name", argv[2]);
        exit(0);
    }

    get_image_size(argv[1], &l, &w);
    grayscale_array = allocate_image_array(l, w);
    bmheader.height = l;
    bmheader.width  = w;

    create_allocate_bmp_file(argv[2], &bmp_file_header, &bmheader);   // create grayscale image file

    rgb2grayscale_array(argv[1], grayscale_array);
    write_bmp_image(argv[2], grayscale_array);

    free_image_array(grayscale_array, l);

    return 0;
}

void rgb2grayscale_array(const char* file_name, short** the_image)
{
    FILE*  fp;
    int    negative = 0,
           pad      = 0;
    long   height   = 0,
           width    = 0;
    struct bmpfileheader file_header;
    struct bitmapheader  bmheader;
    //char temp;
    // struct ctstruct rgb[GRAY_LEVELS+1];

    read_bmp_file_header(file_name, &file_header);
    read_bm_header(file_name, &bmheader);

    fp = fopen(file_name, "rb");
    fseek(fp, file_header.bitmapoffset, SEEK_SET);

    width = bmheader.width;
    if(bmheader.height < 0){
        height   = bmheader.height * (-1);
        negative = 1;
    }
    else
        height = bmheader.height;

    pad = calculate_pad(width);

    if (bmheader.bitsperpixel == 16 && bmheader.compression == 0x03)
        rgb5652grayscale(height, width, fp, the_image, pad);
    else if (bmheader.bitsperpixel == 16 && bmheader.compression == 0x00)
        rgb5552grayscale(height, width, fp, the_image, pad);
    else if (bmheader.bitsperpixel == 24 && bmheader.compression == 0x00)
        bpp242grayscale(height, width, fp, the_image, pad);
    else if (bmheader.bitsperpixel == 32 && bmheader.compression == 0x00)
        bpp322grayscale(height, width, fp, the_image, pad);
    else if (bmheader.bitsperpixel == 8)
        bpp82grayscale(height, width, fp, the_image, pad, file_name, &bmheader);
    else
    {
        printf("Unknow image format\n");
        exit(0);
    }

    if(negative == 0)
        flip_image_array(the_image, height, width);
}

void bpp82grayscale(long height, long width, FILE* fp, short** the_image, int pad, 
                    const char* file_name, struct bitmapheader* bmheader)
{
    union colortable_union* colortable = NULL;
    unsigned char pixel_index;

    printf("bpp8\n");
    colortable = read_allocate_colortable(file_name, bmheader);
    for(int i=0; i<height; i++){
        for(int j=0; j<width; j++){
            fread(&pixel_index, 1, 1, fp);
            // printf("%u\n", pixel_index);
            the_image[i][j] = ((colortable[pixel_index].colortable.red) + 
                               (colortable[pixel_index].colortable.blue) + 
                               (colortable[pixel_index].colortable.green)) / 3;
        }  /* ends loop over j */
        if(pad != 0){
            fseek(fp, pad, SEEK_CUR);
        }  /* ends if pad 1= 0 */
    }  /* ends loop over i */
    free_colortable(colortable);
}

union colortable_union* read_allocate_colortable(const char* file_name, struct bitmapheader* bmheader)
{
    union colortable_union* colortable = NULL;
    char  buffer[10];
    unsigned long  ull;
    FILE  *fp;
    long  table_size = (long)pow(2, bmheader->bitsperpixel);

    fp = fopen(file_name, "rb");
    colortable = malloc(sizeof(union colortable_union) * table_size);

    fseek(fp, 54, SEEK_SET);

    for (int i = 0; i < table_size; i++)
    {
        fread(buffer, 1, 4, fp);
        extract_ulong_from_buffer(buffer, 1, 0, &ull);
        colortable[i].l_num = ull;
    }

    fclose(fp);
    return colortable;
}

void free_colortable(union colortable_union* colortable)
{
    free(colortable);
}

void rgb5652grayscale(long height, long width, FILE* fp, short** the_image, int pad)
{
    union rgb565_union pixel;

    printf("rgb565\n");
    for(int i=0; i<height; i++){
        for(int j=0; j<width; j++){
            fread(&pixel, 1, 2, fp);

            the_image[i][j] = ((pixel.rgb565_struct.red<<3) + 
                               (pixel.rgb565_struct.green<<2) + 
                               (pixel.rgb565_struct.blue<<3)) / 3;
        }  /* ends loop over j */
        if(pad != 0){
            fseek(fp, pad, SEEK_CUR);
        }  /* ends if pad 1= 0 */
    }  /* ends loop over i */
}

void rgb5552grayscale(long height, long width, FILE* fp, short** the_image, int pad)
{
    union rgb555_union pixel;

    printf("rgb555\n");
    for(int i=0; i<height; i++){
        for(int j=0; j<width; j++){
            fread(&pixel, 1, 2, fp);
            the_image[i][j] = ((pixel.rgb555_struct.red<<3) + 
                               (pixel.rgb555_struct.green<<3) + 
                               (pixel.rgb555_struct.blue<<3)) / 3;
        }  /* ends loop over j */
        if(pad != 0){
            fseek(fp, pad, SEEK_CUR);
        }  /* ends if pad 1= 0 */
    }  /* ends loop over i */
}

void bpp242grayscale(long height, long width, FILE* fp, short** the_image, int pad)
{
    union bpp24_union pixel;

    printf("bpp24\n");
    for(int i=0; i<height; i++){
        for(int j=0; j<width; j++){
            fread(&pixel, 1, 3, fp);
            the_image[i][j] = ((pixel.bpp24_struct.red) + 
                               (pixel.bpp24_struct.green) + 
                               (pixel.bpp24_struct.blue)) / 3;
        }  /* ends loop over j */
        if(pad != 0){
            fseek(fp, pad, SEEK_CUR);
        }  /* ends if pad 1= 0 */
    }  /* ends loop over i */
}

void bpp322grayscale(long height, long width, FILE* fp, short** the_image, int pad)
{
    union bpp32_union pixel;

    printf("bpp32\n");
    for(int i=0; i<height; i++){
        for(int j=0; j<width; j++){
            fread(&pixel, 1, 4, fp);
            the_image[i][j] = ((pixel.bpp32_struct.red) + 
                               (pixel.bpp32_struct.green) + 
                               (pixel.bpp32_struct.blue)) / 3;
        }  /* ends loop over j */
        if(pad != 0){
            fseek(fp, pad, SEEK_CUR);
        }  /* ends if pad 1= 0 */
    }  /* ends loop over i */
}

void flip_image_array(short** the_image, long rows, long cols)
{
    int  i, j;
    long rd2;
    short **temp;

    temp = allocate_image_array(rows, cols);
    rd2  = rows/2;
    for(i=0; i<rd2; i++){
        for(j=0; j<cols; j++){
            temp[rows-1-i][j] = the_image[i][j];
        }  /* ends loop over j */
    }  /* ends loop over i */

    for(i=rd2; i<rows; i++){
        for(j=0; j<cols; j++){
            temp[rows-1-i][j] = the_image[i][j];
        }  /* ends loop over j */
    }  /* ends loop over i */

    for(i=0; i<rows; i++)
        for(j=0; j<cols; j++)
            the_image[i][j] = temp[i][j];

    free_image_array(temp, rows);
}  /* ends flip_image_array */

int get_image_size(const char* file_name, long* rows, long* cols)
{
    struct bitmapheader bmph;

    read_bm_header(file_name, &bmph);
    *rows = bmph.height;
    *cols = bmph.width;

    return 1;
}  /* ends get_image_size */

short **allocate_image_array(long length, long width)
{
   int i;
   short **the_array;

   the_array = malloc(length * sizeof(short  *));
   for(i=0; i<length; i++){
      the_array[i] = malloc(width * sizeof(short ));
      if(the_array[i] == '\0'){
         printf("\n\tmalloc of the_image[%d] failed", i);
      }  /* ends if */
   }  /* ends loop over i */
   return(the_array);
}  /* ends allocate_image_array */

int free_image_array(short **the_array, long length)
{
    int i;
    for(i=0; i<length; i++)
        free(the_array[i]);
    return(1);
}  /* ends free_image_array */

int calculate_pad(long width)
{
   int pad = 0;
   pad = ( (width%4) == 0) ? 0 : (4-(width%4));
   return(pad);
}  /* ends calculate_pad */

void create_allocate_bmp_file(const char *file_name, struct bmpfileheader *file_header, struct bitmapheader *bmheader)
{
    char buffer[100];
    int  i, pad = 0;
    FILE *fp;

    pad = calculate_pad(bmheader->width);

    bmheader->size         =  40;
    bmheader->planes       =   1;
    bmheader->bitsperpixel =   8;
    bmheader->compression  =   0;
    bmheader->sizeofbitmap = bmheader->height * 
                            (bmheader->width + pad);
    bmheader->horzres      = 300;
    bmheader->vertres      = 300;
    bmheader->colorsused   = 256;
    bmheader->colorsimp    = 256;

    file_header->filetype     = 0x4D42;
    file_header->reserved1    =  0;
    file_header->reserved2    =  0;
    file_header->bitmapoffset = 14 + 
                                bmheader->size +
                                bmheader->colorsused*4;
    file_header->filesize     = file_header->bitmapoffset +
                                bmheader->sizeofbitmap;

    if((fp = fopen(file_name, "wb")) == NULL){
      printf("\nERROR Could not create file %s",
             file_name);
      exit(2);
    }

    insert_ushort_into_buffer(buffer, 0, file_header->filetype);
    fwrite(buffer, 1, 2, fp);

    insert_ulong_into_buffer(buffer, 0, file_header->filesize);
    fwrite(buffer, 1, 4, fp);

    insert_short_into_buffer(buffer, 0, file_header->reserved1);
    fwrite(buffer, 1, 2, fp);

    insert_short_into_buffer(buffer, 0, file_header->reserved2);
    fwrite(buffer, 1, 2, fp);

    insert_ulong_into_buffer(buffer, 0, file_header->bitmapoffset);
    fwrite(buffer, 1, 4, fp);


      /*********************************************
      *
      *   Write the 40-byte bit map header.
      *
      *********************************************/

    insert_ulong_into_buffer(buffer, 0, bmheader->size);
    fwrite(buffer, 1, 4, fp);

    insert_long_into_buffer(buffer, 0, bmheader->width);
    fwrite(buffer, 1, 4, fp);

    insert_long_into_buffer(buffer, 0, bmheader->height);
    fwrite(buffer, 1, 4, fp);

    insert_ushort_into_buffer(buffer, 0, bmheader->planes);
    fwrite(buffer, 1, 2, fp);

    insert_ushort_into_buffer(buffer, 0, bmheader->bitsperpixel);
    fwrite(buffer, 1, 2, fp);

    insert_ulong_into_buffer(buffer, 0, bmheader->compression);
    fwrite(buffer, 1, 4, fp);

    insert_ulong_into_buffer(buffer, 0, bmheader->sizeofbitmap);
    fwrite(buffer, 1, 4, fp);

    insert_ulong_into_buffer(buffer, 0, bmheader->horzres);
    fwrite(buffer, 1, 4, fp);

    insert_ulong_into_buffer(buffer, 0, bmheader->vertres);
    fwrite(buffer, 1, 4, fp);

    insert_ulong_into_buffer(buffer, 0, bmheader->colorsused);
    fwrite(buffer, 1, 4, fp);

    insert_ulong_into_buffer(buffer, 0, bmheader->colorsimp);
    fwrite(buffer, 1, 4, fp);

      /*********************************************
      *
      *   Write a blank color table.
      *   It has 256 entries (number of colors)
      *   that are each 4 bytes.
      *
      *********************************************/

    buffer[0] = 0x00;

    for(i=0; i<(256*4); i++)
        fwrite(buffer, 1, 1, fp);

      /*********************************************
      *
      *   Write a zero image.  
      *
      *********************************************/

    buffer[0] = 0x00;

    for(i=0; i<bmheader->sizeofbitmap; i++)
        fwrite(buffer, 1, 1, fp);

    fclose(fp);
}  /* ends create_allocate_bmp_file */

void write_bmp_image(const char* file_name, short **array)
{
    char   *buffer;
    FILE   *image_file;
    int    pad = 0;
    int    i, j;
    long   height = 0, width = 0;
    struct bitmapheader  bmheader;
    struct bmpfileheader file_header;
    struct ctstruct rgb[GRAY_LEVELS+1];
    union  short_char_union scu;

    read_bmp_file_header(file_name, &file_header);
    read_bm_header(file_name, &bmheader);

    height = bmheader.height;
    width  = bmheader.width;
    if(height < 0) 
        height = height*(-1);

    buffer = (char *) malloc(width * sizeof(char ));
    for(i=0; i<width; i++)
        buffer[i] = '\0';

    image_file = fopen(file_name, "rb+");

    /****************************************
    *
    *   Write the color table first.
    *
    ****************************************/

    fseek(image_file, 54, SEEK_SET);
    for(i=0; i<GRAY_LEVELS+1; i++){
        rgb[i].blue  = i;
        rgb[i].green = i;
        rgb[i].red   = i;
    }  /* ends loop over i */

    for(i=0; i<bmheader.colorsused; i++){
        buffer[0] = rgb[i].blue;
        fwrite(buffer , 1, 1, image_file);
        buffer[0] = rgb[i].green;
        fwrite(buffer , 1, 1, image_file);
        buffer[0] = rgb[i].red;
        fwrite(buffer , 1, 1, image_file);
        buffer[0] = 0x00;
        fwrite(buffer , 1, 1, image_file);
    }  /* ends loop over i */

    fseek(image_file, file_header.bitmapoffset, SEEK_SET);

    pad = calculate_pad(width);

    for(i=0; i<height; i++){
        for(j=0; j<width; j++){

            if(bmheader.bitsperpixel == 8){
                scu.s_num = 0;
                if(bmheader.height > 0)
                    scu.s_num = array[height-1-i][j];
                else
                    scu.s_num = array[i][j];
                buffer[j] = scu.s_alpha[0];
            }  /* ends if bits_per_pixel == 8 */
            else{
                printf("\nERROR bitsperpixel is not 8");
                exit(1);
            }
        }  /* ends loop over j */

        fwrite(buffer, 1, width, image_file);

        if(pad != 0){
            for(j=0; j<pad; j++)
                buffer[j] = 0x00;
            fwrite(buffer, 1, pad, image_file);
        }  /* ends if pad != 0 */

    }  /* ends loop over i */

    fclose(image_file);
    free(buffer);
}  /* ends write_bmp_image */

void read_bmp_file_header(const char *file_name, struct bmpfileheader *file_header)
{
   char  buffer[10];
   short ss;
   unsigned long  ull;
   unsigned short uss;
   FILE     *fp;

   fp = fopen(file_name, "rb");

   fread(buffer, 1, 2, fp);
   extract_ushort_from_buffer(buffer, 1, 0, &uss);
   file_header->filetype = uss;

   fread(buffer, 1, 4, fp);
   extract_ulong_from_buffer(buffer, 1, 0, &ull);
   file_header->filesize = ull;

   fread(buffer, 1, 2, fp);
   extract_short_from_buffer(buffer, 1, 0, &ss);
   file_header->reserved1 = ss;

   fread(buffer, 1, 2, fp);
   extract_short_from_buffer(buffer, 1, 0, &ss);
   file_header->reserved2 = ss;

   fread(buffer, 1, 4, fp);
   extract_ulong_from_buffer(buffer, 1, 0, &ull);
   file_header->bitmapoffset = ull;

   fclose(fp);

}  /* ends read_bmp_file_header */

void read_bm_header(const char* file_name, struct bitmapheader *bmheader)
{
    char  buffer[10];
    long  ll;
    unsigned long  ull;
    unsigned short uss;
    FILE *fp;

    fp = fopen(file_name, "rb");

    /****************************************
    *
    *   Seek past the first 14 byte header.
    *
    ****************************************/

    fseek(fp, 14, SEEK_SET);

    fread(buffer, 1, 4, fp);
    extract_ulong_from_buffer(buffer, 1, 0, &ull);
    bmheader->size = ull;

    fread(buffer, 1, 4, fp);
    extract_long_from_buffer(buffer, 1, 0, &ll);
    bmheader->width = ll;

    fread(buffer, 1, 4, fp);
    extract_long_from_buffer(buffer, 1, 0, &ll);
    bmheader->height = ll;

    fread(buffer, 1, 2, fp);
    extract_ushort_from_buffer(buffer, 1, 0, &uss);
    bmheader->planes = uss;

    fread(buffer, 1, 2, fp);
    extract_ushort_from_buffer(buffer, 1, 0, &uss);
    bmheader->bitsperpixel = uss;

    fread(buffer, 1, 4, fp);
    extract_ulong_from_buffer(buffer, 1, 0, &ull);
    bmheader->compression = ull;

    fread(buffer, 1, 4, fp);
    extract_ulong_from_buffer(buffer, 1, 0, &ull);
    bmheader->sizeofbitmap = ull;

    fread(buffer, 1, 4, fp);
    extract_ulong_from_buffer(buffer, 1, 0, &ull);
    bmheader->horzres = ull;

    fread(buffer, 1, 4, fp);
    extract_ulong_from_buffer(buffer, 1, 0, &ull);
    bmheader->vertres = ull;

    fread(buffer, 1, 4, fp);
    extract_ulong_from_buffer(buffer, 1, 0, &ull);
    bmheader->colorsused = ull;

    fread(buffer, 1, 4, fp);
    extract_ulong_from_buffer(buffer, 1, 0, &ull);
    bmheader->colorsimp = ull;

    fclose(fp);
}  /* ends read_bm_header */

void extract_long_from_buffer(char buffer[], int lsb, int start, long* number)
{
    union long_char_union lcu;

    if(lsb == 1){
        lcu.l_alpha[0] = buffer[start+0];
        lcu.l_alpha[1] = buffer[start+1];
        lcu.l_alpha[2] = buffer[start+2];
        lcu.l_alpha[3] = buffer[start+3];
    }  /* ends if lsb = 1 */

    if(lsb == 0){
        lcu.l_alpha[0] = buffer[start+3];
        lcu.l_alpha[1] = buffer[start+2];
        lcu.l_alpha[2] = buffer[start+1];
        lcu.l_alpha[3] = buffer[start+0];
    }  /* ends if lsb = 0      */

    *number = lcu.l_num;
    }  /* ends extract_long_from_buffer */

void extract_ulong_from_buffer(char buffer[], int lsb, int start,unsigned long* number)
{
    union ulong_char_union lcu;

    if(lsb == 1){
        lcu.l_alpha[0] = buffer[start+0];
        lcu.l_alpha[1] = buffer[start+1];
        lcu.l_alpha[2] = buffer[start+2];
        lcu.l_alpha[3] = buffer[start+3];
    }  /* ends if lsb = 1 */

    if(lsb == 0){
        lcu.l_alpha[0] = buffer[start+3];
        lcu.l_alpha[1] = buffer[start+2];
        lcu.l_alpha[2] = buffer[start+1];
        lcu.l_alpha[3] = buffer[start+0];
    }  /* ends if lsb = 0      */
    *number = lcu.l_num;
}  /* ends extract_ulong_from_buffer */

void extract_short_from_buffer(char buffer[], int lsb, int start, short* number)
{
    union short_char_union lcu;

    if(lsb == 1){
        lcu.s_alpha[0] = buffer[start+0];
        lcu.s_alpha[1] = buffer[start+1];
    }  /* ends if lsb = 1 */

    if(lsb == 0){
        lcu.s_alpha[0] = buffer[start+1];
        lcu.s_alpha[1] = buffer[start+0];
    }  /* ends if lsb = 0      */

    *number = lcu.s_num;
}  /* ends extract_short_from_buffer */

void extract_ushort_from_buffer(char buffer[], int lsb, int start, unsigned short* number)
{
    union ushort_char_union lcu;

    if(lsb == 1){
        lcu.s_alpha[0] = buffer[start+0];
        lcu.s_alpha[1] = buffer[start+1];
    }  /* ends if lsb = 1 */

    if(lsb == 0){
        lcu.s_alpha[0] = buffer[start+1];
        lcu.s_alpha[1] = buffer[start+0];
    }  /* ends if lsb = 0      */

    *number = lcu.s_num;
}  /* ends extract_ushort_from_buffer */

void insert_short_into_buffer(char buffer[], int start, short number)
{
    union short_char_union lsu;

    lsu.s_num       = number;
    buffer[start+0] = lsu.s_alpha[0];
    buffer[start+1] = lsu.s_alpha[1];

}  /* ends insert_short_into_buffer */

void insert_ushort_into_buffer(char buffer[], int start, unsigned short number)
{
    union ushort_char_union lsu;

    lsu.s_num       = number;
    buffer[start+0] = lsu.s_alpha[0];
    buffer[start+1] = lsu.s_alpha[1];
}  /* ends insert_short_into_buffer */

void insert_long_into_buffer(char buffer[], int start, long number)  
{
    union long_char_union lsu;

    lsu.l_num       = number;
    buffer[start+0] = lsu.l_alpha[0];
    buffer[start+1] = lsu.l_alpha[1];
    buffer[start+2] = lsu.l_alpha[2];
    buffer[start+3] = lsu.l_alpha[3];
}  /* ends insert_short_into_buffer */

void insert_ulong_into_buffer(char buffer[], int start, unsigned long number)
{
    union ulong_char_union lsu;

    lsu.l_num       = number;
    buffer[start+0] = lsu.l_alpha[0];
    buffer[start+1] = lsu.l_alpha[1];
    buffer[start+2] = lsu.l_alpha[2];
    buffer[start+3] = lsu.l_alpha[3];
}  /* ends insert_ulong_into_buffer */

int does_not_exist(const char* file_name)
{
    FILE *image_file;
    int  result;

    result = 1;
    image_file = fopen(file_name, "rb");
    if(image_file != NULL){
        result = 0;
        fclose(image_file);
    }
    return(result);
}  /* ends does_not_exist */

