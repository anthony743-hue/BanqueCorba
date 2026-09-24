#include <iostream>
#include <sstream>
#include <string>
#include <stdexcept>
#include <cstdlib>

#include <mysql/mysql.h>
#include "banque.hh"
#include "ConnectionDB.h"
#include <omniORB4/Naming.hh>

using namespace std;

class BanqueOperatorPOA : public POA_banque::BanqueOperator
{
public:
    BanqueOperatorPOA(ConnectionDB& db, CORBA::ORB_ptr orb)
        : conn_(db),
          orb_(CORBA::ORB::_duplicate(orb)) {}

    CORBA::Double getSolde(CORBA::Long id) override;
    CORBA::Double getSolde(MYSQL& conn, CORBA::Long id);
    void effectuerOperation(CORBA::Double montant, CORBA::Long id) override;
    void effectuerOperation(MYSQL& conn, CORBA::Double montant, CORBA::Long id);
    banque::ListeComptes* listerComptes();

private:
    ConnectionDB& conn_;
    CORBA::ORB_var orb_;

    void exec(MYSQL& conn, const string& sql);
    string echapper(MYSQL& conn, const string& s);
};

void BanqueOperatorPOA::exec(MYSQL& conn, const string& sql)
{
    if (mysql_query(&conn, sql.c_str()) != 0) {
        throw runtime_error(
            string("MySQL error ") + to_string(mysql_errno(&conn))
            + ": " + mysql_error(&conn));
    }
}

string BanqueOperatorPOA::echapper(MYSQL& conn, const string& s)
{
    if (s.empty()) return "";

    string buffer(s.size() * 2, '\0');
    unsigned long n = mysql_real_escape_string(
        &conn, buffer.data(), s.c_str(), s.size());
    buffer.resize(n);
    return buffer;
}

// CORBA::Double BanqueOperatorPOA::getSolde(CORBA::Long id)
// {
//     MYSQL* connection = conn_.open();
//     try {
//         CORBA::Double solde = getSolde(*connection, id);
//         mysql_close(connection);
//         return solde;
//     }
//     catch (...) {
//         mysql_close(connection);
//         throw;
//     }
// }

// CORBA::Double BanqueOperatorPOA::getSolde(MYSQL& connection, CORBA::Long id)
// {
//     ostringstream sql;
//     sql << "SELECT solde FROM comptes WHERE id = " << id;

//     exec(connection, sql.str());

//     MYSQL_RES* res = mysql_store_result(&connection);
//     if (!res) {
//         throw runtime_error(
//             string("MySQL store_result: ") + mysql_error(&connection));
//     }

//     MYSQL_ROW row = mysql_fetch_row(res);
//     if (!row) {
//         mysql_free_result(res);
//         banque::CompteIntrouvable ex;
//         ex.id = id;
//         throw ex;
//     }

//     CORBA::Double solde = atof(row[0]);
//     mysql_free_result(res);
//     return solde;
// }

// void BanqueOperatorPOA::effectuerOperation(CORBA::Double montant,
//                                            CORBA::Long id)
// {
//     MYSQL* conn = conn_.open();
//     try {
//         effectuerOperation(*conn, montant, id);
//         mysql_close(conn);
//     }
//     catch (...) {
//         mysql_close(conn);
//         throw;
//     }
// }

// void BanqueOperatorPOA::effectuerOperation(MYSQL& connection,
//                                            CORBA::Double montant,
//                                            CORBA::Long id)
// {
//     CORBA::Double soldeActuel = getSolde(connection, id);

//     if (soldeActuel + montant < 0.0) {
//         banque::SoldeInsuffisant ex;
//         ex.solde   = soldeActuel;
//         ex.montant = montant;
//         throw ex;
//     }

//     ostringstream sql;
//     sql << "UPDATE comptes SET solde = solde + " << montant
//         << " WHERE id = " << id;

//     exec(connection, sql.str());
// }

// banque::ListeComptes* BanqueOperatorPOA::listerComptes()
// {
//     MYSQL* connection = conn_.open();
//     MYSQL_RES* res = nullptr;

//     try {
//         exec(*connection, "SELECT id, titulaire, solde FROM comptes ORDER BY id");

//         res = mysql_store_result(connection);
//         if (!res) {
//             throw runtime_error(
//                 string("MySQL store_result: ") + mysql_error(connection));
//         }

//         my_ulonglong n = mysql_num_rows(res);

//         banque::ListeComptes* liste = new banque::ListeComptes();
//         liste->length(static_cast<CORBA::ULong>(n));

//         CORBA::ULong i = 0;
//         MYSQL_ROW ligne;
//         while ((ligne = mysql_fetch_row(res)) != nullptr) {
//             (*liste)[i].id        = static_cast<CORBA::Long>(atol(ligne[0]));
//             (*liste)[i].titulaire = CORBA::string_dup(ligne[1] ? ligne[1] : "");
//             (*liste)[i].solde     = atof(ligne[2] ? ligne[2] : "0");
//             ++i;
//         }

//         mysql_free_result(res);
//         mysql_close(connection);
//         return liste;
//     }
//     catch (...) {
//         if (res) mysql_free_result(res);
//         mysql_close(connection);
//         throw;
//     }
// }

int main(int argc, char* argv[])
{
    try {
        CORBA::ORB_var orb = CORBA::ORB_init(argc, argv);

        CORBA::Object_var poaObj =
            orb->resolve_initial_references("RootPOA");
        PortableServer::POA_var poa =
            PortableServer::POA::_narrow(poaObj);
        poa->the_POAManager()->activate();

        ConnectionDB db;
        cout << "[Banque]\n" << db.toString() << endl;

        BanqueOperatorPOA* banqueOper = new BanqueOperatorPOA(db, orb);

        PortableServer::ObjectId_var oid = poa->activate_object(banqueOper);
        CORBA::Object_var ref = poa->id_to_reference(oid);

        CORBA::Object_var objRef =
            orb->resolve_initial_references("NameService");
        CosNaming::NamingContext_var nc =
            CosNaming::NamingContext::_narrow(objRef);

        CosNaming::Name name;
        name.length(1);
        name[0].id   = CORBA::string_dup("Banque");
        name[0].kind = CORBA::string_dup("");

        try {
            nc->bind(name, ref);
            cout << "[Banque] Enregistre sous 'Banque'" << endl;
        }
        catch (const CosNaming::NamingContext::AlreadyBound&) {
            nc->rebind(name, ref);
            cout << "[Banque] Reenregistre sous 'Banque'" << endl;
        }

        cout << "[Banque] Serveur pret." << endl;
        orb->run();

        // poa->destroy(TRUE, TRUE);
        // delete banqueOper;
        // orb->destroy();
    }
    catch (const CosNaming::NamingContext::NotFound&) {
        cerr << "Service de noms joignable mais nom absent.\n";
        return 1;
    }
    catch (const CORBA::TRANSIENT&) {
        cerr << "omniNames est-il demarre ? (CORBA::TRANSIENT)\n";
        return 1;
    }
    catch (const CORBA::Exception& e) {
        cerr << "Exception CORBA : " << e._name() << "\n";
        return 1;
    }
    catch (const exception& e) {
        cerr << "Exception : " << e.what() << "\n";
        return 1;
    }
    return 0;
}