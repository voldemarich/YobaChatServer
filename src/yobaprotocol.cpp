#include <string>
#include "json.h"

using namespace std;

const string TYPED[] = {"ACK", "FAIL", "ISALIVE", ""};
struct transaction{
    string id, transtype, typed;
    string usertoken;
    void* data;
};

string getResponce(string request)
{
    Json::Value req;
    Json::Reader reader;
    if(!reader.parse(request, req, false)){
        return genFail();
    }

    return "";
}

string genFail(string id){

}

string genFail(){

}
