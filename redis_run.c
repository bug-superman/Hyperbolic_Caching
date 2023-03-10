#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <hiredis/hiredis.h>

#define REDIS_HOST "127.0.0.1"
#define REDIS_PORT 6379
#define REDIS_DB   0

#define FILENAME "data.txt"

int main() {
    FILE *fp;
    char line[1024];
    redisContext *c;
    redisReply *reply;

    // 打开文件
    fp = fopen(FILENAME, "r");
    if (fp == NULL) {
        printf("Failed to open file: %s\n", FILENAME);
        return 1;
    }

    // 连接到Redis服务器并选择数据库
    c = redisConnect(REDIS_HOST, REDIS_PORT);
    if (c == NULL || c->err) {
        printf("Failed to connect to Redis server: %s\n", c->errstr);
        fclose(fp);
        return 1;
    }
    reply = redisCommand(c, "SELECT %d", REDIS_DB);
    freeReplyObject(reply);

    // 从文件中读取数据并插入到Redis中
    while (fgets(line, sizeof(line), fp) != NULL) {
        int value = atoi(line);
        reply = redisCommand(c, "RPUSH mylist %d", value);
        freeReplyObject(reply);
    }

    // 断开与Redis服务器的连接并关闭文件
    redisFree(c);
    fclose(fp);

    return 0;
}