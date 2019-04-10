/*************************************************************************
    > File Name: redis_command.h
    > Author: ian
    > Mail: ian@taomee.com 
    > Created Time: Wed Dec 26 01:23:06 2018
 ************************************************************************/
#ifndef __FEEDS_REDIS_COMMAND__
#define __FEEDS_REDIS_COMMAND__

#define REDIS_PRFIX                     "feeds:"
#define REDIS_MID                       REDIS_PRFIX"%u:"

// 保存分组名的有序集合
#define R_GINFO                REDIS_MID"ginfo"
// 保存各个分组内的关注米米号 有序集合
#define R_GLIST                REDIS_MID"flist:%s"
// 保存每个分组关注的每个米米号的详情 HashMap
#define R_F_LIST               REDIS_MID"glist:"
#define R_FMID                 R_F_LIST"%u"
#define R_FMID_GNAME           "gname"
#define R_FMID_TYPE            "type"
#define R_FMID_TIME            "time"

//  保存关注的米米号列表 有序集合
#define R_A_LIST              REDIS_MID"list"
// 保存粉丝列表 有序集合
#define R_FANS                REDIS_MID"fans"

/*==========   关注需执行的redis命令  ======*/
//1.将关注的米米号加入指定分组集合
#define COMMAND_ADD_ATTENTION_1     "ZADD " R_GLIST" %u %u"       //ZADD key score member
//2.将关注米米号添加当前米米号和关注米米号的hashMap映射
#define COMMAND_ADD_ATTENTION_2     "HMSET " R_FMID " " R_FMID_GNAME " %s " R_FMID_TYPE " %u " R_FMID_TIME" %u"            //HMSET key field [field]
//3.将关注米米号添加到当前米米号的所有关注好友集合
#define COMMAND_ADD_ATTENTION_3     "ZADD " R_A_LIST" %u %u"
//4.将自己的米米号添加到关注米米号的粉丝集合
#define COMMAND_ADD_ATTENTION_4     "ZADD " R_FANS" %u %u"
/*==========   取关需执行的redis命令  ======*/
//1.查询关注米米号对应的分组名
#define COMMAND_DROP_ATTENTION_1    "HGET " R_FMID " " R_FMID_GNAME
//2.从分组集合中删除关注米米号
#define COMMAND_DROP_ATTENTION_2    "ZREM " R_GLIST" %u"
//3.从hashMap中删除映射关系
#define COMMAND_DROP_ATTENTION_3    "HDEL " R_FMID " " R_FMID_GNAME " " R_FMID_TYPE " " R_FMID_TIME
//4.从所有关注集合中删除关注米米号
#define COMMAND_DROP_ATTENTION_4    "ZREM " R_A_LIST" %u %u"
//5.从对方粉丝集合删除自己的米米号
#define COMMAND_DROP_ATTENTION_5    "ZREM " R_FANS" %u %u"

/*==========   添加分组需执行的redis命令  ======*/
#define COMMAND_ADD_GROUP           "ZADD " R_GINFO" %u %s"
/*==========   删除分组需执行的redis命令  ======*/
#define COMMAND_DROP_GROUP          "ZREM " R_GINFO" %s"
/*==========   查询分组需执行的redis命令  ======*/
#define COMMAND_QUERY_GROUP         "ZRANGE " R_GINFO" 0 -1"

/*==========   查询组内关注列表需执行的redis命令  ======*/
#define COMMAND_QUERY_BY_GROUP      "ZREVRANGE " R_GLIST" %d %d WITHSCORES"

/*==========   查询所有关注列表需执行的redis命令  ======*/
#define COMMAND_QUERY_LIST          "ZREVRANGE " R_A_LIST" %d %d WITHSCORES"

/*==========   查询粉丝列表需执行的redis命令  ======*/
#define COMMAND_QUERY_FANS          "ZREVRANGE " R_FANS" %d %d WITHSCORES"

/*==========   查询双方关系需执行的redis命令  ======*/
//1.查询我的关注集合是否存在对方米米号（存在：已关注）
#define COMMAND_QUERY_RELATION_1    "ZRANK " R_A_LIST" %u"
//2.查询我的粉丝集合是否存在对方米米号（存在：被关注）
#define COMMAND_QUERY_RELATION_2    "ZRANK " R_FANS" %u"

/*==========   最新更新文章的米米号集合  ======*/
#define FEED_LATEST_MIMIID          REDIS_PRFIX"active"
//1. 发布文章后，更新米米号对应的时间
#define COMMAND_UPDATE_FEED_TIME    "ZADD " FEED_LATEST_MIMIID" %u %u"
//2. 查询最新发布文章作者的米米号
#define COMMAND_QUERY_FEED_TIME     "ZREVRANGEBYSCORE "FEED_LATEST_MIMIID " %u %u LIMIT 0 %d"
#endif

