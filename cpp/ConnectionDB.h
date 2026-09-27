#ifndef CONNECTION_DB_H
#define CONNECTION_DB_H

#include <string>
#include <cstdint>
#include <mysql_driver.h>
#include <mysql_connection.h>

using namespace std;

class ConnectionDB {
public:
    ConnectionDB();

    // ---------- Constructeur explicite ----------
    ConnectionDB(const string& url,const string& host,
                 uint16_t      port,
                 const string& user,
                 const string& password,
                 const string& dbname);

    sql::Connection* getConnection() const;

private:
    string   host_;
    uint16_t port_ = 0;
    string   user_;
    string   pass_;
    string   base_;
    string url_;
};

#endif