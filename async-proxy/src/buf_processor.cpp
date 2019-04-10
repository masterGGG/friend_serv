#include <string.h>
#include <string>
#include "buf_processor.h"
using std::string;
#include "proto.h"
#include "redis_command.h"
#include "log.h"
int BufProcessor::Init(const char *buf, const int len, char* out_buf, redisContext *context)
{
	in_header = (proto_header *)buf;
	in_body = (char*)(in_header + 1);
	in_len = len;
	memset(out_buf, 0, sizeof(out_buf));

	out_header = (proto_header*)out_buf;
	out_header->pkg_len = 0;
	out_header->seq_num = in_header->seq_num;
	out_header->cmd_id = in_header->cmd_id;
	out_header->status_code = 0;
	out_header->user_id = in_header->user_id;
	out_body = (char*)(out_header + 1);

    ctx = context;
	return in_header->cmd_id;
}

void BufProcessor::AddAttention(char* out_buf, const unsigned int max_len)
{
    int code = 0;
    time_t tm;
    redisReply *reply;
    add_attention_req *req = (add_attention_req*)in_body;
    char *p = req->gname;
    char name[64];
    memset(name, 0, sizeof(name));
    memcpy(name, p, req->len);

    tm = time(NULL);

    //1.将关注的米米号加入指定分组集合
    redisAppendCommand(ctx, COMMAND_ADD_ATTENTION_1, in_header->user_id, name, tm, req->m_id);
    //DEBUG_LOG(COMMAND_ADD_ATTENTION_1, in_header->user_id, name.c_str(), tm, req->m_id);
    //2.将关注米米号添加当前米米号和关注米米号的hashMap映射
    redisAppendCommand(ctx, COMMAND_ADD_ATTENTION_2, in_header->user_id, req->m_id, name, req->type+0, tm);
    //DEBUG_LOG(COMMAND_ADD_ATTENTION_2, in_header->user_id, req->m_id, name.c_str(), req->type+0, tm);
    //3.将关注米米号添加到当前米米号的所有关注好友集合
    redisAppendCommand(ctx, COMMAND_ADD_ATTENTION_3, in_header->user_id, tm, req->m_id);
    //DEBUG_LOG(COMMAND_ADD_ATTENTION_3, in_header->user_id, tm, req->m_id);
    //4.将自己的米米号添加到关注米米号的粉丝集合
    redisAppendCommand(ctx, COMMAND_ADD_ATTENTION_4, req->m_id, tm, in_header->user_id);
    //DEBUG_LOG(COMMAND_ADD_ATTENTION_4, req->m_id, tm, in_header->user_id);
	
    code = redisGetReply(ctx, (void **)&reply);
    if (code == REDIS_ERR)
    {
        ERROR_LOG("[REDIS]:Server shutdown");
		GetError(FEED_REDIS_ERR);
        return;
    }
    else if (!CheckZSETReply(code, *reply))
        ERROR_LOG("ZADD feeds:%u:list %u %d %d", in_header->user_id, req->m_id, reply->type, (int)reply->integer);
    freeReplyObject(reply);

    code = redisGetReply(ctx, (void **)&reply);
    if (code == REDIS_ERR)
    {
        ERROR_LOG("[REDIS]:Server shutdown");
		GetError(FEED_REDIS_ERR);
        return;
    }
    else if (!CheckHashMapReply(code, *reply))
        ERROR_LOG("HMSET feeds:%u:glist %u %d %s", in_header->user_id, req->m_id, reply->type, reply->str);
    freeReplyObject(reply);
    
    code = redisGetReply(ctx, (void **)&reply);
    if (code == REDIS_ERR)
    {
        ERROR_LOG("[REDIS]:Server shutdown");
		GetError(FEED_REDIS_ERR);
        return;
    }
    else if (!CheckZSETReply(code, *reply))
        ERROR_LOG("ZADD feeds:%u:list %u %d %d", in_header->user_id, req->m_id, reply->type, (int)reply->integer);
    freeReplyObject(reply);
    
    code = redisGetReply(ctx, (void **)&reply);
    if (code == REDIS_ERR)
    {
        ERROR_LOG("[REDIS]:Server shutdown");
		GetError(FEED_REDIS_ERR);
        return;
    }
    else if (!CheckZSETReply(code, *reply))
        ERROR_LOG("ZADD feeds:%u:list %u %d %d", in_header->user_id, req->m_id, reply->type, (int)reply->integer);
    freeReplyObject(reply);

    out_header->pkg_len = sizeof(proto_header);
}

void BufProcessor::DropAttention(char* out_buf, const unsigned int max_len)
{
    int code = 0;
    redisReply *reply;
    uint32_t f_mid = ((drop_attention_req*)in_body)->m_id;
    char gname[64];

    //1.通过glist找到关注的米米号对应的组别
    DEBUG_LOG("[DROPATTENTION]: 1. mid %u", f_mid);
    reply = (redisReply *)redisCommand(ctx, COMMAND_DROP_ATTENTION_1, in_header->user_id, f_mid);
    if (NULL == reply)
    {
        ERROR_LOG("[REDIS]:Server shutdown");
		GetError(FEED_REDIS_ERR);
        return;
    }
    DEBUG_LOG(COMMAND_DROP_ATTENTION_1, in_header->user_id, f_mid);
    if (reply->type == REDIS_REPLY_STRING)
    {
        memset(gname, 0, sizeof(gname));
        memcpy(gname, reply->str, reply->len);
        DEBUG_LOG("[DROPATTENTION]: 1. gname %s", gname);
    }
    else
        ERROR_LOG("HGET feeds:%u:flist %u %d %s", in_header->user_id, f_mid, reply->type, reply->str);
    freeReplyObject(reply);

    //2.从对应组集合中删除对应的米米号
    redisAppendCommand(ctx, COMMAND_DROP_ATTENTION_2, in_header->user_id, gname, f_mid);
    DEBUG_LOG(COMMAND_DROP_ATTENTION_2, in_header->user_id, gname, f_mid);
    //3.从对应的HashMap中删除米米号的映射关系
    redisAppendCommand(ctx, COMMAND_DROP_ATTENTION_3, in_header->user_id, f_mid);
    DEBUG_LOG(COMMAND_DROP_ATTENTION_3, in_header->user_id, f_mid);
    //4.从所有关注对象的集合中删除对应的关注米米号
    redisAppendCommand(ctx, COMMAND_DROP_ATTENTION_4, in_header->user_id, f_mid);
    DEBUG_LOG(COMMAND_DROP_ATTENTION_4, in_header->user_id, f_mid);
    //5.从关注米米号的粉丝集合中删除自己的关注记录
    redisAppendCommand(ctx, COMMAND_DROP_ATTENTION_5, f_mid, in_header->user_id);
    DEBUG_LOG(COMMAND_DROP_ATTENTION_5, f_mid, in_header->user_id);

    code = redisGetReply(ctx, (void **)&reply);
    if (code == REDIS_ERR)
    {
        ERROR_LOG("[REDIS]:Server shutdown");
		GetError(FEED_REDIS_ERR);
        return;
    }
    else if (!CheckZSETReply(code, *reply))
        ERROR_LOG("ZREM feeds:%u:flist %u %d %s", in_header->user_id, f_mid, reply->type, reply->str);
    freeReplyObject(reply);

    code = redisGetReply(ctx, (void **)&reply);
    if (code == REDIS_ERR)
    {
        ERROR_LOG("[REDIS]:Server shutdown");
		GetError(FEED_REDIS_ERR);
        return;
    }
    else if (!CheckHashMapDelReply(code, *reply))
        ERROR_LOG("HDEL feeds:%u:glist %u %d %s", in_header->user_id, f_mid, reply->type, reply->str);
    freeReplyObject(reply);
    
    code = redisGetReply(ctx, (void **)&reply);
    if (code == REDIS_ERR)
    {
        ERROR_LOG("[REDIS]:Server shutdown");
		GetError(FEED_REDIS_ERR);
        return;
    }
    else if (!CheckZSETReply(code, *reply))
        ERROR_LOG("ZREM feeds:%u:list %u %d %s", in_header->user_id, f_mid, reply->type, reply->str);
    freeReplyObject(reply);
    
    code = redisGetReply(ctx, (void **)&reply);
    if (code == REDIS_ERR)
    {
        ERROR_LOG("[REDIS]:Server shutdown");
		GetError(FEED_REDIS_ERR);
        return;
    }
    else if (!CheckZSETReply(code, *reply))
        ERROR_LOG("ZREM feeds:%u:fans %u %d %s", f_mid, in_header->user_id, reply->type, reply->str);
    freeReplyObject(reply);

    out_header->pkg_len = sizeof(proto_header);
}

void BufProcessor::AddGroup(char* out_buf, const unsigned int max_len)
{
    time_t tm;
    int code = 0;
    redisReply *reply;
    handle_group_req *req = (handle_group_req*)in_body;
    char *p = req->gname;
    char name[64];
    memset(name, 0, sizeof(name));
    memcpy(name, p, req->len);

    tm = time(NULL);
    
    DEBUG_LOG("mid:%u time:%u gname:%s", in_header->user_id, tm, name);
    redisAppendCommand(ctx, COMMAND_ADD_GROUP, in_header->user_id, tm, name);
    
    DEBUG_LOG(COMMAND_ADD_GROUP, in_header->user_id, tm, name);
    code = redisGetReply(ctx, (void **)&reply);
    if (code == REDIS_ERR)
    {
        ERROR_LOG("[REDIS]:Server shutdown");
		GetError(FEED_REDIS_ERR);
        return;
    }
    DEBUG_LOG("Reply: %d %s", reply->type, reply->str);
    freeReplyObject(reply);
    out_header->pkg_len = sizeof(proto_header);
}

void BufProcessor::DropGroup(char* out_buf, const unsigned int max_len)
{
    int code;
    redisReply *reply;
    handle_group_req *req = (handle_group_req*)in_body;
    char *p = req->gname;
    char name[64];
    memset(name, 0, sizeof(name));
    memcpy(name, p, req->len);

    redisAppendCommand(ctx, COMMAND_DROP_GROUP, in_header->user_id, name);
    
    code = redisGetReply(ctx, (void **)&reply);
    if (code == REDIS_ERR)
    {
        ERROR_LOG("[REDIS]:Server shutdown");
		GetError(FEED_REDIS_ERR);
        return;
    }
    DEBUG_LOG("Reply: %d %s", reply->type, reply->str);
    freeReplyObject(reply);
    out_header->pkg_len = sizeof(proto_header);
}

void BufProcessor::QueryGroup(char* out_buf, const unsigned int max_len)
{
    int code = 0;
    redisReply *reply;

    group_info_ack *res = (group_info_ack *)out_body;

    redisAppendCommand(ctx, COMMAND_QUERY_GROUP, in_header->user_id);
    
    code = redisGetReply(ctx, (void **)&reply);
    if (code == REDIS_ERR)
    {
        ERROR_LOG("[REDIS]:Server shutdown");
		GetError(FEED_REDIS_ERR);
        return;
    }
//    DEBUG_LOG("Reply: %d %s", reply->type, reply->str);
    out_header->pkg_len = sizeof(proto_header);
    if (reply->type == REDIS_REPLY_ARRAY)
    {
        res->num = reply->elements;
        //add group_info_ack len to pke_len
        out_header->pkg_len += sizeof(group_info_ack);
        char *p = (char *)res->list;
        for (int i = 0; i< reply->elements; i++)
        {
            group_info *gi = (group_info *)p;
            gi->len = reply->element[i]->len;
            memset(gi->gname, 0, gi->len);
            memcpy(gi->gname, reply->element[i]->str, gi->len);
            DEBUG_LOG("[query_group] : gname:%s(%d)", reply->element[i]->str, gi->len);
            p += sizeof(group_info) + gi->len;
            out_header->pkg_len += sizeof(group_info) + gi->len;
            
            {
                //todo : check pkg_len and max_len
            }
        }
    }
    freeReplyObject(reply);
}

void BufProcessor::QueryList(char* out_buf, const unsigned int max_len)
{
    int code = 0;
    redisReply *reply;

    query_idol_info_req *req = (query_idol_info_req*)in_body;
    query_idol_info_ack *res = (query_idol_info_ack *)out_body;

    //Query all idol mimiID
    if (req->count == 0)
        redisAppendCommand(ctx, COMMAND_QUERY_LIST, req->user_id, req->offset, -1);
    else
        redisAppendCommand(ctx, COMMAND_QUERY_LIST, req->user_id, req->offset, req->offset + req->count-1);
    DEBUG_LOG(COMMAND_QUERY_LIST, in_header->user_id, req->offset, req->count);
    
    code = redisGetReply(ctx, (void **)&reply);
    if (code == REDIS_ERR)
    {
        ERROR_LOG("[REDIS]:Server shutdown");
		GetError(FEED_REDIS_ERR);
        return;
    }
    else
        out_header->pkg_len = sizeof(proto_header);

    if (reply->type == REDIS_REPLY_ARRAY)
    {
        if (req->count == 0)
            res->num = reply->elements/2;
        else
            res->num = reply->elements/2 < req->count ? reply->elements/2 : req->count;
        //add group_info_ack len to pke_len
        out_header->pkg_len += sizeof(group_info_ack);
        char *p = (char *)res->list;
        for (int i = 0; i< res->num; i++)
        {
            idol_info *ii = (idol_info *)p;
            //DEBUG_LOG("[query_group] : gname:%s(%d)", reply->element[i]->str, gi->len);
            ii->m_id = atoi(reply->element[2*i]->str);
            ii->time = atoi(reply->element[2*i + 1]->str);
            p += sizeof(idol_info);
            out_header->pkg_len += sizeof(idol_info);
            
            {
                //todo : check pkg_len and max_len
            }
        }
    }
}

void BufProcessor::QueryListByGroup(char* out_buf, const unsigned int max_len)
{
    int code = 0;
    redisReply *reply;
    //string name = ((query_idol_by_group_req*)in_body)->group_name;
    query_idol_by_group_req *req = (query_idol_by_group_req*)in_body;
    char *pname = req->group.gname;
    char name[64];
    memset(name, 0, sizeof(name));
    memcpy(name, pname, req->group.len);

    query_idol_info_ack *res = (query_idol_info_ack *)out_body;

    if (req->count == 0)
        redisAppendCommand(ctx, COMMAND_QUERY_BY_GROUP, in_header->user_id, name, req->offset, -1);
    else
        redisAppendCommand(ctx, COMMAND_QUERY_BY_GROUP, in_header->user_id, name, req->offset, req->offset + req->count - 1);
    //DEBUG_LOG(COMMAND_QUERY_BY_GROUP, in_header->user_id, name, req->offset, req->count);
    
    code = redisGetReply(ctx, (void **)&reply);
    if (code == REDIS_ERR)
    {
        ERROR_LOG("[REDIS]:Server shutdown");
		GetError(FEED_REDIS_ERR);
        return;
    }
    else
        out_header->pkg_len = sizeof(proto_header);

    if (reply->type == REDIS_REPLY_ARRAY)
    {
        if (req->count == 0)
            res->num = reply->elements/2;
        else
            res->num = reply->elements/2 < req->count ? reply->elements/2 : req->count;
        //add group_info_ack len to pke_len
        out_header->pkg_len += sizeof(group_info_ack);
        char *p = (char *)res->list;
        for (int i = 0; i< res->num; i++)
        {
            idol_info *ii = (idol_info *)p;
            //DEBUG_LOG("[query_group] : gname:%s(%d)", reply->element[i]->str, gi->len);
            ii->m_id = atoi(reply->element[2*i]->str);
            ii->time = atoi(reply->element[2*i + 1]->str);
            p += sizeof(idol_info);
            out_header->pkg_len += sizeof(idol_info);
            
            {
                //todo : check pkg_len and max_len
            }
        }
    }
}

void BufProcessor::QueryFansList(char* out_buf, const unsigned int max_len)
{
    int code = 0;
    redisReply *reply;

    query_fans_info_req *req = (query_fans_info_req*)in_body;
    query_fans_info_ack *res = (query_fans_info_ack *)out_body;

    if (req->count == 0)
        redisAppendCommand(ctx, COMMAND_QUERY_FANS, req->user_id, req->offset, -1);
    else
        redisAppendCommand(ctx, COMMAND_QUERY_FANS, req->user_id, req->offset, req->offset + req->count - 1);
//DEBUG_LOG(COMMAND_QUERY_FANS, in_header->user_id, req->offset, req->count);
    
    code = redisGetReply(ctx, (void **)&reply);
    if (code == REDIS_ERR)
    {
        ERROR_LOG("[REDIS]:Server shutdown");
		GetError(FEED_REDIS_ERR);
        return;
    }
    else
        out_header->pkg_len = sizeof(proto_header);

    if (reply->type == REDIS_REPLY_ARRAY)
    {
        if (req->count == 0)
            res->num = reply->elements/2;
        else
            res->num = reply->elements/2 < req->count ? reply->elements/2 : req->count;
        //add group_info_ack len to pke_len
        out_header->pkg_len += sizeof(group_info_ack);
        char *p = (char *)res->list;
        for (int i = 0; i< res->num; i++)
        {
            idol_info *ii = (idol_info *)p;
            //DEBUG_LOG("[query_group] : gname:%s(%d)", reply->element[i]->str, gi->len);
            ii->m_id = atoi(reply->element[2*i]->str);
            ii->time = atoi(reply->element[2*i + 1]->str);
            p += sizeof(idol_info);
            out_header->pkg_len += sizeof(idol_info);
            
            {
                //todo : check pkg_len and max_len
            }
        }
    }
}

void BufProcessor::QueryRelation(char* out_buf, const unsigned int max_len)
{
    int code;
    redisReply *reply;
    uint32_t f_mid = ((query_relation_req*)in_body)->m_id;
    uint8_t relation = 0;
//1.查询我的关注集合是否存在对方米米号（存在：已关注）
    redisAppendCommand(ctx, COMMAND_QUERY_RELATION_1, in_header->user_id, f_mid);
    DEBUG_LOG(COMMAND_QUERY_RELATION_1, in_header->user_id, f_mid);
//2.查询我的粉丝集合是否存在对方米米号（存在：被关注）
    redisAppendCommand(ctx, COMMAND_QUERY_RELATION_2, in_header->user_id, f_mid);
    DEBUG_LOG(COMMAND_QUERY_RELATION_2, in_header->user_id, f_mid);
    //1.
    code = redisGetReply(ctx, (void **)&reply);
    if (code == REDIS_ERR)
    {
        ERROR_LOG("[REDIS]:Server shutdown");
	    GetError(FEED_REDIS_ERR);
        return ;
    }
    else if (reply->type == REDIS_REPLY_INTEGER)
        relation += 1;
    DEBUG_LOG("1. type:%d int:%u", reply->type, relation);
    freeReplyObject(reply);

    //2.
    code = redisGetReply(ctx, (void **)&reply);
    if (code == REDIS_ERR)
    {
        ERROR_LOG("[REDIS]:Server shutdown");
	    GetError(FEED_REDIS_ERR);
        return;
    }
    else if (reply->type == REDIS_REPLY_INTEGER)
        relation += 2;
    ((query_relation_ack *)out_body)->relation = relation;
    freeReplyObject(reply);
    DEBUG_LOG("2. type:%d int:%u buf:%u", reply->type, ((query_relation_ack *)out_body)->relation);
    if (out_header->status_code == 0)
        out_header->pkg_len = sizeof(proto_header) + sizeof(query_relation_ack);
}

void BufProcessor::GetError(int status)
{
	out_header->pkg_len = sizeof(proto_header);
	out_header->status_code = status;
}

bool BufProcessor::CheckZSETReply(int code, const redisReply &reply)
{
    if (reply.type == REDIS_REPLY_INTEGER)
    {
        if (reply.integer == 1)
            return true;
        else if (reply.integer == 0)
        {
            DEBUG_LOG("[Redis ZSET]ADD repeat record or delete inexist record");
            return true;
        }
        //ZADD will reply integer 0 | 1
    }
    GetError(FEED_REDIS_ERR);
    return false; 
}

bool BufProcessor::CheckHashMapReply(int code, const redisReply &reply)
{
    if (reply.type == REDIS_REPLY_STATUS)
    {
        if (0 == strcmp(reply.str,"OK"))
            return true;
        //ZADD will reply integer 0 | 1
    }
    GetError(FEED_REDIS_ERR);
    return false; 
}
bool BufProcessor::CheckHashMapDelReply(int code, const redisReply &reply)
{
    if (reply.type == REDIS_REPLY_INTEGER)
    {
        if (reply.integer == 3)
            return true;
        else if (reply.integer == 0)
        {
            DEBUG_LOG("[Redis HDEL]DEL delete inexist record");
            return true;
        }
        //HDEL will reply integer 0 | 3
    }
    GetError(FEED_REDIS_ERR);
    return false; 
}
void BufProcessor::UpdateFeedMimiID(char* out_buf, const unsigned int max_len) {
    redisReply *reply;
    feed_mimiid_update_req * req = (feed_mimiid_update_req *)in_body;
    redisAppendCommand(ctx, COMMAND_UPDATE_FEED_TIME, req->timer, in_header->user_id);
    if (redisGetReply(ctx, (void **)&reply) == REDIS_ERR) {
        ERROR_LOG("[REDIS]:Server shutdown");
        GetError(FEED_REDIS_ERR);
        return ;
    }
    freeReplyObject(reply);
    out_header->pkg_len = sizeof(proto_header);
}

void BufProcessor::QueryFeedMimiID(char* out_buf, const unsigned int max_len) {
    redisReply *reply;
    feed_mimiid_query_req *req = (feed_mimiid_query_req *)in_body;
    feed_mimiid_query_ack *res = (feed_mimiid_query_ack *)out_body;

    redisAppendCommand(ctx, COMMAND_QUERY_FEED_TIME, req->end, req->begin, req->count);
    if (redisGetReply(ctx, (void **)&reply) == REDIS_ERR) {
        ERROR_LOG("[REDIS]:Server shutdown");
        GetError(FEED_REDIS_ERR);
        return ;
    }
    out_header->pkg_len = sizeof(proto_header);
    if (reply->type == REDIS_REPLY_ARRAY) {
        res->num = reply->elements;
        out_header->pkg_len += (res->num + 1)*sizeof(uint32_t);
        uint32_t *p = (uint32_t *)res + 1;
        for (int i = 0; i < res->num; i++) 
            *(p++) = atoi(reply->element[i]->str);
    } else {
        ERROR_LOG("[REDIS]:Server shutdown");
        GetError(FEED_REDIS_ERR);
    }

    freeReplyObject(reply);
}
