/*gcc zipfian.c -o zipfian -lm*/
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <math.h>
#include <time.h>

#define N 500000              // 生成数据的总数
#define ALPHA 1.22            // Zipfian分布的参数
#define REQUEST_NUMBER 5000000 // 访问请求50w

double zeta(int n, double alpha)
{
    double sum = 0.0;
    for (int i = 1; i <= n; i++)
    {
        sum += 1.0 / pow(i, alpha);
    }
    return sum;
}

int zipf(double alpha, int n, double zeta_value)
{
    double rand_uniform = (double)rand() / (double)RAND_MAX;
    double sum = 0.0;
    int k;
    for (k = 1; k <= n; k++)
    {
        sum += 1.0 / pow(k, alpha);
        if (sum / zeta_value >= rand_uniform)
        {
            break;
        }
    }
    return k;
}

void fisher_yates_shuffle(int *array, int n)
{
    for (int i = n - 1; i > 0; i--)
    {
        int j = rand() % (i + 1);
        int temp = array[i];
        array[i] = array[j];
        array[j] = temp;
    }
}

int main(void)
{
    int *requests = malloc(REQUEST_NUMBER * sizeof(int));
    double zeta_value = zeta(N, ALPHA);
    for (int i = 0; i < REQUEST_NUMBER; i++)
    {
        requests[i] = (int)zipf(ALPHA, N, zeta_value) % N;
       // printf("i:%d\n", i);
    }
    fisher_yates_shuffle(requests, REQUEST_NUMBER); // 打乱数组
    FILE *fp = fopen("zipfian_data.txt", "w");      // 打开输出文件
    for (int i = 0; i < REQUEST_NUMBER; i++)
    {
        fprintf(fp, "%d\n", requests[i]); // 将生成的数据写入文件
    }
    free(requests);
    fclose(fp); // 关闭输出文件
    return 0;
}