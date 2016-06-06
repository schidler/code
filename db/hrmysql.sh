#!/bin/bash
mysql_user="root"
mysql_pwd="6e79821d03@24680"
CmdPath="/usr/bin"
databases="HuaRuiCloud test"
basepath='/root/backup/'
function_start()
{
        printf "Starting MySQL...\n"
        /bin/sh ${CmdPath}/mysqld_safe --defaults-file=/etc/mysql/my.cnf >/dev/null 2>&1 &
}
function_stop()
{
        printf "Stoping MySQL...\n"
		${CmdPath}/mysqladmin -u${mysql_user} -p${mysql_pwd} -S /var/run/mysqld/mysqld.sock shutdown >/dev/null
		#/etc/init.d/mysql stop
}
function_restart()
{
        printf "Restarting MySQL...\n"
        function_stop
        sleep 2
        function_start
}

function_backup()
{


        printf "Backuping MySQL...\n"

	if [ ! -d $basepath ]; then
   		mkdir -p "$basepath"
	fi

	for db in $databases
   	do
		mysqldump -u${mysql_user} -p${mysql_pwd} --databases $db > $basepath$db-$(date +%Y%m%d%H%M%S).sql
   	done
}

case $1 in
        start)
                function_start
        ;;
        stop)
                function_stop
        ;;
        restart)
                function_restart
        ;;
	backup)
		function_backup
	;;
        *)
                printf "Usage: $0 {start|stop|restart|backup}\n"
esac
