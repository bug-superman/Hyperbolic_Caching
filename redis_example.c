/* gcc -o redis_example redis_example.c -lhiredis*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <hiredis/hiredis.h>
#include <limits.h>

#define REDIS_HOST "127.0.0.1"
#define REDIS_PORT 6379
#define REDIS_DB   0
#define DATAFILE "zipfian_data.txt"
#define MAX_STR 4048
#define N 2000000
#define CACHE_MISS 0
#define CACHE_HIT 1

struct timeval tv;
unsigned int seed_num;
char seed_str[] = "abcdefghijklmnopqrstuvwxyz"
"ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789"; //随机字符串的随机字符集
// 生成指定长度的random字符串
void get_random_str(char* random_str, const int random_len)
{
    int i, random_num, seed_str_len;

    seed_str_len = strlen(seed_str);
    
    gettimeofday(&tv, NULL);
    seed_num = (tv.tv_sec + tv.tv_usec) % UINT_MAX; 
    srand(seed_num);

    for(i = 0; i < random_len; i++)
    {
        random_num = rand()%seed_str_len;
        random_str[i] = seed_str[random_num];
    }

}

void init_value(char **values) {
    for(int i = 0; i < N; i++) {
        values[i] = NULL;
    }
} 
// 将key赋值上随机长度的value
void set_value(char *value) {
    int len = 0;
    gettimeofday(&tv, NULL);
    seed_num = (tv.tv_sec + tv.tv_usec) % UINT_MAX; 
    srand(seed_num);
    len = rand() % MAX_STR;
    if(len == 0) len = 5;
    value = (char *) malloc(sizeof(char) * (len + 1));
    get_random_str(value, len);
}

/* if cache miss return 0, cacahe hit return 1*/
int get_redisCache(redisContext *redis,char *key, char *value) {
     
     redisReply *reply = redisCommand(redis, "GET %s",key);
    if (reply == NULL || reply->type == REDIS_REPLY_ERROR) {
        printf("Error: %s\n", redis->errstr);
        freeReplyObject(reply);
        redisFree(redis);
        exit(EXIT_FAILURE);
    }
    
    // cache miss
    if (reply->str == NULL) {
        reply = redisCommand(redis, "SET %s %s",key, value);
        if (reply == NULL || reply->type == REDIS_REPLY_ERROR) {
        printf("Error: %s\n", redis->errstr);
        freeReplyObject(reply);
        redisFree(redis);
        exit(EXIT_FAILURE);
        }
        return CACHE_MISS;
    }
    return CACHE_HIT;
}
int main(void) {
    // 打开文件
    FILE *fp = fopen(DATAFILE, "r");
    char line[MAX_STR + 1] = "\0";
    char *token;
    char **values = malloc(sizeof(char*) * N);
    int key = 0;
    //初始化工作
    init_value(values); // map将key与value对应起来

    if (fp == NULL) {
        printf("Failed to open file: %s\n", DATAFILE);
         exit(EXIT_FAILURE);
    }
    /* 连接reids*/
    redisContext *redis = redisConnect(REDIS_HOST, REDIS_PORT);
    if (redis == NULL || redis->err) {
        if (redis) {
            printf("Error: %s\n", redis->errstr);
            redisFree(redis);
        } else {
            printf("Can't allocate redis context\n");
        }

        exit(EXIT_FAILURE);
    }
    printf("redis run ok and read file\n");
    /* 开始模拟负载*/
    int cachemiss = 0, cachehit = 0;
    int i = 0;
    while (fgets(line, MAX_STR, fp) != NULL) {
       int len = strcspn(line, "\n");
        line[len] = '\0';
        sscanf(line,"%d",&key);
        
        if(values[key] == NULL) { //第一次加载需要初始化随机长度的value
            set_value(values[key]);
        }
        int ans = get_redisCache(redis, line, values[key]);
        if(ans ==CACHE_HIT) cachehit++;
        else cachemiss++;

        memset(line, '\0',  sizeof(line));
    }
    double cacherate = (double) cachehit / (cachehit + cachemiss);
    printf("cachemiss:%d,cachehit:%d, cacherate:%lf\n",cachemiss, cachehit, cacherate);
    // 关闭文件
    fclose(fp);
    free(values);
    return 0;
}