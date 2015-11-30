#include <rapidjson/document.h>
#include <mysql_connection.h>
#include <mysql_driver.h>
#include <cppconn/prepared_statement.h>
#include <string>
#include <iostream>
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
            prepstmt = conn->prepareStatement("insert into users (login, pwdhash, email) values (?, sha1(?), ?);");
            prepstmt->setString(1, login);
            prepstmt->setString(2, pwd);
            prepstmt->setString(3, email);
            prepstmt->executeUpdate();
            conn->commit();
            return true;
        }
        catch(exception e)
        {
            return false;
        }
    }

    bool validateTokenByName(string user, string token)
    {
        PreparedStatement * prepstmt;
        prepstmt = conn -> prepareStatement("select ((select expiry from tokens where (userid=(select id from users where login=?) and token=?)) > now() is true) as 'res';");
        prepstmt->setString(1, user);
        prepstmt->setString(2, token);
        try
        {
            ResultSet * rs;
            rs = prepstmt->executeQuery();
            int num = rs->getMetaData()->getColumnCount();
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
        prepstmt = conn -> prepareStatement("select (((select expiry from tokens where token=?) > now() is true) as res, (select userid from tokens where token=?) as userid);");
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
            prepstmt = conn -> prepareStatement("delete from tokens where userid=(select id from users where login=?); insert into tokens (userid, token, expiry) values ((select id from users where login=?), ?, now()+interval ? hours)");
            prepstmt->setString(1, login);
            prepstmt->setString(2, login);
            string token = genRandomAlphanumericStr(TOKEN_SIZE);
            prepstmt->setString(3, token);
            prepstmt->setInt(4, TOKEN_VALID_TIME);
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
        if(request["data"].Size() > 1)
        {
            if(request["data"][0].HasMember("login") && request["data"][0].HasMember("password"))
            {
                try
                {
                    PreparedStatement * prepstmt;
                    prepstmt = conn->prepareStatement("select ((select pwdhash from users where login=?) is sha1(?)) as val;");
                    prepstmt->setString(1, request["data"][0]["login"].GetString());
                    prepstmt->setString(2, request["data"][0]["password"].GetString());
                    ResultSet * rs;
                    rs = prepstmt->executeQuery();
                    rs->next();
                    if(rs->getBoolean("val")) return;
                    throw 4;
                }
                catch(exception e)
                {
                    throw 2;
                }

            }
        }
        else throw 0;
    }

    Value retrievePendingMessages(Document & request)
    {
        try
        {
            int recvid= validateToken(request["usertoken"].GetString());
            PreparedStatement * prepstmt;
            prepstmt = conn->prepareStatement("select senderuid, receiveruid, msg from msgs where (receiveruid=? and status=0); update msgs set status=1 where receiveruid=?;");
            prepstmt->setInt(1, recvid);
            prepstmt->setInt(2, recvid);
            ResultSet * rs;
            rs = prepstmt->executeQuery();
            conn->commit();
            Value msgarr;
            msgarr.SetArray();
            Value::AllocatorType alloc;
            ResultSet * namers;
            prepstmt = conn->prepareStatement("select login from users where id=? as recvn;");
            prepstmt->setInt(1, recvid);
            namers = prepstmt->executeQuery();
            namers->next();
            string receiver = namers->getString("recvn");
            while(rs->next()){
                Value n;
                n.SetObject();
                prepstmt = conn->prepareStatement("select login from users where id=? as sendern;");
                prepstmt->setInt(1, rs->getInt("senderid"));
                namers = prepstmt->executeQuery();
                n.AddMember("sender", StringRef(namers->getString("sendern").c_str()), alloc);
                n.AddMember("receiver", StringRef(receiver.c_str()), alloc);
                n.AddMember("message", StringRef(rs->getString("msg").c_str()), alloc);
                msgarr.PushBack(n, alloc);
            }
            return msgarr;
        }
        catch(exception e)
        {
            throw 2;
        }
    }

    void commitNewMessage(Document & request)
    {
        try
        {
            PreparedStatement * prepstmt;
            validateTokenByName(request["data"][0]["sender"].GetString(), request["usertoken"].GetString());
            prepstmt = conn -> prepareStatement("insert into msgs (senderuid, receiveruid, msg) values ((select id from users where (login = ?)), (select id from users where (login = ?)), ?);");
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


};
