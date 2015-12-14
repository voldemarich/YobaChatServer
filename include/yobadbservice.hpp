#include <rapidjson/document.h>
#include <mysql_connection.h>
#include <mysql_driver.h>
#include <cppconn/prepared_statement.h>
#include <string>
#include <vector>
#include "utils.hpp"

#define TOKEN_SIZE 64
#define TOKEN_VALID_TIME 48
//in hours

using namespace rapidjson;
using namespace sql;
using namespace std;

class DbService
{
    Driver * driver;
    Connection * conn;

public:
    DbService(string host, string user, string password, string database)
    {
        driver = get_driver_instance();
        conn = driver->connect(host, user, password);
        conn->setAutoCommit(0);
        conn->setSchema(database);
            }

    ~DbService()
    {
        conn->close();
        delete conn;
    }

    bool registerUser(Document & request)
    {
        try
        {
            PreparedStatement * prepstmt;
            string login, pwd, email;
            login = request["data"][0]["login"].GetString();
            pwd = request["data"][0]["password"].GetString();
            email = request["data"][0]["email"].GetString();
            prepstmt = conn->prepareStatement("insert into users (login, pwdhash, email, dateregister) values (?, sha1(?), ?, now());");
            prepstmt->setString(1, login);
            prepstmt->setString(2, pwd);
            prepstmt->setString(3, email);
            prepstmt->executeUpdate();
            conn->commit();
            return true;
        }
        catch(exception e)
        {
            throw 2; //todo make different exceptions processing
        }
    }

    bool validateTokenByName(string user, string token)
    {
        PreparedStatement * prepstmt;
        prepstmt = conn -> prepareStatement("select ((select expiry from tokens where (userid=(select id from users where login=?) and token=?)) > now() is true) as res;");
        prepstmt->setString(1, user);
        prepstmt->setString(2, token);
        try
        {
            ResultSet * rs;
            rs = prepstmt->executeQuery();
            string a = rs->getMetaData()->getColumnLabel(1);
            rs->next();
            if(rs->getBoolean(a))return true;
            throw 3;
        }
        catch(exception e)
        {
            throw 2;
        }
    }

    int validateToken(string token)
    {
        PreparedStatement * prepstmt;
        prepstmt = conn -> prepareStatement("select ((select expiry from tokens where token=?) > now() is true) as res, (select userid from tokens where token=?) as userid;");
        prepstmt->setString(1, token);
        prepstmt->setString(2, token);
        try
        {
            ResultSet * rs;
            rs = prepstmt->executeQuery();
            rs->next();
            if(rs->getBoolean("res")) return rs->getInt("userid");
            throw 3;
        }
        catch(exception e)
        {
            throw 2;
        }
    }

    string generateToken(string login)
    {
        try
        {
            PreparedStatement * prepstmt;
            prepstmt = conn -> prepareStatement("delete from tokens where userid=(select id from users where login=?);");
            prepstmt->setString(1, login);
            prepstmt->executeUpdate();
            prepstmt = conn->prepareStatement("insert into tokens (userid, token, expiry) values ((select id from users where login=?), ?, now()+interval ? hour);");
            string token = genRandomAlphanumericStr(TOKEN_SIZE);
            prepstmt->setString(1, login);
            prepstmt->setString(2, token);
            prepstmt->setInt(3, TOKEN_VALID_TIME);
            prepstmt->executeUpdate();
            conn->commit();
            return token;
        }
        catch(exception e)
        {
            throw 2;
        }
    }

    void authorize(Document & request)
    {
        if(request["data"].Size() > 0)
        {
            if(request["data"][0].HasMember("login") && request["data"][0].HasMember("password"))
            {
                try
                {
                    PreparedStatement * prepstmt;
                    prepstmt = conn->prepareStatement("select ((select pwdhash from users where login=?) = (sha1(?) collate utf8_general_ci) is true) as val;");
                    prepstmt->setString(1, request["data"][0]["login"].GetString());
                    prepstmt->setString(2, request["data"][0]["password"].GetString());
                    ResultSet * rs;
                    rs = prepstmt->executeQuery();
                    rs->next();
                    if(rs->getBoolean("val")) return;
                    throw 4;
                }
                catch(SQLException e)
                {
                    throw 2;
                }

            }
        }
        else throw 0;
    }

    vector<map<string, string>> retrievePendingMessages(Document & request)
    {
        int recvid= validateToken(request["usertoken"].GetString());
        try
        {
            PreparedStatement * prepstmt;
            prepstmt = conn->prepareStatement("select senderuid, receiveruid, msg, datesent from msgs where (receiveruid=? and status=0);");
            prepstmt->setInt(1, recvid);
            ResultSet * rs;
            rs = prepstmt->executeQuery();
            prepstmt = conn->prepareStatement("update msgs set status=1 where receiveruid=?;"); //ATTENTION! Threading error can occur!
            prepstmt->setInt(1, recvid);
            prepstmt->executeUpdate();
            conn->commit();
            ResultSet * namers;
            prepstmt = conn->prepareStatement("select login from users where id=?;");
            prepstmt->setInt(1, recvid);
            namers = prepstmt->executeQuery();
            namers->next();
            string receiver = namers->getString("login");
            vector<map<string, string>> msgs;
            while(rs->next()){
                map<string, string> onemsg;
                prepstmt = conn->prepareStatement("select login from users where id=?;");
                prepstmt->setInt(1, rs->getInt("senderuid"));
                namers = prepstmt->executeQuery();
                namers->next();
                onemsg.insert(make_pair("sender", namers->getString("login")));
                onemsg.insert(make_pair("receiver", receiver));
                onemsg.insert(make_pair("message", rs->getString("msg")));
                onemsg.insert(make_pair("datesent", rs->getString("datesent")));
                //cout << namers->getString("login") << "    " << rs->getString("msg") << endl;
                msgs.push_back(onemsg);
            }
            return msgs;
        }
        catch(exception e)
        {
            throw 2;
        }
    }

    void commitNewMessage(Document & request)
    {
        validateTokenByName(request["data"][0]["sender"].GetString(), request["usertoken"].GetString());
        try
        {
            PreparedStatement * prepstmt;
            prepstmt = conn -> prepareStatement("insert into msgs (senderuid, receiveruid, msg, datesent) values ((select id from users where (login = ?)), (select id from users where (login = ?)), ?, now());");
            prepstmt->setString(1, request["data"][0]["sender"].GetString());
            prepstmt->setString(2, request["data"][0]["receiver"].GetString());
            prepstmt->setString(3, request["data"][0]["message"].GetString());
            prepstmt->executeUpdate();
            conn->commit();
            return;
            throw 3;
        }
        catch(exception e)
        {
            throw 2;
        }
    }


    vector<string> getUserMatches(Document & request){
        validateToken(request["usertoken"].GetString());
        try{
            PreparedStatement * prepstmt;
            prepstmt = conn -> prepareStatement("select login from users where login like ?;");
            string tomatch = request["data"][0].GetString();
            tomatch += "%"; //Search not only for the matchreq, but for all the str beginning with it
            prepstmt -> setString(1, tomatch);
            ResultSet * rs;
            rs = prepstmt -> executeQuery();
            vector<string> strmatch;
            while(rs->next()){
                strmatch.push_back(rs->getString("login"));
            }
            return strmatch;
        }
        catch(exception e){
            throw 2;
        }
    }

};
