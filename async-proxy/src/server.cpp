#include <string>
#include "async_server.h"

//#include "config_manager.h"
#include "file_config_manager.h"
#include "center_config_manager.h"

#include "proxy.h"
#include "control.h"
#include "service_group.h"

using std::string;


static IProcessor* main_server = NULL;

static ConfigManager *cm = NULL;

extern "C" bool load_config(const char *source, int type)
{
    if (cm && cm->IsLoaded()) {
        return true;
    } else if (cm){
        return cm->LoadConfig(source);
    }

    if (type == 0)
        cm =  new (std::nothrow) FileConfigManager();
    else if(type == 1)
        cm =  new (std::nothrow) CenterConfigManager();
    else
        return false;

    if (!cm) {
        // TODO
        return false;
    }
    return cm->LoadConfig(source);
}

extern "C" bool reload_config(const char *source)
{
    if (!cm || !cm->IsLoaded()) {
        // TODO
        return false;
    }
    return cm->ReloadConfig(source);
}

extern "C" void set_config(const char* key, const char* value)
{
	if (!cm || !cm->IsLoaded()) {
		return;
	}
	cm->SetConfigMap(key, value);
}

extern "C" int config_get_intval(const char *key, int defult)
{
    if (!cm || !cm->IsLoaded()) {
        // TODO
        return defult;
    }
    return cm->ConfigGetIntVal(key, defult);
}

extern "C" const char *config_get_strval(const char *key, const char *defult)
{
    if (!cm || !cm->IsLoaded()) {
        // TODO
        return defult;
    }
    return cm->ConfigGetStrVal(key, defult);
}

extern "C" int plugin_init(int type)
{
    if (type == PROC_WORK) {
     
        DEBUG_LOG("total work num %u, current work index %u", get_work_num(), get_work_idx());
        if (get_work_idx() == get_work_num() - 1) {
            // 最后一个进程为control进程
            main_server = new (std::nothrow) Control();
            if (!main_server || main_server->Init()) {
                ERROR_LOG("new Control or Init failed");
                return -1;
            }
        } else {
            main_server = new (std::nothrow) Proxy();
            if (!main_server || main_server->Init()) {
                ERROR_LOG("new Proxy or Init failed");
                return -1;
            }
        }

    } else if (type == PROC_MAIN) {
        BOOT_LOG(0, "Proxy BuildTime: %s %s", __TIME__, __DATE__);
    }

    return 0;
}

extern "C" int plugin_fini(int type)
{
	DEBUG_LOG("Proxy finiting...");

    if(main_server)
    {
        main_server->Uninit();
        delete main_server;
        main_server = NULL;
    }

	DEBUG_LOG("Proxy finit successfully!");
	return 0;
}

extern "C" int get_pkg_len_cli(const char *buf, uint32_t len) 
{
    if (len < sizeof(uint32_t)) {
        return 0;
    }

    return *(uint32_t*)buf;
}

extern "C" int get_pkg_len_ser(int fd, const char *buf, uint32_t len) 
{
    return main_server->GetPkgLenSer(fd, buf, len);
}

extern "C" int check_open_cli(uint32_t ip, uint16_t port) 
{
	return 0;
}

extern "C" int select_channel(int fd, const char *buf, uint32_t len, uint32_t ip, uint32_t work_num) 
{
    static int serialize_num = 0;
    return (serialize_num++) % (work_num - 1);  // 留出control进程
}

extern "C" int shmq_pushed(int fd, const char *buf, uint32_t len, int flag)
{
    return 0;
}
 
extern "C" void time_event()
{
    // TODO 调试加的限制
    // if (get_work_idx() < get_work_num() - 1) 
    main_server->TimeEvent();

    // 检查超时请求
    // 检查有没有后端服务连接断开，如果有进行重连
}

extern "C" void proc_pkg_cli(int fd, const char *buf, uint32_t len)
{
    main_server->ProcPkgCli(fd, buf, len);
    
    // 从内存池中获取client_info空间，插入hash表
    // 插入fd链表数组，以便客户端关闭时删除其对应的所有请求
    // 根据路由表net_send_ser，即通过ServiceGroup（同时要将此client_info加入到timeout定时器）
}

extern "C" void proc_pkg_ser(int fd, const char *buf, uint32_t len)
{
    main_server->ProcPkgSer(fd, buf, len);
    
    // 从hash表中找到此client_info，未找到则丢弃(可能是超时了)
    // 检查此fd链表数组是否存在，若不存在则客户端已关闭
    // 从timeout定时器中删除(准备返回给客户端或是下一次加入到timeout定时器)
    // 发送到client或是server
}

extern "C" void link_up_cli(int fd, uint32_t ip)
{
    // TODO 调试加的限制
    // if (get_work_idx() < get_work_num() - 1) 
    main_server->LinkUpCli(fd, ip);

    // TODO 初始化此fd对应的fd链表数组
}

extern "C" void link_up_ser(int fd, uint32_t ip, uint16_t port)
{
    main_server->LinkUpSer(fd, ip, port);
    
    // 修改此service为Enable
}

extern "C" void link_down_cli(int fd)
{
    // TODO 调试加的限制
    // if (get_work_idx() < get_work_num() - 1) 
    main_server->LinkDownCli(fd);
    
    // TODO 删除fd链表数组中此fd对应的链表的所有的client_info
}

extern "C" void link_down_ser(int fd)
{
    main_server->LinkDownSer(fd);
    
    // 修改backend_service_map中此service为Disable，并重新异步建立连接(此处实现须谨慎)
}
