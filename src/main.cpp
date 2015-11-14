#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <iostream>
#include <cstring>
#include <cstdlib>
#include <pthread.h>
#include <unistd.h>
#include "mbedtls/net.h"
#include "mbedtls/ssl.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/debug.h"


#define MAXCON 64
#define PORTUSED "4444"

using namespace std;

const string exitst = "exit";

struct addrinfo hints, *res;
struct sockaddr_storage theiraddr;
socklen_t addrsize;
int myserversocket, mynewsocket;
pthread_t threads[MAXCON];
int conns[MAXCON];



void *command_listener(void *arg)
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
    char l[2048];
    while(1)
    {
        /*if(!recv(connID, &l, sizeof l, 0))
        {
            cout << "Connection #" << connID << " is closing. \n";
            close(connID);
            return 0;
        }
        string a (l);
        if((int)a.find(exitst) == 0)
        {
            cout << "Connection #" << connID << " is closing. \n";
            close(connID);
            return 0;
        }*/
        cout << "Connid #" << connID << " said: " << a;
        //string n;
        string n = (a + " - that's what you've said to me faggot \n");
        //n = a + " - that's what you've said to me faggot \n";
        send(connID, n.c_str(), n.length(), 0);
        memset(l, 0, sizeof l);
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
    int curcon = 0;
    pthread_create(&threads[curcon], NULL, command_listener, &myserversocket);
    conns[curcon] = myserversocket;
    curcon++;
    while(1)
    {
        mynewsocket = accept(myserversocket, (sockaddr*)&theiraddr, &addrsize);
        conns[curcon] = mynewsocket;
        pthread_create(&threads[curcon], NULL, init_comm_thread, &mynewsocket);
        curcon++;
    }


}
