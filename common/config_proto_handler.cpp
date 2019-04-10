#include <unistd.h>
#include "config_proto_handler.h"
#include "proto.h"
#include "singleton.h"
#include "async_server.h"
#include "proto.h"
#include "net_utils.h"

bool ConfigSyncProtoHandler::CGetProcessIdentity(string local_path, map<string, string> &config_map)
{
    // 先根据本机ip以及目录获取本服务的server_id
	Common::local_ip = Common::get_local_ip();
    if (Common::local_ip.empty()) {
        ERROR_LOG("get local ip failed");
        return false;
    }

	char in_buf[1024];
	proto_header *in_header = (proto_header*)in_buf;
	
	in_header->pkg_len = sizeof(proto_header) + sizeof(query_config_req);
	in_header->seq_num = 0;
	in_header->cmd_id = 0x1000;    // 访问第一个ServiceGroup
	in_header->status_code = 0;
	in_header->user_id = 0;

	query_config_req *in_body = (query_config_req*)(in_header + 1);
	memset(in_body->ip, '\0', sizeof(in_body->ip));
    memcpy(in_body->ip, (Common::local_ip).c_str(), (Common::local_ip).size() + 1);
    memset(in_body->path, '\0', sizeof(in_body->path));
    memcpy(in_body->path, local_path.c_str(), local_path.size() + 1);
	
	printf("ip:%s, path:%s\n", in_body->ip, in_body->path);
		
    char out_buf[MAX_BUF_LEN];
    size_t out_len;

    // SendAndRecvByInt只保证收到的长度>=8
	printf("len:%u\n", in_header->pkg_len);
    if (!SendAndRecvByNothing(in_buf, in_header->pkg_len, out_buf, sizeof(out_buf), out_len)) {
        ERROR_LOG("ConfigSyncProtoHandler CGetProcessIdentity failed for SendAndRecvByInt failed");
        return false;
    }

    proto_header *out_header = (proto_header*)out_buf;
    if (out_len == sizeof(proto_header) || out_header->status_code) {
		ERROR_LOG("Get config failed.");
        return false;
	}
	
	query_config_ack *out_body = (query_config_ack*)(out_header + 1);
	char *p = (char*)(out_body->info);

	ofstream fout;
	string path;
	config_info *info = NULL;
	for(unsigned int i = 0 ; i < out_body->conf_num; ++i) {
		info = (config_info*) p;
		if(string(info->value).size() > info->value_len - 1) {
			ERROR_LOG("Value format error.");
			return false;
		}

		if(info->type == 1) {
			path = "./conf/" + string(info->key);
			fout.open(path.c_str());
			if(fout.is_open()) {
				fout << string(info->value);
				fout.close();
			} else {
				ERROR_LOG("Creat file:%s failed.", path.c_str());
				return false;
			}
		} else {
			config_map[string(info->key)] = string(info->value);
		}
		p += (sizeof(config_info) + info->value_len);
	}

	return true;
}


bool ConfigSyncProtoHandler::Init(const vector<string> &ips, int port)
{
	vector<HostInfo> list;
	HostInfo *hp = NULL;
	for(unsigned int i = 0 ; i < ips.size(); i++) {
		hp = new HostInfo(ips[i], port);
		list.push_back(*hp);
		delete hp;
	}

	ServiceReplicasInfo sri;
	sri.replicas_strategy = Strategy::STRATEGY_ROLL;
	for(unsigned int i = 0; i < list.size(); i++) {
		sri.vec.push_back(list[i]);
	}

	ServiceGroupInfo sgi;
	sgi.group_strategy = Strategy::STRATEGY_ROLL;
	sgi.vec.push_back(sri);

	return SyncProtoHandler::Init(sgi);
}
