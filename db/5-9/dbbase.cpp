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

int check_connect()
{

	return mysql_ping(&m_mysqlfd);

}


/*
*主机远程注册 
* 返回值  -1  错误；  0 注册成功  ；>0  主机名字已经存在  
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


/* 查找对应的主机名  
*  返回值   -1  错误   0  没有找到对应的   >0  找到的主机个数

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


/************************************************************************
函数名称：
	GetHostIdByName
输入参数：
	无
输出参数：
	无
返回结果：
	Uint32 注册主机个数
描述：
	通过主机名查找该主机的ID
************************************************************************/
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







/*  选择查询 接口   
参数 ：  sql ：  sql命令    retstr  :  结果  
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


/************************************************************************
函数名称：
	SelectQuery
输入参数：
	const string &colstr 字段名字符串
	const string &wheresrt 条件字符串
	const Uint16 &tabletype 表类型
输出参数：
	string & retstr
返回结果：
	int  操作结果
描述：
	选择查询
************************************************************************/
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




/*  
  查询 主机信息    
  返回值   -1  错误   0  正确    1   没有数据   
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

/*  删除主机   */
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
}

/*  删除设备 接口  
返回值  -1  错误；  0 删除成功  或者  没有删除项 
*/
int delete_db_rtu(RtuRemoveInfo* pRtuinfo)
{
	if(pRtuinfo == NULL)
	{
		LOG_ERROR("pRtuinfo is  NULL\n");
		return -1;
	}

	RtuRemoveInfo rtuinfo = *pRtuinfo; // 拿到注册设备信息
	int type =rtuinfo.rtutype;
	/*  拿到  表 名  */
	string tablename=get_table_name(type);


	check_connect();

	char sql[256];
	Uint64 index = rtuinfo.hostadd;
	index = index << 32;
	index += rtuinfo.rtuadd;
	sprintf(sql, "delete from %s where UniqueIndex=%ld", tablename.c_str(), index);

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




/************************************************************************
函数名称：
	GetHostCount
输入参数：
	无
输出参数：
	无
返回结果：
	Int32 注册主机个数
描述：
	统计注册主机个数
************************************************************************/
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


/************************************************************************
函数名称：
	DupicateKeyUpdateQuery
输入参数：
	const string &colstr 列
	const string &valstr 值
	const string &updatestr 更新列
	const Uint16 &tabletype 表类型
输出参数：
	无
返回结果：
	Int32 操作结果
描述：
	更新、插入查询：存在则更新不存在则插入
************************************************************************/
int DupicateKeyUpdateQuery(const string &colstr, const string &valstr, const string &updatestr, const Uint16 &tabletype)
{
	if (colstr.empty() || valstr.empty() || updatestr.empty() || tabletype > MAX_DB_TYPE)
		return -1;

	check_connect();

	string sql = "INSERT INTO ";
	sql += DB_TABLE_NAMELIST[tabletype];
	sql += "(";
	sql += colstr;
	sql += ") ";
	sql += "VALUES(";
	sql += valstr;
	sql += ") on DUPLICATE KEY UPDATE ";
	sql += updatestr;
	LOG_DEBUG("DupicateKeyUpdateQuery len=%ld sql=%s\n", sql.length(), sql.c_str());

	LOG_DEBUG("DupicateKeyUpdateQuery mysql_query ret=%d\n", ret);

	return mysql_query(&m_mysqlfd, sql.c_str());
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










