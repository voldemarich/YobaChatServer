#include <rapidjson/document.h>
#include <mysql_connection.h>
#include <mysql_driver.h>
#include <cppconn/prepared_statement.h>
#include <string>
#include "utils.hpp"

using namespace rapidjson;
using namespace sql;
using namespace std;

class DbService
{
    Driver * driver;
    Connection * conn;
    Statement * stmt;
    PreparedStatement * prepstmt;

public:
    DbService(string host, string user, string password, string database)
    {
        driver = get_driver_instance();
        conn = driver->connect(host, user, password);
        conn->setAutoCommit(0);
        conn->setSchema(database);
        stmt = conn->createStatement();
    }

    ~DbService()
    {
        delete stmt;
        conn->close();
        delete conn;
    }

    bool registerUser(Document & request)
    {
        try
        {
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

    Value retrievePendingMessages(Document & request) {}

    string generateDbToken(Document & request) {}

    bool validate_token(string user, string token){

    }

    bool commitNewMessage(Document & request)
    {
        try
        {
            //if(!validate_token())
            prepstmt = conn -> prepareStatement("insert into msgs (senderuid, receiveruid, msg) values ((select id from users where (login = ?)), (select id from users where (login = ?)), ?)");
            prepstmt->setString(1, request["data"][0]["sender"].GetString());
            prepstmt->setString(2, request["data"][0]["receiver"].GetString());
            prepstmt->setString(3, request["data"][0]["message"].GetString());
            prepstmt->executeUpdate();
            conn->commit();
            return true;
        }
        catch(exception e)
        {
            return false;
        }
    }


};
