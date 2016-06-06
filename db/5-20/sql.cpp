
/** 
  * @file sql.cpp
  * @brief  测试文件                                                                  
  */

#include <iostream>
#include <stdio.h>
#include <sstream>
#include "dbbase.h"
using namespace std;


MysqlConInfo  sql_coninfo={"localhost","root","123456","test"};



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


 /**
 *    将 字节顺序为低高字节集转换为32位无符号整型
 */
inline Uint32 LHCharToUint32(const Uint8* buf)
{
	Uint32 val = 0;
	val = LHCharToUint16(buf + 2);
	val = val << 16;

	val += LHCharToUint16(buf);
	return val;
};

/** 
 * @mainpage    华睿云服务器数据库设计
 * @section    介绍
 *            本工程总共只有4个文件
 * @ref    base.h
 * @ref    dbbase.h
 * @ref    dbbase.cpp
 * @ref    sql.cpp   \n                                                                   
 */
int main(int argc, char const *argv[])
{
	init_mysql(&sql_coninfo);


	/*********************************************************插入接口***************************************************************/



	/*  ***********************1插入通用设备  *************************/	
		
	// RtuRegInfo info;
	// info.hostadd=354353153;
	// info.rtuadd=11;
	// info.rtutype=RTU_TYPE_LIGHTDET;   //RTU_TYPE_BED   RTU_TYPE_IRPROBE  RTU_TYPE_LIGHTDET 
	// get_sys_time(info.rturegtime);
	// info.rtustate=RTU_STATE_VALID;
	// int ret=insert_db_rtu(info);


	// ret=insert_db_rtu(info);

	/*  ***********************2 主机注册  *************************/	
		// HostRegInfo hostinfo;
		// sprintf(hostinfo.hostname,"crazygeek");
		// sprintf(hostinfo.hostpwd,"123456");
		// hostinfo.hostadd=354353149;	
		// get_sys_time(hostinfo.hostregtime);
		// hostinfo.hoststate=RTU_STATE_VALID;
		// hostinfo.hostip=19216801;
		// get_sys_time(hostinfo.lastcomtime);
		// insert_db_host(hostinfo);

	/************************3 创建 、修改 情景  ***********************************/
		SceneInfo info;
		info.hostadd=354353152;
		info.sceneid=11;
		sprintf(info.scenename,"上班");
		info.scenertunum=1;
		info.scenepicid=9;
		get_sys_time(info.rturegtime);
		info.rtubind=NULL;
		info.rtubind=(RtuBind*)malloc(sizeof(RtuBind)*info.scenertunum);
		info.rtubind[0].type=1;
		info.rtubind[0].add=1;
		info.rtubind[0].oper=1;
		info.rtubind[0].delay=1;
		if(update_db_sceneinfo(info) !=0 )
		{
			printf("update fail\n");
		}
	/************************4 创建 、修改 任务 ***********************************/
		TaskInfo taskinfo;
		memset(&taskinfo,0,sizeof(taskinfo));
		taskinfo.hostadd=354353153;
		taskinfo.taskid=11;
		strcpy(taskinfo.taskname,"setup");
		taskinfo.taskrtunum=1;
		get_sys_time(taskinfo.taskregtime);
		//info.rtubind=NULL;
		taskinfo.rtubind=(RtuBind*)malloc(sizeof(RtuBind)*taskinfo.taskrtunum);
		taskinfo.rtubind[0].type=1;
		taskinfo.rtubind[0].add=1;
		taskinfo.rtubind[0].oper=1;
		taskinfo.rtubind[0].delay=1;
		update_db_taskinfo(taskinfo);

	/*********************5 添加  编辑应用设备 ****************************/
	 //   AppRtuInfo info;
	 //   info.hostadd=354353150;
	 //   info.rtuadd=11;
	 //   info.rtutype= 2;// 电视  
	 //   sprintf(info.rtuname,"mytv1111111");
		// info.irrtuadd=12345678;
		// get_sys_time(info.rturegtime);
		// info.rtustate=RTU_STATE_VALID;
		// if(update_db_apprtu(info) !=0 )
		// {
		// 	printf("update fail\n");
		// }

	/************************************************查询接口*****************************************************/
	/*************1 主机信息查询*******/
	// HostRegInfo hostinfo;
	// int ret=select_db_hostinfo(&hostinfo,"crazycrazy");
	// if(ret == 0)
	// {
	// 	printf("%d\n", hostinfo.hostadd);
	// 	printf("%s\n", hostinfo.hostpwd);
	// 	printf("%d\n", hostinfo.hoststate);
	// 	printf("%s\n", hostinfo.lastcomtime);		
	// 	printf("%d\n", hostinfo.hostip);	
	// }

	/************************************************删除接口*****************************************************/
	/***********************1 删除设备，情景，任务  使用同一个接口  ************/
	RtuRemoveInfo rtuinfo={
	354353152,
	11,
	RTU_TYPE_SCENEINFO,    //RTU_TYPE_USEDEVICE    RTU_TYPE_BED     RTU_TYPE_TASKINFO  RTU_TYPE_SCENEINFO
	};	
	delete_db_rtu(rtuinfo);
	/***********************2 删除主机   ************/

	//delete_db_host("crazygeek");


	/***********************************************更新接口*****************************************************/
	/************1  更新常规设备   控制  绑定****************  */
	 // NativeRtuInfo rtuinfo;
	 // memset(&rtuinfo,0,sizeof(rtuinfo));
	 // RtuRegInfo info;
	 // info.hostadd=354353153;
	 // info.rtuadd=11;
	 // info.rtutype=RTU_TYPE_CTRLBOX;
	 // get_sys_time(info.rturegtime);
	 // info.rtustate=RTU_STATE_VALID;

	 // rtuinfo.rtuinfo=info;
	 // rtuinfo.relayinfo[0].val=(2 & 0x03);
	 // rtuinfo.relayinfo[1].val=(2 & 0x03);
	 // rtuinfo.bind_num=2;
	 // rtuinfo.relayinfo[0].id=0;
	 // rtuinfo.relayinfo[1].id=1;
	 // strcpy(rtuinfo.relayinfo[0].name,"hello");
	 // strcpy(rtuinfo.rtuinfo.rtuname,"hello");
	 // update_db_nativertu(rtuinfo,1);

	/*********************2 更新安防设备  控制*******************/
	// SafetyRtu  safetyrtu;  //RTU_TYPE_IRPROBE
	// safetyrtu.rtuinfo.rtutype=RTU_TYPE_IRPROBE;
	// safetyrtu.rtuinfo.hostadd=354353153;
	// safetyrtu.rtuinfo.rtuadd=11;
	// safetyrtu.rtuplace=0;
	// safetyrtu.rtuactstate=0;
	// update_db_safetyrtu(safetyrtu);
	/*********************3 更新主机信息*******************/
	// HostRegInfo hostinfo;
	// hostinfo.hostip=0xfffffffff;
	// strcpy(hostinfo.hostname,"crazycrazy");
	// update_db_host(hostinfo);
	/*****************4  插入情景面板  ************************/
		// SceneRtu  scenertu;
		// scenertu.rtuinfo.hostadd=354353153;
		// scenertu.rtuinfo.rtuadd=11;
		// scenertu.rtuinfo.rtutype=RTU_TYPE_IRPROBE;
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
		// update_db_scenertu(scenertu);
	/******************5 更新智能床设备   控制结果保存**********************/
	// ExNativeRtu rtuinfo;
	// rtuinfo.rtuinfo.hostadd=354353153;
	// rtuinfo.rtuinfo.rtuadd=11;
	// rtuinfo.relayinfo[0].val=3;
	// rtuinfo.relayinfo[1].val=3;
	// rtuinfo.relayinfo[2].val=3;
	// rtuinfo.relayinfo[3].val=3;
	// rtuinfo.relayinfo[4].val=3;
	// update_db_exnativertu(rtuinfo);

	/*****************6 修改主机密码**********/
	// HostMan hostman;
	// strcpy(hostman.oldname,"crazyman");
	// strcpy(hostman.oldpwd,"123456");
	// strcpy(hostman.newname,"manman");
	// strcpy(hostman.newpwd,"654321");
	// update_db_hostname(hostman);


    /*****************7  更新 传感器设备 *****************
	typedef struct tagDetectorRtu
	{
		RtuRegInfo rtuinfo;       //设备头信息 

		Uint8      valtype;      
		DetectorInfo detectorinfo;
	}DetectorRtu;
	操作类型  
	 如果是探测：0x01光照，0x02可燃气危险指数，0x03可燃气浓度，0x04温湿度空气质量，0x05湿敏告警
 	如果是 设置  ： 0x01设置光照联动动作值及联动标志，同理0x02、0x03、0x04、0x05、0x06分别设置燃气、温度、湿度、空气质量、湿敏联动动作值及联动标志
	
     ****************************************************************/
 //    DetectorRtu  rtuinfo;
 //    memset(&rtuinfo,0,sizeof(rtuinfo));
 //    rtuinfo.rtuinfo=info;
 //    rtuinfo.valtype=5;
 //    rtuinfo.detectorinfo.flag=1;
 //    rtuinfo.detectorinfo.val[0]=1000;
 //    rtuinfo.detectorinfo.downval=1000;
 //    rtuinfo.detectorinfo.upval=1000;
	// update_db_detectorrtu(rtuinfo, 1);


	/***************** 8 传感器设备联动更新****************/
	// DetectorBind rtuinfo;
	// rtuinfo.hostadd=354353153;
	// rtuinfo.rtuadd=11;
	// rtuinfo.level=5;
	// rtuinfo.bind_num=1;
	// rtuinfo.rtubind=(RtuBind*)malloc(sizeof(RtuBind)*rtuinfo.bind_num);
	// rtuinfo.rtubind[0].type=1;
	// rtuinfo.rtubind[0].add=1;
	// rtuinfo.rtubind[0].oper=1;
	// rtuinfo.rtubind[0].delay=1;
	// update_db_detectorbind(rtuinfo);


	/*************** 9 门锁密码设置********************/
	// SafetyPwdInfo  pwdinfo;
	// pwdinfo.id=1;
	// pwdinfo.hostadd=354353153;
	// pwdinfo.rtuadd=11;
	// strcpy(pwdinfo.oldpwd,"");
	// strcpy(pwdinfo.newpwd,"1234567890");

	// int ret=update_db_safetypwd(pwdinfo);
	// if(ret ==0 )
	// {
	// 	printf("update success..\n");
	// }

		/********************10  更新用户设备信息 ************ 
	typedef struct tagRtuUpdate
	{
		Uint32 hostadd;     //  主机地址
		Uint32 rtuadd;      //   设备地址  
		Uint8  rtutype;    //  设备类型  
		Uint8  opertype;  // 描述类型  0x00-0x03表示继电器序号，即修改绑定的负载的名称和安装位置   0xff表示修改设备或情景的名称和安装位置
		char   rtuname[32];   //  设备名称
		char   rtupath[32];    //设备路径 
	}RtuUpdate;

	 *****************************************************************/
	// RtuUpdate rtuinfo;
	// rtuinfo.hostadd=354353153;
	// rtuinfo.rtuadd=11;
	// rtuinfo.rtutype=RTU_TYPE_BED;
	// rtuinfo.opertype=0x04;
	// strcpy(rtuinfo.rtuname,"hahahaha");
	// strcpy(rtuinfo.rtupath,"/home/geek");
	// update_db_rtuinfo(rtuinfo);

	/****************************************************** 其他接口 ****************************************/
	/*  1  获取当前主机数  */
	// int count=GetHostCount();
	// printf("host count:%d\n",count);


	// Uint32 ip;
	// GetIpByHostAdd(354353150, ip);
	// printf("ip:%d\n",ip);

	return 0;
}

