/** 
 * @file  dbbase.cpp
 * @brief 数据库接口实现   
 * @details   增删查改
 * @author     menqi - 1083734876@qq.com
 * @date     2016-05-06 16:45
 * @version  1.0
 * @par Copyright (c):  menqi                                                             
 */
#include "dbbase.h"
#include <pthread.h>
MYSQL m_mysqlfd;   /**< 数据库连接描述符 */
pthread_mutex_t mutex ;

/**
 * @brief IsNativeRtu   
 * 查看某设备是否是常规设备
 * @param type   设备类型
 *
 * @return   true or false 
 */
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


/**
 * @brief IsSafetyRtu 
 * 查看某设备是否是安防设备
 * @param type  设备类型
 *
 * @return   true or false
 */
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
 * @param  info    
 *   连接mysql时，mysql相关信息结构体   
 *
 * @return  0  成功  -1  失败
 */
int init_mysql(MysqlConInfo *info)
{
		printf("+++++++++++++initmysql+++++++++++++\n");
		mysql_init(&m_mysqlfd);
		mysql_options(&m_mysqlfd, MYSQL_SET_CHARSET_NAME, "gbk");
		char value = 1;
		mysql_options(&m_mysqlfd, MYSQL_OPT_RECONNECT, &value);
		//int time_out = 3;  // 设置读写超时
		//mysql_options(&m_mysqlfd,MYSQL_OPT_READ_TIMEOUT,(const char*)&time_out);
		if (!mysql_real_connect(&m_mysqlfd, info->hostname, info->username, info->password, info->dbname, 03306, NULL/*Unix Socket 连接类型*/,0/*运行成ODBC数据库标志*/))
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

		pthread_mutex_init(&mutex,NULL); 
		return 0;

}



/**
 * @brief get_table_name 
 *    根据设备类型   拿到对应的表名
 * @param  type
 *     设备类型
 * @return   表名   为  string 类型  
 */
std::string  get_table_name(Uint8 type)
{
		Uint32 i;
		std::string ret = "";
		//LOG_DEBUG("type:%u\n",type);
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
 * @return  
 */
static int check_connect()
{

		return mysql_ping(&m_mysqlfd);

}

/**
 * @brief insert_mysql 
 *  数据库基本插入操作 
 * @param  sql_cmd
 *   插入命令  为string 类型
 * @return  
 */
static int insert_mysql(string sql_cmd)
{
		/* 执行插入操作  */
		int ret = mysql_query(&m_mysqlfd, sql_cmd.c_str());

		if(ret != 0)
		{
				LOG_ERROR("mysql_query failed select error:%s\n ", mysql_error(&m_mysqlfd));
				return -1;
		}
		return 0;
}


/**
 * @brief db_insert_query 
 *  数据库插入
 * @param  colstr   列字段
 * @param  valstr   值字段
 * @param  tablename  数据表名
 *
 * @return  
 */
static int db_insert_query(const string &colstr, const string &valstr, const string &tablename)
{
		/*插入前 检查数据连接  */
		pthread_mutex_lock(&mutex);
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
		int nret= insert_mysql(sql_cmd);
		pthread_mutex_unlock(&mutex);
		return nret;
}



/**
 * @brief DupicateKeyUpdateQuery 
 *  更新、插入查询：存在则更新不存在则插入
 * @param  colstr  列
 * @param  valstr  值 
 * @param  updatestr  要更新的列
 * @param  tablename  表名
 *
 * @return  
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
 * @param  sql   查找sql 命令
 * @param  retstr  返回列字段，以逗号隔开
 *
 * @return  
 */
static int select_query(const string & sql, string &retstr)
{
		pthread_mutex_lock(&mutex);
		check_connect();

		int nRet = mysql_query(&m_mysqlfd, sql.c_str());
		if (nRet != 0)
		{
				LOG_ERROR("mysql_query failed select error:%s\n ", mysql_error(&m_mysqlfd));
				return -1;
		}
		/*
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
		 */

		MYSQL_RES *result;
		Uint32 index = 0;
		do
		{

				if (!(result= mysql_store_result(&m_mysqlfd)))
				{
						exit(1);
				}
				MYSQL_ROW  mysqlrow;
				Uint32 fieldcoun = mysql_field_count(&m_mysqlfd);//返回查询结果中的列数（column数）
				printf("total affected rows: %ld\n", fieldcoun);
				while ((mysqlrow = mysql_fetch_row(result)) != NULL)
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
				}
				mysql_free_result(result);
		} while (!mysql_next_result(&m_mysqlfd));
		LOG_DEBUG("select success  len=%ld ret=%s\n", retstr.length(), retstr.c_str());
		pthread_mutex_unlock(&mutex);
		return retstr.empty()?1:0;
}


/**
 * @brief db_select_query 
 *  查找操作
 * @param  colstr   列字段
 * @param  wheresrt 过滤字段
 * @param  tablename 表名
 * @param  retstr   查找结果
 *
 * @return  
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
 * @param  setstr     设置字符串
 * @param  wherestr   条件字符串
 * @param  tablename  表名
 *
 * @return  
 */
static int db_update_query(const string &setstr, const string &wherestr, const string &tablename)
{
		if (setstr.empty() || wherestr.empty() || tablename.empty())
				return -1;

		pthread_mutex_lock(&mutex);
		//check_connect();

		string sql = "update ";
		sql += tablename;
		sql += " set ";
		sql += setstr;
		sql += " where ";
		sql += wherestr;

		LOG_DEBUG("UpdateQuery len=%ld sql=%s\n", sql.length(), sql.c_str());
		int nret=mysql_query(&m_mysqlfd, sql.c_str());
		pthread_mutex_unlock(&mutex);
		return nret;
}

/**  查看对应的设备是否已经存在 */
static int findRtuadd(Uint32 rtuadd,const string & tablename)
{
		pthread_mutex_lock(&mutex);
		check_connect();
		char sql_cmd[96];
		sprintf(sql_cmd, "select 1 from %s where RtuAdd=%u", tablename.c_str(), rtuadd);
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

		mysql_free_result(pres);
		pthread_mutex_unlock(&mutex);

		return count;
}


/**********************************************开放接口*********************************************/


/**
 * @brief findHostname 查找对应的主机名
 *
 * @param  hostname  主机名 
 *
 * @return  返回值   -1  错误   0  没有找到对应的   >0  找到的主机个数
 */
int findHostname(char* hostname)
{
		if (hostname == NULL)
		{
				LOG_ERROR("hostname is null..\n");
				return -1;
		}
		pthread_mutex_lock(&mutex);
		check_connect();

		char sql_cmd[96];
		sprintf(sql_cmd, "select 1 from HostInfo where HostName='%s' limit 1", hostname);

		int ret = mysql_query(&m_mysqlfd, sql_cmd);
		if (ret != 0)
		{
				LOG_ERROR("query error:%s\n ",mysql_error(&m_mysqlfd));
				return -1;
		}
		int count = 0;
		/*
		   MYSQL_RES   *pres = mysql_use_result(&m_mysqlfd);//返回执行结果，适用于数据量较大时
		   MYSQL_RES   *pres = mysql_use_result(&m_mysqlfd);//返回执行结果，适用于数据量较大时
		   if (pres == NULL)
		   {
		   LOG_ERROR("mysql_use_result failed\n");
		   return -1;
		   }

		   MYSQL_ROW  mysqlrow;
		   while ((mysqlrow = mysql_fetch_row(pres)) != NULL)
		   {
		   count++;
		   }
		   printf("count11111111:%d\n",mysql_num_rows(pres));
		   mysql_free_result(pres);
		 */
		//printf("count22222222:%d\n",count);
		MYSQL_RES *pres=NULL;
		do
		{
				pres = mysql_store_result(&m_mysqlfd);
				count = mysql_num_rows(pres);//返回上面函数返回结果的行数，对select有效
				printf("count11111111:%d\n",mysql_num_rows(pres));
				mysql_free_result(pres);
				//count++; 
		}while( !mysql_next_result(&m_mysqlfd) );

		pthread_mutex_unlock(&mutex);
		return count;
}

/**
 * @brief GetHostIdByName 
 *        根据主机名查找对应的ID
 * @param  hostname  主机名
 * @param  hostid  找到的hostadd放入该参数 
 *
 * @return  -1  查找错误  0  查找正确
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
 * @return    主机个数
 */
int GetHostCount()
{
		pthread_mutex_lock(&mutex);
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
		pthread_mutex_unlock(&mutex);

		return count;
}



/********************************************插入接口****************************************************/

/**
 * @brief insert_db_rtu 
 * 注册设备接口 ，将设备写入数据库
 * @param  rtuinfo  插入设备结构体  
 *
 * @return -1 插入失败  0  插入成功
 */
int insert_db_rtu(RtuRegInfo & rtuinfo)
{

		Uint16 type =rtuinfo.rtutype;

		/*  拿到  表 名  */
		string tablename=get_table_name(type);
		if(strcmp(tablename.c_str(),"") == 0 )  
		{
				LOG_ERROR("no such table.\n");
				return -1;
		}
		Uint64 UniqueIndex;
		UniqueIndex = rtuinfo.hostadd;
		UniqueIndex = UniqueIndex << 32;
		UniqueIndex += rtuinfo.rtuadd;

		string colstr="UniqueIndex,HostAdd,RtuAdd,RtuType,RtuRegTime,RtuState";
		string valstr;
		ostringstream ostr;  //include <sstream>
		ostr.str("");
		ostr << UniqueIndex << "," << rtuinfo.hostadd << "," << rtuinfo.rtuadd << "," << type << "," <<"'" <<rtuinfo.rturegtime <<"'" <<"," <<rtuinfo.rtustate ;
		valstr = ostr.str();

		if(findRtuadd(rtuinfo.rtuadd,tablename) > 0)
		{
				/* 注册前删除该设备对应的信息*/
				char sql_cmd[96];
				sprintf(sql_cmd, "delete from %s where RtuAdd=%u", tablename.c_str(), rtuinfo.rtuadd);
				if(mysql_query(&m_mysqlfd, sql_cmd)!=0)
				{
						return -1;
				}

		}

		if(db_insert_query(colstr,valstr,tablename)!=0)
		{
				LOG_ERROR("db_insert_query error...\n");
				return -1;
		}

		ostr.str("");
		ostr << "UniqueIndex,HostAdd,RtuAdd";
		colstr = ostr.str();
		ostr.str("");
		ostr << UniqueIndex << "," << rtuinfo.hostadd << "," << rtuinfo.rtuadd;
		valstr = ostr.str();
		/* 如果是安防设备   ，则插入另外的表 */
		if(strcmp(tablename.c_str(),DB_TABLE_NAMELIST[6]) == 0 )
		{

				tablename=DB_TABLE_NAMELIST[7];  //SafetyPwdInfo
				if(db_insert_query(colstr,valstr,tablename)!=0)
				{
						return -1;
				}
				// tablename=DB_TABLE_NAMELIST[8];  //SafetySetBind
				// if(db_insert_query(colstr,valstr,tablename)!=0)
				// {
				// 	return -1;
				// }
				// tablename=DB_TABLE_NAMELIST[9];  //SafetyRemoveBind
				// if(db_insert_query(colstr,valstr,tablename)!=0)
				// {
				// 	return -1;
				// }

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
 * @param  hostinfo   主机信息结构体
 *
 * @return  -1  插入失败   0  插入成功   >0  主机名字已经存在  
 */
int insert_db_host(HostRegInfo & hostinfo)
{

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
 * @param  sceneinfo  情景信息结构体  绑定的设备信息需要malloc后传入 
 *
 * @return  
 */
int insert_db_sceneinfo(SceneInfo & sceneinfo)
{

		Uint64 UniqueIndex;
		UniqueIndex = sceneinfo.hostadd;
		UniqueIndex = UniqueIndex << 32;
		UniqueIndex += sceneinfo.sceneid;

		string colstr = "UniqueIndex,HostAdd,SceneID,SceneName,ScenePicID,RtuRegTime,SceneRtuNum";
		string valstr;
		string updatestr;
		ostringstream ostr; 
		ostr.str("");
		ostr << UniqueIndex << "," << sceneinfo.hostadd << "," << sceneinfo.sceneid << ",'" << sceneinfo.scenename << "'," 
				<< sceneinfo.scenepicid << ",'" << sceneinfo.rturegtime << "'," << sceneinfo.scenertunum;
		valstr = ostr.str();

		ostr.str("");
		ostr << "UniqueIndex=" << UniqueIndex << ",HostAdd=" << sceneinfo.hostadd << ",SceneID=" << sceneinfo.sceneid 
				<< ",SceneName='" << sceneinfo.scenename<< "',ScenePicID=" << sceneinfo.scenepicid << ",SceneRtuNum=" << sceneinfo.scenertunum;
		updatestr = ostr.str();

		for (Int16 i = 0; i < sceneinfo.scenertunum; i++)
		{
				if (!colstr.empty())
						colstr.append(",");
				if (!valstr.empty())
						valstr.append(",");
				if (!updatestr.empty())
						updatestr.append(",");

				ostr.str("");
				ostr << "Rtu" << i + 1 << "Type=" << sceneinfo.rtubind[i].type << ",Rtu" << i + 1 << "Add=" << sceneinfo.rtubind[i].add 
						<< ",Rtu" << i + 1 << "Oper=" << sceneinfo.rtubind[i].oper << ",Rtu" << i + 1 << "Delay=" << sceneinfo.rtubind[i].delay;
				updatestr += ostr.str();
				ostr.str("");
				ostr << "Rtu" << i + 1 << "Type" << ",Rtu" << i + 1 << "Add" << ",Rtu" << i + 1 << "Oper" << ",Rtu" << i + 1 << "Delay";
				colstr += ostr.str();
				ostr.str("");
				ostr << sceneinfo.rtubind[i].type << "," << sceneinfo.rtubind[i].add << "," << sceneinfo.rtubind[i].oper << "," << sceneinfo.rtubind[i].delay;
				valstr += ostr.str();
		}

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
 * @param  rtuinfo  应用设备结构体
 *    
 * @return  
 */
int insert_db_apprtu(AppRtuInfo & rtuinfo)
{
		Uint64 UniqueIndex;
		UniqueIndex = rtuinfo.hostadd;
		UniqueIndex = UniqueIndex << 32;
		UniqueIndex += rtuinfo.rtuadd;

		string colstr = "UniqueIndex,HostAdd,RtuAdd,RtuType,RtuRegTime,RtuName,IRRtuAdd,IRLearn,RtuState";
		string valstr;
		string updatestr;
		ostringstream ostr; 
		ostr.str("");
		ostr << UniqueIndex << "," << rtuinfo.hostadd << "," << rtuinfo.rtuadd << "," << rtuinfo.rtutype << ",'" <<rtuinfo.rturegtime <<"','"<< rtuinfo.rtuname 
				<<"',"<<rtuinfo.irrtuadd<<"," << 0 <<","<<RTU_STATE_VALID;
		valstr = ostr.str();

		ostr.str("");
		ostr << "UniqueIndex=" << UniqueIndex << ",HostAdd=" << rtuinfo.hostadd << ",RtuAdd=" << rtuinfo.rtuadd << ",RtuType=" 
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


/**
 * @brief insert_db_taskinfo 
 * 设置 编辑定时任务  
 * @param  taskinfo  任务信息  绑定的设备信息需要malloc后传入 
 */
int insert_db_taskinfo(TaskInfo & taskinfo)
{
		ostringstream ostr;
		string updatestr;
		string colstr;
		string valstr;
		Uint64 uniqueindex = taskinfo.hostadd;
		uniqueindex = uniqueindex << 32;
		uniqueindex += taskinfo.taskid;
		if (taskinfo.taskrtunum > 0)
		{
				ostr.str("");
				ostr << "UniqueIndex,HostAdd,TaskID,TaskName,TaskPicID,TaskRegTime,TaskRtuNum,TaskCycle,TaskStdtime";
				colstr = ostr.str();

				ostr.str("");
				ostr << uniqueindex << "," << taskinfo.hostadd << "," << taskinfo.taskid << ",'" << taskinfo.taskname << "'," 
						<< taskinfo.taskpicid << ",'" << taskinfo.taskregtime<< "'," << taskinfo.taskrtunum << "," << taskinfo.taskcycle << ",'" 
						<< taskinfo.taskstdtime[0] << ":" << taskinfo.taskstdtime[1] << ":" << taskinfo.taskstdtime[2] << "'";
				valstr = ostr.str();
				ostr.str("");
				ostr << "UniqueIndex=" << uniqueindex << ",HostAdd=" << taskinfo.hostadd << ",TaskID=" << taskinfo.taskid 
						<< ",TaskName='" << taskinfo.taskname << "',TaskPicID=" << taskinfo.taskpicid << ",TaskRtuNum=" 
						<< taskinfo.taskrtunum << ",TaskCycle=" << taskinfo.taskcycle 
						<< ",TaskStdtime=" << "'" << taskinfo.taskstdtime[0] << ":" << taskinfo.taskstdtime[1] << ":" << taskinfo.taskstdtime[2] << "'";
				updatestr = ostr.str();
		}

		for (Int16 i = 0; i < taskinfo.taskrtunum; i++)
		{
				if (!colstr.empty())
						colstr.append(",");
				if (!valstr.empty())
						valstr.append(",");
				if (!updatestr.empty())
						updatestr.append(",");

				ostr.str("");
				ostr << "Rtu" << i + 1 << "Type=" << taskinfo.rtubind[i].type << ",Rtu" << i + 1 << "Add=" << taskinfo.rtubind[i].add 
						<< ",Rtu" << i + 1 << "Oper=" << taskinfo.rtubind[i].oper << ",Rtu" << i + 1 << "Delay=" << taskinfo.rtubind[i].delay;
				updatestr += ostr.str();
				ostr.str("");
				ostr << "Rtu" << i + 1 << "Type" << ",Rtu" << i + 1 << "Add" << ",Rtu" << i + 1 << "Oper" << ",Rtu" << i + 1 << "Delay";
				colstr += ostr.str();
				ostr.str("");
				ostr << taskinfo.rtubind[i].type << "," << taskinfo.rtubind[i].add << "," << taskinfo.rtubind[i].oper << "," << taskinfo.rtubind[i].delay;
				valstr += ostr.str();
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
 * @param  rtuinfo
 * @param  mode    模式    0  控制      1  绑定负载
 *   控制 ：   传入  RelayInfo 数组中  val   
 *   绑定负载：  传入  bind_num ,  RelayInfo 数组中  id,type,name[] ,path[] 
 *  设备头信息 RtuRegInfo 中的hostadd,rtuadd  必须传入    
 * @return    0   更新成功  -1  更新失败   
 */
int  update_db_nativertu(NativeRtuInfo & rtuinfo, Uint8 mode)
{

		ostringstream ostr;
		string setstr;
		string wherestr;
		//string tablename=get_table_name(rtuinfo.rtuinfo.rtutype);
		string tablename="NativeRtu";
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
						//printf("%d\n", relayval);
						if (!setstr.empty())
						{
								setstr.append(",");
						}
						ostr.str("");
						ostr << "Relay" << i + 1 << "Val=" << relayval;
						setstr += ostr.str();
				}

		}
		else if(mode ==1)
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
		else 
		{
				LOG_ERROR("invalid mode..\n");
				return -1;		
		}
		return db_update_query(setstr,wherestr,tablename);
}



/**
 * @brief update_db_safetyrtu 
 *   更新安防设备  
 * @param  rtuinfo
 * 控制：  传入  布防状态 ， 动作状态 
 *  hostadd ,rtuadd 必须 传入 
 * @return  
 */
int  update_db_safetyrtu(SafetyRtu & rtuinfo)
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


		ostr.str("");
		ostr << "RtuActState=" << rtuinfo.rtuactstate << ",RtuPlace=" <<rtuinfo.rtuplace ;
		setstr = ostr.str();	

		
		//pthread_mutex_lock(&mutex);
		return db_update_query(setstr,wherestr,tablename);
		//pthread_mutex_unlock(&mutex);
		//return nret;
}

/**
 * @brief update_db_sceneinfo 
 * 修改 情景 
 */
int  update_db_sceneinfo(SceneInfo & rtuinfo)
{
		return insert_db_sceneinfo(rtuinfo);
}


/**
 * @brief update_db_apprtu 
 * 修改应用设备
 */
int update_db_apprtu(AppRtuInfo & rtuinfo)
{
		return insert_db_apprtu(rtuinfo);
}

/**
 * @brief update_db_host 
 *  更新主机信息表  
 * @param  hostinfo  
 *    更新信息： 传入  hostname ,hostip   
 * @return  
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
		printf("wjj test....count:%d\n",count);
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
 * @param  hostinfo  
 *    更新信息： 传入  原 新 主机名  密码 
 * @return  
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
 * @param  hostinfo  该结构体所有字段必须传入
 *    更新 设备名称和路径
 * @return  
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
				printf("hello........\n");
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
 * @param  rtuinfo  
 *修改绑定： bind_num  , 每个button的信息 ，包括id ,type,add,oper 
 * @return  
 */
int  update_db_scenertu(SceneRtu & rtuinfo)
{

		ostringstream ostr;
		string setstr;
		string wherestr;
		//string tablename=get_table_name(rtuinfo.rtuinfo.rtutype);
		string tablename="SceneRtu";
		Uint64 UniqueIndex;
		UniqueIndex = rtuinfo.rtuinfo.hostadd;
		UniqueIndex = UniqueIndex << 32;
		UniqueIndex += rtuinfo.rtuinfo.rtuadd;
		ostr.str("");
		ostr << "UniqueIndex=" << UniqueIndex;
		wherestr = ostr.str();
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

		return db_update_query(setstr,wherestr,tablename);
}


/**
 * @brief update_db_exnativertu 
 *  更新智能床  
 * @param  rtuinfo
 *  控制 ：  relayinfo 数组  中val值传入   床头位置（1）  床尾位置（1）  床头振动（1） 床尾振动（1） 灯状态（1）：0x00表示关灯，0x01表示开灯 
 * @return  
 */
int  update_db_exnativertu(ExNativeRtu & rtuinfo)
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

		for (Int16 i = 0; i < 5; i++)
		{
				relayval = rtuinfo.relayinfo[i].val;

				if (!setstr.empty())
						setstr.append(",");

				ostr.str("");
				ostr << "Relay" << i + 1 << "Val=" << relayval;
				setstr += ostr.str();
		}
		return db_update_query(setstr,wherestr,tablename);
}



/**
 * @brief update_db_taskinfo 
 * 编辑定时任务
 */
int update_db_taskinfo(TaskInfo & taskinfo)
{
		return insert_db_taskinfo(taskinfo);
}



/**
 * @brief update_db_detectorrtu 
 *  更新传感器设备   
 * @param  rtuinfo  
 * @param  mode  模式 0  更新 val 值  1  更新联动标志，阈值
 *  更新val :  传入 val数组值 ,默认为0    更新联动标志： 传入  flag ,downval ,upval
 * @return  
 */
int update_db_detectorrtu(DetectorRtu & rtuinfo, Uint8 mode)
{

		ostringstream ostr;
		string setstr;
		string wherestr;
		string tablename="DetectorRtu";
		Uint64 UniqueIndex;
		UniqueIndex = rtuinfo.rtuinfo.hostadd;
		UniqueIndex = UniqueIndex << 32;
		UniqueIndex += rtuinfo.rtuinfo.rtuadd;
		ostr.str("");
		ostr << "UniqueIndex=" << UniqueIndex;
		wherestr = ostr.str();

		if (mode == 0)
		{
				for (Int16 i = 0; i < 3; i++)
				{
						if (!setstr.empty())
								setstr.append(",");
						ostr.str("");
						ostr << "RtuVal" << i + 1 << "=" << rtuinfo.detectorinfo.val[i];
						setstr += ostr.str();
				}	
		}
		else if(mode == 1 )
		{

				ostr.str("");
				switch (rtuinfo.valtype)
				{
						case 0x01:
						case 0x02:
						case 0x03:
						case 0x06:
								ostr << "RtuActFlag1=" << rtuinfo.detectorinfo.flag << ",RtuDownActVal1=" << rtuinfo.detectorinfo.downval << ",RtuUpActVal1=" << rtuinfo.detectorinfo.upval;
								break;
						case 0x04:
								ostr << "RtuActFlag2=" << rtuinfo.detectorinfo.flag << ",RtuDownActVal2=" << rtuinfo.detectorinfo.downval << ",RtuUpActVal2=" << rtuinfo.detectorinfo.upval;
								break;
						case 0x05:
								ostr << "RtuActFlag3=" << rtuinfo.detectorinfo.flag << ",RtuDownActVal3=" << rtuinfo.detectorinfo.downval << ",RtuUpActVal3=" << rtuinfo.detectorinfo.upval;
								break;
						default:
								break;
				}
				setstr += ostr.str();
		}
		else
		{
				LOG_ERROR("mode error.\n");
				return -1;
		}

		return db_update_query(setstr,wherestr,tablename);

}


/**
 * @brief update_db_detectorbind   绑定联动接口
 * @param rtuinfo  绑定的设备信息需要malloc后传入 
 */
int update_db_detectorbind(DetectorBind & rtuinfo)
{
		ostringstream ostr;
		string setstr;
		string wherestr;
		string tablename="DetectorBind";
		Uint64 UniqueIndex;
		UniqueIndex = rtuinfo.hostadd;
		UniqueIndex = UniqueIndex << 32;
		UniqueIndex += rtuinfo.rtuadd;
		ostr.str("");
		ostr << "UniqueIndex=" << UniqueIndex;
		wherestr = ostr.str();

		Uint16 level=rtuinfo.level;
		Uint16 bind_num=rtuinfo.bind_num;
		ostr.str("");
		if (rtuinfo.bind_num > 0)
		{
				ostr << "L" << level << "Num=" << bind_num;
				setstr = ostr.str();
		}

		for (Int16 i = 0; i < rtuinfo.bind_num; i++)
		{

				ostr.str("");
				ostr << ",L" << level << "Bind" << i + 1 << "Type=" << rtuinfo.rtubind[i].type << ",L" << level 
						<< "Bind" << i + 1 << "Add=" << rtuinfo.rtubind[i].add << ",L" << level 
						<< "Bind" << i + 1 << "Oper=" << rtuinfo.rtubind[i].oper << ",L" << level 
						<< "Bind" << i + 1 << "Delay=" << rtuinfo.rtubind[i].delay;
				setstr += ostr.str();
		}

		return db_update_query(setstr,wherestr,tablename);

}


/**
 * @brief update_db_safetypwd 
 * 门锁密码更新接口  包括新开锁密码添加，修改  管理员密码修改
 */
int update_db_safetypwd(SafetyPwdInfo & rtuinfo)
{

		ostringstream ostr;
		string setstr;
		string wherestr;
		string retstr;
		string tablename="SafetyPwdInfo";
		Uint16 id=rtuinfo.id;

		string colstr;
		ostr.str("");
		ostr << "Pwd" <<id ;
		colstr = ostr.str();

		Uint64 UniqueIndex;
		UniqueIndex = rtuinfo.hostadd;
		UniqueIndex = UniqueIndex << 32;
		UniqueIndex += rtuinfo.rtuadd;
		ostr.str("");
		ostr << "UniqueIndex=" << UniqueIndex;
		wherestr = ostr.str();

		if (strcmp(rtuinfo.oldpwd, "") != 0)
		{
				db_select_query(colstr, wherestr, tablename,retstr);

				Int8 szpwd[16];
				Int32 dbpwdlen = retstr.length();
				for (Int16 i = 0; i < MAX_PASSWORD_LEN && i < dbpwdlen - 1; i++)
						szpwd[i] = retstr.c_str()[i];
				szpwd[15] = 0;

				if (strcmp(rtuinfo.oldpwd, szpwd) == 0)
				{
						ostr.str("");
						ostr << "Pwd" <<id << "='" << rtuinfo.newpwd << "'";
						setstr = ostr.str();		
				}	
				else
				{
						LOG_ERROR("passwrod auth error.\n");
						return -1;
				}	
		}
		else
		{
				if(rtuinfo.id != 0)
				{
						ostr.str("");
						ostr << "Pwd" << id << "='" << rtuinfo.newpwd << "'";
						setstr = ostr.str();			
				}

		}
		return db_update_query(setstr,wherestr,tablename);

}





/*************************************************查找接口****************************************************/

/**
 * @brief select_db_hostinfo  
 *           根据主机名 查询主机信息  ，用于  APP远程登陆
 * @param  pHostinfo
 * @param  hostname
 *   HostAdd,HostPwd,HostIp,HostState,LastComTime
 * @return  返回值   -1  错误   0  正确    1   没有数据 
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
		//string colstr="HostAdd,HostPwd,HostState,LastComTime";
		string colstr="HostAdd,HostPwd,HostIp,HostState,LastComTime";;
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
				while (token != NULL && index < 5)//4因为只查询了4列
				{
						if (index == 0)
								pHostinfo->hostadd = atoi(token);
						else if (index == 1)
								strcpy(pHostinfo->hostpwd, token);
						else if (index == 2)
								pHostinfo->hostip = atoi(token);
						else if (index == 3)
								pHostinfo->hoststate =(Rtustatus)atoi(token);
						else if (index == 4)
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
 * @param  hostname
 *	 主机名
 * @return  
 */
int delete_db_host(char* hostname)
{
		//check_connect();
		char sql[1024];
		int  len = 0;
		int  ret = 0;
		for (int i = 1; i < MAX_DB_TYPE; i++)
		{
				len = 0;
				len += sprintf(sql + len, "DELETE %s.* ", DB_TABLE_NAMELIST[i]);
				len += sprintf(sql + len, "FROM %s ", DB_TABLE_NAMELIST[i]);
				len += sprintf(sql + len, "LEFT JOIN %s ON %s.HostAdd = %s.HostAdd ", DB_TABLE_NAMELIST[0], DB_TABLE_NAMELIST[i], DB_TABLE_NAMELIST[0]);
				len += sprintf(sql + len, "WHERE %s.HostName = '%s'", DB_TABLE_NAMELIST[0], hostname);
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
		len += sprintf(sql + len, "DELETE FROM %s WHERE HostName='%s' ", DB_TABLE_NAMELIST[0], hostname);
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
 * @param  rtuinfo
 *
 * @return  -1  删除出错 0 删除成功  或者  没有删除项
 */
int delete_db_rtu(RtuRemoveInfo & rtuinfo)
{

		Uint8 type =rtuinfo.rtutype;
		LOG_DEBUG("wjj test type=%u\n",type);
		/*  拿到  表 名  */
		string tablename=get_table_name(type);

		if(strcmp(tablename.c_str(),"")==0)
		{
				LOG_ERROR("no such table...\n");
				return -1;
		}


		//check_connect();
		pthread_mutex_lock(&mutex);
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
		pthread_mutex_unlock(&mutex);
		return 0;
}



/** 根据 hostadd 拿到 ip */
/*Uint32 GetIpByHostAdd(Uint32 hostadd, Uint32 & hostip)
  {


  check_connect();

  char sql[96];
  sprintf(sql, "select HostIp from HostInfo where HostAdd='%u'", hostadd);
  LOG_DEBUG("GetIpByHostAdd sql=%s,len=%d\n", sql,strlen(sql));

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

  hostip = 0;
  Uint32 index = 0;
  MYSQL_ROW  mysqlrow;
  while ((mysqlrow = mysql_fetch_row(pres)) != NULL)
  {	//后继的调用正常返回的内容
  if (mysqlrow[0])
  {
  hostip = (Uint32)atoi(mysqlrow[0]);
  }
  index++;
  LOG_DEBUG("select success fieldcoun=%d index=%d hostip=%08x\n", mysql_field_count(&m_mysqlfd), index, hostip);
  }
  mysql_free_result(pres);

  if (index > 1)
  return -1;
  return 0;
  }*/
/****************************************************
  通过主机地址返回主机IP
 *****************************************************/
Uint32 GetIpByHostAdd(Uint32 hostadd, Uint32 & hostip)
{


		check_connect();

		char sql[96];
		//sprintf(sql, "select HostIp from HostInfo where HostAdd='%u'", hostadd);
		sprintf(sql, "select HostVersion from HostInfo where HostAdd='%u'", hostadd);
		//LOG_DEBUG("GetIpByHostAdd sql=%s\n", sql);

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

		hostip = 0;
		Uint32 index = 0;
		MYSQL_ROW  mysqlrow;
		while ((mysqlrow = mysql_fetch_row(pres)) != NULL)
		{	//后继的调用正常返回的内容
				if (mysqlrow[0])
				{
						hostip = (Uint32)atoi(mysqlrow[0]);
				}
				index++;
				//	LOG_DEBUG("select success fieldcoun=%d index=%d hostip=%08x\n", mysql_field_count(&m_mysqlfd), index, hostip);
		}
		mysql_free_result(pres);

		if (index > 1)
				return -1;
		return 0;
}

/**************************************************************************/















