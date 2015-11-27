#include <string>
#include <map>
#include <boost/algorithm/string/replace.hpp>
#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>
#include "base64_tools.hpp"
#include "yobadbservice.hpp"

#define stopseq "~~//END//~~\n"
#define IDSIZE 32
#define TOKENSIZE 64

using namespace std;
using namespace rapidjson;

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

string genToken(Document & request) {}

string genAck(Document & request)
{
    Document resp;
    resp.SetObject();
    Document::AllocatorType& alloc = resp.GetAllocator();
    resp.AddMember("id", Value(StringRef(request["id"].GetString())), alloc);
    resp.AddMember("typed", Value("ACK"), alloc);
    resp.AddMember("usertoken", Value(StringRef(request["usertoken"].GetString())), alloc);
    resp.AddMember("data", Value().SetObject(), alloc);
    return prepareToSend(domToString(resp));
}

string genFail(Document & request, string reason, int errorcode)
{
    Document resp;
    resp.SetObject();
    Document::AllocatorType& alloc = resp.GetAllocator();
    resp.AddMember("id", Value(StringRef(request["id"].GetString())), alloc);
    resp.AddMember("typed", Value("FAIL"), alloc);
    resp.AddMember("usertoken", Value(StringRef(request["usertoken"].GetString())), alloc);
    Value * data = new Value();
    data->SetObject();
    data->AddMember("errordescr", Value(StringRef(reason.c_str())), alloc);
    data->AddMember("errorcode", Value(errorcode), alloc);
    resp.AddMember("data", *data, alloc);
    return prepareToSend(domToString(resp));
}

string genFail(string reason, int errorcode)
{
    Document resp;
    resp.SetObject();
    Document::AllocatorType& alloc = resp.GetAllocator();
    resp.AddMember("id", Value(StringRef(genRandomAlphanumericStr(IDSIZE).c_str())), alloc);
    resp.AddMember("typed", Value("FAIL"), alloc);
    resp.AddMember("usertoken", Value(""), alloc);
    Value * data = new Value();
    data->SetObject();
    data->AddMember("errordescr", Value(StringRef(reason.c_str())), alloc);
    data->AddMember("errorcode", Value(errorcode), alloc);
    resp.AddMember("data", *data, alloc);
    return prepareToSend(domToString(resp));
}

string genMessages(Document & request) {}

string typeGenSwitch(Document & request, DbService * dbcon)
{
    string typed = request["typed"].GetString();
    if(typed == "REG")
    {
        if(dbcon->registerUser(request)) return genToken(request);
        return genFail(request, "1 Registration with non-unique params", 1);
    }
    if(typed == "MSG")
    {
        if(dbcon->commitNewMessage(request)) return genAck(request);
        return genFail(request, "2 Message malformed/db not accessible", 2);
    }
    if(typed == "AUTH") return genToken(request);
    if(typed == "HASMSG") return genMessages(request);
    //if(typed == "RNTOCK") {}
    //if(typed == "INVALTOCK") {}
    if(typed == "ALV") return genAck(request);
    //to be continued

    return genFail(request, "0 Request malformed", 0);
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
        if(!jsondoc.IsObject()) return genFail("0 Request malformed", 0);
        if(!validate_transaction(jsondoc)) return genFail("0 Request malformed", 0);
        return typeGenSwitch(jsondoc, dbcon);
    }
    catch(exception e)
    {
        return genFail("0 Request malformed", 0);
    }
}
