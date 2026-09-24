#!/usr/bin/env bash
# ---------------------------------------------------------------------------
# Installation de toutes les dépendances (Debian / Ubuntu)
# Usage : bash scripts/01-installer-dependances.sh
# ---------------------------------------------------------------------------
set -e


# --- Outils de compilation C++ ---------------------------------------------
sudo apt install -y build-essential make pkg-config

# --- omniORB 4 : l'ORB C++, son compilateur IDL et son service de noms ------
#   omniorb              : outils (catior, nameclt...)
#   omniidl              : compilateur IDL
#   omniorb-idl          : fichiers .idl des services COS
#   omniorb-nameserver   : omniNames (service de noms)
#   libomniorb4-dev      : en-têtes + bibliothèques
sudo apt install -y omniorb omniidl omniorb-idl omniorb-nameserver libomniorb4-dev

# --- MySQL : serveur + bibliothèque cliente C ------------------------------
sudo apt install -y mysql-server libmysqlclient-dev
# Variante MariaDB :
# sudo apt install -y mariadb-server libmariadb-dev libmariadb-dev-compat

# --- Java 8 (idlj et le paquet CORBA org.omg.* sont retirés depuis JDK 11) --
sudo apt install -y openjdk-8-jdk

echo
echo "=== Verification ==="
omniidl -V || true
mysql_config --version
javac -version
echo
echo "Si 'javac -version' n'affiche pas 1.8, basculez avec :"
echo "  sudo update-alternatives --config javac"
echo "  sudo update-alternatives --config java"
