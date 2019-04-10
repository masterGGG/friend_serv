#include "proto_handler.h"

ISyncProtoHandler::~ISyncProtoHandler() {
    if (m_sg) {
        delete m_sg;
    }
}

bool ISyncProtoHandler::Init(const ServiceGroupInfo &sgi)
{
    m_sg = CreateServiceGroup();
    if (!m_sg) {
        ERROR_LOG("ISyncProtoHandler Init failed for CreateServiceGroup failed");
        return false;
    }

    if (!m_sg->Init(sgi)) {
        ERROR_LOG("ISyncProtoHandler Init failed for ISyncServiceGroup Init failed");
        return false;
    }
    
    m_inited = true;
    return true;
}

bool ISyncProtoHandler::ReloadServiceGroup(const ServiceGroupInfo &sgi)
{
    // TODO 后续增加strategy类型以及replicas数目的检查
    
    // 必须初始化以后才能Reload
    if (!IsInited()) {
        ERROR_LOG("ISyncProtoHandler ReloadServiceGroup failed for ISyncProtoHandler not inited");
        return false;
    }

    if (sgi == m_sg->GetServiceGroupInfo()) {
        DEBUG_LOG("ISyncProtoHandler ReloadServiceGroup ServiceGroup not change");
        return true;
    }

    ISyncServiceGroup *sg = CreateServiceGroup();
    if (!sg) {
        ERROR_LOG("ISyncProtoHandler ReloadServiceGroup failed for CreateServiceGroup failed");
        return false;
    }

    if (!sg->Init(sgi)) {
        ERROR_LOG("ISyncProtoHandler ReloadServiceGroup failed for ISyncServiceGroup Init failed");
        return false;
    }

    delete m_sg;
    m_sg = sg;

    return true;
}

void ISyncProtoHandler::PrintStatusInfo()
{
    if (m_sg) {
        m_sg->PrintStatusInfo();
    }
}

ISyncServiceGroup *SyncProtoHandler::CreateServiceGroup() {
    return new (std::nothrow) SyncServiceGroup();
}

bool SyncProtoHandler::SendAndRecvByNothing(const void *in_buf, size_t in_len, 
                                        void *out_buf, size_t out_len)
{
    if (IsInited()) {
        // SyncServiceGroup *asg = dynamic_cast<SyncServiceGroup*>(m_sg);
        ISendAndRecvByNothing *asg = dynamic_cast<ISendAndRecvByNothing*>(m_sg);
        if (!asg) {
            ERROR_LOG("SyncProtoHandler Send failed for dynamic_cast m_sg to ISendAndRecvByNothing failed");
            return false;
        }
        return asg->SendAndRecvByNothing(in_buf, in_len, out_buf, out_len);
    }

    return false;
}

bool SyncProtoHandler::SendAndRecvByNothing(const void *in_buf, size_t in_len, 
                                        void *out_buf, size_t out_buf_len,
                                        size_t &out_len) 
{
    if (IsInited()) {
        // SyncServiceGroup *asg = dynamic_cast<SyncServiceGroup*>(m_sg);
        ISendAndRecvByNothing *asg = dynamic_cast<ISendAndRecvByNothing*>(m_sg);
        if (!asg) {
            ERROR_LOG("SyncProtoHandler Send failed for dynamic_cast m_sg to ISendAndRecvByNothing failed");
            return false;
        }
        return asg->SendAndRecvByNothing(in_buf, in_len, out_buf, out_buf_len, out_len);
    }

    return false;
}

ISyncServiceGroup *IntSyncProtoHandler::CreateServiceGroup() {
    return new (std::nothrow) IntSyncServiceGroup();
}

bool IntSyncProtoHandler::SendAndRecvByInt(const void *in_buf, size_t in_len, 
                                        void *out_buf, size_t out_len, int seed)
{
    if (IsInited()) {
        // IntSyncServiceGroup *asg = dynamic_cast<IntSyncServiceGroup*>(m_sg);
        ISendAndRecvByInt *asg = dynamic_cast<ISendAndRecvByInt*>(m_sg);
        if (!asg) {
            ERROR_LOG("IntSyncProtoHandler Send failed for dynamic_cast m_sg to ISendAndRecvByInt failed");
            return false;
        }
        return asg->SendAndRecvByInt(in_buf, in_len, out_buf, out_len, seed);
    }

    return false;
}

bool IntSyncProtoHandler::SendAndRecvByInt(const void *in_buf, size_t in_len, 
                                        void *out_buf, size_t out_buf_len,
                                        size_t &out_len, int seed)
{
    if (IsInited()) {
        // IntSyncServiceGroup *asg = dynamic_cast<IntSyncServiceGroup*>(m_sg);
        ISendAndRecvByInt *asg = dynamic_cast<ISendAndRecvByInt*>(m_sg);
        if (!asg) {
            ERROR_LOG("IntSyncProtoHandler Send failed for dynamic_cast m_sg to ISendAndRecvByInt failed");
            return false;
        }
        return asg->SendAndRecvByInt(in_buf, in_len, out_buf, 
                                     out_buf_len, out_len, seed);
    }

    return false;
}

ISyncServiceGroup *StringSyncProtoHandler::CreateServiceGroup() {
    return new (std::nothrow) StringSyncServiceGroup();
}

bool StringSyncProtoHandler::SendAndRecvByString(const void *in_buf, size_t in_len, 
                                      void *out_buf, size_t out_len, const string &seed)
{
    if (IsInited()) {
        // StringSyncServiceGroup *asg = dynamic_cast<StringSyncServiceGroup*>(m_sg);
        ISendAndRecvByString *asg = dynamic_cast<ISendAndRecvByString*>(m_sg);
        if (!asg) {
            ERROR_LOG("StringSyncProtoHandler Send failed for dynamic_cast m_sg to ISendAndRecvByString failed");
            return false;
        }
        return asg->SendAndRecvByString(in_buf, in_len, out_buf, out_len, seed);
    }

    return false;
}

bool StringSyncProtoHandler::SendAndRecvByString(const void *in_buf, size_t in_len, 
                                                 void *out_buf, size_t out_buf_len,
                                                 size_t &out_len, const string &seed)
{
    if (IsInited()) {
        // StringSyncServiceGroup *asg = dynamic_cast<StringSyncServiceGroup*>(m_sg);
        ISendAndRecvByString *asg = dynamic_cast<ISendAndRecvByString*>(m_sg);
        if (!asg) {
            ERROR_LOG("StringSyncProtoHandler Send failed for dynamic_cast m_sg to ISendAndRecvByString failed");
            return false;
        }
        return asg->SendAndRecvByString(in_buf, in_len, out_buf, 
                                        out_buf_len, out_len, seed);
    }

    return false;
}

/////////////////////////////////////////////////////////////
IAsyncProtoHandler::~IAsyncProtoHandler() {
    if (m_sg) {
        delete m_sg;
    }
}

bool IAsyncProtoHandler::Init(const ServiceGroupInfo &sgi)
{
    m_sg = CreateServiceGroup();
    if (!m_sg) {
        ERROR_LOG("IAsyncProtoHandler Init failed for CreateServiceGroup failed");
        return false;
    }

    if (!m_sg->Init(sgi, this)) {
        ERROR_LOG("IAsyncProtoHandler Init failed for AsyncServiceGroup Init failed");
        return false;
    }
    
    m_inited = true;
    return true;
}

bool IAsyncProtoHandler::ReloadServiceGroup(const ServiceGroupInfo &sgi)
{
    // TODO 后续增加strategy类型以及replicas数目的检查
    
    // 必须初始化以后才能Reload
    if (!IsInited()) {
        ERROR_LOG("IAsyncProtoHandler ReloadServiceGroup failed for IAsyncProtoHandler not inited");
        return false;
    }

    //const ServiceGroupInfo &tmp = m_sg->GetServiceGroupInfo();
    if (sgi == m_sg->GetServiceGroupInfo()) {
        DEBUG_LOG("IAsyncProtoHandler ReloadServiceGroup ServiceGroup not change");
        return true;
    }

    IAsyncServiceGroup *sg = CreateServiceGroup();
    if (!sg) {
        ERROR_LOG("IAsyncProtoHandler ReloadServiceGroup failed for CreateServiceGroup failed");
        return false;
    }

    if (!sg->Init(sgi, this)) {
        ERROR_LOG("IAsyncProtoHandler ReloadServiceGroup failed for IAsyncServiceGroup Init failed");
        return false;
    }

    delete m_sg;
    m_sg = sg;

    return true;
}

void IAsyncProtoHandler::PrintStatusInfo()
{
    if (m_sg) {
        m_sg->PrintStatusInfo();
    }
}

//AsyncProtoHandler::~AsyncProtoHandler(){
//    if (m_sg) {
//        delete m_sg;
//    }
//}

//bool AsyncProtoHandler::Init(const ServiceGroupInfo &sgi)
//{
//    m_sg = new (std::nothrow) AsyncServiceGroup();
//    if (!m_sg) {
//        ERROR_LOG("AsyncProtoHandler Init failed for new AsyncServiceGroup failed");
//        return false;
//    }
//
//    if (!m_sg->Init(sgi, this)) {
//        ERROR_LOG("AsyncProtoHandler Init failed for AsyncServiceGroup Init failed");
//        return false;
//    }
//    
//    // sg->SetProtoHandler(this);
//    m_inited = true;
//    return true;
//}

IAsyncServiceGroup *AsyncProtoHandler::CreateServiceGroup() {
    return new (std::nothrow) AsyncServiceGroup();
}

bool AsyncProtoHandler::Send(const void *in_buf, size_t in_len) 
{
    if (IsInited()) {
        // AsyncServiceGroup *asg = dynamic_cast<AsyncServiceGroup*>(m_sg);
        ISend *asg = dynamic_cast<ISend*>(m_sg);
        if (!asg) {
            ERROR_LOG("AsyncProtoHandler Send failed for dynamic_cast m_sg to ISend failed");
            return false;
        }
        return asg->Send(in_buf, in_len);
    }

    return false;
}

//IntAsyncProtoHandler::~IntAsyncProtoHandler()
//{
//    if (m_sg) {
//        delete m_sg;
//    }
//}

//bool IntAsyncProtoHandler::Init(const ServiceGroupInfo &sgi)
//{
//    m_sg = new (std::nothrow) IntAsyncServiceGroup();
//    if (!m_sg) {
//        ERROR_LOG("IntAsyncProtoHandler Init failed for new IntAsyncServiceGroup failed");
//        return false;
//    }
//
//    if (!m_sg->Init(sgi, this)) {
//        ERROR_LOG("IntAsyncProtoHandler Init failed for IntAsyncServiceGroup Init failed");
//        return false;
//    }
//    
//    // sg->SetProtoHandler(this);
//    m_inited = true;
//    return true;
//}
    
IAsyncServiceGroup *IntAsyncProtoHandler::CreateServiceGroup() {
    return new (std::nothrow) IntAsyncServiceGroup();
}

bool IntAsyncProtoHandler::SendByInt(const void *in_buf, size_t in_len, int seed)
{
    if (IsInited()) {
        // return m_sg->SendByInt(in_buf, in_len, seed);
        //IntAsyncServiceGroup *asg = dynamic_cast<IntAsyncServiceGroup*>(m_sg);
        ISendByInt *asg = dynamic_cast<ISendByInt*>(m_sg);
        if (!asg) {
            ERROR_LOG("IntAsyncProtoHandler Send failed for dynamic_cast m_sg to ISendByInt failed");
            return false;
        }
        return asg->SendByInt(in_buf, in_len, seed);
    }

    return false;
}


//StringAsyncProtoHandler::~StringAsyncProtoHandler()
//{
//    if (m_sg) {
//        delete m_sg;
//    }
//}

//bool StringAsyncProtoHandler::Init(const ServiceGroupInfo &sgi)
//{
//    m_sg = new (std::nothrow) StringAsyncServiceGroup();
//    if (!m_sg) {
//        ERROR_LOG("StringAsyncProtoHandler Init failed for new StringAsyncServiceGroup failed");
//        return false;
//    }
//
//    if (!m_sg->Init(sgi, this)) {
//        ERROR_LOG("StringAsyncProtoHandler Init failed for StringAsyncServiceGroup Init failed");
//        return false;
//    }
//    
//    // sg->SetProtoHandler(this);
//    m_inited = true;
//    return true;
//}
    
IAsyncServiceGroup *StringAsyncProtoHandler::CreateServiceGroup() {
    return new (std::nothrow) StringAsyncServiceGroup();
}
    
bool StringAsyncProtoHandler::SendByString(const void *in_buf, size_t in_len, const string &seed)
{
    if (IsInited()) {
        // return m_sg->SendByString(in_buf, in_len, seed);
        // StringAsyncServiceGroup *asg = dynamic_cast<StringAsyncServiceGroup*>(m_sg);
        ISendByString *asg = dynamic_cast<ISendByString*>(m_sg);
        if (!asg) {
            ERROR_LOG("StringAsyncProtoHandler Send failed for dynamic_cast m_sg to StringAsyncServiceGroup failed");
            return false;
        }
        return asg->SendByString(in_buf, in_len, seed);
    }

    return false;
}
