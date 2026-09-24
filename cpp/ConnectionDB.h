#ifndef CONNECTION_DB_H
#define CONNECTION_DB_H

#include <string>
#include <cstdint>
#include "mysql/mysql.h"

using namespace std;

class ConnectionDB {
public:
    ConnectionDB();

    // ---------- Constructeur explicite ----------
    ConnectionDB(const string& host,
                 uint16_t      port,
                 const string& user,
                 const string& password,
                 const string& dbname);

    // ---------- Informations ----------
    string   getHost() const;
    uint16_t getPort() const;
    string   getUser() const;
    string   getBase() const;
    string   toString() const;   // mot de passe masqué

    MYSQL* open() const;

private:
    string   host_;
    uint16_t port_ = 0;
    string   user_;
    string   pass_;
    string   base_;
};

#endif