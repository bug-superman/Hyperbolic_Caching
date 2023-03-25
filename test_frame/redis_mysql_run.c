/* gcc -o redis_mysql_run redis_mysql_run.c -lhiredis -lmysqlclient*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <hiredis/hiredis.h>
#include <limits.h>
#include <mysql/mysql.h>

#define MYSQL_HOST "43.139.102.147"
#define MYSQL_USEPORT 1024
#define MYSQL_USER "root"
#define MYSQL_PASSWORD "1024"
#define MYSQL_DATABASE "hc_redis_db"
#define MYSQL_TABLE "simulated_data"

#define REDIS_HOST "127.0.0.1"
#define REDIS_PORT 6379
#define REDIS_DB   0
#define DATAFILE "zipfian_data_a.txt"
#define MAX_STR 4050
#define CACHE_MISS 0
#define CACHE_HIT 1

 #define N 500000   

/* if cache miss return 0, cacahe hit return 1*/
int get_Cache(redisContext *redis,MYSQL *mysql,int key) {
     
     redisReply *reply = redisCommand(redis, "GET %d",key);
    if (reply == NULL || reply->type == REDIS_REPLY_ERROR) {
        printf("Error: %s\n", redis->errstr);
        freeReplyObject(reply);
        redisFree(redis);
        exit(EXIT_FAILURE);
    }
    MYSQL_RES *res;
    MYSQL_ROW row;
    // cache miss
     if (reply->str == NULL) {   
      char  mysql_statement[MAX_STR] = "\0";
       sprintf(mysql_statement, "SELECT * FROM simulated_data WHERE data_key = %d; ", key);
        if(mysql_query(mysql, mysql_statement)) {
             fprintf(stderr, "Error : %s\n", mysql_error(mysql));   
             mysql_close(mysql);
             exit(EXIT_FAILURE);  
        }
        res = mysql_use_result(mysql);
         if((row = mysql_fetch_row(res)) == NULL) {
            mysql_free_result(res);
            mysql_close(mysql);
           exit(EXIT_FAILURE);
        }
        // insert data to redis
        reply = redisCommand(redis, "SET %d %s",key, row[1]);
        if (reply == NULL || reply->type == REDIS_REPLY_ERROR) {
        printf("Error: %s\n", redis->errstr);
        freeReplyObject(reply);
        redisFree(redis);
        exit(EXIT_FAILURE);
        }

         mysql_free_result(res);
         freeReplyObject(reply);
        return CACHE_MISS;
    }
    freeReplyObject(reply);
    return CACHE_HIT;
}
redisContext* init_redis() {
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
    return redis;
}
MYSQL *init_mySQL(){
    MYSQL *conn = mysql_init(NULL);
    if(conn == NULL)
    {
        printf("Error: can't create MySQL connection\n");
        return NULL;
    }

    if(mysql_real_connect(conn, MYSQL_HOST, MYSQL_USER, MYSQL_PASSWORD, MYSQL_DATABASE, MYSQL_USEPORT, NULL, 0) == NULL)
    {
        printf("Error: can't connect to MySQL server\n");
        mysql_close(conn);
        return NULL;
    }
    printf("MySQL init access!\n");
    return conn;
}
int compare(const void *a, const void *b) {
    return (*(double *)a - *(double *)b);
}

void calculate_statistics(double *response_times, int num_responses) {
    double min_response_time = response_times[0];
    double max_response_time = response_times[0];
    double sum_response_time = 0.0;

    for (int i = 0; i < num_responses; i++) {
        if (response_times[i] < min_response_time) {
            min_response_time = response_times[i];
        }
        if (response_times[i] > max_response_time) {
            max_response_time = response_times[i];
        }
        sum_response_time += response_times[i];
    }
    double avg_response_time = sum_response_time / num_responses;

    qsort(response_times, num_responses, sizeof(double), compare);

    int p50_index = (int)(0.5 * num_responses);
    int p95_index = (int)(0.95 * num_responses);
    int p99_index = (int)(0.99 * num_responses);

    double p50_response_time = response_times[p50_index];
    double p95_response_time = response_times[p95_index];
    double p99_response_time = response_times[p99_index];

    printf("Min response time: %.2lf ms\n", min_response_time);
    printf("Avg response time: %.2lf ms\n", avg_response_time);
    printf("P50 response time: %.2lf ms\n", p50_response_time);
    printf("P95 response time: %.2lf ms\n", p95_response_time);
    printf("P99 response time: %.2lf ms\n", p99_response_time);
    printf("Max response time: %.2lf ms\n", max_response_time);
    printf("Sum response time: %.2lf ms\n", sum_response_time);
}
void run(MYSQL *mysql, redisContext* redis, double *response_times,int num_responses) {
    // 打开文件
    char line[MAX_STR + 1] = "\0";
    FILE *fp = fopen(DATAFILE, "r");
    if (fp == NULL) {
        printf("Failed to open file: %s\n", DATAFILE);
         exit(EXIT_FAILURE);
    }
    printf("redis run ok and read file\n");
    /* 开始模拟负载*/
    int cachemiss = 0, cachehit = 0, number = 0;
    int i = 0, key;
     while (fgets(line, MAX_STR, fp) != NULL) {
       // printf("read is ok!\n");
       int len = strcspn(line, "\n");
        line[len] = '\0';
        sscanf(line,"%d",&key);
        struct timeval start, end;
        gettimeofday(&start, NULL);
        int ans = get_Cache(redis, mysql, key);
        gettimeofday(&end, NULL);
        if( i < N)
        response_times[i++] = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1000000.0;; 
        printf("i:%d, response_times:%lf\n",i,response_times[i]);
        if(ans ==CACHE_HIT) cachehit++;
        else cachemiss++;
        memset(line, '\0',  sizeof(line));
    }
    double cacherate = (double) cachehit / (cachehit + cachemiss);
    printf("cachemiss:%d,cachehit:%d, cacherate:%lf\n",cachemiss, cachehit,cacherate);
    fclose(fp);
}

int main(void) {
     /* 连接reids, mysql*/
    MYSQL *mysql = init_mySQL();
    redisContext *redis = init_redis();
    double *response_times = malloc( N * sizeof(double));

    run(mysql, redis, response_times, N);
    calculate_statistics(response_times, N);
    free(response_times);
    return 0;
}