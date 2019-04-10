#include <errno.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include "log.h"
#include "shmq.h"
#include "global.h"
#include "plugin.h"
#include "reactor.h"
#include "tcp_linker.h"
#include "tcp_async_linker.h"

// 连接是有超时时间的同步连接，会比较耗时(默认情况下最多会耗时2秒)，但是对于和后端建立
// 长连接并不经常断开的情况下是可以接受的，但是更好的办法是异步连接，在epoll中去处理连
// 接建立后的初始化。
extern "C" int net_connect_ser(const char *ip, unsigned short port, int ms_timeout)
{
    struct sockaddr_in in;
    in.sin_family = AF_INET;
    in.sin_port = htons(port);
    if (inet_pton(AF_INET, ip, &in.sin_addr) <= 0)
        return -2;

    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd == -1)
        return -1;

    if ((uint32_t)fd >= g_reactor.max_handler()) {
        close(fd);
        return -1;
    }

    if (ms_timeout > 0) {
        struct timeval tv;
        tv.tv_sec = ms_timeout / 1000;
        tv.tv_usec = (ms_timeout % 1000) * 1000;
        setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));
    }

    if (connect(fd, (struct sockaddr *)&in, sizeof(in)) == -1) {
        close(fd);
        return -1;
    }

    if (ms_timeout > 0) {
        struct timeval tv;
        tv.tv_sec = 0;
        tv.tv_usec = 0;
        setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));
    }

    c_tcp_linker *linker = new (std::nothrow) c_tcp_linker();

    if (!linker) {
        close(fd);
        return -1;
    }

    if (!linker->start(fd, in)) {
        // close(fd);
        delete linker;
        return -1;
    }

    return fd;
}

extern "C" int async_net_connect_ser(const char *ip, unsigned short port)
{
    struct sockaddr_in in;
    in.sin_family = AF_INET;
    in.sin_port = htons(port);
    if (inet_pton(AF_INET, ip, &in.sin_addr) <= 0)
        return -2;

    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd == -1)
        return -1;

    if ((uint32_t)fd >= g_reactor.max_handler()) {
        close(fd);
        return -1;
    }

    c_tcp_async_linker *linker = new (std::nothrow) c_tcp_async_linker();

    if (!linker) {
        close(fd);
        return -1;
    }

    if (!linker->start(fd, in)) {
        // delete linker;
        return -1;
    }

    return fd;
}

extern "C" int net_send_ser(int fd, const void *buf, int len)
{
    if ((uint32_t)fd >= g_reactor.max_handler() || len <= 0)
        return -1;

    c_handler *handler = g_reactor.get_handler(fd);
    if (!handler)
        return -1;

    handler->send_pkg(buf, (uint32_t)len);
    return len;
}

extern "C" void net_close_ser(int fd)
{
    if ((uint32_t)fd >= g_reactor.max_handler())
        return;

    c_handler *handler = g_reactor.get_handler(fd);
    if (!handler)
        return;

    handler->shut();
}

extern "C" int net_send_cli(int fd, const void *buf, int len)
{
    if ((uint32_t)fd >= g_max_connect || g_link_flags[fd] == 0 || len <= 0)
        return -1;

    struct shm_block_t sb;
	char logbuf[PACKAGE_MAX_SIZE * 3 + 1] = {0};

    sb.fd = fd;
    sb.id = g_link_flags[fd];
    sb.len = len;
    sb.type = PROTO_BLOCK;
	ASC2HEX_TEST(logbuf, (char*)buf, (sb.len > PACKAGE_MAX_SIZE) ? PACKAGE_MAX_SIZE : sb.len);
	DEBUG_LOG("[3]Send msg to connect process: SEND BUFFER:[%s]BUFFER LENGTH:[%d]", logbuf, sb.len);
	DEBUG_LOG("[3]Send msg to connect process: cli_fd[%d], sb_id[%d], channel[%d]", sb.fd, sb.id, g_work_channel);
    send_push(g_work_channel, &sb, (const uint8_t *)buf, true);
    return len;
}

extern "C" void net_close_cli(int fd)
{
    if ((uint32_t)fd >= g_max_connect || g_link_flags[fd] == 0)
        return;

    struct shm_block_t sb;
    sb.fd = fd;
    sb.id = g_link_flags[fd];
    sb.len = 0;
    sb.type = CLOSE_BLOCK;
    send_push(g_work_channel, &sb, NULL, true);

    g_link_flags[fd] = 0;
    return;
}

extern "C" int net_send_cli_conn(int fd, const void *buf, int len)
{
    return net_send_ser(fd, buf, len);
}

