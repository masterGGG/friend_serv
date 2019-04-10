#ifndef SERVICE_REPLICAS_H
#define SERVICE_REPLICAS_H

#include <unordered_map>
#include <string>
#include <set>

#include "service.h"
#include "strategy.h"

using std::unordered_map;
using std::string;
using std::set;

class ServiceReplicasInfo
{
public:
    ServiceReplicasInfo():replicas_strategy(Strategy::STRATEGY_MIN), forbidden(false){}

    bool operator == (const ServiceReplicasInfo& rhs) const {
        if (replicas_strategy != rhs.replicas_strategy ||
            forbidden != rhs.forbidden ||     
            vec.size() != rhs.vec.size()) {
            return false;
        }

        for (int i = 0; i < (int)vec.size(); ++i) {
            if (vec[i] != rhs.vec[i]) {
                return false;
            }
        }
        return true;
    }
    bool operator != (const ServiceReplicasInfo& rhs) const {
        return !(*this == rhs);
    }
public:
    vector<HostInfo> vec;
    Strategy::strategy_type replicas_strategy;
    bool forbidden;
};

// 一组service备份，包含多个service
class ServiceReplicas : public Element
{
public:
    virtual ~ServiceReplicas();
    bool Init(const ServiceReplicasInfo &sri);
    void Close();
    //virtual Service *CreateService(const string& ip, int port, int timeout_ms) = 0;
    virtual Service *CreateService(const HostInfo &hi) = 0;
    //void Reload(const ServiceReplicasInfo &sri);
    void PrintStatusInfo();

protected:
    ServiceReplicas();
    bool IsInited() {
        return m_inited;
    }
    void ClearStrategy();
    void ClearServices();
    void SetInited(bool init = true){
        m_inited = init;   
    }

protected:
    bool m_inited;
    ServiceReplicasInfo m_sri;
    Strategy *m_strategy;
    vector<Element*> m_services;
};

class SyncServiceReplicas : public ServiceReplicas, public ISendAndRecv
{
public:
    // 不需要支持根据seed来进行选择
    virtual bool SendAndRecv(const void *in_buf, size_t in_len, void *out_buf, size_t out_len);
    virtual bool SendAndRecv(const void *in_buf, size_t in_len, void *out_buf, size_t out_buf_len, size_t &out_len);

protected:
    //virtual Service *CreateService(const string& ip, int port, int timeout_ms){
    //    return new (std::nothrow) SyncService(ip, port, timeout_ms);
    //}
    virtual Service *CreateService(const HostInfo &hi){
        return new (std::nothrow) SyncService(hi);
    }
};

class AsyncServiceReplicas : public ServiceReplicas, public ISend
{
public:
    virtual bool Send(const void *in_buf, size_t in_len);

protected:
    //virtual Service *CreateService(const string& ip, int port, int timeout_ms){
    //    return new (std::nothrow) AsyncService(ip, port, timeout_ms);
    //}
    virtual Service *CreateService(const HostInfo &hi){
        return new (std::nothrow) AsyncService(hi);
    }
};

#endif
