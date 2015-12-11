#!/bin/bash
# Launches the YobaServer Instance

serverport=$(cat /etc/YobaChatServer/serverport.cfg);
dbhost=$(cat /etc/YobaChatServer/dbhost.cfg);
dbport=$(cat /etc/YobaChatServer/dbport.cfg);
dbname=$(cat /etc/YobaChatServer/dbname.cfg);
dbuser=$(cat /etc/YobaChatServer/dbuser.cfg);
dbpass=$(cat /etc/YobaChatServer/dbpass.cfg);

/usr/bin/yobachatserver $serverport $dbhost $dbport $dbname $dbuser $dbpass;
