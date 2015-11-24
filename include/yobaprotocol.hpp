#include <string>
#include <map>
#include "json.h"

using namespace std;

const map TYPED = {"ACK", "FAIL", "ISALIVE", ""};
struct transaction{
    string id, transtype, typed;
    string usertoken;
    map<string, string> data;
};

string genFail(string id){

}

string genFail(){
    return "";
}

string getResponce(string request)
{

    Json::Value req;
    Json::Reader reader;
    if(!reader.parse(request, req, false)){
        return genFail();
    }


    return "";
}

bool validate_transaction(transaction trans){

}
