# ---------------------------------------------------------------------------
# Makefile du serveur CORBA C++ (omniORB 4 + MySQL Connector/C++)
# ---------------------------------------------------------------------------

IDL_FILE = ../idl/ecole.idl
CIBLE    = ecole_serveur

CXX      = g++
CXXFLAGS = -Wall -O2 -std=c++17 -I. $(shell mysql_config --include)

LDLIBS   = -lomniORB4 -lomniDynamic4 -lCOS4 -lomnithread -lpthread \
           -lmysqlcppconn $(shell mysql_config --libs)

# --- Fichiers générés -------------------------------------------------------
GEN_HH  = ecole.hh
GEN_CC  = ecoleSK.cc ecoleDynSK.cc
GEN_OBJ = ecoleSK.o  ecoleDynSK.o

# --- Fichiers écrits à la main ---------------------------------------------
SRC_HAND = EtudeOperator.cpp ConnectionDB.cpp
OBJ_HAND = EtudeOperator.o   ConnectionDB.o

OBJETS = $(OBJ_HAND) $(GEN_OBJ)

# ---------------------------------------------------------------------------
all: $(CIBLE)

# 1. Génération IDL
$(GEN_HH) $(GEN_CC): $(IDL_FILE)
	omniidl -bcxx -Wba $(IDL_FILE)

# 2. Compilation générique : tout .cpp -> .o
%.o: %.cpp $(GEN_HH) ConnectionDB.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

# 3. Compilation des fichiers générés
ecoleSK.o: ecoleSK.cc $(GEN_HH)
	$(CXX) $(CXXFLAGS) -c $< -o $@

ecoleDynSK.o: ecoleDynSK.cc $(GEN_HH)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# 4. Édition de liens
$(CIBLE): $(OBJETS)
	$(CXX) $(OBJETS) -o $(CIBLE) $(LDLIBS)

# 5. Nettoyage
clean:
	rm -f $(CIBLE) *.o $(GEN_HH) $(GEN_CC)

.PHONY: all clean