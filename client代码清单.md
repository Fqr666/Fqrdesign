

***客户端：Client.c***
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#define PORT 8000      // 服务端监听的端口号

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: %s <server IP> <filename>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    char *ip = argv[1];                    // 服务端IP地址
    char *filename = argv[2];              // 要发送的文件名
    
    char buffer[1024];                     // 缓冲区
    int sockfd;                            // 套接字文件描述符
    struct sockaddr_in serv_addr;          // 服务端地址结构体
    ssize_t read_len, sent_len, total_len = 0;  // 读取、发送、总共发送的数据长度
    FILE *fp;                              // 文件指针
    
    /* 打开要传输的文件 */
    if ((fp = fopen(filename, "rb")) == NULL) {
        perror("fopen");
        exit(EXIT_FAILURE);
    }
    
    /* 创建套接字 */
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }
    
    /* 初始化服务端地址结构体 */
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(ip);
    serv_addr.sin_port = htons(PORT);
    
    /* 建立连接 */
    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1) {
        perror("connect");
        exit(EXIT_FAILURE);
    }
    
    printf("Connected to server %s:%d\n", ip, PORT);
    
    /* 读取并发送数据 */
    while ((read_len = fread(buffer, 1, sizeof(buffer), fp)) > 0) {
        sent_len = send(sockfd, buffer, read_len, 0);
        if (sent_len != read_len) {
            perror("send");
            exit(EXIT_FAILURE);
        }
        total_len += sent_len;
    }
    
    /* 关闭套接字和文件 */
    close(sockfd);
    fclose(fp);
    
    printf("Sent %zd bytes of file '%s' to server\n", total_len, filename);
    
    return 0;
}

