#define ARRAY_SIZE 3

int is_in_array(short x, short y, short height, short width)
{
    if (x >= 0 && x < width && y >= 0 && y < height)
        return 1;
    else
        return 0;
}

/*
 * element
 * v0  v1  v2
 * v3  v4  v5
 * v6  v7  v8
 *
 */
void filtering(short** in_array, short** out_array, long height, long width)
{
    short value[9];

    for (int i = 0; i < height; i++){
        for (int j = 0; j < width; j++){
            value[0] = is_in_array(j-1, i-1, height, width) ? in_array[i-1][j-1] : 0;
            value[1] = is_in_array(j, i-1, height, width) ? in_array[i-1][j] : 0;
            value[2] = is_in_array(j+1, i-1, height, width) ? in_array[i-1][j+1] : 0;
            value[3] = is_in_array(j-1, i, height, width) ? in_array[i][j-1] : 0;
            value[4] = in_array[i][j];
            value[5] = is_in_array(j+1, i, height, width) ? in_array[i][j+1] : 0;
            value[6] = is_in_array(j-1, i+1, height, width) ? in_array[i+1][j-1] : 0;
            value[7] = is_in_array(j, i+1, height, width) ? in_array[i+1][j] : 0;
            value[8] = is_in_array(j+1, i+1, height, width) ? in_array[i+1][j+1] : 0;

            /* Arithmetic Mean Filter */
            for (int k = 0; k < ARRAY_SIZE*ARRAY_SIZE; k++)
                out_array[i][j] += value[k];
            out_array[i][j] /= ARRAY_SIZE*ARRAY_SIZE;

            /* median filtering */
            /*out_array[i][j] = mid_val(value, 9);
            if (out_array[i][j] < 0x00)
                out_array[i][j] = 0x00;
            else if (out_array[i][j] > 0xff)
                out_array[i][j] = 0xff;*/
        }
    }
}
