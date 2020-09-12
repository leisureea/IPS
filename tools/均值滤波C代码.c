
// 本函数是实现 3 * 3 均值滤波
//函数的的当前点的像素 = 3 * 3 范围的平均值
// @para src 输入图像数据
// @para srcW 输入图形宽
// @para srcH 输入图像的高
// @para channels 通道数 
// @para dest 输出图像 ， 输出图像的大小和输入一样
    int MeanFilter(unsigned char * src, int srcW, int srcH, int channels, unsigned char * dest)
    {
        int i = 0;
        int j = 0;
        int k = 0;

        int ModelWH = 3;
        int halfM = ModelWH / 2;

        int m = 0;
        int n = 0;
        if (channels > 4 || channels < 1)
        {
            return -1;
        }

        for (i = 0; i < srcH; i++)
        {
            for (j = 0; j < srcW; j++)
            {
                int sum = 0;

                int temp[4] = { 0, 0, 0, 0 };
                int count[4] = { 0, 0, 0, 0 };
                for (n = i - halfM; n <= i + halfM; n++)
                {
                    for (m = j - halfM; m <= j + halfM; m++)
                    {
                        for (k = 0; k < channels; k++)
                        {
                            unsigned char temp1 = 0;
                            if (n > 0 && n < srcH && m > 0 && m < srcW)
                            {
                                temp1 = src[(n * srcW + m) * channels + k];

                                count[k]++;
                                temp[k] += temp1;
                            }

                        }

                    }
                }

                for (k = 0; k < channels; k++)
                {
                    dest[(i * srcW + j) * channels + k] = (unsigned char)((temp[k] * 1.0f / (count[k])+0.5f));
                }
            }
        }
        return 0;
    }
