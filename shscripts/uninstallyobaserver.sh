#!/bin/bash
# Uninstall the yobaserver.

killall -9 yobachatserver;
rm /usr/bin/yobachatserver*;
rm -rf /etc/YobaChatServer;

read -p "Uninstall libs?(n)" -n 1 repl;
if [[ ! $repl =~ ^[Yy]$ ]]
then
	apt-get remove libmysqlcppconn7 libboost-system1.55.0 libboost-random1.55.0;
fi
