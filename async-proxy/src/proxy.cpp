#include <string.h>
#include <string>
#include <vector>
#include "async_server.h"
#include "log.h"
#include "proto.h"
#include "proxy.h"
#include "buf_processor.h"
#include "redis_sentinel.h"

using std::string;
using std::vector;

BufProcessor *buf_handler = NULL;
RedisSentinelInfo *s_ctx = NULL;
RedisMasterInfo *m_ctx = NULL;
int Proxy::Init() 
{
    string ip;
    int port;
    char para[32];

    int timer = config_get_intval("redis_timeout", 50000);
    string name = config_get_strval("redis_master_name", "mymaster");
    string pass = config_get_strval("redis_master_pwd", "ta0mee@123");
    int num = config_get_intval("redis_sentinel_num", 0);

    s_ctx = new RedisSentinelInfo(num, timer, name);

    for (int i = 0; i < num; i++) {
        snprintf(para, sizeof(para), "redis_sentinel_ip%d", i+1);
        ip = config_get_strval(para, "");
        snprintf(para, sizeof(para), "redis_sentinel_port%d", i+1);
        port = config_get_intval(para, 0);
        s_ctx->AddServer(ip, port);
    }

    /*
    if (-1 == s_ctx->Init()) {
        delete s_ctx;
        return -1;
    }
    */
    ip = config_get_strval("redis_ip", "127.0.0.1");
    port = config_get_intval("redis_port", 6379);
    s_ctx->CheckOrUpdate(ip, port);
    //DEBUG_LOG("[REDIS]: Configuration ip:port - %s:%d", ip.c_str(), port);
    m_ctx = new RedisMasterInfo(ip, port, pass, timer);
    if (-1 == m_ctx->Init()) {
        delete s_ctx;
        delete m_ctx;
        return -1;
    }
    
	buf_handler = new BufProcessor();
    return 0;      
}

int Proxy::Uninit() 
{
	if(buf_handler) {
		delete buf_handler;
	}
    return 0;
}

int Proxy::GetPkgLenSer(int fd, const char *buf, uint32_t len) 
{
    if (len < sizeof(uint32_t)) {
        return 0;
    }
    return *(uint32_t*)buf;
}

void Proxy:: TimeEvent() 
{
    redisContext *ctx = m_ctx->GetContext();
    if (ctx->err != 0)
    {
        string ip = "0.0.0.0";
        int port = 0;
        int ret = s_ctx->CheckOrUpdate(ip, port);
        if (ret == -1) 
            ERROR_LOG("Can not connect to redis Sentinel");
        else if (ret == 1) {
            if (-1 == m_ctx->ReInit(ip, port))
                ERROR_LOG("Can not reconnect failover redis Server");
        }
        else {
            if (-1 == m_ctx->Init())
                ERROR_LOG("Can not reconnect current redis Server");
        }
    }
}

void Proxy:: ProcPkgCli(int fd, const char *buf, uint32_t len) 
{
	char out_buf[MAX_BUF_LEN];
	int cmd_id = buf_handler->Init(buf, len, out_buf, m_ctx->GetContext());

    proto_header *ack_head = (proto_header*)out_buf;
	if(cmd_id == ADD_ATTENTION_CMD) {
	    buf_handler->AddAttention(out_buf, MAX_BUF_LEN);
	}
    else if (cmd_id == DROP_ATTENTION_CMD) {
	    buf_handler->DropAttention(out_buf, MAX_BUF_LEN);
    }
	else if(cmd_id == ADD_GROUP_CMD) {
	    buf_handler->AddGroup(out_buf, MAX_BUF_LEN);
	}
	else if(cmd_id == DROP_GROUP_CMD) {
	    buf_handler->DropGroup(out_buf, MAX_BUF_LEN);
	}
	else if(cmd_id == QUERY_GROUP_CMD) {
	    buf_handler->QueryGroup(out_buf, MAX_BUF_LEN);
	}
	else if(cmd_id == QUERY_IDOL_LIST_CMD) {
	    buf_handler->QueryList(out_buf, MAX_BUF_LEN);
	}
	else if(cmd_id == QUERY_IDOL_BY_GROUP_CMD) {
	    buf_handler->QueryListByGroup(out_buf, MAX_BUF_LEN);
	}
	else if(cmd_id == QUERY_FANS_CMD) {
	    buf_handler->QueryFansList(out_buf, MAX_BUF_LEN);
	}
	else if(cmd_id == QUERY_RELATION_CMD) {
	    buf_handler->QueryRelation(out_buf, MAX_BUF_LEN);
	} else if (cmd_id == FEED_MIMIID_UPDATE_CMD) {
        // Use new protocol to save latest userid who updated status
        buf_handler->UpdateFeedMimiID(out_buf, MAX_BUF_LEN);
	} else if (cmd_id == FEED_MIMIID_QUERY_CMD) {
        // 0XA102 to query latest userid who updated status
        buf_handler->QueryFeedMimiID(out_buf, MAX_BUF_LEN);
    } else if (cmd_id == MASTER_CHANGE_CMD) {
	    proto_header *in_header = (proto_header *)buf;
        master_change_req *req = (master_change_req *)((char*)(in_header + 1));
        int port = req->port;
        char nip[16];
        memset(nip, 0, sizeof(nip));
        memcpy(nip, req->ip, req->len);
		//ERROR_LOG("Cmd id invail, ip:%s port:%d", nip, port);
        string ip = nip;
        if (-1 == m_ctx->ReInit(ip, port)) {
		    ERROR_LOG("Can not connect to new master ip:%s port:%d", nip, port);
		    buf_handler->GetError(PARAM_ERR);
        }
    } else{
		ERROR_LOG("Cmd id invail, cmd_id:%u addattenrion[%u]", cmd_id, ADD_ATTENTION_CMD);
		buf_handler->GetError(PARAM_ERR);
	}
 
    if(ack_head->pkg_len > MAX_BUF_LEN) {
        ERROR_LOG("Back pack is too long, pkg_len:%u", ack_head->pkg_len);
        buf_handler->GetError(SYS_ERR);
    }
    DEBUG_LOG("Res: %d", ack_head->pkg_len);
	net_send_cli(fd, out_buf, ack_head->pkg_len);
}

void Proxy:: ProcPkgSer(int fd, const char *buf, uint32_t len) 
{
    
}

void Proxy:: LinkUpCli(int fd, uint32_t ip) 
{

}

void Proxy:: LinkUpSer(int fd, uint32_t ip, uint16_t port) 
{

}

void Proxy:: LinkDownCli(int fd) 
{

}

void Proxy:: LinkDownSer(int fd) 
{

}

