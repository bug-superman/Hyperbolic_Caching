#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/time.h>
#include<limits.h>
#include<cstring>
#define N 1300000 // 生成数据的总数
#define ALPHA 1.0 // Zipfian分布的参数


struct timeval tv;
unsigned int seed_num;


// 交换两个元素
void swap(int *a, int *b) {
    int temp = *a;
       *a = *b;
      *b = temp;
}

// 打乱数组
void shuffle(int* arr, int size) {
    int i, j;
    for (i = size - 1; i > 0; i--) {
        j = rand() % (i + 1);
        swap(&arr[i], &arr[j]);
    }
}

int main(void) {
    int i, j, k, unique_size;
    double z, sum_prob, prob, rand_num;
    int *freq = (int*)malloc(N * sizeof(int)); // 存储每个元素的频率
    int *keys = (int*)malloc(N * sizeof(int));
    
    seed_num = (tv.tv_sec + tv.tv_usec) % UINT_MAX; 
    srand(seed_num); // 初始化随机数生成器
    for (i = 1; i <= N; i++) {
        z = log(i) / log(2.0) + ALPHA; // 计算当前元素的概率
        sum_prob += pow(2.0, -z); // 累加概率
    }
    for (i = 1; i <= N; i++) {
        z = log(i) / log(2.0) + ALPHA; // 计算当前元素的概率
        prob = pow(2.0, -z) / sum_prob; // 计算当前元素的概率
        freq[i - 1] = (int)(prob * N + 0.5); // 根据概率计算元素出现的频率
    }
    k = 0;
    int len = 0;
    for (i = 0; i < N; i++) {
        for (j = 0; j <= freq[i] && k  < N; j++) {
            keys[k++]= i + 1; // 将当前元素按照对应频率生成到数据中
        }
    }
    printf("OK!!!\n");
    shuffle(keys, N); // 打乱数组
    FILE* fp = fopen("zipfian_data.txt", "w"); // 打开输出文件
    for (i = 0; i < N ; i++) {
        fprintf(fp, "%d\n", keys[i]); // 将生成的数据写入文件
    }
   free(keys);
   free(freq);
   fclose(fp); // 关闭输出文件
}