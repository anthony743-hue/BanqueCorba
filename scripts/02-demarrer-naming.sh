#!/usr/bin/env bash
# ---------------------------------------------------------------------------
# Démarrage du service de noms CORBA (omniNames) sur le port 2809.
# Ce service est PARTAGÉ par le serveur C++, le serveur Java et le client Java.
# Usage : bash scripts/02-demarrer-naming.sh          (1er lancement)
#         bash scripts/02-demarrer-naming.sh --reset  (repart de zéro)
# ---------------------------------------------------------------------------
set -e

LOGDIR=/var/lib/omniNames
PORT=2809

sudo mkdir -p "$LOGDIR"

if [ "$1" = "--reset" ]; then
    echo "Suppression des journaux omniNames..."
    sudo rm -f "$LOGDIR"/*.log
fi

if ls "$LOGDIR"/*.log >/dev/null 2>&1; then
    # Redémarrage : ne PAS remettre -start, sinon omniNames refuse de démarrer
    echo "Redemarrage d'omniNames (journal existant)..."
    sudo omniNames -logdir "$LOGDIR" -errlog "$LOGDIR/omniNames.err" &
else
    echo "Premier demarrage d'omniNames sur le port $PORT..."
    sudo omniNames -start $PORT -logdir "$LOGDIR" -errlog "$LOGDIR/omniNames.err" &
fi

sleep 2
echo "omniNames ecoute sur corbaloc::localhost:$PORT/NameService"
