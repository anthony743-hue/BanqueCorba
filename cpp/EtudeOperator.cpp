// EtudeOperator.cpp
#include <iostream>
#include <string>
#include <stdexcept>
#include <cstdlib>
#include <memory>
#include <vector>

#include <mysql_driver.h>
#include <mysql_connection.h>
#include <cppconn/exception.h>
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>
#include <cppconn/resultset.h>

#include <omniORB4/CORBA.h>
#include <omniORB4/Naming.hh>

#include "ecole.hh"
#include "ConnectionDB.h"

using namespace std;

class EtudeOperatorPOA : public POA_ecole::EtudeOperator
{
public:
    EtudeOperatorPOA(ConnectionDB &db, CORBA::ORB_ptr orb)
        : conn_(db),
          orb_(CORBA::ORB::_duplicate(orb)) {}

    void insertEtudiant(const ecole::ListeEtudiant &liste) override;
    void insertEtudiant(sql::Connection *conn,
                        const ecole::ListeEtudiant &liste);

    ecole::ListeEtudiant *getAll(CORBA::Long page,
                                 CORBA::Long par_page) override;

private:
    ConnectionDB &conn_;
    CORBA::ORB_var orb_;
};

void EtudeOperatorPOA::insertEtudiant(const ecole::ListeEtudiant &liste)
{
    sql::Connection *conn = nullptr;

    try
    {
        conn = conn_.getConnection();
        insertEtudiant(conn, liste);
        delete conn;
    }
    catch (const CORBA::Exception &)
    {
        delete conn;
        throw;
    }
    catch (const exception &e)
    {
        delete conn;
        ecole::ErreurBD ex;
        ex.message = CORBA::string_dup(e.what());
        throw ex;
    }
    catch (...)
    {
        delete conn;
        ecole::ErreurBD ex;
        ex.message = CORBA::string_dup("Erreur inconnue");
        throw ex;
    }
}

void EtudeOperatorPOA::insertEtudiant(sql::Connection *conn,
                                      const ecole::ListeEtudiant &liste)
{
    unique_ptr<sql::PreparedStatement> stmt(
        conn->prepareStatement("INSERT INTO etudiant (id, name) VALUES (?, ?)"));

    for (CORBA::ULong i = 0; i < liste.length(); ++i)
    {
        stmt->setInt(1, static_cast<int>(liste[i].id));
        stmt->setString(2, liste[i].name.in());
        stmt->executeUpdate();
    }
}

ecole::ListeEtudiant *EtudeOperatorPOA::getAll(CORBA::Long page,
                                               CORBA::Long par_page)
{
    if (page < 1)
        page = 1;
    if (par_page < 1)
        par_page = 10;
    if (par_page > 1000)
        par_page = 1000;

    CORBA::Long offset = (page - 1) * par_page;

    sql::Connection *conn = nullptr;

    try
    {
        conn = conn_.getConnection();

        std::unique_ptr<sql::PreparedStatement> stmt(
            conn->prepareStatement(
                "SELECT id, name FROM etudiant ORDER BY id LIMIT ? OFFSET ?"));

        stmt->setInt(1, static_cast<int>(par_page));
        stmt->setInt(2, static_cast<int>(offset));

        std::unique_ptr<sql::ResultSet> res(stmt->executeQuery());

        std::vector<std::pair<CORBA::Long, std::string>> rows;
        while (res->next())
        {
            CORBA::Long id = static_cast<CORBA::Long>(res->getInt("id"));
            std::string nom = res->getString("name").c_str();
            rows.emplace_back(id, std::move(nom));
        }

        ecole::ListeEtudiant *liste =
            new ecole::ListeEtudiant(static_cast<CORBA::ULong>(rows.size()));

        for (CORBA::ULong i = 0; i < rows.size(); ++i)
        {
            (*liste)[i].id = rows[i].first;
            (*liste)[i].name = CORBA::string_dup(rows[i].second.c_str());
        }

        delete conn;
        return liste;
    }
    catch (const CORBA::Exception &)
    {
        delete conn;
        throw;
    }
    catch (const exception &e)
    {
        delete conn;
        ecole::ErreurBD ex;
        ex.message = CORBA::string_dup(e.what());
        throw ex;
    }
    catch (...)
    {
        delete conn;
        ecole::ErreurBD ex;
        ex.message = CORBA::string_dup("Erreur inconnue");
        throw ex;
    }
}

int main(int argc, char *argv[])
{
    try
    {
        CORBA::ORB_var orb = CORBA::ORB_init(argc, argv);

        CORBA::Object_var poaObj =
            orb->resolve_initial_references("RootPOA");
        PortableServer::POA_var poa =
            PortableServer::POA::_narrow(poaObj);
        poa->the_POAManager()->activate();

        ConnectionDB db("tcp://localhost:3306", "", 3306,
                        "banque", "banque", "banque");

        EtudeOperatorPOA *etudeOper = new EtudeOperatorPOA(db, orb);

        PortableServer::ObjectId_var oid = poa->activate_object(etudeOper);
        CORBA::Object_var ref = poa->id_to_reference(oid);

        CORBA::Object_var nsObj =
            orb->resolve_initial_references("NameService");
        CosNaming::NamingContext_var nc =
            CosNaming::NamingContext::_narrow(nsObj);

        CosNaming::Name name;
        name.length(1);
        name[0].id = CORBA::string_dup("EtudeOperator");
        // name[0].kind = CORBA::string_dup("");
        nc->rebind(name, ref);

        cout << "EtudeOperator est enregistré et prêt.\n";

        orb->run();

        poa->destroy(true, true);
        orb->destroy();
    }
    catch (const CORBA::Exception &e)
    {
        cerr << "CORBA exception : " << e._name() << "\n";
        return 1;
    }
    catch (const exception &e)
    {
        cerr << "Exception : " << e.what() << "\n";
        return 1;
    }
    return 0;
}