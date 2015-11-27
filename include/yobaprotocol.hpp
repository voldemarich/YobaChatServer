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
    valresult &= jsondoc["id"].IsString();
    valresult &= jsondoc["typed"].IsString();
    return valresult;
}

string prepareToSend(string w)
{
    return encode64(w)+stopseq;
}

string domToString(Document & dom){
    StringBuffer buffer;
    Writer<StringBuffer> writer(buffer);
    dom.Accept(writer);
    return buffer.GetString();
}

string genToken(Document & request){}

string genAck(Document & request){
    Document resp;
    resp.SetObject();
    Document::AllocatorType& alloc = resp.GetAllocator();
    resp.AddMember("id", Value(StringRef(request["id"].GetString())), alloc);
    resp.AddMember("typed", Value("ACK"), alloc);
    resp.AddMember("usertoken", Value(StringRef(request["usertoken"].GetString())), alloc);
    resp.AddMember("data", Value().SetObject(), alloc);
    return prepareToSend(domToString(resp));
}

string genFail(Document & request)
{
    Document resp;
    resp.SetObject();
    Document::AllocatorType& alloc = resp.GetAllocator();
    resp.AddMember("id", Value(StringRef(request["id"].GetString())), alloc);
    resp.AddMember("typed", Value("FAIL"), alloc);
    resp.AddMember("usertoken", Value(StringRef(request["usertoken"].GetString())), alloc);
    resp.AddMember("data", Value().SetObject(), alloc);
    return prepareToSend(domToString(resp));
}

string genFail()
{
    Document resp;
    resp.SetObject();
    Document::AllocatorType& alloc = resp.GetAllocator();
    resp.AddMember("id", Value(StringRef(genRandomAlphanumericStr(IDSIZE).c_str())), alloc);
    resp.AddMember("typed", Value("FAIL"), alloc);
    resp.AddMember("usertoken", Value(""), alloc);
    resp.AddMember("data", Value().SetObject(), alloc);
    return prepareToSend(domToString(resp));
}

string genMessages(Document & request){}

string typeGenSwitch(Document & request)
{
    string typed = request["typed"].GetString();
    if(typed == "REG")
    {
        if(registerUser(request)) return genToken(request);
    }
    if(typed == "MSG")
    {
        if(commitNewMessage(request)) return genAck(request);
    }
    if(typed == "AUTH") return genToken(request);
    if(typed == "HASMSG") return genMessages(request);
    //if(typed == "RNTOCK") {}
    //if(typed == "INVALTOCK") {}
    if(typed == "ALV") return genAck(request);
    //to be continued

    return genFail(request);
}

string getResponce(string request)
{
    boost::replace_all(request, stopseq, "");
    boost::replace_all(request, " ", "");
    boost::replace_all(request, "\n", "");
    try
    {
        string clear_request = decode64(request);
        Document jsondoc;
        jsondoc.Parse(decode64(request).c_str());
        if(!jsondoc.IsObject()) return genFail();
        if(!validate_transaction(jsondoc)) return genFail();
        return typeGenSwitch(jsondoc);
    }
    catch(exception e)
    {
        return genFail();
    }
}
