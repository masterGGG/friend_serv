#ifndef SERVICE_H
#define SERVICE_H

#include <unordered_map>
#include <string>
#include <set>

#include "tcp_client.h"
#include "strategy.h"

using std::unordered_map;
using std::string;
using std::set;

class HostInfo
{
public:
    // HostInfo(){}
    HostInfo(const string &i, int p, int t = 2000, bool forbidden = false);
    bool operator == (const HostInfo &rhs) const {
        if (ip != rhs.ip 
            || port != rhs.port 
            || timeout_ms != rhs.timeout_ms
            || forbidden != rhs.forbidden) {
            return false;
        }
        return true;
    }
    bool operator != (const HostInfo &rhs) const {
        return !(*this == rhs);
    }
public:
    string  ip;
    int     port;
    int     timeout_ms;
    bool    forbidden;
};

struct StrategyConf
{
    // ServiceGroup选择ServiceReplicas的策略
    Strategy::strategy_type replicas_strategy;

    // ServiceReplicas选择Service的策略
    Strategy::strategy_type service_strategy;
};

struct Proto
{
    uint32_t len;
    uint32_t seq_num;
    char     buf[0];
};

class ReInitable
{
public:
    virtual ~ReInitable(){}
    virtual bool Init() = 0;
    virtual bool ReInit() = 0;
    static void ProcessReInit(); // timer中调用

public:
    // 需要注意，此static的set为所有子类所共享，继承自此类的子类须一定要注意
    static set<ReInitable*> reinit_set;

protected:
    static void SetToReInit(ReInitable *ri);
    static void CancelFromReInit(ReInitable *ri);
};


// service表示线上的一个服务，对于c++程序来说，对应一个IP + port
class Service : public ReInitable, public Element
{
public:
    // virtual void Reload(const HostInfo &hi);    // 隔一段时间拉取配置，若配置有变则重连
    void PrintStatusInfo();
    
protected:
    //Service(){}
    //Service(const string &ip, int port, int timeout_ms)
    //    :m_ip(ip),m_port(port),m_timeout_ms(timeout_ms){}
    Service(const HostInfo &hi):m_host_info(hi){}
    virtual void Shut() = 0;

protected:
    //string  m_ip;
    //int     m_port;
    //int     m_timeout_ms;
    //bool    m_forbidden;
    HostInfo  m_host_info;
};

class ISendAndRecv
{
public:
    virtual ~ISendAndRecv(){}
    virtual bool SendAndRecv(const void *in_buf, size_t in_len, void *out_buf, size_t out_len) = 0;
    virtual bool SendAndRecv(const void *in_buf, size_t in_len, void *out_buf, size_t out_buf_len, size_t &out_len) = 0;
};

class SyncService : public Service, public TcpClient, public ISendAndRecv
{
public:
    // SyncService(const string& ip, int port, int timeout_ms);
    SyncService(const HostInfo &hi);
    virtual ~SyncService();
    virtual bool Init();
    virtual bool ReInit();
    virtual void Shut();
    virtual bool SendAndRecv(const void *in_buf, size_t in_len, void *out_buf, size_t out_len);
    virtual bool SendAndRecv(const void *in_buf, size_t in_len, void *out_buf, size_t out_buf_len, size_t &out_len);
};

class ISend
{
public:
    virtual ~ISend(){}
    virtual bool Send(const void *buf, size_t len) = 0;
};

class AsyncService : public Service, public ISend
{
public:
    // AsyncService(const string& ip, int port, int timeout_ms);
    AsyncService(const HostInfo &hi);
    virtual ~AsyncService();
    virtual bool Init();
    virtual void LinkUp();
    virtual void Shut();
    virtual bool ReInit();
    virtual void LinkDown();
    virtual bool Send(const void *buf, size_t len);
    
    static void SetServiceConnected(int fd) {
        int enabled_count = 0;
        unordered_map<int, AsyncService*>::iterator it = fd_service_map.begin();
        for (; it != fd_service_map.end(); ++it) {
            if (it->second->IsEnable()) enabled_count++;
        }
        DEBUG_LOG("sizeof fd_service_map is %d, enabled_count is %d", (int)fd_service_map.size(), enabled_count);
        AsyncService * as = (AsyncService*)fd_service_map[fd];

        if (!as) {
            ERROR_LOG("AsyncService SetServiceConnected failed for Service not found, fd %d", fd);
            return;
        }
        as->LinkUp();
    }

    static void SetServiceDisconnected(int fd) {
        AsyncService * as = (AsyncService*)fd_service_map[fd];

        if (!as) {
            ERROR_LOG("AsyncService SetServiceConnected failed for Service not found, fd %d", fd);
            return;
        }
        as->LinkDown();
    }
    
    static AsyncService *GetServiceByFd(int fd){
        unordered_map<int, AsyncService*>::iterator it = fd_service_map.find(fd);
        if (it != fd_service_map.end()) {
            return it->second;
        }
        return NULL;
    }
    
public:
    static unordered_map<int, AsyncService*> fd_service_map; // 在link_up_ser中使用

private:
    int       m_fd;
    long      m_connect_time_ms;
    bool      m_connecting;
};

#endif
