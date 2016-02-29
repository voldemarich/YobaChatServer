#!/bin/bash
# Installs the server, configures the db if perpared. Remember to install db and configure the user
# for it on your own!

apt-get install libmysqlcppconn7 libboost-system1.55.0 libboost-random1.55.0;
mkdir /etc/YobaChatServer;
cp ../bin/Release/messengerserver /usr/bin/yobachatserver;
echo "Please specify the following:";
echo "database host";
read dbhost;
echo "database port";
read dbport;
echo "database name";
read dbname;
echo "user";
read dbuser;
echo "password";
read password;
echo "Now specify the port on which the server will be listening";
read serverport;
echo $dbhost > /etc/YobaChatServer/dbhost.cfg;
echo $dbname > /etc/YobaChatServer/dbname.cfg;
echo $dbport > /etc/YobaChatServer/dbport.cfg;
echo $dbuser > /etc/YobaChatServer/dbuser.cfg;
echo $password > /etc/YobaChatServer/dbpassword.cfg;
echo $serverport > /etc/YobaChatServer/serverport.cfg;
cp startyobaserver.sh /etc/YobaChatServer;
cp stopyobaserver.sh /etc/YobaChatServer;
cp restartyobaserver.sh /etc/YobaChatServer;
ln -s /etc/YobaChatServer/startyobaserver.sh /usr/bin/yobachatserver-start; chmod +x /usr/bin/yobachatserver-start;
ln -s /etc/YobaChatServer/stopyobaserver.sh /usr/bin/yobachatserver-stop; chmod +x /usr/bin/yobachatserver-stop;
ln -s /etc/YobaChatServer/restartyobaserver.sh /usr/bin/yobachatserver-restart; chmod +x /usr/bin/yobachatserver-restart;
mysql -h $dbhost -P $dbport -u $dbuser -p$password $dbname < ../sqltools/prepareDB.sql;
echo "Installation done, server is configured. Now run yobachatserver-start to start the server";
