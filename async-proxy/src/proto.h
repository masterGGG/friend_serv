#ifndef _CENTER_PROTO_H
#define _CENTER_PROTO_H
#include <stdint.h>
#define MAX_BUF_LEN (1024*1024)

#define SYS_ERR 1001                /// 系统出错 一般是内存出错
#define PARAM_ERR 1002              /// 参数错误
#define FEED_REDIS_ERR 1003

typedef struct {
    uint32_t pkg_len;
    uint32_t seq_num;
    uint16_t cmd_id;
    uint32_t status_code;
    uint32_t user_id;
} __attribute__((packed)) proto_header;

/**
 * @brief 添加关注时，需要接受的数据
 */
typedef struct {
    uint32_t m_id;
    uint16_t type;
    uint32_t len;
    char gname[0];
}__attribute__((packed)) user_info;

typedef struct {
    uint32_t len;
    char gname[0];
}__attribute__((packed)) group_info;

typedef struct {
    uint32_t m_id;
}__attribute__((packed)) fid_info;

typedef struct {
    uint32_t m_id;
    uint32_t time;
}__attribute__((packed)) fans_info;

typedef fans_info idol_info;

/*+++++++++++++++++++++ 0XB0** 修改接口 ++++++++++++++++++++++++*/
//===================关注接口==========================
#define ADD_ATTENTION_CMD	0xB001
typedef user_info add_attention_req;
//===================取关==========================
#define DROP_ATTENTION_CMD	0xB002
typedef fid_info drop_attention_req;
//===================添加自定义的组==========================
#define ADD_GROUP_CMD	0xB003
typedef group_info handle_group_req;
//===================删除自定义的组==========================
#define DROP_GROUP_CMD	0xB004
/*+++++++++++++++++++++ 0XB1** 读取接口 ++++++++++++++++++++++++*/
//==================查询自己拥有的组名===========================
#define QUERY_GROUP_CMD	0xB101
typedef struct {
    uint32_t num;
    group_info list[0];
}__attribute__((packed)) group_info_ack;
//====================查询指定全部关注列表=========================
#define QUERY_IDOL_LIST_CMD	0xB102
typedef struct {
    uint32_t user_id;
    uint32_t offset;
    uint32_t count;
}__attribute__((packed)) query_idol_info_req;
typedef struct {
	uint32_t num;
    idol_info list[0];
}__attribute__((packed)) query_idol_info_ack;
//====================查询指定组的关注列表=========================
#define QUERY_IDOL_BY_GROUP_CMD	0xB103
typedef struct {
    uint32_t offset;
    uint32_t count;
    group_info group;
}__attribute__((packed)) query_idol_by_group_req;
//=====================查询我的粉丝数========================
#define QUERY_FANS_CMD	0xB104
typedef query_idol_info_req query_fans_info_req;
typedef struct {
	uint32_t num;
    fans_info list[0];
}__attribute__((packed)) query_fans_info_ack;
//=====================查询双方状态========================
#define QUERY_RELATION_CMD	0xB105
typedef fid_info query_relation_req;
typedef struct {
    uint8_t relation;
}__attribute__((packed)) query_relation_ack;

int atoi_safe(const char *nptr);
//=====================Redis主备切换通知========================
#define MASTER_CHANGE_CMD	0xA001

typedef struct {
	uint32_t port;
	uint32_t len;
    char ip[0];
}__attribute__((packed)) master_change_req;

//=====================最近更新文章的mimi号========================
#define FEED_MIMIID_UPDATE_CMD	0xA101
typedef struct {
	uint32_t timer;
}__attribute__((packed)) feed_mimiid_update_req;

#define FEED_MIMIID_QUERY_CMD	0xA102
typedef struct {
	uint32_t begin;
	uint32_t end;
	uint32_t count;
}__attribute__((packed)) feed_mimiid_query_req;
typedef struct {
	uint32_t num;
    uint32_t mimiid[0];
}__attribute__((packed)) feed_mimiid_query_ack;

#endif
