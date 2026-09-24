#include "ConnectionDB.h"
#include <sstream>
#include <stdexcept>

// ---------------------------------------------------------------
// Constructeur par défaut
// ---------------------------------------------------------------
ConnectionDB::ConnectionDB()
    : host_("localhost"), port_(3306),
      user_("banque"), pass_("banque"), base_("banque") {}

// ---------------------------------------------------------------
// Constructeur explicite
// ---------------------------------------------------------------
ConnectionDB::ConnectionDB(const string& host,
                           uint16_t      port,
                           const string& user,
                           const string& password,
                           const string& dbname)
    : host_(host), port_(port), user_(user), pass_(password), base_(dbname) {}

// ---------------------------------------------------------------
// Accesseurs
// ---------------------------------------------------------------
string   ConnectionDB::getHost() const { return host_; }
uint16_t ConnectionDB::getPort() const { return port_; }
string   ConnectionDB::getUser() const { return user_; }
string   ConnectionDB::getBase() const { return base_; }

string ConnectionDB::toString() const {
    ostringstream oss;
    oss << "ConnectionDB {\n"
        << "  host     = " << host_ << "\n"
        << "  port     = " << port_ << "\n"
        << "  user     = " << user_ << "\n"
        << "  password = " << (pass_.empty() ? "(vide)" : "********") << "\n"
        << "  base     = " << base_ << "\n"
        << "}";
    return oss.str();
}

// ---------------------------------------------------------------
// open : ouvre une nouvelle connexion MySQL
// L'appelant est responsable de mysql_close().
// ---------------------------------------------------------------
MYSQL* ConnectionDB::open() const {
    if (port_ == 0) {
        throw invalid_argument("ConnectionDB: port invalide (0)");
    }

    MYSQL* conn = mysql_init(nullptr);
    if (!conn) {
        throw runtime_error("ConnectionDB: mysql_init a echoue");
    }

    if (!mysql_real_connect(conn,
                            host_.c_str(),
                            user_.c_str(),
                            pass_.c_str(),
                            base_.c_str(),
                            port_,
                            nullptr,
                            0)) {
        string err = string("MySQL error ")
                   + to_string(mysql_errno(conn))
                   + ": " + mysql_error(conn);
        mysql_close(conn);        // libération avant throw
        throw runtime_error(err);
    }

    return conn;
}