#include <rapidjson/document.h>
#include <mysql_connection.h>
#include <mysql_driver.h>
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
    DbService(string host, string user, string password, string database){
        driver = get_driver_instance();
        conn = driver->connect(host, user, password);
        conn->setAutoCommit(0);
        conn->setSchema(database);
        stmt = conn->createStatement();
    }

    ~DbService(){
        delete stmt;
        conn->close();
        delete conn;
    }

    bool registerUser(Document & request) {
        try{
            prepstmt->
            return true;
        }
        catch(exception e){
            return false;
        }
    }

    Value retrievePendingMessages(Document & request) {}

    string generateDbToken(Document & request) {}

    bool commitNewMessage(Document & request) {}


};
