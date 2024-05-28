***服务端：Server.c***
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>

#define PORT 8000      // 服务端监听的端口号

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: %s <file directory> <filename>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    char *dir = argv[1];                   // 接收到的文件目录
    char *filename = argv[2];              // 接收到的文件名
    
    char buffer[1024];                     // 缓冲区
    int listen_fd, conn_fd;                // 监听和连接文件描述符
    struct sockaddr_in serv_addr, client_addr; // 服务端和客户端地址结构体
    socklen_t cli_len;                     // 客户端地址字节长度
    ssize_t recv_len, written_len, total_len = 0;   // 接收、写入、总共接收的数据长度
    
    /* 创建监听套接字 */
    if ((listen_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }
    
    /* 初始化服务端地址结构体 */
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(PORT);
    
    /* 绑定套接字和地址 */
    if (bind(listen_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1) {
        perror("bind");
        exit(EXIT_FAILURE);
    }
    
    /* 监听连接请求 */
    if (listen(listen_fd, 10) == -1) {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    
    printf("Listening on port %d...\n", PORT);
    
    /* 接受连接请求 */
    cli_len = sizeof(client_addr);
    if ((conn_fd = accept(listen_fd, (struct sockaddr *)&client_addr, &cli_len)) == -1) {
        perror("accept");
        exit(EXIT_FAILURE);
    }
    
    printf("Accepted connection from %s:%d\n",
            inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
    
    /* 打开文件并准备写入 */
    char filepath[1024];
    sprintf(filepath, "%s/%s", dir, filename);  // 拼接文件路径
    FILE *fp = fopen(filepath, "wb");
    if (fp == NULL) {
        perror("fopen");
        exit(EXIT_FAILURE);
    }
    
    /* 接收数据并写入文件 */
    while ((recv_len = recv(conn_fd, buffer, sizeof(buffer), 0)) > 0) {
        written_len = fwrite(buffer, 1, recv_len, fp);
        if (written_len != recv_len) {
            perror("fwrite");
            exit(EXIT_FAILURE);
        }
        total_len += written_len;
    }
    if (recv_len < 0 && errno != EAGAIN && errno != EWOULDBLOCK) {
        perror("recv");
        exit(EXIT_FAILURE);
    }
    
    /* 关闭套接字和文件 */
    close(conn_fd);
    fclose(fp);
    
    printf("Received %zd bytes and saved to file '%s'\n", total_len, filepath);
    
    return 0;
}
