#include "ConnectionDB.h"
#include <sstream>
#include <stdexcept>
#include <string>

using namespace std;


// ---------------------------------------------------------------
// Constructeur par défaut
// ---------------------------------------------------------------
ConnectionDB::ConnectionDB()
    : url_(""),host_("localhost"), port_(3306),
      user_("banque"), pass_("banque"), base_("banque") {}

// ---------------------------------------------------------------
// Constructeur explicite
// ---------------------------------------------------------------
ConnectionDB::ConnectionDB(const string& url,const string& host,
                           uint16_t      port,
                           const string& user,
                           const string& password,
                           const string& dbname)
    :url_(url), host_(host), port_(port), user_(user), pass_(password), base_(dbname) {}

sql::Connection* ConnectionDB::getConnection() const {

    sql::Driver* driver = get_driver_instance();
    sql::Connection* conn = driver->connect(url_, user_, pass_);
    conn->setSchema(base_);
    return conn;
}