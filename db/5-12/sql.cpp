#include <iostream>
#include <stdio.h>
#include <sstream>
#include "dbbase.h"
using namespace std;


MysqlConInfo  sql_coninfo={"localhost","root","123456","HuaRuiCloud"};


/**
 *    将 低高字节转换为  16 位  无符号整型
 */
inline Uint16 LHCharToUint16(const Uint8* buf)
{
	Uint16 val = 0;

	val = (*(buf + 1)) & 0xff;
	val = (val << 8);
	val += (*(buf)) & 0xff;

	return val;
};

/************************************************************************
  函数名称：
  LHCharToUint32
  输入参数：
  PUint8 buf 待转换的字节集
  输出参数：
  无
  返回结果：
  Uint32 转换结果
  描述：
  将字节顺序为低高字节集转换为32位无符号整型
 ************************************************************************/
inline Uint32 LHCharToUint32(const Uint8* buf)
{
	Uint32 val = 0;
	val = LHCharToUint16(buf + 2);
	val = val << 16;

	val += LHCharToUint16(buf);
	return val;
};


int main(int argc, char const *argv[])
{
	init_mysql(&sql_coninfo);

/*  ************************************插入设备  *************************/	
		
		// RtuRegInfo info;
		// info.hostadd=354353153;
		// info.rtuadd=11;
		// Uint64 uniqueindex = info.hostadd;
		// uniqueindex = uniqueindex << 32;
		// uniqueindex +=info.rtuadd;
		// info.rtutype=RTU_TYPE_SCENE;
		// get_sys_time(info.rturegtime);
		// info.rtustate=RTU_STATE_VALID;
		// info.uniqueindex=uniqueindex;
		// int ret=insert_db_rtu(&info);

/******************************************************************************/


/***************************************插入主机*****************************************/
		// HostRegInfo hostinfo;
		// sprintf(hostinfo.hostname,"geekcrazy");
		// sprintf(hostinfo.hostpwd,"123456");
		// hostinfo.hostadd=354353154;	
		// get_sys_time(hostinfo.hostregtime);
		// hostinfo.hoststate=RTU_STATE_VALID;
		// hostinfo.hostip=19216801;
		// get_sys_time(hostinfo.lastcomtime);
		//int ret=insert_db_host(&hostinfo);


	/*	HostRegInfo hostinfo2;
		ret=select_db_hostinfo(&hostinfo2,"geekcrazy");
		if(ret == 0)
		{
		printf("%d\n", hostinfo.hostadd);
		printf("%s\n", hostinfo.hostpwd);
		printf("%d\n", hostinfo.hoststate);
		printf("%s\n", hostinfo.lastcomtime);		
		}
		RtuRemoveInfo rtuinfo={
		354353154,
		11,
		RTU_TYPE_HOST,
		};*/

	//ret=delete_db_rtu(&rtuinfo);
	//int count=GetHostCount();
	//printf("host count:%d\n",count);

	//delete_db_host("geekcrazy");





/************************创建 、修改 情景  *****************************************


typedef struct tagSceneInfo
{
	Uint64 uniqueindex;// 唯一索引
	Uint32 hostadd;    //  主机地址
	Uint32 sceneid;     //  情景ID
	Uint32 scenepicid;  //   情景图标ID 
	char scenename[32]; //  情景名字
	char rturegtime[30]; //  注册时间
	short scenertunum;   //  情景设定的设备数
}SceneInfo;


*********************************************/
	// SceneInfo info;
	// info.hostadd=354353154;
	// info.sceneid=11;
	// Uint64 uniqueindex = info.hostadd;
	// uniqueindex = uniqueindex << 32;
	// uniqueindex +=info.sceneid;
	// sprintf(info.scenename,"上班");
	// info.uniqueindex=uniqueindex;
	// info.scenertunum=10;
	// info.scenepicid=9;
	// get_sys_time(info.rturegtime);
	// if(update_db_sceneinfo(info) !=0 )
	// {
	// 	printf("update fail\n");
	// }
	// RtuRemoveInfo rtuinfo={
	// 354353154,
	// 11,
	// RTU_TYPE_SCENEINFO,
	// };	
	// delete_db_rtu(&rtuinfo);



	/**********************************************添加 编辑应用设备 *********
	   typedef struct tagAppRtuInfo
	   {
	   Uint64 uniqueindex;// 唯一索引
	   Uint32 hostadd;    //  主机地址
	   Uint32 rtuadd;     //  设备地址
	   Uint32 rtutype;    //  设备类型
	   char rturegtime[30]; //  注册时间
	   char rtuname[32];    //设备名称 
	   Uint32 irrtuadd;   // 绑定的红外转发器地址
	   Uint32 irlearn;    // 红外学习标志  
	   Rtustatus rtustate; // 注册状态	
	   }AppRtuInfo;


	***********************************************************/
   AppRtuInfo info;
   info.hostadd=354353154;
   info.rtuadd=11;
   Uint64 uniqueindex = info.hostadd;
   uniqueindex = uniqueindex << 32;
   uniqueindex +=info.rtuadd;
   info.rtutype= 2;// 电视  
   sprintf(info.rtuname,"mytv1111111");
	info.irrtuadd=12345678;
	get_sys_time(info.rturegtime);
	info.rtustate=RTU_STATE_VALID;
	info.uniqueindex=uniqueindex;
	if(update_db_apprtu(info) !=0 )
	{
		printf("update fail\n");
	}
	// RtuRemoveInfo rtuinfo={
	// 354353154,
	// 11,
	// RTU_TYPE_USEDEVICE,
	// };	
	// delete_db_rtu(&rtuinfo);







/*********************************更新常规设备*********************************  */
/*	 NativeRtuInfo rtuinfo;
	 memset(&rtuinfo,0,sizeof(rtuinfo));
	 RtuRegInfo info;

	 Uint64 uniqueindex = info.hostadd;
	 uniqueindex = uniqueindex << 32;
	 uniqueindex +=info.rtuadd;
	 info.hostadd=354353153;
	 info.rtuadd=11;
	 info.rtutype=RTU_TYPE_CTRLBOX;
	 get_sys_time(info.rturegtime);
	 info.rtustate=RTU_STATE_VALID;
	 info.uniqueindex=uniqueindex;
	 rtuinfo.rtuinfo=info;
	 rtuinfo.relayinfo[0].val=(2 & 0x03);
	 rtuinfo.relayinfo[1].val=(2 & 0x03);
	 rtuinfo.bind_num=2;
	 rtuinfo.relayinfo[0].id=0;
	 rtuinfo.relayinfo[1].id=1;
	 strcpy(rtuinfo.relayinfo[0].name,"hello");
	 strcpy(rtuinfo.rtuinfo.rtuname,"hello");
	update_db_nativertu(rtuinfo,2);*/


/*****************************更新安防设备************************************************


typedef struct tagSafetyRtu
{
	RtuRegInfo rtuinfo;       //设备头信息 
	Uint16 rtuplace;          //设备布防状态   0x00：撤防，0x01布防，0xff表示未知
	Uint16 rtuactstate;  	  //设备动作状态   0x00：门磁合上、红外探测无入侵、门铃无意义，0x01：门磁分开、红外探测有入侵、门铃无意义；

}SafetyRtu;

	// SafetyRtu  safetyrtu;  //RTU_TYPE_IRPROBE
	// safetyrtu.rtuinfo=info;
	// safetyrtu.rtuplace=0;
	// safetyrtu.rtuactstate=0;
	// strcpy(safetyrtu.rtuinfo.rtuname,"hello");
	// strcpy(safetyrtu.rtuinfo.rtupath,"hello");
	// update_db_safetyrtu(safetyrtu, 1);

*********************************************************************************/

/****************************************更新主机信息***************************************/
/*	hostinfo.hostip=123456;
	strcpy(hostinfo.hostname,"crazyman");
	update_db_host(hostinfo,1,"geekcrazy");*/

/*******************************************插入情景面板  **********************
typedef struct tagSceneBind
{
	Uint16 id;          //  按键码  
	Uint16 type;        //  绑定类型  
	Uint32 add;         //  绑定设备/ 情景地址 
	Uint32 oper;          //  绑定设备继电器1-4的操作
}SceneBind;

typedef struct tagSceneRtu
{
	RtuRegInfo rtuinfo;       //设备头信息 
	Uint16 bind_num; 
	SceneBind  scene_bind[4]; 
}SceneRtu;
*******************************************************/
	// SceneRtu  scenertu;
	// scenertu.rtuinfo=info;
	// strcpy(scenertu.rtuinfo.rtuname,"hello");
	// strcpy(scenertu.rtuinfo.rtupath,"hello");
	// scenertu.bind_num=2;
	// scenertu.scene_bind[0].id=1;
	// scenertu.scene_bind[0].type=1;
	// scenertu.scene_bind[0].add=1;
	// scenertu.scene_bind[0].oper=1;
	// scenertu.scene_bind[1].id=2;
	// scenertu.scene_bind[1].type=1;
	// scenertu.scene_bind[1].add=1;
	// scenertu.scene_bind[1].oper=1;
	// update_db_scenertu(scenertu, 2);


	return 0;
}

