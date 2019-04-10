#ifndef _PROTO_H
#define _PROTO_H

#define MAX_BUF_LEN (1024*1024)

typedef struct {
    uint32_t pkg_len;
    uint32_t seq_num;
    uint16_t cmd_id;
    uint32_t status_code;
    uint32_t user_id;
} __attribute__((packed)) proto_header;

#define QUERY_CONF_CMD	0x1000

typedef struct {
	char ip[64];
	char path[1024];
}__attribute__((packed)) query_config_req;

typedef struct {
	uint32_t type;
	uint32_t value_len;
	char key[64];
	char value[0];
}__attribute__((packed)) config_info;

typedef struct {
	uint32_t conf_num;
	config_info info[0];
}__attribute__((packed)) query_config_ack;

//=============================================
#define QUERY_STRATEGY_CMD	0x2000
//typedef struct {
//	char name[64];
//}__attribute__((packed)) NameInfo;
//
//typedef struct {
//	uint32_t name_num;
//	NameInfo names[0];
//}__attribute__((packed)) ServQueryGroupReq;

//
//typedef struct {
//	uint32_t group_num;
//	ServiceGroupInfo ServiceGroupInfo[0];
//}__attribute__((packed)) ServQueryGroupAck;
//
//typedef struct {
//	char group_strategy[64];
//	uint8_t forbidden;
//	ServiceReplicasInfo vec[0];
//}__attribute__((packed)) ServiceGroupInfo;
//
//typedef struct {
//}__attribute__((packed)) ServiceReplicasInfo;
//
//typedef struct {
//}__attribute__((packed)) HostInfo;

int atoi_safe(const char *nptr);
#endif
