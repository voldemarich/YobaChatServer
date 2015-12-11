# YobaChatServer
This is the server supporting YOBAchat protocol. Originally written in C++ flavoured by pthreads and JSON.

The documentation for the YOBAchat protocol v0.0.1 can be watched [here](https://docs.google.com/document/d/1eZ8uFUfumk6VpDptsb01hf_1rgTCPvlQm49Mx8LYJrQ/edit?usp=sharing)

The convenient java adapter implementation is [here](https://github.com/2fuckoff/Client)

### Fast installation guide
*Preferrably run as root*
Firstly, install mysql-server and mysql-client, configure one database and a user with full permissions for it.
Then, do the following.
1. \# git clone https://github.com/voldemarich/YobaChatServer.git
2. \# cd YobaChatServer/shscripts
3. \# sh install.sh *//Follow the instructions*
4. \# yobachatserver-start

You're done, GL:HF.
