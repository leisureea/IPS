#define GRAY_LEVELS          255

void calculate_histogram(long height, long width, short **image, unsigned long histogram[])
{
    short k;
    for(int i=0; i < height; i++){
        for(int j=0; j < width; j++){
            k = image[i][j];
            histogram[k] = histogram[k] + 1;
        }
    }
} 

void histogram_equalization(short** in_array, short** out_array, long height, long width)
{
    unsigned long sum, sum_of_h[GRAY_LEVELS];
    double constant;
    unsigned long histogram[GRAY_LEVELS] = {};

    calculate_histogram(height, width, in_array, histogram);
    sum = 0;
    for(int i=0; i < GRAY_LEVELS; i++){
        sum         = sum + histogram[i];
        sum_of_h[i] = sum;
    }

    constant = (double)(GRAY_LEVELS)/(double)(height*width);
    for(int i = 0, k = 0; i < height; i++){
        for(int j = 0; j < width; j++){
            k = in_array[i][j];
            out_array[i][j] = sum_of_h[k] * constant;
        }
    }
}