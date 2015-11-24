#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <iostream>
#include <cstring>
#include <pthread.h>
#include <unistd.h>
#include <vector>

#define EXITCOMM "exit"
#define MAXCON 64
#define PORTUSED "4444"
#define PACKET 512
#define MAX_PACKETS_PER_TRANSACTION 30

using namespace std;

void closeconn(int connID)
{
    cout << "Connection #" << connID << " is closing. \n";
    close(connID);
    pthread_exit(NULL);
}

void chkcomm(string cm, int connID){
    if(cm == EXITCOMM) closeconn(connID);
}


void *init_console_listener_thread(void *arg)
{
    vector<int> conns = *(vector<int>*)arg;
    while(1)
    {
        string a = "";
        cin >> a;
        if((int)(a.find(EXITCOMM)) == 0)
        {
            for(int i = MAXCON-1; i>=0; i--)
            {
                close(conns[i]);
            }
            exit(0);
        }
    }
}

void *init_comm_thread(void *arg)
{
    int connID = *(int*)arg;
    cout << "Smb connected with the connid " << connID << "\n";
    char * buffer = new char[1];
    while(1){
        bzero(&len, strlen(len));
        if(!recv(connID, &len, 32, 0)) closeconn(connID);
        int sz;
        if(!(sz = atoi(len))) continue;
        buffer = (char*)malloc((sizeof (char))*PACKET);
        bzero(buffer, sizeof *buffer);
        string a = "";
        bzero(buffer, PACKET);
        if(!recv(connID, buffer, sz, 0)) closeconn(connID);
        a += buffer;
        cout << a << "\n";
        send(connID, a.c_str(), a.length(), 0);
        }
        pthread_exit(NULL);
        //free(buffer);
}

int initsock(vector<int>* conns)
{
    struct addrinfo hints, *res;
    struct sockaddr_storage theiraddr;
    socklen_t addrsize;
    int myserversocket, mynewsocket;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;  // use IPv4 or IPv6, whichever
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;     // fill in my IP for me

    getaddrinfo(NULL, PORTUSED, &hints, &res);
    myserversocket = socket(res->ai_family, res->ai_socktype, res->ai_protocol);

    int optval = 1;
    setsockopt(myserversocket, SOL_SOCKET, SO_REUSEADDR, (char*)&optval, sizeof optval);

    if(bind(myserversocket, res->ai_addr, res->ai_addrlen) == -1)
    {
        cout << "CANNOT BIND TO PORT\n";
        throw 1;
    }
    if(listen(myserversocket, MAXCON) == -1)
    {
        cout << "CANNOT LISTEN TO PORT";
        throw 1;
    }

    addrsize = sizeof theiraddr;
    pthread_t newthread;
    (*conns).push_back(myserversocket);
    pthread_create(&newthread, NULL, init_console_listener_thread, conns);
    while(1)
    {
        mynewsocket = accept(myserversocket, (sockaddr*)&theiraddr, &addrsize);
        if(mynewsocket != -1)
        {
            (*conns).push_back(mynewsocket);
            pthread_t newthread;
            pthread_create(&newthread, NULL, init_comm_thread, &mynewsocket);
        }
    }
}
