// ---------------------------------------------------------------------------
// ClientBanque.java
// Client CORBA écrit en JAVA qui pilote le serveur C++ (BanqueOperator).
// ---------------------------------------------------------------------------

import banque.BanqueOperator;
import banque.BanqueOperatorHelper;
import banque.Compte;
import banque.SoldeInsuffisant;
import banque.CompteIntrouvable;

import org.omg.CORBA.ORB;
import org.omg.CosNaming.NamingContextExt;
import org.omg.CosNaming.NamingContextExtHelper;

import java.util.Scanner;

public class ClientBanque {

    private static final Scanner sc = new Scanner(System.in);

    public static void main(String[] args) {
        try {
            // 1. Initialisation de l'ORB côté client
            ORB orb = ORB.init(args, null);

            // 2. Résolution du service de noms (omniNames, partagé avec le C++)
            NamingContextExt nc = NamingContextExtHelper.narrow(
                    orb.resolve_initial_references("NameService"));

            // 3. Récupération de la référence distante puis "narrow" vers le type IDL
            BanqueOperator banque = BanqueOperatorHelper.narrow(
                    nc.resolve_str("Banque"));

            if (banque == null) {
                System.err.println("Objet 'Banque' introuvable dans le service de noms.");
                return;
            }
            System.out.println("Connecte au serveur Banque (C++) via CORBA/IIOP.\n");

            boucleMenu(banque);

        } catch (Exception e) {
            System.err.println("Erreur : " + e);
            e.printStackTrace();
        }
    }

    private static void boucleMenu(BanqueOperator banque) {
        while (true) {
            System.out.println("========= BANQUE CORBA =========");
            System.out.println("1. Lister les comptes");
            System.out.println("2. Consulter un solde");
            System.out.println("3. Deposer");
            System.out.println("4. Retirer");
            System.out.println("5. Verifier l'existence d'un compte");
            System.out.println("0. Quitter");
            System.out.print("Choix : ");

            String choix = sc.nextLine().trim();
            try {
                switch (choix) {
                    case "1": lister(banque);        break;
                    case "2": solde(banque);         break;
                    case "3": deposer(banque);       break;
                    case "4": retirer(banque);       break;
                    case "5": verifierExiste(banque); break;
                    case "0": System.out.println("Au revoir."); return;
                    default : System.out.println("Choix invalide.");
                }
            } catch (SoldeInsuffisant e) {
                System.out.printf(">> Solde insuffisant : solde=%.2f, demande=%.2f%n",
                        e.solde, e.montant);
            } catch (CompteIntrouvable e) {
                System.out.println(">> Compte introuvable : " + e.titulaire);
            } catch (org.omg.CORBA.COMM_FAILURE e) {
                System.out.println(">> Serveur injoignable (COMM_FAILURE).");
            } catch (NumberFormatException e) {
                System.out.println(">> Saisie numerique invalide.");
            }
            System.out.println();
        }
    }

    // ----------------------------- opérations ------------------------------

    // -----------------------------------------------------------------
    // 1. Lister les comptes
    // -----------------------------------------------------------------
    private static void lister(BanqueOperator banque) {
        Compte[] comptes = banque.listerComptes();

        if (comptes == null || comptes.length == 0) {
            System.out.println("Aucun compte enregistre.");
            return;
        }

        System.out.println();
        System.out.printf("%-6s | %-25s | %12s%n", "ID", "TITULAIRE", "SOLDE");
        System.out.println("-------+---------------------------+-------------");
        for (Compte c : comptes) {
            System.out.printf("%-6d | %-25s | %12.2f%n",
                    c.id, c.titulaire, c.solde);
        }
        System.out.println("Total : " + comptes.length + " compte(s).");
    }

    // -----------------------------------------------------------------
    // 2. Consulter un solde
    // -----------------------------------------------------------------
    private static void solde(BanqueOperator banque)
            throws CompteIntrouvable {
        System.out.print("ID du compte : ");
        long id = Long.parseLong(sc.nextLine().trim());

        double solde = banque.getSolde(id);
        System.out.printf(">> Solde du compte %d : %.2f%n", id, solde);
    }

    // -----------------------------------------------------------------
    // 3. Déposer (effectuerOperation avec montant positif)
    // -----------------------------------------------------------------
    private static void deposer(BanqueOperator banque)
            throws SoldeInsuffisant, CompteIntrouvable {
        System.out.print("ID du compte        : ");
        long id = Long.parseLong(sc.nextLine().trim());

        System.out.print("Montant a deposer   : ");
        double montant = Double.parseDouble(sc.nextLine().trim());

        if (montant <= 0) {
            System.out.println(">> Le montant doit etre strictement positif.");
            return;
        }

        banque.effectuerOperation(montant, id);

        double nouveauSolde = banque.getSolde(id);
        System.out.printf(">> Depot effectue. Nouveau solde : %.2f%n", nouveauSolde);
    }

    // -----------------------------------------------------------------
    // 4. Retirer (effectuerOperation avec montant négatif)
    // -----------------------------------------------------------------
    private static void retirer(BanqueOperator banque)
            throws SoldeInsuffisant, CompteIntrouvable {
        System.out.print("ID du compte        : ");
        long id = Long.parseLong(sc.nextLine().trim());

        System.out.print("Montant a retirer   : ");
        double montant = Double.parseDouble(sc.nextLine().trim());

        if (montant <= 0) {
            System.out.println(">> Le montant doit etre strictement positif.");
            return;
        }

        // Convention : montant négatif = retrait côté serveur
        banque.effectuerOperation(-montant, id);

        double nouveauSolde = banque.getSolde(id);
        System.out.printf(">> Retrait effectue. Nouveau solde : %.2f%n", nouveauSolde);
    }

    // -----------------------------------------------------------------
    // 5. Vérifier l'existence d'un compte
    // isCompteExist retourne void : succès = compte existe,
    // exception CompteIntrouvable = compte absent.
    // -----------------------------------------------------------------
    private static void verifierExiste(BanqueOperator banque)
            throws CompteIntrouvable {
        System.out.print("ID du compte a verifier : ");
        long id = Long.parseLong(sc.nextLine().trim());

        banque.isCompteExist(id);
        System.out.println(">> Le compte " + id + " existe.");
    }
}