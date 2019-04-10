#include <sstream>
#include <vector>
#include <cstring>

#include <sys/types.h>
#include <sys/socket.h>
#include <iostream>

#include "common.h"
#include "service.h"
#include "service_group.h"
#include "async_server.h"
#include "time_utils.h"

using std::vector;

ServiceGroup::ServiceGroup()
:m_inited(false)
,m_strategy(NULL)
{
}


ServiceGroup::~ServiceGroup()
{
    Close();
}

void ServiceGroup::ClearReplicases()
{
    vector<Element*>::iterator it = m_replicases.begin();
    for (; it != m_replicases.end(); ) {
        if (*it) {
            delete (*it);
        }
        it = m_replicases.erase(it);
    }
}

void ServiceGroup::ClearStrategy()
{
    if (m_strategy) {
        delete m_strategy;
        m_strategy = NULL;
    }
}

const ServiceGroupInfo& ServiceGroup::GetServiceGroupInfo() const
{
    return m_sgi;
}

void ServiceGroup::Close()
{
    ClearStrategy();
    ClearReplicases();
    SetInited(false);
}

// 此函数会在定时器中调用
//void ServiceGroup::Reload(const ServiceGroupInfo &sgi)
//{
//    if (!IsInited()) {
//        ERROR_LOG("ServiceGroup Reload failed for not inited");
//        return;
//    }
//
//    // 简单处理
//    if (sgi.group_strategy != m_sgi.group_strategy || 
//        sgi.vec.size() != m_sgi.vec.size()) {
//        DEBUG_LOG("ServiceGroup configurtion changed, need to Reload");
//        Close();
//        m_sgi = sgi;
//        Init(m_sgi);
//        return;
//    }
//    
//    // 若replicas个数，以及group策略没有变化，则直接Reload下层
//    for (int i = 0; i < (int)m_sgi.vec.size(); ++i) {
//        ServiceReplicas *sr = (ServiceReplicas *)m_replicases[i];
//        sr->Reload(sgi.vec[i]);
//    }
//
//    return;
//}


//void ServiceGroup::Reload(const ServiceGroupInfo &sgi)
//{
//    //if (!IsInited()) {
//    //    ERROR_LOG("ServiceGroup Reload failed for not inited");
//    //    return;
//    //}
//    
//    // 先设置为未初始化，不可用
//    this->SetInited(false);
//
//    if (sgi.forbidden) {
//        Close();            // Send均返回false
//        return;
//    }
//    
//    // 处理vec
//    int len = std::min(sgi.vec.size(), m_sgi.vec.size());
//    for (int i = 0; i < len; ++i) {
//        m_sgi.vec[i].Reload(sgi.vec[i]);
//    }
//
//    if (sgi.vec.size() > m_sgi.vec.size()) {
//        // 增加了replicas
//
//    } 
//    
//    if (sgi.vec.size() < m_sgi.vec.size()){
//        // 减少了replicas
//        for (int i = sgi.vec.size(); i < m_sgi.vec.size(); ++i) {
//            ServiceReplicas *replicas = CreateReplicas();
//            
//            if (!replicas) {
//                ERROR_LOG("ServiceGroup Reload uncomplete for CreateReplicas failed");
//                continue;
//            }
//
//            if (!replicas->Init(*it)) {
//                ERROR_LOG("ServiceGroup _Init failed for replicas _Init failed");
//                Close();
//                return false;
//            }
//
//            this->AddChild(replicas);
//            m_replicases.push_back(replicas);
//        }
//    }
//
//    // sgi.vec = m_sgi.vec;
// 
//    // 最后处理strategy
//    if (sgi.group_strategy != m_sgi.group_strategy) {
//        ClearStrategy();
//        m_strategy = CreateStrategy(m_sgi.group_strategy, m_replicases);
//        if (!m_strategy) {
//            // TODO
//            Close();
//            return;
//        }
//    }
//
//    return;
//}


Strategy *ServiceGroup::CreateStrategy(Strategy::strategy_type strategy, 
                                       const vector<Element*> &replicases)
{
    Strategy *s = NULL;
    switch (strategy)
    {
        case Strategy::STRATEGY_FIXED:
            s = new (std::nothrow) FixedStrategy(replicases);
            break;
        case Strategy::STRATEGY_ROLL:
            s = new (std::nothrow) RollStrategy(replicases);
            break;
        case Strategy::STRATETY_INT_REGION:
            s = new (std::nothrow) IntRegionStrategy(replicases);
            break;
        case Strategy::STRATETY_STRING_REGION:
            s = new (std::nothrow) StringRegionStrategy(replicases);
            break;
        default:
            // TODO
            s = NULL;
    }
    return s;
}

bool ServiceGroup::CheckGroupInfo(const ServiceGroupInfo &sgi) {
    if (!CheckStrategy(sgi.group_strategy)) {
        return false;
    }
    return true;
}

bool ServiceGroup::_Init(const ServiceGroupInfo &sgi)
{
    if (IsInited()) {
        ERROR_LOG("ServiceGroup _Init failed for already inited");
        return false;
    }

    m_sgi = sgi;        // 此处operator=没有问题

    if (sgi.forbidden) {
        DEBUG_LOG("ServiceGroup _Init status forbidden");
        Close();        // 此时未初始化，Send会失败
        return true;
    }
    
    if (!CheckGroupInfo(sgi)) {
        ERROR_LOG("ServiceGroup _Init failed for CheckGroupInfo failed");
        return false;
    }

    vector<ServiceReplicasInfo>::const_iterator it = m_sgi.vec.begin();
    for (; it != m_sgi.vec.end(); ++it) {
        ServiceReplicas *replicas = CreateReplicas();

        if (!replicas) {
            ERROR_LOG("ServiceGroup _Init failed for CreateReplicas failed");
            Close();
            return false;
        }

        this->AddChild(replicas);
        
        if (!replicas->Init(*it)) {
            ERROR_LOG("ServiceGroup _Init failed for replicas _Init failed");
            Close();
            return false;
        }
		
        // 连接正常或下次重连
        m_replicases.push_back(replicas);
    }

    m_strategy = CreateStrategy(m_sgi.group_strategy, m_replicases);
    
    if (!m_strategy) {
        // TODO
        Close();
        return false;
    }

    SetInited();
    return true;
}

void ServiceGroup::PrintStatusInfo()
{
    DEBUG_LOG("==========ServiceGroup PrintStatusInfo===========");
    DEBUG_LOG("ServiceGroup stategy is %d", m_sgi.group_strategy);
    DEBUG_LOG("ServiceGroup %s", this->IsEnable() ? "enabled" : "disabled");
    vector<Element*>::iterator it = m_replicases.begin();
    for (; it != m_replicases.end(); ++it) {
        ServiceReplicas *sr = (ServiceReplicas*) *it;
        sr->PrintStatusInfo();
    }
    DEBUG_LOG("=================================================");
}


bool ISyncServiceGroup::Init(const ServiceGroupInfo &sgi) {
    return _Init(sgi);
}

bool SyncServiceGroup::SendAndRecvByNothing(const void *in_buf, size_t in_len, 
                                        void *out_buf, size_t out_len)
{
    if (!IsInited()) {
        // TODO
        return false;
    }

    //SyncServiceReplicas *chosen = NULL;
    //chosen = (SyncServiceReplicas*)m_strategy->ChooseByNothing();

    IChooseByNothing *c = dynamic_cast<IChooseByNothing*>(m_strategy);

    if (!c) {
        ERROR_LOG("SyncServiceGroup SendAndRecvByNothing failed for dynamic_cast m_strategy to IChooseByNothing failed");
        return false;
    }

    //SyncServiceReplicas *chosen = 
    //        dynamic_cast<SyncServiceReplicas*>(c->ChooseByNothing());
    ISendAndRecv *chosen = 
            dynamic_cast<ISendAndRecv*>(c->ChooseByNothing());

    if (!chosen) {
        ERROR_LOG("SyncServiceGroup SendAndRecvByNothing failed for ChooseByNothing get NULL or dynamic_cast to ISendAndRecv failed");
        return false;
    }

    return chosen->SendAndRecv(in_buf, in_len, out_buf, out_len);
}

bool SyncServiceGroup::SendAndRecvByNothing(const void *in_buf, size_t in_len, 
                                        void *out_buf, size_t out_buf_len, 
                                        size_t &out_len)
{
    if (!IsInited()) {
        // TODO
        return false;
    }

    //SyncServiceReplicas *chosen = NULL;
    //chosen = (SyncServiceReplicas*)m_strategy->ChooseByNothing();

    IChooseByNothing *c = dynamic_cast<IChooseByNothing*>(m_strategy);

    if (!c) {
        ERROR_LOG("SyncServiceGroup SendAndRecvByNothing failed for dynamic_cast m_strategy to IChooseByNothing failed");
        return false;
    }

    //SyncServiceReplicas *chosen = 
    //        dynamic_cast<SyncServiceReplicas*>(c->ChooseByNothing());
    ISendAndRecv *chosen = 
            dynamic_cast<ISendAndRecv*>(c->ChooseByNothing());

    if (!chosen) {
        ERROR_LOG("SyncServiceGroup SendAndRecvByNothing failed for ChooseByNothing get NULL or dynamic_cast to ISendAndRecv failed");
        return false;
    }

    return chosen->SendAndRecv(in_buf, in_len, out_buf, out_buf_len, out_len);
}

bool IntSyncServiceGroup::SendAndRecvByInt(const void *in_buf, size_t in_len, 
                                    void *out_buf, size_t out_len, int seed)
{
    if (!IsInited()) {
        // TODO
        return false;
    }

    //SyncServiceReplicas *chosen = NULL;
    //chosen = (SyncServiceReplicas*)m_strategy->ChooseByInt(seed);

    //if (!chosen) {
    //    ERROR_LOG("IntSyncServiceGroup SendAndRecvByInt failed for ChooseByInt get NULL");
    //    return false;
    //}

    IChooseByInt *c = dynamic_cast<IChooseByInt*>(m_strategy);

    if (!c) {
        ERROR_LOG("IntSyncServiceGroup SendAndRecvByInt failed for dynamic_cast m_strategy to IChooseByInt failed");
        return false;
    }

    //SyncServiceReplicas *chosen = 
    //        dynamic_cast<SyncServiceReplicas*>(c->ChooseByInt(seed));
    ISendAndRecv *chosen = 
            dynamic_cast<ISendAndRecv*>(c->ChooseByInt(seed));

    if (!chosen) {
        ERROR_LOG("IntSyncServiceGroup SendAndRecvByInt failed for ChooseByInt get NULL or dynamic_cast to ISendAndRecv failed");
        return false;
    }

    return chosen->SendAndRecv(in_buf, in_len, out_buf, out_len);
}

bool IntSyncServiceGroup::SendAndRecvByInt(const void *in_buf, size_t in_len, 
                                    void *out_buf, size_t out_buf_len,
                                    size_t &out_len, int seed)
{
    if (!IsInited()) {
        // TODO
        return false;
    }

    //SyncServiceReplicas *chosen = NULL;
    //chosen = (SyncServiceReplicas*)m_strategy->ChooseByInt(seed);

    //if (!chosen) {
    //    ERROR_LOG("IntSyncServiceGroup SendAndRecvByInt failed for ChooseByInt get NULL");
    //    return false;
    //}
    
    IChooseByInt *c = dynamic_cast<IChooseByInt*>(m_strategy);

    if (!c) {
        ERROR_LOG("IntSyncServiceGroup SendAndRecvByInt failed for dynamic_cast m_strategy to IChooseByInt failed");
        return false;
    }

    //SyncServiceReplicas *chosen = 
    //        dynamic_cast<SyncServiceReplicas*>(c->ChooseByInt(seed));
    ISendAndRecv *chosen = 
            dynamic_cast<ISendAndRecv*>(c->ChooseByInt(seed));

    if (!chosen) {
        ERROR_LOG("IntSyncServiceGroup SendAndRecvByInt failed for ChooseByInt get NULL or dynamic_cast to ISendAndRecv failed");
        return false;
    }

    return chosen->SendAndRecv(in_buf, in_len, out_buf, out_buf_len, out_len);
}

bool StringSyncServiceGroup::SendAndRecvByString(const void *in_buf, size_t in_len, 
                                       void *out_buf, size_t out_len, const string &seed)
{
    if (!IsInited()) {
        // TODO
        return false;
    }

    //SyncServiceReplicas *chosen = NULL;
    //chosen = (SyncServiceReplicas*)m_strategy->ChooseByString(seed);

    //if (!chosen) {
    //    ERROR_LOG("StringSyncServiceGroup SendAndRecvByString failed for ChooseByString get NULL");
    //    return false;
    //}

    IChooseByString *c = dynamic_cast<IChooseByString*>(m_strategy);

    if (!c) {
        ERROR_LOG("StringSyncServiceGroup SendAndRecvByString failed for dynamic_cast m_strategy to IChooseByString failed");
        return false;
    }

    //SyncServiceReplicas *chosen = 
    //        dynamic_cast<SyncServiceReplicas*>(c->ChooseByString(seed));
    ISendAndRecv *chosen = 
            dynamic_cast<ISendAndRecv*>(c->ChooseByString(seed));

    if (!chosen) {
        ERROR_LOG("StringSyncServiceGroup SendAndRecvByString failed for ChooseByString get NULL or dynamic_cast to ISendAndRecv failed");
        return false;
    }

    return chosen->SendAndRecv(in_buf, in_len, out_buf, out_len);
}

bool StringSyncServiceGroup::SendAndRecvByString(const void *in_buf, size_t in_len, 
                                       void *out_buf, size_t out_buf_len,
                                       size_t &out_len, const string &seed)
{
    if (!IsInited()) {
        // TODO
        return false;
    }

    //SyncServiceReplicas *chosen = NULL;
    //chosen = (SyncServiceReplicas*)m_strategy->ChooseByString(seed);

    //if (!chosen) {
    //    ERROR_LOG("StringSyncServiceGroup SendAndRecvByString failed for ChooseByString get NULL");
    //    return false;
    //}

    IChooseByString *c = dynamic_cast<IChooseByString*>(m_strategy);

    if (!c) {
        ERROR_LOG("StringSyncServiceGroup SendAndRecvByString failed for dynamic_cast m_strategy to IChooseByString failed");
        return false;
    }

    //SyncServiceReplicas *chosen = 
    //        dynamic_cast<SyncServiceReplicas*>(c->ChooseByString(seed));
    ISendAndRecv *chosen = 
            dynamic_cast<ISendAndRecv*>(c->ChooseByString(seed));

    if (!chosen) {
        ERROR_LOG("StringSyncServiceGroup SendAndRecvByString failed for ChooseByString get NULL or dynamic_cast to ISendAndRecv failed");
        return false;
    }

    return chosen->SendAndRecv(in_buf, in_len, out_buf, out_buf_len, out_len);
}

///////////////////////////////////////////////////////////////////////

bool IAsyncServiceGroup::Init(const ServiceGroupInfo &sgi, IAsyncProtoHandler *aph)
{
    if (!aph) {
        ERROR_LOG("IAsyncServiceGroup Init failed for NULL iaph");
        return false;
    }

    m_aph = aph;

    return _Init(sgi);
}

bool AsyncServiceGroup::Send(const void *in_buf, size_t in_len)
{
    if (!IsInited()) {
        // TODO
        return false;
    }

    //AsyncServiceReplicas *chosen = NULL;
    //chosen = (AsyncServiceReplicas*)m_strategy->ChooseByNothing();

    //if (!chosen) {
    //    ERROR_LOG("AsyncServiceGroup Send failed for ChooseByNothing get NULL");
    //    return false;
    //}

    IChooseByNothing *c = dynamic_cast<IChooseByNothing*>(m_strategy);

    if (!c) {
        ERROR_LOG("AsyncServiceGroup Send failed for dynamic_cast m_strategy to IChooseByNothing failed");
        return false;
    }

    //AsyncServiceReplicas *chosen = 
    //        dynamic_cast<AsyncServiceReplicas*>(c->ChooseByNothing());
    ISend *chosen = 
            dynamic_cast<ISend*>(c->ChooseByNothing());

    if (!chosen) {
        ERROR_LOG("AsyncServiceGroup Send failed for ChooseByNothing get NULL or dynamic_cast to ISend failed");
        return false;
    }

    return chosen->Send(in_buf, in_len);
}

bool IntAsyncServiceGroup::SendByInt(const void *in_buf, size_t in_len, int seed)
{
    if (!IsInited()) {
        ERROR_LOG("IntAsyncServiceGroup SendByInt failed for not init");
        return false;
    }

    //AsyncServiceReplicas *chosen = NULL;
    //chosen = (AsyncServiceReplicas*)m_strategy->ChooseByInt(seed);

    //if (!chosen) {
    //    ERROR_LOG("IntAsyncServiceGroup SendByInt failed for ChooseByInt get NULL");
    //    return false;
    //}

    IChooseByInt *c = dynamic_cast<IChooseByInt*>(m_strategy);

    if (!c) {
        ERROR_LOG("IntAsyncServiceGroup SendByInt failed for dynamic_cast m_strategy to IChooseByInt failed");
        return false;
    }

    //AsyncServiceReplicas *chosen = 
    //        dynamic_cast<AsyncServiceReplicas*>(c->ChooseByInt(seed));
    ISend *chosen = 
            dynamic_cast<ISend*>(c->ChooseByInt(seed));

    if (!chosen) {
        ERROR_LOG("IntAsyncServiceGroup SendByInt failed for ChooseByInt get NULL or dynamic_cast to ISend failed");
        return false;
    }

    return chosen->Send(in_buf, in_len);
}

bool StringAsyncServiceGroup::SendByString(const void *in_buf, size_t in_len, 
                                           const string &seed)
{
    if (!IsInited()) {
        // TODO
        return false;
    }

    //AsyncServiceReplicas *chosen = NULL;
    //chosen = (AsyncServiceReplicas*)m_strategy->ChooseByString(seed);

    //if (!chosen) {
    //    ERROR_LOG("StringAsyncServiceGroup SendByString failed for ChooseByString get NULL");
    //    return false;
    //}

    IChooseByString *c = dynamic_cast<IChooseByString*>(m_strategy);

    if (!c) {
        ERROR_LOG("StringAsyncServiceGroup SendByString failed for dynamic_cast m_strategy to IChooseByString failed");
        return false;
    }

    //AsyncServiceReplicas *chosen = 
    //        dynamic_cast<AsyncServiceReplicas*>(c->ChooseByString(seed));
    ISend *chosen = 
            dynamic_cast<ISend*>(c->ChooseByString(seed));

    if (!chosen) {
        ERROR_LOG("StringAsyncServiceGroup SendByString failed for ChooseByString get NULL or dynamic_cast to ISend failed");
        return false;
    }

    return chosen->Send(in_buf, in_len);
}






