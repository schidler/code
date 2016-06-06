/**************************************************************
 * Copyright (C) 2016-2016 All rights reserved.
 * @Version: 1.0
 * @Created: 2016-05-06 16:45
 * @Author: menqi - 1083734876@qq.com
 * @Description: 
 *        mysql 服务器接口函数 
 * @History: 
 **************************************************************/

#include "dbbase.h"


MYSQL m_mysqlfd;   /*数据库连接描述符*/


bool  IsNativeRtu(Uint8 type)
{
	if (type == RTU_TYPE_SWITCH 
			|| type == RTU_TYPE_PLUG 
			|| type == RTU_TYPE_CTRLBOX 
			|| type == RTU_TYPE_RGB 
			|| type == RTU_TYPE_CURTAIN
			|| type == RTU_TYPE_ROBTIC)
		return true;
	else
		return false;
}


bool  IsSafetyRtu(Uint8 type)
{
	if (type == RTU_TYPE_IRPROBE 
		|| type == RTU_TYPE_DOORCONTACT 
		|| type == RTU_TYPE_DOORLOCK 
		|| type == RTU_TYPE_DOORBELL)
		return true;
	else
		return false;
}


/**
 * @brief init_mysql 
 *   初始化mysql , 连接，指定数据库
 * @Param: info    
 * 连接mysql时，mysql相关信息结构体   
 *
 * Returns: 0  成功  1  失败
 */
int init_mysql(MysqlConInfo *info)
{
	printf("+++++++++++++initmysql+++++++++++++\n");
	mysql_init(&m_mysqlfd);
	mysql_options(&m_mysqlfd, MYSQL_SET_CHARSET_NAME, "gbk");
	char value = 1;
	mysql_options(&m_mysqlfd, MYSQL_OPT_RECONNECT, &value);

	if (!mysql_real_connect(&m_mysqlfd, info->hostname, info->username, info->password, info->dbname, 03306, NULL/*Unix Socket 连接类型*/, 0/*运行成ODBC数据库标志*/))
	{
		mysql_close(&m_mysqlfd);
		return -1;
	}

	//使得由db指定的数据库成为 在由mysql指定的连接上的缺省(当前)数据库
	if (mysql_select_db(&m_mysqlfd, info->dbname) != 0)
	{
		printf("mysql_select_db failure \n");
		mysql_close(&m_mysqlfd);
		return -1;	
	}

	return 0;

}



/**
 * @brief get_table_name 
 *    根据设备类型   拿到对应的表名
 * @Param: type
 *     设备类型
 * Returns:  表名   为  string 类型  
 */
std::string  get_table_name(Uint8 type)
{
	Uint32 i;
	std::string ret = "";

	for (i = 0; i < sizeof(gTypeInfoList) / sizeof(gTypeInfoList[0]); i ++) {
		
		if (gTypeInfoList[i].rtutype == type) {
			ret = gTypeInfoList[i].tablename;
			break;
		}
	}
	return ret;  
}

/**
 * @brief check_connect 检查连接  
 *
 * Returns: 
 */
static int check_connect()
{

	return mysql_ping(&m_mysqlfd);

}

/**
 * @brief insert_mysql 
 *  数据库基本插入操作 
 * @Param: sql_cmd
 *   插入命令  为string 类型
 * Returns: 
 */
static int insert_mysql(string sql_cmd)
{
	/* 执行插入操作  */
	int ret = mysql_query(&m_mysqlfd, sql_cmd.c_str());

	if(ret != 0)
	{
		LOG_ERROR("insert error...\n");
		return -1;
	}
	return 0;
}


/**
 * @brief db_insert_query 
 *  数据库插入
 * @Param: colstr   列字段
 * @Param: valstr   值字段
 * @Param: tablename  数据表名
 *
 * Returns: 
 */
static int db_insert_query(const string &colstr, const string &valstr, const string &tablename)
{
	/*插入前 检查数据连接  */
	check_connect();

	if(strcmp(tablename.c_str(),"") == 0 )  
	{
		LOG_ERROR("no such table.\n");
		return -1;
	}

	string sql_cmd = "INSERT INTO ";
	sql_cmd += tablename;

	sql_cmd += "(";
	sql_cmd += colstr;
	sql_cmd += ") ";
	sql_cmd += "VALUES(";
	sql_cmd += valstr;
	sql_cmd += ")";
	LOG_DEBUG("db_insert_query len=%ld sql_cmd=%s\n", sql_cmd.length(), sql_cmd.c_str());
	return insert_mysql(sql_cmd);
}



/**
 * @brief DupicateKeyUpdateQuery 
 *  更新、插入查询：存在则更新不存在则插入
 * @Param: colstr  列
 * @Param: valstr  值 
 * @Param: updatestr  要更新的列
 * @Param: tablename  表名
 *
 * Returns: 
 */
static int DupicateKeyUpdateQuery(const string &colstr, const string &valstr, const string &updatestr, const string &tablename)
{
	if (colstr.empty() || valstr.empty() || updatestr.empty() || tablename.empty())
		return -1;

	check_connect();

	string sql = "INSERT INTO ";
	sql += tablename;
	sql += "(";
	sql += colstr;
	sql += ") ";
	sql += "VALUES(";
	sql += valstr;
	sql += ") on DUPLICATE KEY UPDATE ";
	sql += updatestr;
	LOG_DEBUG("DupicateKeyUpdateQuery len=%ld sql=%s\n", sql.length(), sql.c_str());

	return mysql_query(&m_mysqlfd, sql.c_str());
}

/**
 * @brief select_query 
 * 查找操作
 * @Param: sql   查找sql 命令
 * @Param: retstr  返回列字段，以逗号隔开
 *
 * Returns: 
 */
static int select_query(const string & sql, string &retstr)
{
	check_connect();

	int nRet = mysql_query(&m_mysqlfd, sql.c_str());
	if (nRet != 0)
	{
		LOG_ERROR("mysql_query failed select error:%s\n ", mysql_error(&m_mysqlfd));
		return -1;
	}

	MYSQL_RES   *pres = mysql_use_result(&m_mysqlfd);//返回执行结果，适用于数据量较大时
	if (pres == NULL)
	{
		LOG_ERROR("mysql_use_result failed\n ");
		return -1;
	}

	Uint32 fieldcoun = mysql_field_count(&m_mysqlfd);//返回查询结果中的列数（column数）
	Uint32 index = 0;
	MYSQL_ROW  mysqlrow;
	while ((mysqlrow = mysql_fetch_row(pres)) != NULL)
	{//后继的调用正常返回的内容
		index = 0;
		while (index < fieldcoun)
		{
			if (mysqlrow[index])
			{
				retstr.append(mysqlrow[index]);
				retstr.append(",");
			}
			index++;
		}
		LOG_DEBUG("select success fieldcoun=%d len=%ld ret=%s\n", fieldcoun, retstr.length(), retstr.c_str());
	}
	mysql_free_result(pres);

	return retstr.empty()?1:0;
}


/**
 * @brief db_select_query 
 *  查找操作
 * @Param: colstr   列字段
 * @Param: wheresrt 过滤字段
 * @Param: tablename 表名
 * @Param: retstr   查找结果
 *
 * Returns: 
 */
static Uint32 db_select_query(const string &colstr, const string &wheresrt, const string &tablename, string &retstr)
{
	retstr.clear();
	if (colstr.empty())
	{
		LOG_ERROR("colstr is empty.\n");
		return -1;
	}
	if(strcmp(tablename.c_str(),"") == 0 )  
	{
		LOG_ERROR("no such table.\n");
		return -1;
	}


	string sql = "SELECT ";
	sql += colstr;
	sql += " from ";
	sql += tablename;

	if (!wheresrt.empty())
	{
		sql += " where ";
		sql += wheresrt;
	}
	LOG_DEBUG("SelectQuery len=%ld sql=%s\n", sql.length(), sql.c_str());

	return select_query(sql,retstr);
}


/**
 * @brief db_update_query 
 *  更新查询
 * @Param: setstr     设置字符串
 * @Param: wherestr   条件字符串
 * @Param: tablename  表名
 *
 * Returns: 
 */
static int db_update_query(const string &setstr, const string &wherestr, const string &tablename)
{
	if (setstr.empty() || wherestr.empty() || tablename.empty())
		return -1;

	check_connect();

	string sql = "update ";
	sql += tablename;
	sql += " set ";
	sql += setstr;
	sql += " where ";
	sql += wherestr;

	LOG_DEBUG("UpdateQuery len=%ld sql=%s\n", sql.length(), sql.c_str());

	return mysql_query(&m_mysqlfd, sql.c_str());
}




/**********************************************开放接口*********************************************/


/**
 * @brief findHostname 查找对应的主机名
 *
 * @Param: hostname  主机名 
 *
 * Returns: 返回值   -1  错误   0  没有找到对应的   >0  找到的主机个数
 */
int findHostname(char* hostname)
{
	if (hostname == NULL)
	{
		LOG_ERROR("hostname is null..\n");
		return -1;
	}

	check_connect();

	char sql_cmd[96];
	sprintf(sql_cmd, "select 1 from HostInfo where HostName='%s' limit 1", hostname);

	int ret = mysql_query(&m_mysqlfd, sql_cmd);
	if (ret != 0)
	{
		LOG_ERROR("query error:%s\n ",mysql_error(&m_mysqlfd));
		return -1;
	}

	MYSQL_RES   *pres = mysql_use_result(&m_mysqlfd);//返回执行结果，适用于数据量较大时
	if (pres == NULL)
	{
		LOG_ERROR("mysql_use_result failed\n");
		return -1;
	}

	MYSQL_ROW  mysqlrow;
	int count = 0;
	while ((mysqlrow = mysql_fetch_row(pres)) != NULL)
	{
		count++;
	}
	//Int32 count = mysql_num_rows(pres);//返回上面函数返回结果的行数，对select有效
	mysql_free_result(pres);

	return count;
}

/**
 * @brief GetHostIdByName 
 *        根据主机名查找对应的ID
 * @Param: hostname  主机名
 * @Param: hostid  找到的id放入该参数 
 *
 * Returns: -1  查找错误  0  查找正确
 */
Uint32 GetHostIdByName(char* hostname, Uint32 & hostid)
{
	if (hostname == NULL)
	{
		LOG_ERROR("hostname is NULL\n");
		return -1;
	}

	check_connect();

	char sql[96];
	sprintf(sql, "select HostAdd from HostInfo where HostName='%s'", hostname);
	LOG_DEBUG("GetHostIdByName sql=%s\n", sql);

	int ret = mysql_query(&m_mysqlfd, sql);
	if (ret != 0)
	{
		LOG_ERROR("mysql_query failed select error:%s\n ", mysql_error(&m_mysqlfd));
		return -1;
	}

	MYSQL_RES   *pres = mysql_use_result(&m_mysqlfd);//返回执行结果，适用于数据量较大时
	if (pres == NULL)
	{
		LOG_ERROR("mysql_use_result failed\n ");
		return -1;
	}

	hostid = 0;
	Uint32 index = 0;
	MYSQL_ROW  mysqlrow;
	while ((mysqlrow = mysql_fetch_row(pres)) != NULL)
	{	//后继的调用正常返回的内容
		if (mysqlrow[0])
		{
			hostid = (Uint32)atoi(mysqlrow[0]);
		}
		index++;
		LOG_DEBUG("select success fieldcoun=%d index=%d hostid=%08x\n", mysql_field_count(&m_mysqlfd), index, hostid);
	}
	mysql_free_result(pres);

	if (index > 1)
		return -1;
	return 0;
}

/**
 * @brief GetHostCount 
 *   拿到 连接上来的主机个数  
 * Returns:   主机个数
 */
int GetHostCount()
{
	check_connect();

	char sql[64];
	sprintf(sql, "select count(*) from HostInfo");
	int ret = mysql_query(&m_mysqlfd, sql);
	if (ret != 0)
	{
		LOG_ERROR("mysql_query failed select error:%s\n ", mysql_error(&m_mysqlfd));

		return -1;
	}

	MYSQL_RES   *pres = mysql_use_result(&m_mysqlfd);//返回执行结果，适用于数据量较大时
	if (pres == NULL)
	{
		LOG_ERROR("mysql_use_result failed\n ");
		return -1;
	}

	MYSQL_ROW  mysqlrow;
	Uint32 fieldcoun = mysql_field_count(&m_mysqlfd);//返回查询结果中的列数（column数）
	Uint32 index = 0;
	int count = 0;
	while ((mysqlrow = mysql_fetch_row(pres)) != NULL)
	{//后继的调用正常返回的内容
		index = 0;
		while (index < fieldcoun)
		{
			if (mysqlrow[index])
			{
				count = atoi(mysqlrow[index]);
			}
			index++;
		}
		LOG_DEBUG("select success fieldcoun=%d count=%d\n", fieldcoun, count);
	}
	mysql_free_result(pres);

	return count;
}



/********************************************插入接口****************************************************/

/**
 * @brief insert_db_rtu 
 * 注册设备接口 ，将设备写入数据库
 * @Param: pRtuinfo  插入设备结构体  
 *
 * Returns:-1 插入失败  0  插入成功
 */
int insert_db_rtu(RtuRegInfo* pRtuinfo)
{

	if(pRtuinfo == NULL)
	{
		LOG_ERROR("pRtuinfo is  NULL\n");
		return -1;
	}

	RtuRegInfo rtuinfo = *pRtuinfo; // 拿到注册设备信息
	int type =rtuinfo.rtutype;

	/*  拿到  表 名  */
	string tablename=get_table_name(type);
	if(strcmp(tablename.c_str(),"") == 0 )  
	{
		LOG_ERROR("no such table.\n");
		return -1;
	}

	string colstr="UniqueIndex,HostAdd,RtuAdd,RtuType,RtuRegTime,RtuState";
	string valstr;
	ostringstream ostr;  //include <sstream>
	ostr.str("");
	ostr << rtuinfo.uniqueindex << "," << rtuinfo.hostadd << "," << rtuinfo.rtuadd << "," << type << "," <<"'" <<rtuinfo.rturegtime <<"'" <<"," <<rtuinfo.rtustate ;
	valstr = ostr.str();

	if(db_insert_query(colstr,valstr,tablename)!=0)
	{
		LOG_ERROR("db_insert_query error...\n");
		return -1;
	}

	ostr.str("");
	ostr << "UniqueIndex,HostAdd,RtuAdd";
	colstr = ostr.str();
	ostr.str("");
	ostr << rtuinfo.uniqueindex << "," << rtuinfo.hostadd << "," << rtuinfo.rtuadd;
	valstr = ostr.str();
	/* 如果是安防设备   ，则插入另外的表 */
	if(strcmp(tablename.c_str(),DB_TABLE_NAMELIST[6]) == 0 )
	{

		tablename=DB_TABLE_NAMELIST[7];
		if(db_insert_query(colstr,valstr,tablename)!=0)
		{
			return -1;
		}
		tablename=DB_TABLE_NAMELIST[8];
		if(db_insert_query(colstr,valstr,tablename)!=0)
		{
			return -1;
		}
		tablename=DB_TABLE_NAMELIST[9];
		if(db_insert_query(colstr,valstr,tablename)!=0)
		{
			return -1;
		}

	}
	/* 如果是传感器设备   ，则插入另外的表 */
	else if (strcmp(tablename.c_str(),DB_TABLE_NAMELIST[11]) == 0 )
	{
		tablename=DB_TABLE_NAMELIST[12];
		if(db_insert_query(colstr,valstr,tablename)!=0)
		{
			return -1;
		}		
	}


	return  0;
}


/**
 * @brief insert_db_host  主机注册接口
 *
 * @Param: pHostinfo   主机信息结构体
 *
 * Returns: -1  插入失败   0  插入成功   >0  主机名字已经存在  
 */
int insert_db_host(HostRegInfo* pHostinfo)
{
	if(pHostinfo == NULL)
	{
		LOG_ERROR("pHostinfo is  NULL\n");
		return -1;
	}

	HostRegInfo hostinfo = *pHostinfo; // 拿到注册设备信息



	int count=findHostname(hostinfo.hostname);
	if(count < 0)
	{
		LOG_ERROR("findHostname error..\n");
		return -1;
	}
	else if(count == 0)
	{
		/*  插入主机  */
		ostringstream ostr;
		string colstr = "HostName,HostAdd,HostPwd,HostRegTime,HostState,HostIP,LastComTime";
		string valstr;
		ostr.str("");
		ostr << "'" << hostinfo.hostname << "'," << hostinfo.hostadd << ",'" << hostinfo.hostpwd << "','"<< hostinfo.hostregtime << "'," 
			<< RTU_STATE_VALID << "," << hostinfo.hostip << ",'" << hostinfo.lastcomtime << "'";
		valstr = ostr.str();

		string tablename=DB_TABLE_NAMELIST[0];

		if(db_insert_query(colstr,valstr,tablename)!=0)
		{
			return -1;
		}
	}
	else 
	{

		LOG_PRINT("host exist..\n");
		return count;
	}

	return 0;
}



/**
 * @brief insert_db_sceneinfo 
 * 创建、修改情景
 * @Param: pSceneInfo  情景信息结构体
 *
 * Returns: 
 */
int insert_db_sceneinfo(SceneInfo* pSceneInfo)
{

	if(pSceneInfo == NULL)
	{
		LOG_ERROR("pSceneInfo is  NULL\n");
		return -1;
	}

	SceneInfo sceneinfo = *pSceneInfo; // 

	string colstr = "UniqueIndex,HostAdd,SceneID,SceneName,ScenePicID,RtuRegTime,SceneRtuNum";
	string valstr;
	string updatestr;
	ostringstream ostr; 
	ostr.str("");
	ostr << sceneinfo.uniqueindex << "," << sceneinfo.hostadd << "," << sceneinfo.sceneid << ",'" << sceneinfo.scenename << "'," 
		<< sceneinfo.scenepicid << ",'" << sceneinfo.rturegtime << "'," << sceneinfo.scenertunum;
	valstr = ostr.str();

	ostr.str("");
	ostr << "UniqueIndex=" << sceneinfo.uniqueindex << ",HostAdd=" << sceneinfo.hostadd << ",SceneID=" << sceneinfo.sceneid 
		<< ",SceneName='" << sceneinfo.scenename<< "',ScenePicID=" << sceneinfo.scenepicid << ",SceneRtuNum=" << sceneinfo.scenertunum;
	updatestr = ostr.str();


	string tablename=DB_TABLE_NAMELIST[3];  //DB_TYPE_SCENEINFO  情景信息   
	if(DupicateKeyUpdateQuery(colstr,valstr,updatestr,tablename) != 0 )
	{
		LOG_ERROR("DupicateKeyUpdateQuery error...\n");
		return -1;
	}


	return  0;
}

/**
 * @brief insert_db_apprtu 
 * 添加，编辑  应用设备 
 * @Param: pAppRtuInfo  应用设备结构体
 *    
 * Returns: 
 */
int insert_db_apprtu(AppRtuInfo* pAppRtuInfo)
{

	if(pAppRtuInfo == NULL)
	{
		LOG_ERROR("pAppRtuInfo is  NULL\n");
		return -1;
	}

	AppRtuInfo rtuinfo = *pAppRtuInfo; // 

	string colstr = "UniqueIndex,HostAdd,RtuAdd,RtuType,RtuRegTime,RtuName,IRRtuAdd,IRLearn,RtuState";
	string valstr;
	string updatestr;
	ostringstream ostr; 
	ostr.str("");
	ostr << rtuinfo.uniqueindex << "," << rtuinfo.hostadd << "," << rtuinfo.rtuadd << "," << rtuinfo.rtutype << ",'" <<rtuinfo.rturegtime <<"','"<< rtuinfo.rtuname 
		<<"',"<<rtuinfo.irrtuadd<<"," << 0 <<","<<RTU_STATE_VALID;
	valstr = ostr.str();

	ostr.str("");
	ostr << "UniqueIndex=" << rtuinfo.uniqueindex << ",HostAdd=" << rtuinfo.hostadd << ",RtuAdd=" << rtuinfo.rtuadd << ",RtuType=" 
		<< rtuinfo.rtutype << ",RtuName='" << rtuinfo.rtuname << "',IRRtuAdd=" <<rtuinfo.irrtuadd<< ",IRLearn=" << 0 << ",RtuState=" << RTU_STATE_VALID;
	updatestr = ostr.str();

	string tablename=DB_TABLE_NAMELIST[5];  //DB_TYPE_USEDEVICE  应用设备类型   
	if(DupicateKeyUpdateQuery(colstr,valstr,updatestr,tablename) != 0 )
	{
		LOG_ERROR("DupicateKeyUpdateQuery error...\n");
		return -1;
	}
	return  0;
}




/* 设置 编辑定时任务  */
int insert_db_taskinfo(TaskInfo & taskinfo)
{
	ostringstream ostr;
	string updatestr;
	string colstr;
	string valstr;
	Uint64 uniqueindex = taskinfo.hostadd;
	uniqueindex = uniqueindex << 32;
	uniqueindex += taskinfo.taskid;
	if (taskinfo.task_bind_num > 0)
	{
		ostr.str("");
		ostr << "UniqueIndex,HostAdd,TaskID,TaskName,TaskPicID,TaskRegTime,TaskRtuNum,TaskCycle,TaskStdtime";
		colstr = ostr.str();

		ostr.str("");
		ostr << uniqueindex << "," << taskinfo.hostadd << "," << taskinfo.taskid << ",'" << taskinfo.taskname << "'," 
			<< taskinfo.taskpicid << ",'" << taskinfo.taskregtime<< "'," << taskinfo.task_bind_num << "," << taskinfo.taskcycle << ",'" 
			<< taskinfo.taskstdtime[0] << ":" << taskinfo.taskstdtime[1] << ":" << taskinfo.taskstdtime[2] << "'";
		valstr = ostr.str();
		ostr.str("");
		ostr << "UniqueIndex=" << uniqueindex << ",HostAdd=" << taskinfo.hostadd << ",TaskID=" << taskinfo.taskid 
			<< ",TaskName='" << taskinfo.taskname << "',TaskPicID=" << taskinfo.taskpicid << ",TaskRtuNum=" 
			<< taskinfo.task_bind_num << ",TaskCycle=" << taskinfo.taskcycle 
			<< ",TaskStdtime=" << "'" << taskinfo.taskstdtime[0] << ":" << taskinfo.taskstdtime[1] << ":" << taskinfo.taskstdtime[2] << "'";
		updatestr = ostr.str();
	}



	string tablename=DB_TABLE_NAMELIST[13];  //DB_TYPE_TASKINFO 
	if(DupicateKeyUpdateQuery(colstr,valstr,updatestr,tablename) != 0 )
	{
		LOG_ERROR("DupicateKeyUpdateQuery error...\n");
		return -1;
	}

}


/**********************************************更新接口********************************************************/

/**
 * @brief update_db_nativertu 
 *      常规设备更新
 * @Param: rtuinfo
 * @Param: mode    模式    0  控制      1  编辑 常规设备信息   2   绑定负载
 *   控制 ：   传入  RelayInfo 数组中  val   
 *   绑定负载：  传入  bind_num ,  RelayInfo 数组中  id,type,name[] ,path[] 
 *   编辑   ：  传入  RelayInfo [0] 中 id, name[] ,path[] 
 *  设备头信息 RtuRegInfo 中的hostadd,rtuadd  必须传入    
 * Returns:   0   更新成功  -1  更新失败   
 */
int  update_db_nativertu(NativeRtuInfo & rtuinfo, Uint8 mode)
{

	ostringstream ostr;
	string setstr;
	string wherestr;
	string tablename=get_table_name(rtuinfo.rtuinfo.rtutype);
	Uint16 relayval;
	Uint64 UniqueIndex;
	UniqueIndex = rtuinfo.rtuinfo.hostadd;
	UniqueIndex = UniqueIndex << 32;
	UniqueIndex += rtuinfo.rtuinfo.rtuadd;
	ostr.str("");
	ostr << "UniqueIndex=" << UniqueIndex;
	wherestr = ostr.str();

	if(mode == 0)
	{
		/* 处理控制等帧 */
		for (Int16 i = 0; i < 4;i++)
		{
			relayval = rtuinfo.relayinfo[i].val;
			printf("%d\n", relayval);
			if (!setstr.empty())
			{
				setstr.append(",");
			}
			ostr.str("");
			ostr << "Relay" << i + 1 << "Val=" << relayval;
			setstr += ostr.str();
		}

	}
	else if(mode ==2)
	{
		/*  绑定负载 */
		Uint16 relayid = 0;
		LOG_DEBUG("bind_num:%d\n",rtuinfo.bind_num);
		for (Int16 i = 0; i < rtuinfo.bind_num; i++)
		{
			relayid = rtuinfo.relayinfo[i].id;	
			if (!setstr.empty())
				setstr.append(",");
			ostr.str("");
			ostr << "Relay" << relayid + 1 << "Type=" << rtuinfo.relayinfo[i].type << ",Relay" << relayid + 1 << "Name='" 
				<< rtuinfo.relayinfo[i].name << "',Relay" << relayid + 1 << "Path='" << rtuinfo.relayinfo[i].path << "'";
			setstr += ostr.str();
		}
	}
	else if(mode ==1 )
	{
		Uint16 relayid = 0;
		ostr.str("");
		relayid = rtuinfo.relayinfo[0].id;
		ostr << "Relay" << relayid + 1 << "Name='" << rtuinfo.relayinfo[0].name 
			<< "',Relay" << relayid + 1 << "Path='" << rtuinfo.relayinfo[0].path << "'";
		setstr += ostr.str();		
	}
	return db_update_query(setstr,wherestr,tablename);
}



/**
 * @brief update_db_safetyrtu 
 *   更新安防设备  
 * @Param: rtuinfo
 * @Param: mode ：  0  控制   1  编辑
 * 控制：  传入  布防状态 ， 动作状态   编辑：传入 设备名称  ，设备路径
 *  hostadd ,rtuadd 必须 传入 
 * Returns: 
 */
int  update_db_safetyrtu(SafetyRtu & rtuinfo, Uint8 mode)
{

	ostringstream ostr;
	string setstr;
	string wherestr;
	string tablename=get_table_name(rtuinfo.rtuinfo.rtutype);
	Uint64 UniqueIndex;
	UniqueIndex = rtuinfo.rtuinfo.hostadd;
	UniqueIndex = UniqueIndex << 32;
	UniqueIndex += rtuinfo.rtuinfo.rtuadd;
	ostr.str("");
	ostr << "UniqueIndex=" << UniqueIndex;
	wherestr = ostr.str();

	if (mode == 0)
	{
		ostr.str("");
		ostr << "RtuActState=" << rtuinfo.rtuactstate << ",RtuPlace=" <<rtuinfo.rtuplace ;
		setstr = ostr.str();	
	}
	else if(mode == 1 )
	{
		ostr.str("");
		ostr << "RtuName='" << rtuinfo.rtuinfo.rtuname << "',RtuPath='"<<rtuinfo.rtuinfo.rtupath<< "'";
		setstr += ostr.str();	

	}
	else
	{
		LOG_ERROR("type error.\n");
		return -1;
	}

	return db_update_query(setstr,wherestr,tablename);
}

/*   修改 情景    */
int  update_db_sceneinfo(SceneInfo & rtuinfo)
{
	return insert_db_sceneinfo(&rtuinfo);
}

/* 修改应用设备 */
int update_db_apprtu(AppRtuInfo & rtuinfo)
{
	return insert_db_apprtu(&rtuinfo);
}

/**
 * @brief update_db_host 
 *  更新主机信息表  
 * @Param: hostinfo  
 *    更新信息： 传入  hostname ,hostip   
 * Returns: 
 */
int update_db_host(HostRegInfo & hostinfo)
{

	ostringstream ostr;
	string setstr;
	string wherestr;
	ostr.str("");
	ostr << "HostName='" << hostinfo.hostname << "'";
	wherestr = ostr.str();	

	string tablename=get_table_name(RTU_TYPE_HOST);

	int count=findHostname(hostinfo.hostname);
	if(count < 0)
	{
		LOG_ERROR("findHostname error..\n");
		return -1;
	}
	else if(count == 0)
	{
		LOG_ERROR("hostname no found..\n");
		return -1;
	}
	else if(count > 0)
	{
		/*  更新主机  */
		char lastcomtime[30];
		get_sys_time(lastcomtime);
		ostr.str("");
		ostr << "HostIP=" << hostinfo.hostip << ",LastComTime='" <<lastcomtime<< "'";
		setstr = ostr.str();		
	}



	return db_update_query(setstr,wherestr,tablename);

}

/**
 * @brief update_db_hostname 
 *  用户管理   
 * @Param: hostinfo  
 *    更新信息： 传入  原 新 主机名  密码 
 * Returns: 
 */
int update_db_hostname(HostMan & hostinfo)
{
	ostringstream ostr;
	string setstr;
	string wherestr;
	string retstr;

	string colstr = "HostPwd";
	
	ostr.str("");
	ostr << "HostName='" << hostinfo.oldname << "'";
	wherestr = ostr.str();
	string tablename = "HostInfo";

	db_select_query(colstr, wherestr, tablename,retstr);
	Int8 szpwd[33];
	Int32 dbpwdlen = retstr.length();
	for (Int16 i = 0; i < MAX_PASSWORD_LEN && i < dbpwdlen - 1; i++)
		szpwd[i] = retstr.c_str()[i];
	szpwd[32] = 0;
	if (strcmp(hostinfo.oldpwd, szpwd) == 0)
	{
		ostr.str("");
		ostr << "HostName='" << hostinfo.newname << "',HostPwd='" << hostinfo.newpwd << "'";
		setstr = ostr.str();		
	}
	else
	{
		return -1;
	}

	return db_update_query(setstr,wherestr,tablename);


}

/**
 * @brief update_db_rtuinfo 
 *  用户设备信息编辑  
 * @Param: hostinfo  该结构体所有字段必须传入
 *    更新 设备名称和路径
 * Returns: 
 */
int update_db_rtuinfo(RtuUpdate & hostinfo)
{
	ostringstream ostr;
	string setstr;
	string wherestr;
	Uint8 opertype=hostinfo.opertype;
	ostr.str("");
	string tablename= get_table_name(hostinfo.rtutype);
	if (opertype == 0xff && hostinfo.rtutype == RTU_TYPE_SCENEINFO)
	{
		ostr << "SceneName='" << hostinfo.rtuname << "'";
		setstr += ostr.str();
	}
	else if (opertype == 0xff && hostinfo.rtutype == RTU_TYPE_TASKINFO)
	{
		ostr << "TaskName='" << hostinfo.rtuname << "'";
		setstr += ostr.str();
	}
	else if (opertype == 0xff)
	{
		ostr << "RtuName='" << hostinfo.rtuname << "',RtuPath='" << hostinfo.rtupath << "'";
		setstr += ostr.str();
	}
	else if (opertype < 3 && IsNativeRtu(hostinfo.rtutype))
	{
		ostr << "Relay" << opertype + 1 << "Name='" << hostinfo.rtuname << "',Relay" << opertype + 1 << "Path='" << hostinfo.rtupath << "'";
		setstr += ostr.str();
	}
	else if (opertype < 16 && (hostinfo.rtutype == RTU_TYPE_BED))
	{
		ostr << "Relay" << opertype + 1 << "Name='" << hostinfo.rtuname << "',Relay" << opertype + 1 << "Path='" << hostinfo.rtupath << "'";
		setstr += ostr.str();
	}

	Uint64 UniqueIndex = hostinfo.hostadd;
	UniqueIndex = UniqueIndex << 32;
	UniqueIndex += hostinfo.rtuadd;
	ostr.str("");
	ostr << "UniqueIndex=" << UniqueIndex;
	wherestr = ostr.str();

	return db_update_query(setstr,wherestr,tablename);


}

/**
 * @brief update_db_scenertu 
 *  更新情景面板 
 * @Param: rtuinfo  
 * @Param: mode  模式：  1  编辑  2 修改绑定  
 * 编辑：  设备名称，设备路径     修改绑定： bind_num  , 每个button的信息 ，包括id ,type,add,oper 
 * Returns: 
 */
int  update_db_scenertu(SceneRtu & rtuinfo, Uint8 mode)
{

	ostringstream ostr;
	string setstr;
	string wherestr;
	string tablename=get_table_name(rtuinfo.rtuinfo.rtutype);
	Uint64 UniqueIndex;
	UniqueIndex = rtuinfo.rtuinfo.hostadd;
	UniqueIndex = UniqueIndex << 32;
	UniqueIndex += rtuinfo.rtuinfo.rtuadd;
	ostr.str("");
	ostr << "UniqueIndex=" << UniqueIndex;
	wherestr = ostr.str();

	if(mode == 0)
	{
		LOG_ERROR("invalid mode...\n");
		return -1;
	}
	else if(mode ==2)
	{
		/*  绑定修改  */  
		Uint16 buttonid = 0;
		for (Int16 i = 0; i < rtuinfo.bind_num; i++)
		{
			buttonid = rtuinfo.scene_bind[i].id;	
			if (!setstr.empty())
				setstr.append(",");
			ostr.str("");
			ostr << "Button" << buttonid << "Type=" << rtuinfo.scene_bind[i].type << ",Button" << buttonid << "Add=" 
				<< rtuinfo.scene_bind[i].add << ",Button" << buttonid << "Oper=" << rtuinfo.scene_bind[i].oper;
			setstr += ostr.str();
		}
	}
	else if(mode ==1 )   // 编辑
	{
		ostr.str("");
		ostr << "RtuName='" << rtuinfo.rtuinfo.rtuname << "',RtuPath='"<<rtuinfo.rtuinfo.rtupath<< "'";
		setstr += ostr.str();		
	}
	return db_update_query(setstr,wherestr,tablename);
}

/* mode ：  0  控制   1  编辑 */
/**
 * @brief update_db_exnativertu 
 *  更新智能床  
 * @Param: rtuinfo
 * @Param: mode  0  控制  1  编辑
 *  控制 ：  relayinfo 数组  中val值传入   床头位置（1）  床尾位置（1）  床头振动（1） 床尾振动（1） 灯状态（1）：0x00表示关灯，0x01表示开灯
 * 编辑   relayinfo[0]  中  name ,path 传入  
 * Returns: 
 */
int  update_db_exnativertu(ExNativeRtu & rtuinfo, Uint8 mode)
{

	ostringstream ostr;
	string setstr;
	string wherestr;
	string tablename=get_table_name(RTU_TYPE_BED);
	Uint64 UniqueIndex;
	UniqueIndex = rtuinfo.rtuinfo.hostadd;
	UniqueIndex = UniqueIndex << 32;
	UniqueIndex += rtuinfo.rtuinfo.rtuadd;
	ostr.str("");
	ostr << "UniqueIndex=" << UniqueIndex;
	wherestr = ostr.str();
	Uint16 relayval = 0;
	if(mode == 0)
	{
		for (Int16 i = 0; i < 5; i++)
		{
			relayval = rtuinfo.relayinfo[i].val;

			if (!setstr.empty())
				setstr.append(",");

			ostr.str("");
			ostr << "Relay" << i + 1 << "Val=" << relayval;
			setstr += ostr.str();
		}
	}
	else if(mode == 1)  // 编辑
	{
		ostr.str("");
		ostr << "Relay1Name='" << rtuinfo.relayinfo[0].name << "',Relay1Path='" << rtuinfo.relayinfo[0].path << "'";
		setstr = ostr.str();
	}

	return db_update_query(setstr,wherestr,tablename);
}


/* 编辑定时任务  */
int update_db_taskinfo(TaskInfo & taskinfo)
{
	return insert_db_taskinfo(taskinfo);
}




/*************************************************查找接口****************************************************/

/**
 * @brief select_db_hostinfo  
 *           根据主机名 查询主机信息  ，用于  APP远程登陆
 * @Param: pHostinfo
 * @Param: hostname
 *
 * Returns: 返回值   -1  错误   0  正确    1   没有数据 
 */
int select_db_hostinfo(HostRegInfo* pHostinfo,char* hostname)
{

	if(hostname == NULL)
	{
		LOG_ERROR("hostname  is  NULL\n");
		return -1;
	}
	if(pHostinfo == NULL)
	{
		LOG_ERROR("HostRegInfo  is  NULL\n");
		return -1;		
	}

	ostringstream ostr;
	string colstr="HostAdd,HostPwd,HostState,LastComTime";;
	string wherestr;	
	ostr.str("");
	ostr << "HostName='" << hostname << "'";
	wherestr = ostr.str();
	string retstr;
	string tablename=DB_TABLE_NAMELIST[0];
	int ret=db_select_query(colstr, wherestr, tablename, retstr);
	if(ret == 0)
	{
		char* token;
		char szret[128];
		strcpy(szret, retstr.c_str());
		LOG_DEBUG("retstr=%s\n",retstr.c_str());
		int index = 0;
		token = strtok(szret, ",");
		while (token != NULL && index < 4)//4因为只查询了4列
		{
			if (index == 0)
				pHostinfo->hostadd = atoi(token);
			else if (index == 1)
				strcpy(pHostinfo->hostpwd, token);
			else if (index == 2)
				pHostinfo->hoststate =(Rtustatus)atoi(token);
			else if (index == 3)
				strcpy(pHostinfo->lastcomtime,token);
			index++;
			token = strtok(NULL, ",");
		}
	}
	else if(ret == 1)
	{
		LOG_DEBUG("db_select_query nodata..\n");
		return 1;
	}
	else
	{
		LOG_ERROR("db_select_query fail..\n");
		return -1;
	}

	return 0;

}




/*******************************************删除接口***************************************************/

/**
 * @brief delete_db_host 
 *   删除主机设备
 * @Param: hostname
 *
 * Returns: 
 */
int delete_db_host(char* hostname)
{
	check_connect();
	char sql[1024];
	int  len = 0;
	int  ret = 0;
	for (int i = 1; i < MAX_DB_TYPE; i++)
	{
		len = 0;
		len += sprintf(sql + len, "DELETE %s.* ", DB_TABLE_NAMELIST[i]);
		len += sprintf(sql + len, "FROM %s ", DB_TABLE_NAMELIST[i]);
		len += sprintf(sql + len, "LEFT JOIN %s ON %s.HostAdd = %s.HostAdd ", DB_TABLE_HOSTINFO, DB_TABLE_NAMELIST[i], DB_TABLE_HOSTINFO);
		len += sprintf(sql + len, "WHERE %s.HostName = '%s'", DB_TABLE_HOSTINFO, hostname);
		sql[len] = 0;

		LOG_DEBUG("DeleteHost sql=%s\n", sql);
		ret = mysql_query(&m_mysqlfd, sql);
		if (ret != 0)
		{
			LOG_ERROR("mysql_query failed select error:%s\n ", mysql_error(&m_mysqlfd));
			return -1;
		}
	}

	len = 0;
	len += sprintf(sql + len, "DELETE FROM %s WHERE HostName='%s' ", DB_TABLE_HOSTINFO, hostname);
	sql[len] = 0;

	LOG_DEBUG("DeleteHost sql=%s\n", sql);
	ret = mysql_query(&m_mysqlfd, sql);
	if (ret != 0)
	{
		LOG_ERROR("mysql_query failed select error:%s\n ", mysql_error(&m_mysqlfd));
		return -1;
	}	
	return 0;
}

/**
 * @brief delete_db_rtu 
 *  删除设备 接口   不包括删除主机  
 * @Param: pRtuinfo
 *
 * Returns: -1  删除出错 0 删除成功  或者  没有删除项
 */
int delete_db_rtu(RtuRemoveInfo* pRtuinfo)
{
	if(pRtuinfo == NULL)
	{
		LOG_ERROR("pRtuinfo is  NULL\n");
		return -1;
	}

	RtuRemoveInfo rtuinfo = *pRtuinfo;
	Uint8 type =rtuinfo.rtutype;
	LOG_DEBUG("wjj test type=%u\n",type);
	/*  拿到  表 名  */
	string tablename=get_table_name(type);

	if(strcmp(tablename.c_str(),"")==0)
	{
		LOG_ERROR("no such table...\n");
		return -1;
	}


	check_connect();

	char sql[256];
	Uint64 index = rtuinfo.hostadd;
	index = index << 32;
	index += rtuinfo.rtuadd;
	sprintf(sql, "delete from %s where UniqueIndex=%ld", tablename.c_str(), index);

	LOG_DEBUG("delete sql=%s\n",sql);
	int ret = mysql_query(&m_mysqlfd, sql);
	if (ret != 0)
	{
		LOG_ERROR("mysql_query failed delete error:%s\n ", mysql_error(&m_mysqlfd));
		return -1;
	}

	/* 如果是安防设备   ，则删除另外的表 */
	if(strcmp(tablename.c_str(),DB_TABLE_NAMELIST[6]) == 0 )
	{

		tablename=DB_TABLE_NAMELIST[7];
		memset(sql,0,256);
		sprintf(sql, "delete from %s where UniqueIndex=%ld", tablename.c_str(), index);
		if(mysql_query(&m_mysqlfd, sql)!=0)
		{
			return -1;
		}

		tablename=DB_TABLE_NAMELIST[8];
		memset(sql,0,256);
		sprintf(sql, "delete from %s where UniqueIndex=%ld", tablename.c_str(), index);
		if(mysql_query(&m_mysqlfd, sql)!=0)
		{
			return -1;
		}

		tablename=DB_TABLE_NAMELIST[9];
		memset(sql,0,256);
		sprintf(sql, "delete from %s where UniqueIndex=%ld", tablename.c_str(), index);
		if(mysql_query(&m_mysqlfd, sql)!=0)
		{
			return -1;
		}

	}
	/* 如果是传感器设备   ，则删除另外的表 */
	else if (strcmp(tablename.c_str(),DB_TABLE_NAMELIST[11]) == 0 )
	{
		tablename=DB_TABLE_NAMELIST[12];
		memset(sql,0,256);
		sprintf(sql, "delete from %s where UniqueIndex=%ld", tablename.c_str(), index);
		if(mysql_query(&m_mysqlfd, sql)!=0)
		{
			return -1;
		}		
	}
	return 0;
}


/**************************************************************************/












/**************************************************************/

/*  给出这样一个接口   ，只要传入一个结构体  ，
	并且该结构体是我们定义的结构体，那么就进行插入
	操作 

	这是一个总的接口
	有问题    ------------------

 */

int insert_into(void* info)
{
	if(info == NULL)
	{
		LOG_ERROR("info  is  NULL\n");
		return -1;
	}

	RtuRegInfo rtuinfo = *(RtuRegInfo *)info; // 拿到注册设备信息	
	if(rtuinfo.uniqueindex)
	{
		insert_db_rtu(&rtuinfo);
	}
	return 0;
}












