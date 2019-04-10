#include <sstream>
#include <vector>
#include <cstring>

#include <sys/types.h>
#include <sys/socket.h>
#include <iostream>

#include "common.h"
#include "service.h"
#include "service_replicas.h"
#include "async_server.h"
#include "time_utils.h"

using std::vector;

ServiceReplicas::ServiceReplicas()
:m_inited(false)
,m_strategy(NULL)
{
}

ServiceReplicas::~ServiceReplicas()
{
    Close();
}

void ServiceReplicas::ClearStrategy()
{
    if (m_strategy) {
        delete m_strategy;
        m_strategy = NULL;
    }
}

void ServiceReplicas::ClearServices()
{
    vector<Element*>::iterator it = m_services.begin();
    for (; it != m_services.end(); ) {
        if (*it) {
            delete (*it);
        }
        it = m_services.erase(it);
    }
}

void ServiceReplicas::Close()
{
    ClearStrategy();
    ClearServices();
    SetInited(false);
}

//void ServiceReplicas::Reload(const ServiceReplicasInfo &sri)
//{
//    if (!IsInited()) {
//        ERROR_LOG("ServiceReplicas Reload failed for not inited");
//        return;
//    }
//
//    // 简单处理
//    if (sri.replicas_strategy != m_sri.replicas_strategy || 
//        sri.vec.size() != m_sri.vec.size()) {
//        DEBUG_LOG("ServiceReplicas configurtion changed, need to Reload");
//        Close();
//        m_sri = sri;
//        Init(m_sri);
//        return;
//    }
//    
//    // 若service个数，以及group策略没有变化，则直接Reload下层
//    for (int i = 0; i < (int)m_sri.vec.size(); ++i) {
//        Service *s = (Service *)m_services[i];
//        s->Reload(sri.vec[i]);
//    }
//
//    return;
//}

void ServiceReplicas::PrintStatusInfo()
{
    DEBUG_LOG("===ServiceReplicas PrintStatusInfo===");
    DEBUG_LOG("ServiceReplicas stategy is %d", m_sri.replicas_strategy);
    DEBUG_LOG("ServiceReplicas %s", this->IsEnable() ? "enabled" : "disabled");
    vector<Element*>::iterator it = m_services.begin();
    for (; it != m_services.end(); ++it) {
        Service *s = (Service*) *it;
        s->PrintStatusInfo();
    }
    DEBUG_LOG("=====================================");
}

bool ServiceReplicas::Init(const ServiceReplicasInfo &sri)
{
    if (IsInited()) {
        ERROR_LOG("ServiceReplicas Init failed for already inited");
        return false;
    }

    m_sri = sri;

    if (sri.forbidden) {
        DEBUG_LOG("ServiceReplicas Init status forbidden");
        Close();
        return true;
    }
    
    if(m_sri.replicas_strategy != Strategy::STRATEGY_FIXED &&
       m_sri.replicas_strategy != Strategy::STRATEGY_ROLL) {
        // TODO
        return false;
    }

    vector<HostInfo>::const_iterator it = m_sri.vec.begin();
    for (; it != m_sri.vec.end(); ++it) {
        //Service *service = new (std::nothrow) Service(it->ip, it->port, it->timeout_ms);
        // Service *service = CreateService(it->ip, it->port, it->timeout_ms);
        Service *service = CreateService(*it);
        if (!service) {
            // TODO
            Close();
            return false;
        }

        this->AddChild(service);

        if (!service->Init()) {
            ERROR_LOG("ServiceReplicas Init failed for Service Init failed");
            Close();
            return false;
        }
		
        // 连接正常或下次重连
        m_services.push_back(service);
    }

    switch (m_sri.replicas_strategy)
    {
        case Strategy::STRATEGY_FIXED:
            m_strategy = new (std::nothrow) FixedStrategy(m_services);
            break;
        case Strategy::STRATEGY_ROLL:
            m_strategy = new (std::nothrow) RollStrategy(m_services);
            break;
        default:
            // TODO
            m_strategy = NULL;
    }
    
    if (!m_strategy) {
        // TODO
        Close();
        return false;
    }

    SetInited();
    return true;
}

bool SyncServiceReplicas::SendAndRecv(const void *in_buf, size_t in_len, void *out_buf, size_t out_len)
{
    if (!IsInited()) {
        // TODO
        return false;
    }

    //SyncService *chosen = NULL;
    //chosen = (SyncService*)m_strategy->ChooseByNothing();
    //
    //if (!chosen) {
    //    ERROR_LOG("SyncServiceReplicas SendAndRec failed for ChooseByNothing get NULL");
    //    return false;
    //}

    IChooseByNothing *c = dynamic_cast<IChooseByNothing*>(m_strategy);

    if (!c) {
        ERROR_LOG("SyncServiceReplicas SendAndRecv failed for dynamic_cast m_strategy to IChooseByNothing failed");
        return false;
    }

    ISendAndRecv *chosen = 
            dynamic_cast<ISendAndRecv*>(c->ChooseByNothing());

    if (!chosen) {
        ERROR_LOG("SyncServiceReplicas SendAndRecv failed for ChooseByNothing get NULL or dynamic_cast to ISendAndRecv failed");
        return false;
    }

    return chosen->SendAndRecv(in_buf, in_len, out_buf, out_len);
}

bool SyncServiceReplicas::SendAndRecv(const void *in_buf, size_t in_len, void *out_buf, size_t out_buf_len, size_t &out_len)
{
    if (!IsInited()) {
        // TODO
        return false;
    }

    //SyncService *chosen = NULL;
    //chosen = (SyncService*)m_strategy->ChooseByNothing();
    //
    //if (!chosen) {
    //    ERROR_LOG("SyncServiceReplicas SendAndRec failed for ChooseByNothing get NULL");
    //    return false;
    //}

    IChooseByNothing *c = dynamic_cast<IChooseByNothing*>(m_strategy);

    if (!c) {
        ERROR_LOG("SyncServiceReplicas SendAndRecv failed for dynamic_cast m_strategy to IChooseByNothing failed");
        return false;
    }

    ISendAndRecv *chosen = 
            dynamic_cast<ISendAndRecv*>(c->ChooseByNothing());

    if (!chosen) {
        ERROR_LOG("SyncServiceReplicas SendAndRecv failed for ChooseByNothing get NULL or dynamic_cast to ISendAndRecv failed");
        return false;
    }

    return chosen->SendAndRecv(in_buf, in_len, out_buf, out_buf_len, out_len);
}

bool AsyncServiceReplicas::Send(const void *in_buf, size_t in_len)
{
    if (!IsInited()) {
        ERROR_LOG("AsyncServiceReplicas Send failed for not init");
        return false;
    }

    //AsyncService *chosen = NULL;
    //chosen = (AsyncService*)m_strategy->ChooseByNothing();
    //
    //if (!chosen) {
    //    ERROR_LOG("AsyncServiceReplicas Send failed for ChooseByNothing get NULL");
    //    return false;
    //}

    IChooseByNothing *c = dynamic_cast<IChooseByNothing*>(m_strategy);

    if (!c) {
        ERROR_LOG("AsyncServiceReplicas Send failed for dynamic_cast m_strategy to IChooseByNothing failed");
        return false;
    }

    ISend *chosen = 
            dynamic_cast<ISend*>(c->ChooseByNothing());

    if (!chosen) {
        ERROR_LOG("AsyncServiceReplicas Send failed for ChooseByNothing get NULL or dynamic_cast to ISend failed");
        return false;
    }

    return chosen->Send(in_buf, in_len);
}

