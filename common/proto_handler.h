#ifndef PROTO_HANDLER_H
#define PROTO_HANDLER_H

#include "service_group.h"

class ServiceGroupInfo;
class ISyncServiceGroup;
class IAsyncServiceGroup;
class AsyncServiceGroup;
class IntAsyncServiceGroup;
class StringAsyncServiceGroup;

class IProtoHandler
{
public:
    IProtoHandler():m_inited(false){}
    virtual ~IProtoHandler(){}
    virtual bool Init(const ServiceGroupInfo &sgi) = 0;
    bool IsInited() {
        return m_inited;
    }
    virtual bool ReloadServiceGroup(const ServiceGroupInfo &sgi) = 0;
    virtual void PrintStatusInfo() = 0;

    // 真正的逻辑代码，各个ServiceGroup有不同的处理函数，名称各有不同
    // proc_pkg_cli调用或是proc_pkg_ser中在其他ProtoHandler中调用
    
protected:
    bool m_inited;
};

class ISyncProtoHandler : public IProtoHandler
{
public:
    virtual ~ISyncProtoHandler();
    virtual bool Init(const ServiceGroupInfo &sgi);
    virtual bool ReloadServiceGroup(const ServiceGroupInfo &sgi);
    virtual ISyncServiceGroup *CreateServiceGroup() = 0;
    virtual void PrintStatusInfo();
protected:
    ISyncServiceGroup *m_sg;
};

//class ISendAndRecvByNothing
//{
//public:
//    virtual ~ISendAndRecvByNothing(){}
//    virtual bool SendAndRecvByNothing(const void *in_buf, size_t in_len, 
//                                      void *out_buf, size_t out_len) = 0;
//    virtual bool SendAndRecvByNothing(const void *in_buf, size_t in_len, void *out_buf, 
//                                      size_t out_buf_len, size_t &out_len) = 0;
//};

//class ISendAndRecvByNothing
//{
//pubilc:
//    virtual ~ISendAndRecvByNothing(){}
//    virtual bool SendAndRecvByNothing(const void *in_buf, size_t in_len, 
//                                      void *out_buf, size_t out_len) = 0;
//    virtual bool SendAndRecvByNothing(const void *in_buf, size_t in_len, void *out_buf, 
//                                      size_t out_buf_len, size_t &out_len) = 0;
//};

class SyncProtoHandler : public ISyncProtoHandler, public ISendAndRecvByNothing
{
public:
    SyncProtoHandler(){}
    virtual ISyncServiceGroup *CreateServiceGroup();

protected:
    virtual bool SendAndRecvByNothing(const void *in_buf, size_t in_len, void *out_buf, size_t out_len);
    virtual bool SendAndRecvByNothing(const void *in_buf, size_t in_len, void *out_buf, size_t out_buf_len, size_t &out_len);
};


class IntSyncProtoHandler : public ISyncProtoHandler, public ISendAndRecvByInt
{
public:
    IntSyncProtoHandler(){}
    virtual ISyncServiceGroup *CreateServiceGroup();

protected:
    virtual bool SendAndRecvByInt(const void *in_buf, size_t in_len, void *out_buf, size_t out_len, int seed);
    virtual bool SendAndRecvByInt(const void *in_buf, size_t in_len, void *out_buf, size_t out_buf_len, size_t &out_len, int seed);
};


class StringSyncProtoHandler : public ISyncProtoHandler, public ISendAndRecvByString
{
public:
    StringSyncProtoHandler(){}
    virtual ISyncServiceGroup *CreateServiceGroup();

protected:
    virtual bool SendAndRecvByString(const void *in_buf, size_t in_len, void *out_buf, size_t out_len, const string &seed);
    virtual bool SendAndRecvByString(const void *in_buf, size_t in_len, void *out_buf, size_t out_buf_len, size_t &out_len, const string &seed);

};

//////////////////////////////////////////////////////////////////

class IAsyncProtoHandler : public IProtoHandler
{
public:
    virtual ~IAsyncProtoHandler();
    virtual bool Init(const ServiceGroupInfo &sgi);
    virtual bool ReloadServiceGroup(const ServiceGroupInfo &sgi);
    // proc_pkg_ser通过fd找到此ProtoHandler后的处理
    virtual int GetPkgLenSer(const char *buf, uint32_t len) = 0;
    virtual void ProcPkgSer(const char *buf, uint32_t len) = 0;
    virtual bool GetSeqNum(const char *buf, uint32_t len, uint32_t &seq_num) = 0;
    virtual IAsyncServiceGroup *CreateServiceGroup() = 0;
    virtual void PrintStatusInfo();
    
protected:
    IAsyncServiceGroup *m_sg;
};

class AsyncProtoHandler : public IAsyncProtoHandler
{
public:
    AsyncProtoHandler(){}
    //virtual ~AsyncProtoHandler();   
    //virtual bool Init(AsyncServiceGroup *sg);
    //virtual bool Init(const ServiceGroupInfo &sgi);
    virtual IAsyncServiceGroup *CreateServiceGroup();

protected:
    virtual bool Send(const void *in_buf, size_t in_len);

//protected:
//    AsyncServiceGroup *m_sg;
};

class IntAsyncProtoHandler : public IAsyncProtoHandler
{
public:
    IntAsyncProtoHandler(){}
    //virtual ~IntAsyncProtoHandler();
    //virtual bool Init(const ServiceGroupInfo &sgi);
    virtual IAsyncServiceGroup *CreateServiceGroup();

protected:
    virtual bool SendByInt(const void *in_buf, size_t in_len, int seed);

//protected:
//    IntAsyncServiceGroup *m_sg;
};

class StringAsyncProtoHandler : public IAsyncProtoHandler
{
public:
    StringAsyncProtoHandler(){}
    //virtual ~StringAsyncProtoHandler();
    //virtual bool Init(const ServiceGroupInfo &sgi);
    virtual IAsyncServiceGroup *CreateServiceGroup();

protected:
    virtual bool SendByString(const void *in_buf, size_t in_len, const string &seed);

//protected:
//    StringAsyncServiceGroup *m_sg;
};

#endif
