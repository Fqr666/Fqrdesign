#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>

#define PORT 8888
#define MAX_BUF_SIZE 1024

int main(int argc, char *argv[])
{
    int sock_fd, len;
    struct sockaddr_in server_addr;
    char buffer[MAX_BUF_SIZE] = {0};
    char file_path[128] = {0};

    if (argc < 3)
    {
        printf("Usage: %s [IP address] [FilePath]\n", argv[0]);
        return -1;
    }

    // 创建socket套接字
    if ((sock_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // 设置服务器信息
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);

    // 将IP地址转换为网络字节序
    if (inet_pton(AF_INET, argv[1], &server_addr.sin_addr) <= 0)
    {
        perror("Invalid address/ Address not supported");
        exit(EXIT_FAILURE);
    }

    // 连接到服务器
    if (connect(sock_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("Failed to connect to server");
        exit(EXIT_FAILURE);
    }

    // 获取待传输文件的路径和名称，并发送到服务器
    len = strlen(argv[2]);
    argv[2][len] = '\0';
    if (send(sock_fd, argv[2], len, 0) != len)
    {
        perror("Send file path error");
        close(sock_fd);
        exit(EXIT_FAILURE);
    }

    // 打开指定路径上的文件，并获取文件大小
    FILE *fp = fopen(argv[2], "rb");
    if(fp == NULL){
        perror("Open file error");
        close(sock_fd);
        exit(EXIT_FAILURE);
    }
    fseek(fp, 0L, SEEK_END);
    int file_size = ftell(fp);
    fseek(fp, 0L, SEEK_SET);

    // 发送文件大小信息到服务器，同时等待应答
    int recv_size;
    if (send(sock_fd, &file_size, sizeof(int), 0) != sizeof(int))
    {
        perror("Send file size error");
        close(sock_fd);
        fclose(fp);
        exit(EXIT_FAILURE);
    }
    if ((recv_size = recv(sock_fd, buffer, MAX_BUF_SIZE, 0)) < 0)
    {
        perror("Receive response error");
        close(sock_fd);
        fclose(fp);
        exit(EXIT_FAILURE);
    }

    // 发送文件内容到服务器
    int total_size = 0;
    while ((len = fread(buffer, sizeof(char), MAX_BUF_SIZE, fp)) > 0)
    {
        if (send(sock_fd, buffer, len, 0) != len)
        {
            perror("Send file data error");
            close(sock_fd);
            fclose(fp);
            exit(EXIT_FAILURE);
        }
        total_size += len;
        memset(buffer, 0, MAX_BUF_SIZE);
    }

    fclose(fp);
    printf("File transfer successful, size = %d bytes.\n", total_size);

    // 关闭连接
    close(sock_fd);
    return 0;
}
