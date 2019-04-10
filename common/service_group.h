#ifndef SERVICE_GROUP_H
#define SERVICE_GROUP_H

#include <unordered_map>
#include <string>
#include <set>

#include "service.h"
#include "strategy.h"
// #include "proto_handler.h"
#include "service_replicas.h"

using std::unordered_map;
using std::string;
using std::set;

class ServiceGroupInfo
{
public:
    ServiceGroupInfo():group_strategy(Strategy::STRATEGY_MIN),forbidden(false){}
    bool operator == (const ServiceGroupInfo& rhs) const {
        if (group_strategy != rhs.group_strategy ||
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
    bool operator != (const ServiceGroupInfo& rhs) const {
        return !(*this == rhs);
    }
public:
    vector<ServiceReplicasInfo> vec;
    Strategy::strategy_type group_strategy;
    bool forbidden;
};

class IProtoHandler;
class ISyncProtoHandler;
class IAsyncProtoHandler;

// 一组ServiceReplicas，包含多个ServiceReplicas，一起对外提供服务
class ServiceGroup : public Element
{
public:
    virtual ~ServiceGroup();
    void Close();
    virtual ServiceReplicas *CreateReplicas() = 0;
    // void Reload(const ServiceGroupInfo &sgi);   // 此函数在定时器中调用
    Strategy *CreateStrategy(Strategy::strategy_type strategy, 
                             const vector<Element*> &replicases);
    const ServiceGroupInfo &GetServiceGroupInfo() const;
    ServiceGroupInfo GetServiceGroupInfo() {
        return m_sgi;
    }
    void PrintStatusInfo();

protected:
    ServiceGroup();
    bool _Init(const ServiceGroupInfo &sgi);
    virtual bool CheckGroupInfo(const ServiceGroupInfo &sgi);
    virtual bool CheckStrategy(Strategy::strategy_type strategy) = 0;
    void ClearStrategy();
    void ClearReplicases();
    void SetInited(bool init = true){
        m_inited = init;   
    }
    bool IsInited() {
        return m_inited;
    }

protected:
    bool m_inited;
    ServiceGroupInfo m_sgi;
    Strategy *m_strategy;
    vector<Element*> m_replicases;
};

class ISyncServiceGroup : public ServiceGroup
{
public:
    bool Init(const ServiceGroupInfo &sgi);
    virtual ~ISyncServiceGroup(){}
protected:
    ISyncServiceGroup(){}

    virtual ServiceReplicas *CreateReplicas() {
        return new (std::nothrow) SyncServiceReplicas();
    }
};

class ISendAndRecvByNothing
{
public:
    virtual ~ISendAndRecvByNothing(){}
    virtual bool SendAndRecvByNothing(const void *in_buf, size_t in_len, 
                                      void *out_buf, size_t out_len) = 0;
    virtual bool SendAndRecvByNothing(const void *in_buf, size_t in_len, void *out_buf, 
                                      size_t out_buf_len, size_t &out_len) = 0;
};

class SyncServiceGroup : public ISyncServiceGroup, public ISendAndRecvByNothing
{
public:
    virtual bool SendAndRecvByNothing(const void *in_buf, size_t in_len, void *out_buf, size_t out_len);
    virtual bool SendAndRecvByNothing(const void *in_buf, size_t in_len, void *out_buf, size_t out_buf_len, size_t &out_len);

protected:
    virtual bool CheckStrategy(Strategy::strategy_type strategy) {
        if (strategy == Strategy::STRATEGY_FIXED ||
            strategy == Strategy::STRATEGY_ROLL) {
            return true;
        }
        return false;
    }
};

class ISendAndRecvByInt
{
public:
    virtual ~ISendAndRecvByInt(){}
    virtual bool SendAndRecvByInt(const void *in_buf, size_t in_len, void *out_buf, size_t out_len, int seed) = 0;
    virtual bool SendAndRecvByInt(const void *in_buf, size_t in_len, void *out_buf, size_t out_buf_len, size_t &out_len, int seed) = 0;
};

class IntSyncServiceGroup : public ISyncServiceGroup, public ISendAndRecvByInt
{
public:
    virtual bool SendAndRecvByInt(const void *in_buf, size_t in_len, void *out_buf, size_t out_len, int seed);
    virtual bool SendAndRecvByInt(const void *in_buf, size_t in_len, void *out_buf, size_t out_buf_len, size_t &out_len, int seed);

protected:
    virtual bool CheckStrategy(Strategy::strategy_type strategy) {
        if (strategy == Strategy::STRATETY_INT_REGION) {
            return true;
        }
        return false;
    }
};

class ISendAndRecvByString
{
public:
    virtual ~ISendAndRecvByString(){}
    virtual bool SendAndRecvByString(const void *in_buf, size_t in_len, void *out_buf, size_t out_len, const string &seed) = 0;
    virtual bool SendAndRecvByString(const void *in_buf, size_t in_len, void *out_buf, size_t out_buf_len, size_t &out_len, const string &seed) = 0;
};

class StringSyncServiceGroup : public ISyncServiceGroup, public ISendAndRecvByString
{
public:
    virtual bool SendAndRecvByString(const void *in_buf, size_t in_len, void *out_buf, size_t out_len, const string &seed);
    virtual bool SendAndRecvByString(const void *in_buf, size_t in_len, void *out_buf, size_t out_buf_len, size_t &out_len, const string &seed);

protected:
    virtual bool CheckStrategy(Strategy::strategy_type strategy) {
        if (strategy == Strategy::STRATETY_STRING_REGION) {
            return true;
        }
        return false;
    }
};

//////////////////////////////////////////////////////////////////////////////////

class IAsyncServiceGroup : public ServiceGroup
{
public:
    IAsyncServiceGroup():m_aph(NULL){}
    virtual ~IAsyncServiceGroup(){}
    virtual IAsyncProtoHandler *GetProtoHandler() {
        return m_aph;
    }
    virtual void SetProtoHandler(IAsyncProtoHandler *aph){
        m_aph = aph;
    }
    bool Init(const ServiceGroupInfo &sgi, IAsyncProtoHandler *aph);

protected:
    virtual ServiceReplicas *CreateReplicas() {
        return new (std::nothrow) AsyncServiceReplicas();
    }

protected:
    IAsyncProtoHandler *m_aph;
};

class AsyncServiceGroup : public IAsyncServiceGroup, public ISend
{
public:
    virtual bool Send(const void *in_buf, size_t in_len);

protected:
    virtual bool CheckStrategy(Strategy::strategy_type strategy){
        if (strategy == Strategy::STRATEGY_FIXED ||
            strategy == Strategy::STRATEGY_ROLL) {
            return true;
        }
        return false;
    }
};

class ISendByInt
{
public:
    virtual ~ISendByInt(){}
    virtual bool SendByInt(const void *in_buf, size_t in_len, int seed) = 0;
};

class IntAsyncServiceGroup : public IAsyncServiceGroup, public ISendByInt
{
public:
    virtual bool SendByInt(const void *in_buf, size_t in_len, int seed);

protected:
    virtual bool CheckStrategy(Strategy::strategy_type strategy){
        if (strategy == Strategy::STRATETY_INT_REGION) {
            return true;
        }
        return false;
    }
};

class ISendByString
{
public:
    virtual ~ISendByString(){}
    virtual bool SendByString(const void *in_buf, size_t in_len, const string &seed) = 0;
};

class StringAsyncServiceGroup : public IAsyncServiceGroup, public ISendByString
{
public:
    virtual bool SendByString(const void *in_buf, size_t in_len, const string &seed);

protected:
    virtual bool CheckStrategy(Strategy::strategy_type strategy){
        if (strategy == Strategy::STRATETY_STRING_REGION) {
            return true;
        }
        return false;
    }
};

#endif
