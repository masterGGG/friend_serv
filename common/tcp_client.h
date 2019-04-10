/**
 * =====================================================================================
 *   Compiler   g++
 *   Company    TaoMee.Inc, ShangHai.
 *   Copyright  Copyright (c) 2013, TaoMee.Inc, ShangHai.
 *
 *   @brief   淘米统计平台公共库，各服务模块共享。
 *   @author  ianguo<ianguo@taomee.com>
 *   @date    2013-12-10
 * =====================================================================================
 */

#ifndef TCP_CLIENT_HPP
#define TCP_CLIENT_HPP

#include <string>
#include <ctime>
#include <cstring>
#include <cerrno>

#include <unistd.h>
#include <sys/uio.h>
#include <sys/socket.h>

#include "common.h"

using std::string;

/**
 * TCP client封装类，利用阻塞模式连接到server。
 * 使用本类一定要定时检测socket是否关闭（调用reconnnect,或is_real_alive函数），
 * 以保证socket能及时关闭，否则socket在被动关闭时(server关闭socket)，
 * client socket会长时间处于close_wait状态。
 */
class TcpClient
{
public:
    enum
    {
        TC_ERROR = -1,
        TC_INVALID_PARAMS = -2,
        TC_TIMEOUT = 0,
        TC_OK = 1
    };

    TcpClient() : m_fd(-1), m_ip(""), m_port(0), m_timeout_ms(2000)
    {}
    TcpClient(const string& ip, int port, int timeout_ms) : m_fd(-1), m_ip(ip), m_port(port), m_timeout_ms(timeout_ms)
    {}
    virtual ~TcpClient()
    {
        Close();
    }

    int Connect(); // 利用ip地址建立连接
    int Connect(const string& host, const string& port);
    int ReconnectHost();
    int Send(const void* buf, size_t len);
    int Recv(void* buf, size_t len);
    int Writev(const struct iovec *iov, int iovcnt);
    int Readv(const struct iovec *iov, int iovcnt);
    void Close();   // 注意此处不要virtual，否则上面的函数有可能会调用子类的Close方法

    // 调用recv()系统函数测试socket是否alive
    bool IsRealAlive();
    // 仅测试fd是否存在.
    bool IsAlive();
    /**
     * param: 超时时间，注意以秒为单位。
     */
    int SetTimeout(int t_ms);

    int GetIpPort(uint32_t& ip, uint16_t& port);
private:
    int ConnectHost(); // 先解析域名，得到ip：port，然后连接。

public:
    int m_fd;
private:
    string m_str_host; // 域名地址
    string m_ip; // 由域名解析出来到ip地址
    string m_str_port;
    int m_port;

    int m_timeout_ms;

    // disable copy constructor
    TcpClient(const TcpClient& tc);
    TcpClient& operator = (const TcpClient& tc);
};

inline int TcpClient::ReconnectHost()
{
    if(IsRealAlive() == false)
    {
        DEBUG_LOG("disconnected to %s:%d, try to reconnecthost.", m_ip.c_str(), m_port);
        this->Close();
        this->ConnectHost();
    }

    return 0;
}

inline int TcpClient::Connect(const string& host, const string& port)
{
    m_str_host = host;
    m_str_port = port;

    return ConnectHost();
}

inline bool TcpClient::IsAlive()
{
    return (m_fd < 0) ? false : true;
}

inline void TcpClient::Close()
{
    if(m_fd >= 0)
    {
        ::shutdown(m_fd, SHUT_RDWR);

        if(::close(m_fd) != 0)
            ERROR_LOG("close socket failed: %s", strerror(errno));
        m_fd = -1;
    }
}
#endif
