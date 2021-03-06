/**************************************************************
 * Copyright (C) 2016-2016 All rights reserved.
 * @Version: 1.0
 * @Created: 2016-05-06 16:43
 * @Author: menqi - 1083734876@qq.com
 * @Description: 
 *      mysql 服务器接口头文件
 * @History: 
 **************************************************************/
#ifndef   __DBBASE_H_
#define   __DBBASE_H_

#include <stdio.h>
#include <mysql/mysql.h>
#include <sstream>
#include <iostream>
#include <string.h>
#include "base.h"

#define MAX_NAME_LEN 256
#define MAX_USERNAME_LEN		32			//用户名长度
#define MAX_PASSWORD_LEN		16          //用户密码长度
#define MAX_SQLUSERNAME_LEN     16          //数据库用户名长度
#define MAX_SQLPASSWD_LEN       32          //数据库密码

using namespace std;


//////////////////////////////////////////////////////////
//数据表名称定义
const Int8 DB_NAME[] = "HuaRuiCloud";
const Int8 DB_TABLE_HOSTINFO[] = "HostInfo";
const Int8 DB_TABLE_NATIVERTU[] = "NativeRtu";
const Int8 DB_TABLE_SCENEINFO[] = "SceneInfo";
const Int8 DB_TABLE_SCENEINFOCONTINUED[] = "SceneInfoContinued";
const Int8 DB_TABLE_SCENERTU[] = "SceneRtu";
const Int8 DB_TABLE_TRANSPONRTU[] = "TransponRtu";
const Int8 DB_TABLE_SAFETYRTU[] = "SafetyRtu";
const Int8 DB_TABLE_SAFETYPWD[] = "SafetyPwdInfo";
const Int8 DB_TABLE_SAFETYSET[] = "SafetySetBind";
const Int8 DB_TABLE_SAFETYREMOVE[] = "SafetyRemoveBind";
const Int8 DB_TABLE_APPRTU[] = "AppRtu";
const Int8 DB_TABLE_DETECTORRTU[] = "DetectorRtu";
const Int8 DB_TABLE_DETECTORBIND[] = "DetectorBind";
const Int8 DB_TABLE_TASKINFO[] = "TaskInfo";
const Int8 DB_TABLE_EXNATIVERTU[] = "ExNativeRtu";
const Int8 DB_TABLE_RTUTYPE[] = "RtuTypeInfo";
const Int8 DB_TABLE_USEDEVTYPE[] = "UseDevTypeInfo";
//const Int8 DB_TABLE_UPDATEPACKINFO[] = "UpdatePackInfo";


/* table name  */
const char DB_TABLE_NAMELIST[][32] = 
{
	"HostInfo",
	"NativeRtu",
	"SceneRtu",
	"SceneInfo",
	"TransponRtu",
	"AppRtu",
	"SafetyRtu",
	"SafetyPwdInfo",
	"SafetySetBind",
	"SafetyRemoveBind",
	"SceneInfoContinued",
	"DetectorRtu",
	"DetectorBind",
	"TaskInfo",
	"ExNativeRtu",
	//here add new tables...
	"RtuTypeInfo",
	"UseDevTypeInfo",
	//	"UpdatePackInfo",
};

#define DB_TYPE_HOSTINFO        0   //主机信息
#define DB_TYPE_NATIVERTU		1	//常规设备数据对象类型，即对应设备类型中的智能开关、智能插座、继电器控制盒、RGB灯、窗帘控制器
#define DB_TYPE_SCENERTU		2	//情景面板数据对象类型，即对应设备类型中的情景面板
#define	DB_TYPE_SCENEINFO		3	//情景信息数据对象类型，与设备类型无关
#define DB_TYPE_TRANSPOND       4   //转发设备数据对象类型，即对应设备类型中的蓝牙遥控器、红外转发器
#define DB_TYPE_USEDEVICE       5   //应用设备对象类型，即对应设备类型中的应用设备
#define DB_TYPE_SAFETY          6   //安防设备数据对象类型，即对应设备类型中的红外入侵探测器、智能门磁、智能门锁、智能门铃
#define DB_TYPE_SAFETY_PWD      7   //安防设备数据的密码
#define DB_TYPE_SAFETY_SET      8   //安防设备数据的设防联动
#define DB_TYPE_SAFETY_REMOVE   9   //安防设备数据的撤防联动
#define DB_TYPE_SCENEINFO_CON   10  //情景信息续表
#define DB_TYPE_DETECTORRTU     11  //传感器设备数据对象类型
#define DB_TYPE_DETECTORBIND    12  //传感器设备联动
#define DB_TYPE_TASKINFO        13
#define DB_TYPE_EXNATIVERTU     14
#define MAX_DB_TYPE             15
#define DB_TYPE_RTUTYPE         (MAX_DB_TYPE) 
#define DB_TYPE_USEDEVTYPE      (MAX_DB_TYPE+1)  
//#define DB_TYPE_UPDATEPACK      (MAX_DB_TYPE+2)

//////////////////////////////////////////////////////////
//设备类型定义    rtu  type  
#define RTU_TYPE_HOST		 0		//主机类型
#define RTU_TYPE_SWITCH		 1		//智能开关类型
#define RTU_TYPE_SCENE		 2		//情景面板类型
#define RTU_TYPE_PLUG		 3		//智能插座类型
#define RTU_TYPE_CTRLBOX	 4		//继电器控制盒类型
#define RTU_TYPE_RGB		 5		//RGB灯
#define RTU_TYPE_BTREMOTE    6      //蓝牙遥控器
#define RTU_TYPE_IRTRANS     7      //红外转发器
#define RTU_TYPE_CURTAIN     8      //窗帘控制器
#define RTU_TYPE_IRPROBE     9      //红外入侵探测器
#define RTU_TYPE_DOORCONTACT 10     //智能门磁
#define RTU_TYPE_DOORLOCK    11     //智能门锁
#define RTU_TYPE_DOORBELL    12     //智能门铃
#define RTU_TYPE_LIGHTDET    13     //光照探测器
#define RTU_TYPE_GASDET      14     //可燃气探测器
#define RTU_TYPE_TPMHUMDET   15     //温湿度空气质量探测器
#define RTU_TYPE_DAMDET      16     //湿敏探测器
#define RTU_TYPE_ROBTIC      17     //机械手
#define RTU_TYPE_BED         18     //智能床

#define RTU_TYPE_USERINFO   0xf8    //用户信息
#define RTU_TYPE_FLOORINFO  0xf9    //楼层信息
#define RTU_TYPE_USEDEVICE  0xfa    //应用设备，如用户的电视、空调、机机盒、摄像头
#define RTU_TYPE_SERVER     0xfb
#define RTU_TYPE_TASKINFO	0xfc	//任务信息，即存储的数据
#define RTU_TYPE_SCENEINFO	0xfd	//情景信息，即存储的数据
#define RTU_TYPE_APP		0xfe	//APP应用程序
#define RTU_TYPE_INVALID	0xff	//无效

#define MIN_RTU_TYPE		(RTU_TYPE_HOST)
#define MAX_RTU_TYPE		((RTU_TYPE_BED)+1)



/* 设备类型同 table  关联    */
struct DBTypeInfo
{
	Int8 rtutype;
	const char tablename[32];
};
static DBTypeInfo gTypeInfoList[]={
	{0,"HostInfo"},                  //主机类型
	{1,"NativeRtu"},                 //智能开关类型
	{2,"SceneRtu"},                  //情景面板类型
	{3,"NativeRtu"},                 //智能插座类型
	{4,"NativeRtu"},                 //继电器控制盒
	{5,"NativeRtu"},                 //RGB灯
	{6,"TransponRtu"},               //蓝牙遥控器
	{7,"TransponRtu"},               //红外转发器
	{8,"NativeRtu"},                 //窗帘控制器
	{9,"SafetyRtu"},                 //红外入侵探测
	{10,"SafetyRtu"},                //智能门磁
	{11,"SafetyRtu"},                //智能门锁
	{12,"SafetyRtu"},                //智能门铃
	{13,"DetectorRtu"},              //光照探测器
	{14,"DetectorRtu"},              //可燃气探测器
	{15,"DetectorRtu"},              //温湿度空气质
	{16,"DetectorRtu"},              //湿敏探测器
	{17,"NativeRtu"},                //机械手
	{18,"ExNativeRtu"},              //智能床

	{0xfa,"AppRtu"},                 //应用设备，如
	{0xfd,"SceneInfo"},              //情景信息，即
	
};	


/* mysql 连接信息 结构体  */
typedef struct SqlConinfoDef 
{
	char hostname[MAX_NAME_LEN];
	char username[MAX_SQLUSERNAME_LEN];
	char password[MAX_SQLPASSWD_LEN];
	char dbname[MAX_NAME_LEN];

} MysqlConInfo;


typedef enum eRtustatus
{
	RTU_STATE_NEW = 1,		//新加状态，如注册过程中刚找到还没有上送到APP
	RTU_STATE_REG,       	//注册过程状态，即APP已经知道该设备存在，但还没有允许设备注册到主机中的一种状态
	RTU_STATE_VALID,		//有效状态，即已经允许注册到主机中，
	RTU_STATE_INVALID,		//无效状态，如删除后的状态
}Rtustatus;



/*   设备注册结构体      */
typedef struct tagRtuRegInfo
{
	Uint64 uniqueindex;// 唯一索引
	Uint32 hostadd;    //  主机地址
	Uint32 rtuadd;     //  设备地址
	Uint32 rtutype;    //  设备类型
	char rturegtime[30]; //  注册时间
	Rtustatus rtustate; // 注册状态

}RtuRegInfo;

/* 删除设备结构体 */
typedef struct tagRtuRemoveInfo
{
	Uint32 hostadd;    //  主机地址
	Uint32 rtuadd;     //  设备地址
	Uint32 rtutype;    //  设备类型

}RtuRemoveInfo;

/* 主机注册结构体 */
typedef struct tagHostRegInfo
{	
	char hostname[MAX_USERNAME_LEN];  //主机名
	Uint32  hostadd;  //主机地址
	char hostpwd[MAX_PASSWORD_LEN];   //主机密码
	char hostregtime[30];              //  注册时间
	Rtustatus hoststate;               // 注册状态
	Uint32  hostip;                      //  主机  ip ：  bytes[3:2:1:0]
	char lastcomtime[30];             //  最近通信时间  
	
}HostRegInfo;






/**
 * @brief init_mysql   连接数据库接口
 *
 * @Param: info    连接mysql时，mysql相关信息结构体  
 *
 * Returns: -1 连接失败   0  成功
 */
int init_mysql(MysqlConInfo *info);  

/**
 * @brief insert_db_rtu 插入设备接口  ，不包括插入主机
 *
 * @Param: pRtuinfo  插入设备结构体  
 *
 * Returns:-1 插入失败  0  插入成功
 */
int insert_db_rtu(RtuRegInfo* pRtuinfo);

/**
 * @brief check_connect 检查连接  
 *
 * Returns: 
 */
int check_connect();

/**
 * @brief findHostname 查找对应的主机名
 *
 * @Param: hostname  
 *
 * Returns: 返回值   -1  错误   0  没有找到对应的   >0  找到的主机个数
 */
int findHostname(char* hostname);

/**
 * @brief insert_db_host  插入主机  接口
 *
 * @Param: pHostinfo   主机信息结构体
 *
 * Returns: -1  插入失败   0  插入成功 
 */
int insert_db_host(HostRegInfo* pHostinfo);



/**
 * @brief GetHostIdByName 
 *        根据主机名查找对应的ID
 * @Param: hostname  主机名
 * @Param: hostid  找到的id放入该参数 
 *
 * Returns: -1  查找错误  0  查找正确
 */
Uint32 GetHostIdByName(char* hostname, Uint32 & hostid);

/**
 * @brief select_db_hostinfo  
 *           根据主机名 查询主机信息  ，用于  APP远程登陆
 * @Param: pHostinfo
 * @Param: hostname
 *
 * Returns: 返回值   -1  错误   0  正确    1   没有数据 
 */
int select_db_hostinfo(HostRegInfo* pHostinfo,char* hostname);

/**
 * @brief delete_db_rtu 
 *  删除设备 接口   不包括删除主机  
 * @Param: pRtuinfo
 *
 * Returns: -1  删除出错 0 删除成功  或者  没有删除项
 */
int delete_db_rtu(RtuRemoveInfo* pRtuinfo);

/**
 * @brief GetHostCount 
 *   拿到 连接上来的主机个数  
 * Returns:   主机个数
 */
int GetHostCount();

/**
 * @brief delete_db_host 
 *   删除主机设备
 * @Param: hostname
 *
 * Returns: 
 */
int delete_db_host(char* hostname);

/**
 * @brief get_table_name 
 *    根据设备类型   拿到对应的表名
 * @Param: type
 *     设备类型
 * Returns:  表名   为  string 类型  
 */
std::string  get_table_name(int type);



#endif
