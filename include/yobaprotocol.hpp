#include <string>
#include <map>
#include <boost/algorithm/string/replace.hpp>
#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>
#include "base64_tools.hpp"
#include "yobadbservice.hpp"

#define stopseq "~~//END//~~\n"
#define SUCCESS 200
#define IDSIZE 32

using namespace std;
using namespace rapidjson;

const string errorcodes[] = {"0:message malformed", "1:reg with non-unique params", "2:db not accessible",
 "3:token invalid", "4:authorization data invalid"};

bool validate_transaction(Document & jsondoc)
{
    bool valresult = true;
    valresult &= jsondoc.HasMember("id");
    valresult &= jsondoc.HasMember("typed");
    valresult &= jsondoc.HasMember("usertoken");
    valresult &= jsondoc.HasMember("data");
    if(valresult)
    {
        valresult &= jsondoc["id"].IsString();
        valresult &= jsondoc["typed"].IsString();
        valresult &= jsondoc["data"].IsArray();
    }
    return valresult;
}

string prepareToSend(string w)
{
    return encode64(w)+stopseq;
}

string domToString(Document & dom)
{
    StringBuffer buffer;
    Writer<StringBuffer> writer(buffer);
    dom.Accept(writer);
    return buffer.GetString();
}

Document formBasicProtocolDoc(string id, string type, string usertoken)
{
    Document resp;
    resp.SetObject();
    Document::AllocatorType& alloc = resp.GetAllocator();
    resp.AddMember("id", Value(StringRef(id.c_str())), alloc);
    resp.AddMember("typed", Value(StringRef(type.c_str())), alloc);
    resp.AddMember("usertoken", Value(StringRef(usertoken.c_str())), alloc);
    resp.AddMember("data", Value().SetArray(), alloc);
    return resp;
}

//Todo transfer validity time
string genToken(Document & request, string token) {
    Document resp = formBasicProtocolDoc(request["id"].GetString(), "TOK", token);
    return prepareToSend(domToString(resp));
}

string genAck(Document & request)
{
    Document resp = formBasicProtocolDoc(request["id"].GetString(), "ACK", request["usertoken"].GetString());
    return prepareToSend(domToString(resp));
}

string genFail(Document & request, string reason, int errorcode)
{
    Document resp = formBasicProtocolDoc(request["id"].GetString(), "FAIL", request["usertoken"].GetString());
    Document::AllocatorType& alloc = resp.GetAllocator();
    Value * data = new Value();
    data->SetObject();
    data->AddMember("errordescr", Value(StringRef(reason.c_str())), alloc);
    data->AddMember("errorcode", Value(errorcode), alloc);
    resp["data"].PushBack(*data, alloc);
    return prepareToSend(domToString(resp));
}

string genFail(string reason, int errorcode)
{
    Document resp = formBasicProtocolDoc(genRandomAlphanumericStr(32), "FAIL", "");;
    Document::AllocatorType& alloc = resp.GetAllocator();
    Value * data = new Value();
    data->SetObject();
    data->AddMember("errordescr", Value(StringRef(reason.c_str())), alloc);
    data->AddMember("errorcode", Value(errorcode), alloc);
    resp["data"].PushBack(*data, alloc);
    return prepareToSend(domToString(resp));
}

string genMessages(Document & request) {}

string typeGenSwitch(Document & request, DbService * dbcon)
{
try{
    string typed = request["typed"].GetString();
    if(typed == "REG")
    {
        bool validation = true;
        validation &= (request["data"].Size() > 0);
        if(validation)
        {
            validation &= request["data"][0].HasMember("login");
            validation &= request["data"][0].HasMember("password");
            validation &= request["data"][0].HasMember("email");
        }
        if(validation)
        {
            dbcon->registerUser(request);
            return genAck(request); //return genToken(request);
        }
    }
    if(typed == "MSG")
    {
        bool validation = true;
        validation &= request["data"].Size() > 0;
        if(validation)
        {
            validation &= request["data"][0].HasMember("sender");
            validation &= request["data"][0].HasMember("receiver");
            validation &= request["data"][0].HasMember("message");
        }
        if(validation)
        {
            dbcon->commitNewMessage(request);
            return genAck(request);
        }
    }
    if(typed == "AUTH") {
        dbcon->authorize(request);
        return genToken(request, dbcon->generateToken(request["data"][0]["login"].GetString()));
    }
    if(typed == "HASMSG")
        return genMessages(request);
    //if(typed == "RNTOCK") {}
    //if(typed == "INVALTOCK") {}
    if(typed == "ALV") return genAck(request);
    //to be continued
}
catch(int e){
    return genFail(request, errorcodes[e], e);
    }
}

string getResponce(string request, DbService * dbcon)
{
    boost::replace_all(request, stopseq, "");
    boost::replace_all(request, " ", "");
    boost::replace_all(request, "\n", "");
    try
    {
        string clear_request = decode64(request);
        Document jsondoc;
        jsondoc.Parse(decode64(request).c_str());
        if(!jsondoc.IsObject()) return genFail(errorcodes[0], 0);
        if(!validate_transaction(jsondoc)) return genFail(errorcodes[0], 0);
        return typeGenSwitch(jsondoc, dbcon);
    }
    catch(exception e)
    {
        return genFail(errorcodes[0], 0);
    }
}
