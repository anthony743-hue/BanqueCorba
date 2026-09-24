# Projet CORBA — Banque (Java ↔ C++ ↔ MySQL)

Projet pédagogique minimal pour appréhender le standard **CORBA** (OMG) :
deux langages, deux ORB différents, un seul contrat `IDL`, une base MySQL.

---

## 1. Architecture

```
                        ┌───────────────────────────┐
                        │   omniNames (port 2809)   │   Service de noms COS
                        │  "Banque" -> IOR C++      │
                        │  "Audit"  -> IOR Java     │
                        └─────────▲────────▲────────┘
                       résolution │        │ résolution
                                  │        │
   ┌──────────────────┐   IIOP    │        │   IIOP   ┌────────────────────┐
   │ Client Java      │───────────┴──► ◄───┴──────────│ Serveur Audit Java │
   │ (ClientBanque)   │   appelle banque::Banque      │ (banque::Audit)    │
   └──────────────────┘                               └────────▲───────────┘
                                │                              │
                                ▼                              │ IIOP
                     ┌────────────────────────┐                │
                     │  Serveur Banque C++    │────────────────┘
                     │  (omniORB + MySQL C)   │  appelle banque::Audit
                     └───────────┬────────────┘
                                 │ API C MySQL
                                 ▼
                          ┌─────────────┐
                          │ MySQL banque│
                          └─────────────┘
```

Les deux sens sont démontrés :

| Sens | Appelant | Appelé | Interface IDL |
|---|---|---|---|
| Java → C++ | `ClientBanque` | serveur C++ | `banque::Banque` |
| C++ → Java | serveur C++ | `ServeurAudit` | `banque::Audit` (`oneway`) |

L'interopérabilité fonctionne parce que les deux ORB parlent **GIOP/IIOP** et
partagent les mêmes **Repository IDs** générés depuis le même fichier `.idl`.

---

## 2. Arborescence

```
banque-corba/
├── idl/banque.idl                  contrat unique (OMG IDL)
├── sql/schema.sql                  base MySQL + jeu d'essai
├── cpp/
│   ├── BanqueServeur.cpp           servant + main (omniORB)
│   └── Makefile                    omniidl + g++
├── java/
│   ├── src/ClientBanque.java       client console
│   ├── src/ServeurAudit.java       serveur d'audit
│   └── Makefile                    idlj + javac
└── scripts/
    ├── 01-installer-dependances.sh
    └── 02-demarrer-naming.sh
```

---

## 3. Dépendances

| Composant | Rôle | Paquet Debian/Ubuntu |
|---|---|---|
| **omniORB 4** | ORB C++ | `libomniorb4-dev`, `omniorb` |
| **omniidl** | compilateur IDL → C++ | `omniidl`, `omniorb-idl` |
| **omniNames** | service de noms CORBA | `omniorb-nameserver` |
| **JDK 8** | ORB Java + `idlj` intégrés | `openjdk-8-jdk` |
| **MySQL** | base de données | `mysql-server` |
| **libmysqlclient** | API C MySQL utilisée par le C++ | `libmysqlclient-dev` |
| **g++ / make** | compilation | `build-essential` |

> **Important — version de Java.** L'ORB CORBA (`org.omg.*`) et l'outil `idlj`
> ont été **retirés du JDK à partir de la version 11** (JEP 320). Utilisez le
> **JDK 8**, ou bien **JacORB** (voir §8).

### Installation

```bash
bash scripts/01-installer-dependances.sh
```

Ou manuellement :

```bash
sudo apt update
sudo apt install -y build-essential make pkg-config
sudo apt install -y omniorb omniidl omniorb-idl omniorb-nameserver libomniorb4-dev
sudo apt install -y mysql-server libmysqlclient-dev
sudo apt install -y openjdk-8-jdk
```

Vérifier que Java 8 est bien actif :

```bash
javac -version                       # doit afficher 1.8.x
sudo update-alternatives --config javac
sudo update-alternatives --config java
```

---

## 4. Base de données

```bash
sudo systemctl start mysql
sudo mysql -u root -p < sql/schema.sql
```

Le serveur C++ se connecte par défaut avec `banque/banque` sur `localhost`.
Ces valeurs sont surchargeables par variables d'environnement :

```bash
export DB_HOST=localhost DB_USER=banque DB_PASS=banque DB_NAME=banque
```

---

## 5. Compilation

### 5.1 Côté C++ (souches + serveur)

```bash
cd cpp
make
```

Ce que fait le `Makefile`, étape par étape :

```bash
# 1. IDL -> C++   (produit banque.hh, banqueSK.cc, banqueDynSK.cc)
omniidl -bcxx -Wba ../idl/banque.idl

# 2. Compilation
g++ -Wall -O2 -I. $(mysql_config --include) -c BanqueServeur.cpp banqueSK.cc banqueDynSK.cc

# 3. Édition de liens
g++ BanqueServeur.o banqueSK.o banqueDynSK.o -o banque_serveur \
    -lomniORB4 -lomniDynamic4 -lCOS4 -lomnithread -lpthread $(mysql_config --libs)
```

| Fichier généré | Contenu |
|---|---|
| `banque.hh` | déclarations C++ des types et interfaces IDL |
| `banqueSK.cc` | **stub** (proxy client) + **skeleton** (`POA_banque::Banque`) |
| `banqueDynSK.cc` | support `Any` / `TypeCode` |

### 5.2 Côté Java (souches + client + serveur d'audit)

```bash
cd java
make
```

Détail des commandes :

```bash
# 1. IDL -> Java  (crée le paquet src/banque/)
idlj -fall -td src ../idl/banque.idl

# 2. Compilation
mkdir -p classes
javac -d classes src/banque/*.java src/*.java
```

Fichiers générés par `idlj` pour chaque interface :
`Banque.java` (interface), `BanqueHelper.java` (narrow/typecode),
`BanqueHolder.java` (paramètres `out`/`inout`), `_BanqueStub.java` (proxy),
`BanquePOA.java` (squelette serveur), plus `Compte*.java` et `ErreurBanque*.java`.

---

## 6. Exécution (4 terminaux, dans cet ordre)

```bash
# ---- Terminal 1 : service de noms ----------------------------------------
bash scripts/02-demarrer-naming.sh
# équivalent manuel :
# sudo mkdir -p /var/lib/omniNames
# omniNames -start 2809 -logdir /var/lib/omniNames

# ---- Terminal 2 : serveur d'audit Java -----------------------------------
# cd java
# java -cp classes ServeurAudit \
#      -ORBInitRef NameService=corbaloc::localhost:2809/NameService

# ---- Terminal 3 : serveur Banque C++ -------------------------------------
cd cpp
./banque_serveur \
     -ORBInitRef NameService=corbaloc::localhost:2809/NameService

# ---- Terminal 4 : client Java --------------------------------------------
cd java
java -cp classes ClientBanque \
     -ORBInitRef NameService=corbaloc::localhost:2809/NameService
```

Vérifier le contenu du service de noms à tout moment :

```bash
nameclt list
# ou
catior -i <IOR collé depuis la console>
```

### Scénario de test

1. Client Java → option **1** : la liste vient de MySQL via le serveur C++.
2. Option **2** : création d'un compte → l'identifiant `AUTO_INCREMENT` remonte
   du C++ vers Java, et une ligne `[AUDIT …]` apparaît dans le terminal 2.
3. Option **5** avec un montant trop grand → le C++ lève `banque::ErreurBanque`,
   attrapée en Java dans un `catch (ErreurBanque e)` : **une exception IDL
   traverse la frontière des langages**.
4. Option **6** : virement transactionnel (`START TRANSACTION` / `ROLLBACK`).

---

## 7. Les notions CORBA illustrées par ce projet

| Notion | Où la voir |
|---|---|
| **IDL** | `idl/banque.idl` — contrat neutre, seul point commun aux deux codes |
| **Mapping de langage** | `omniidl` vs `idlj` à partir du même fichier |
| **Stub / Skeleton** | `_BanqueStub.java` (client) / `POA_banque::Banque` (serveur) |
| **ORB** | `CORBA::ORB_init(...)` en C++, `ORB.init(...)` en Java |
| **POA** | `RootPOA`, `activate_object`, `the_POAManager()->activate()` |
| **Servant** | `BanqueImpl`, `AuditImpl` |
| **IOR** | `orb->object_to_string(ref)` affiché au démarrage |
| **Service de noms (COS Naming)** | `rebind("Banque")`, `resolve_str("Banque")` |
| **GIOP / IIOP** | protocole utilisé entre omniORB et l'ORB Java |
| **Narrow** | `BanqueHelper.narrow(...)`, `banque::Audit::_narrow(...)` |
| **Exception utilisateur** | `ErreurBanque` levée en C++, attrapée en Java |
| **`oneway`** | `Audit::journaliser` — appel asynchrone sans retour |
| **Exceptions système** | `TRANSIENT`, `COMM_FAILURE`, `OBJECT_NOT_EXIST` |

### Correspondance des types

| IDL | C++ (omniORB) | Java (idlj) |
|---|---|---|
| `long` | `CORBA::Long` | `int` |
| `double` | `CORBA::Double` | `double` |
| `string` | `char*` / `CORBA::String_var` | `String` |
| `struct Compte` | `banque::Compte` | `banque.Compte` |
| `sequence<Compte>` | `banque::ListeComptes` | `Compte[]` |
| `exception` | classe dérivée de `CORBA::UserException` | classe dérivée de `UserException` |
| `interface` | `banque::Banque_var` | `banque.Banque` |
| `oneway void` | `void` (retour immédiat) | `void` |

---

## 8. Variante JDK 11+ : JacORB

Si vous ne pouvez pas installer le JDK 8, remplacez l'ORB du JDK par **JacORB** ;
le code source Java reste identique, seules les commandes changent.

```bash
# Téléchargement (https://www.jacorb.org) puis :
export JACORB_HOME=/opt/jacorb
export CLASSPATH=$JACORB_HOME/lib/jacorb.jar:$JACORB_HOME/lib/slf4j-api.jar:classes

# 1. IDL -> Java  (remplace idlj)
$JACORB_HOME/bin/idl -d src ../idl/banque.idl

# 2. Compilation
javac -cp $CLASSPATH -d classes src/banque/*.java src/*.java

# 3. Exécution (il faut désigner explicitement l'ORB JacORB)
java -cp $CLASSPATH \
     -Dorg.omg.CORBA.ORBClass=org.jacorb.orb.ORB \
     -Dorg.omg.CORBA.ORBSingletonClass=org.jacorb.orb.ORBSingleton \
     ClientBanque -ORBInitRef NameService=corbaloc::localhost:2809/NameService
```

---

## 9. Dépannage

| Symptôme | Cause / solution |
|---|---|
| `CORBA::TRANSIENT` au démarrage du serveur | omniNames n'est pas lancé, ou mauvais port dans `-ORBInitRef` |
| `omniNames: cannot start` au 2ᵉ lancement | ne pas remettre `-start` : `omniNames -logdir /var/lib/omniNames`, ou `rm /var/lib/omniNames/*.log` |
| `NotFound` côté client | le serveur C++ n'est pas démarré, ou les deux processus visent des services de noms différents |
| `Connexion MySQL impossible` | service arrêté (`sudo systemctl start mysql`) ou utilisateur `banque` absent → rejouer `sql/schema.sql` |
| `mysql_config: command not found` | installer `libmysqlclient-dev` |
| `cannot find -lCOS4` | installer `libomniorb4-dev` (fournit les stubs COS) |
| `idlj: command not found` | JDK 11+ actif → basculer sur le JDK 8 ou passer à JacORB (§8) |
| `MARSHAL` / `BAD_PARAM` à l'appel | IDL modifié d'un seul côté : régénérer **les deux** (`make clean && make`) |
| `omniNames` occupé (port 2809) | `sudo lsof -i :2809` puis tuer le processus, ou changer de port partout |
| `[avertissement] service Audit indisponible` | normal si le serveur Java d'audit n'est pas lancé ; la banque fonctionne quand même |

---

## 10. Ordre de lecture conseillé

1. `idl/banque.idl` — le contrat.
2. `cpp/BanqueServeur.cpp`, à partir du `main` — ORB → POA → servant → naming.
3. `java/src/ClientBanque.java` — ORB → naming → narrow → appels distants.
4. `java/src/ServeurAudit.java` — même schéma que le `main` C++, en Java.
5. Les fichiers générés (`banque.hh`, `src/banque/_BanqueStub.java`) pour voir
   concrètement ce que le compilateur IDL a produit.
