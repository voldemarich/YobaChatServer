#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <iostream>
#include <cstring>
#include <pthread.h>
#include <unistd.h>
#include <vector>

#define MAXCON 64
#define PORTUSED "4444"

using namespace std;

const string exitst = "exit";

struct addrinfo hints, *res;
struct sockaddr_storage theiraddr;
socklen_t addrsize;
int myserversocket, mynewsocket;
vector<int> conns;

void closeconn(int connid)
{
    cout << "Connection #" << connid << " is closing. \n";
    close(connid);
}


void *server_console_listener(void *arg)
{
    while(1)
    {
        string a = "";
        cin >> a;
        if((int)a.find(exitst) == 0)
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
    while(1)
    {
        char len[32];
        memset(len, 0, sizeof len);
        int rsvalrecv = recv(connID, &len, sizeof len, 0);
        if(!rsvalrecv)
        {
            closeconn(connID);
            pthread_exit(NULL);
        }
        string slen (len);
        int instrlen = atoi(slen.c_str());
        if(instrlen == 0) continue;
        cout << "Strlen accepted is: " << instrlen << "\n";
        char* l = new char[instrlen];
        memset(l, 0, sizeof &l);
        int rsstrrecv = recv(connID, l, instrlen, 0);
        if(!rsstrrecv)
        {
            closeconn(connID);
            return 0;
        }
        string a (l);
        if((int)a.find(exitst) == 0)
        {
            cout << "Connection #" << connID << " is closing. \n";
            close(connID);
            pthread_exit(NULL);
        }
        cout << "Connid #" << connID << " said: " << a << "\n";
        string n = (a + " - that's what you've said to me faggot \n");
        send(connID, n.c_str(), n.length(), 0);
    }
}

int main()
{
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
    conns.push_back(myserversocket);
    pthread_create(&newthread, NULL, server_console_listener, &myserversocket);
    while(1)
    {
        mynewsocket = accept(myserversocket, (sockaddr*)&theiraddr, &addrsize);
        conns.push_back(mynewsocket);
        pthread_t newthread;
        pthread_create(&newthread, NULL, init_comm_thread, &mynewsocket);
    }


}
