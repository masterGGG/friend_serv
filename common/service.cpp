#include <sstream>
#include <vector>
#include <cstring>

#include <sys/types.h>
#include <sys/socket.h>
#include <iostream>

#include "common.h"
#include "service.h"
#include "async_server.h"
#include "time_utils.h"

using std::vector;

HostInfo::HostInfo(const string &i, int p, int t, bool f)
:ip(i)
,port(p)
,timeout_ms(t)
,forbidden(f)
{}



set<ReInitable*> ReInitable::reinit_set;

// timer中调用，只有在timer中可以Enable Service
void ReInitable::ProcessReInit()
{
    set<ReInitable*>::iterator it = reinit_set.begin();
    for (; it != reinit_set.end();) {
        if (*it == NULL) {
            // TODO 不应该到此
            reinit_set.erase(it++);
            continue;
        }
        (*it)->ReInit();
        ++it;
    }
}

void ReInitable::SetToReInit(ReInitable *ri) 
{
    reinit_set.insert(ri);
}

void ReInitable::CancelFromReInit(ReInitable *ri) 
{
    reinit_set.erase(ri);
}



//void Service::Reload(const HostInfo &hi)
//{
//    if (hi.ip != m_ip ||
//        hi.port != m_port ||
//        hi.timeout_ms != m_host_info.timeout_ms) {
//        DEBUG_LOG("Service configuration changed, Service need to reload, 
//                  from %s %d %d to %s %d %d", m_ip.c_str(), m_port, 
//                  m_host_info.timeout_ms, hi.ip.c_str(), hi.port, hi.timeout_ms);
//        m_ip = hi.ip;
//        m_port = hi.port;
//        m_host_info.timeout_ms = hi.timeout_ms;
//        Shut();
//        Init();
//    }
//    return;
//}

void Service::PrintStatusInfo()
{
    DEBUG_LOG("Service %s %d %s", m_host_info.ip.c_str(), m_host_info.port, 
              this->IsEnable() ? "enabled" : "disabled");
}


// SyncService处理重连

//SyncService::SyncService(const string& ip, int port, int timeout_ms)
//:Service(ip, port, timeout_ms)
//,TcpClient(ip, port, timeout_ms)
//{
//}

SyncService::SyncService(const HostInfo &hi)
:Service(hi)
,TcpClient(hi.ip, hi.port, hi.timeout_ms)
{
}

SyncService::~SyncService()
{
    // 基类已经析构了，连接已关闭，其实只用CancelFromReInit(this)即可;
    Shut();
}

// 初始化时调用
bool SyncService::Init()
{
    if (this->IsEnable() || m_host_info.timeout_ms <=0 || m_host_info.timeout_ms > 10000) {
        // 正常情况不会是Enable状态
        ERROR_LOG("SyncService Init failed, current status %s, m_host_info.timeout_ms is %d", 
                  this->IsEnable() ? "enable" : "disable", m_host_info.timeout_ms);
        Shut();
        return false;
    }

    if (m_host_info.forbidden) {
        DEBUG_LOG("SyncService Init status forbidden");
        Shut();
        return true;
    }

    int ret = TcpClient::Connect();
    if (ret == TcpClient::TC_INVALID_PARAMS) {
        // 参数错误，无需重连
        // TODO
        return false;
    } else if (ret == TcpClient::TC_ERROR) {
        // 可能是暂时的网络问题，后续在timer中重连
        //this->Disable();
        Shut();
        ReInitable::SetToReInit(this);
        // 防止因部分服务暂时有网络问题而导致当前服务起不来
        return true;
    }
    
    // 连接正常

    // 或者简单做处理
    //if (ret <= 0) {
    //    return false;
    //}
    
    this->Enable();
    ReInitable::CancelFromReInit(this);

    return true;
}

bool SyncService::ReInit()
{
    Shut();
    return Init();
}

void SyncService::Shut()
{
    this->Disable();
    TcpClient::Close();
    ReInitable::CancelFromReInit(this);
}

bool SyncService::SendAndRecv(const void *in_buf, size_t in_len, void *out_buf, size_t out_len)
{
    unsigned int ret = TcpClient::Send(in_buf, in_len);

    //if (ret == TcpClient::TC_ERROR) {
    //    // 发送时出问题，已断开连接，后续需要在timer中启动
    //    Shut();
    //    SetToReInit(this);
    //    return false;
    //} else if (ret == TcpClient::TC_TIMEOUT) {
    //    // 第一次发送即失败，并未更改状态，待对端压力缓解即可继续传输
    //    return false;
    //} else if (ret == TcpClient::TC_INVALID_PARAMS){
    //    // 传入参数有误
    //    return false;
    //}
    
    if (ret != in_len) {
        Shut();
        SetToReInit(this);
        return false;
    }

    ret = TcpClient::Recv(out_buf, out_len);

    if (ret != out_len) {
        Shut();
        SetToReInit(this);
        return false;
    }

    return true;
}


// 返回协议前4个字节表示长度
bool SyncService::SendAndRecv(const void *in_buf, size_t in_len, void *out_buf, size_t out_buf_len, size_t &out_len)
{
    if (out_buf_len < sizeof(Proto)) {
        // TODO
        return false;
    }

    unsigned int ret = TcpClient::Send(in_buf, in_len);

    //if (ret == TcpClient::TC_ERROR) {
    //    // 发送时出问题，已断开连接，后续需要在timer中启动
    //    Shut();
    //    SetToReInit(this);
    //    return false;
    //} else if (ret == TcpClient::TC_TIMEOUT) {
    //    // 第一次发送即失败，并未更改状态，待对端压力缓解即可继续传输
    //    return false;
    //} else if (ret == TcpClient::TC_INVALID_PARAMS){
    //    // 传入参数有误
    //    return false;
    //}
    
    //if (ret != in_len) {
    //    Shut();
    //    SetToReInit(this);
    //    return false;
    //}

    //// 固定先接收4个字节
    //ret = TcpClient::Recv(out_buf, 4);

    //if (ret != 4) {
    //    Shut();
    //    SetToReInit(this);
    //    return false;
    //}
    //
    //Proto *p = (Proto*)out_buf;
    //if (p->len < sizeof(Proto) || p->len > out_buf_len) {
    //    Shut();
    //    SetToReInit(this);
    //    return false;
    //}

    //ret = TcpClient::Recv((uint8_t*)out_buf + 4, p->len - 4);

    //if (ret != p->len - 4) {
    //    Shut();
    //    SetToReInit(this);
    //    return false;
    //}

    //out_len = p->len;

    //return true;

    ///////////////////////////////////////////////
    do {
        if (ret != in_len) {
            break;
        }
        
        size_t proto_len = sizeof(((Proto*)0)->len);

        // 固定先接收4个字节
        ret = TcpClient::Recv(out_buf, proto_len);

        if (ret != proto_len) {
            break;
        }

        Proto *p = (Proto*)out_buf;
        if (p->len < sizeof(Proto) || p->len > out_buf_len) {
            break;
        }

        ret = TcpClient::Recv((uint8_t*)out_buf + proto_len, p->len - proto_len);

        if (ret != p->len - proto_len) {
            break;
        }

        out_len = p->len;
        return true;
    } while (0);

    Shut();
    SetToReInit(this);
    return false;
}

/////////////////////////////////////////////////////////////////////////////
unordered_map<int, AsyncService*> AsyncService::fd_service_map;

//AsyncService::AsyncService(const string &ip, int port, int timeout_ms)
//:Service(ip, port, timeout_ms)
//,m_fd(-1)
//,m_connect_time_ms(0)
//,m_connecting(false)
//{
//}

AsyncService::AsyncService(const HostInfo &hi)
:Service(hi)
,m_fd(-1)
,m_connect_time_ms(0)
,m_connecting(false)
{
}

AsyncService::~AsyncService()
{
    Shut();
}
bool AsyncService::Init()
{
    if (this->IsEnable() || m_host_info.timeout_ms <=0 || m_host_info.timeout_ms > 10000) {
        // 正常情况不会是Enable状态
        ERROR_LOG("AsyncService Init failed, current status %s, m_host_info.timeout_ms is %d", 
                  this->IsEnable() ? "enable" : "disable", m_host_info.timeout_ms);
        Shut();
        return false;
    }

    if (m_host_info.forbidden) {
        DEBUG_LOG("AsyncService Init status forbidden");
        Shut();
        return true;
    }
    
    if (m_connecting) {
        // 此时正在连接，且Disable的轮询到此，m_fd >= 0
        if (Common::GetCurrentTimeMs() - m_connect_time_ms <= m_host_info.timeout_ms) {
            // 若未超时，则不做处理，下次再连
            DEBUG_LOG("AsyncService Init now connecting and not timeout, m_fd %d, ip %s, port %d",
                      m_fd, m_host_info.ip.c_str(), m_host_info.port);
            return true;
        } else {
            // 若超时了，则断开重连，重新异步连接
            DEBUG_LOG("AsyncService Init now connecting and timeout, m_fd %d, ip %s, port %d", m_fd, m_host_info.ip.c_str(), m_host_info.port);
            Shut();
        }
    }

    m_fd = async_net_connect_ser(m_host_info.ip.c_str(), m_host_info.port);

    if (m_fd == -1) {
        // 连接失败，也返回成功，后续重连
        DEBUG_LOG("AsyncService Init async_connect return -1 set to reinit");
        m_connecting = false;
        ReInitable::SetToReInit(this);
        return true;
    } else if (m_fd < 0) {  // "<"还是"<="?
        // 参数错误，重连无效
        ERROR_LOG("AsyncService Init failed for async_net_connect_ser return %d, ip is %s, port is %d", m_fd, m_host_info.ip.c_str(), m_host_info.port);
        m_connecting = false;
        ReInitable::CancelFromReInit(this);
        return false;
    }

    // 连接请求发送成功，等待后续连接成功的消息，此时m_fd有效
    m_connect_time_ms = Common::GetCurrentTimeMs();
    m_connecting = true;
    ReInitable::SetToReInit(this);

    DEBUG_LOG("add fd %d to fd_service_map for link up, dest ip %s, port %d", 
              m_fd, m_host_info.ip.c_str(), m_host_info.port);
    fd_service_map.insert(std::make_pair(m_fd, this));

    return true;
}

// 检测到连接已建立，此时tcp_async_linker已建立并此fd已加入epoll
void AsyncService::LinkUp()
{
    DEBUG_LOG("AsyncService LinkUp, m_fd %d, ip %s, port %d", 
              m_fd, m_host_info.ip.c_str(), m_host_info.port);
    m_connecting = false;
    this->Enable();
    ReInitable::CancelFromReInit(this);
}

void AsyncService::Shut()
{
    DEBUG_LOG("AsyncService Shut  m_fd %d, ip %s, port %d",
              m_fd, m_host_info.ip.c_str(), m_host_info.port);
    if (m_fd >= 0) {
        net_close_ser(m_fd);
        fd_service_map.erase(m_fd);
    }
    
    this->Disable();
    m_connecting = false;
    m_fd = -1;
    ReInitable::CancelFromReInit(this);
}

bool AsyncService::ReInit()
{
    DEBUG_LOG("AsyncService ReInit ip is %s, port is %d", m_host_info.ip.c_str(), m_host_info.port);
    this->Disable();

    if (!m_connecting) {
        Shut();
    }

    return Init();
}

// 被动关闭，可能在发送、接收、或是刚建立好连接之后
void AsyncService::LinkDown()
{
    // 此时不能ReInit()，因为link_down_ser()后会关闭连接及其资源，导致net_close_ser重复析构
    // 只能在下一个时间片去重连
    // 只需要修改AsyncService类的状态
    if (m_fd >= 0) {
        fd_service_map.erase(m_fd);
    }

    this->Disable();
    m_connecting = false;
    m_fd = -1;
    ReInitable::SetToReInit(this);
}

bool AsyncService::Send(const void *buf, size_t len)
{
    if (!buf || len <= 0) {
        ERROR_LOG("AsyncService Send failed for invalid param, buf is %s, len is %d", 
                  buf ? "not NULL" : "NULL, len", (int)len);
        // return -2;
        return false;
    }
    
    int ret = net_send_ser(m_fd, buf, len);
    
    if (ret <= 0) {
        Shut();
        ReInitable::SetToReInit(this);
        // return -1;
        return false;
    }

    // return ret;
    return true;
}

