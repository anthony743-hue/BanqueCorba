# ---------------------------------------------------------------------------
# Makefile Java (JDK 8 : idlj et le paquet org.omg.* sont fournis d'origine)
#   make        : génère les souches IDL puis compile
#   make clean  : nettoie
# ---------------------------------------------------------------------------

IDL_FILE = ../idl/banque.idl
SRC      = src
CLASSES  = classes

all: compile

# --- 1. Compilation IDL -> Java --------------------------------------------
# -fall : souches client + squelettes serveur   -td : répertoire de sortie
stubs: $(IDL_FILE)
	idlj -fall -td $(SRC) $(IDL_FILE)

# --- 2. Compilation Java ---------------------------------------------------
compile: stubs
	mkdir -p $(CLASSES)
	javac -d $(CLASSES) $(SRC)/banque/*.java $(SRC)/*.java

clean:
	rm -rf $(CLASSES) $(SRC)/banque

.PHONY: all stubs compile clean
