/* gcc mysql_example.c -o mysql_example -lmysqlclient*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <limits.h>
#include <mysql/mysql.h>
#define MYSQL_HOST "43.139.102.147"
#define MYSQL_USEPORT 1024
#define MYSQL_USER "root"
#define MYSQL_PASSWORD "1024"
#define MYSQL_DATABASE "hc_redis_db"
#define MYSQL_TABLE "simulated_data"
#define MAX_STR 4000
#define N 1000000


struct timeval tv;
unsigned int seed_num;
char seed_str[] = "abcdefghijklmnopqrstuvwxyz"
"ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890"; //随机字符串的随机字符集
// 生成指定长度的random字符串
void get_random_str(char* random_str, const int random_len)
{
    int i, random_num, seed_str_len;

    seed_str_len = 51;
    
    gettimeofday(&tv, NULL);
    seed_num = (tv.tv_sec + tv.tv_usec) % UINT_MAX; 
    srand(seed_num);

    for(i = 0; i < random_len; i++)
    {
        random_num = rand()%seed_str_len;
        random_str[i] = seed_str[random_num];
    }

}


// 将key赋值上随机长度的value
char* set_value(char *value) {
    int len = 0;
    gettimeofday(&tv, NULL);
    seed_num = (tv.tv_sec + tv.tv_usec) % UINT_MAX; 
    srand(seed_num);
    len = rand() % MAX_STR;
    if(len <= 0) len = 5;
    get_random_str(value, len);
    return value;
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
void insert_mySQLDATA(MYSQL *conn) {
    char sql_statement[ MAX_STR +  1000] = "\0";
    char value[MAX_STR + 10] = "\0";
    for(int i = 0; i <= N; i++) {
         set_value(value);
         sprintf(sql_statement, "INSERT INTO simulated_data(data_key, data_value) VALUES (%d, '%s');", i, value);
        while (mysql_query(conn, sql_statement)){
        fprintf(stderr, "Error inserting row %d: %s\n", i, mysql_error(conn));     
         break;
        }
        memset(sql_statement, '\0',  sizeof(sql_statement));
        memset(value,'\0',  sizeof(value));
    }
}
int main(void) {
   MYSQL *conn =  init_mySQL();
    if(conn != NULL) {
        insert_mySQLDATA(conn);
    }
    return 0;
}