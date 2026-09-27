// ---------------------------------------------------------------------------
// ClientBanque.java
// Client CORBA écrit en JAVA qui pilote le serveur C++ (EtudeOperator).
// ---------------------------------------------------------------------------

import ecole.Etudiant;
import ecole.EtudeOperator;
import ecole.EtudeOperatorHelper;
import ecole.ErreurBD;

import org.omg.CORBA.ORB;
import org.omg.CosNaming.NamingContextExt;
import org.omg.CosNaming.NamingContextExtHelper;
import org.omg.CORBA.SystemException;
import org.omg.CORBA.UserException;

import java.util.ArrayList;
import java.util.Properties;
import java.util.Scanner;

public class EcoleClient {

    public static void main(String[] args) {
        try {
            // Properties props = new Properties();
            // props.put("org.omg.CORBA.ORBInitialPort", "2809");
            // props.put("org.omg.CORBA.ORBInitialHost", "localhost");
            // // 1. Initialisation de l'ORB côté client
            ORB orb = ORB.init(args, null);

            // 2. Résolution du service de noms (omniNames, partagé avec le C++)
            NamingContextExt nc = NamingContextExtHelper.narrow(
                    orb.resolve_initial_references("NameService"));

            // 3. Récupération de la référence distante puis "narrow" vers le type IDL
            EtudeOperator etude = EtudeOperatorHelper.narrow(
                    nc.resolve_str("EtudeOperator"));

            // // 4. Lecture du CSV -> ArrayList<Object>
            // String pathFile = "../../data/test.txt";
            // ArrayList<Object> dataList = new ArrayList<>();
            // FileHandler.readFile(pathFile, dataList, new Etudiant());

            // // 5. Conversion ArrayList<Object> -> Etudiant[] (sequence IDL)
            // Etudiant[] liste = new Etudiant[dataList.size()];
            // for (int i = 0; i < dataList.size(); i++) {
            // liste[i] = (Etudiant) dataList.get(i);
            // }langage

            // // 6. Appel distant : insertEtudiant(in ListeEtudiant)
            // etude.insertEtudiant(liste);
            Etudiant[] liste = etude.getAll(1, 3);
            System.out.println("Page 1 : " + liste.length + " etudiant(s) recu(s)");
            for (Etudiant e : liste) {
                System.out.println("  id=" + e.id + "  nom=" + e.name);
            }
        } catch (ecole.ErreurBD ex) {
            System.err.println("Erreur BD : " + ex.message); // "port invalide" lisible
        } catch (UserException | SystemException e) {
            System.err.println("CORBA exception : " + e);
            e.printStackTrace();
        } catch (Exception e) {
            System.err.println("Erreur : " + e);
            e.printStackTrace();
        }
    }
}