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

MYSQL m_mysqlfd;   ///数据库连接描述符
/**
 * @brief init_mysql 
 *
 * @Param: info    连接mysql时，mysql相关信息结构体   
 *
 * Returns: 
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



std::string  get_table_name(int type)
{
	u32 i;
	std::string ret = "";
	for (i = 0; i < sizeof(gTypeInfoList) / sizeof(gTypeInfoList[0]); i ++) {
		if (gTypeInfoList[i].rtutype == type) {
			ret = gTypeInfoList[i].tablename;
			break;
		}
	}
	return ret;  

}

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


static int db_insert_query(const string &colstr, const string &valstr, const string &tablename)
{
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


int insert_db_rtu(RtuRegInfo* pRtuinfo)
{

	if(pRtuinfo == NULL)
	{
		LOG_ERROR("pRtuinfo is  NULL\n");
		return -1;
	}

	RtuRegInfo rtuinfo = *pRtuinfo; // 拿到注册设备信息
	int type =rtuinfo.rtutype;


	/*插入前 检查数据连接  */
	check_connect();

	/*  拿到  表 名  */
	string tablename=get_table_name(type);

 
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

int check_connect()
{

	return mysql_ping(&m_mysqlfd);

}


/*
*主机远程注册 
* 返回值  -1  错误；  0 注册成功  ；2 主机名字已经存在  
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


/* 查找对应的主机名  */
int findHostname(char* hostname)
{
	if (hostname == NULL)
	{
		LOG_ERROR("hostname is null..\n");
		return -1;
	}

	/*插入前 检查数据连接  */
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
		printf("haha  this is a error .%d\n",rtuinfo.uniqueindex);
		printf("sizeof :%d\n",sizeof(*(RtuRegInfo *)info));
		printf("sizeof :%d\n",sizeof(RtuRegInfo));
		insert_db_rtu(&rtuinfo);
	}
	return 0;
}







