# ---------------------------------------------------------------------------
# Makefile Java (JDK 8 : idlj et le paquet org.omg.* sont fournis d'origine)
#   make        : génère les souches IDL puis compile
#   make run    : exécute le client
#   make clean  : nettoie
# ---------------------------------------------------------------------------

IDL_FILE = ../idl/ecole.idl
SRC      = src
CLASSES  = classes
MAIN     = EcoleClient

SOURCES  = $(shell find $(SRC) -name "*.java")

all: compile

# --- 1. Génération des stubs IDL -> Java -----------------------------------
# -fall : souches client + squelettes serveur   -td : répertoire de sortie
$(SRC)/ecole/EtudeOperator.java: $(IDL_FILE)
	idlj -fall -td $(SRC) $(IDL_FILE)

stubs: $(SRC)/ecole/EtudeOperator.java

# --- 2. Compilation Java ---------------------------------------------------
compile: stubs
	mkdir -p $(CLASSES)
	javac -d $(CLASSES) $(SOURCES)

# --- 3. Exécution du client ------------------------------------------------
run: compile
	java -cp $(CLASSES) \
	     -Dorg.omg.CORBA.ORBInitialHost=localhost \
	     -Dorg.omg.CORBA.ORBInitialPort=2809 \
	     $(MAIN)

# --- 4. Nettoyage ----------------------------------------------------------
clean:
	rm -rf $(CLASSES) $(SRC)/ecole

.PHONY: all stubs compile run clean