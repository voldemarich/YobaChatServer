#include <iostream>
#include <cstdlib>
#include "boost/asio.hpp"
#include "prsockets_boost.hpp"

using namespace std;

//const string exitst = "exit";

int main(int argc, char * argv[])
{
    /*if(argc < 7){
        cout << "The args order - serverport, dbhost, dbport, dbname, dbuser, dbpassword";
        exit(EXIT_FAILURE);
    }
    int serverport = atoi(argv[1]);
    if(serverport < 8){
        cout << "put the valid port number as the first arg";
        exit(EXIT_FAILURE);
    }*/
    pid_t pid, sid;
    pid = fork();
    if(pid < 0) exit(EXIT_FAILURE);
    if(pid > 0) exit(EXIT_SUCCESS);
    sid = setsid();
    if(sid < 0) exit(EXIT_FAILURE);
    //initsrv(serverport, argv[2], argv[3], argv[4], argv[5], argv[6]);
    initsrv(4444, "localhost", "3306", "messengerserver", "messenger", "qwerty123");
    return 0;
}
