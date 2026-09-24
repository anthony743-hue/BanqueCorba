# ---------------------------------------------------------------------------
# Makefile du serveur CORBA C++ (omniORB 4 + MySQL)
#   make         : génère les souches IDL puis compile
#   make clean   : supprime tout ce qui est généré
# ---------------------------------------------------------------------------

IDL_FILE = ../idl/banque.idl
CIBLE    = banque_serveur

CXX      = g++
CXXFLAGS = -Wall -O2 -I. $(shell mysql_config --include)

# Bibliothèques omniORB : ORB + support Any/TypeCode + stubs CosNaming + threads
LDLIBS   = -lomniORB4 -lomniDynamic4 -lCOS4 -lomnithread -lpthread \
           $(shell mysql_config --libs)

SOURCES_GEN = banqueSK.cc banqueDynSK.cc
OBJETS      = BanqueServeur.o banqueSK.o banqueDynSK.o

all: $(CIBLE)

# --- 1. Compilation IDL -> C++ ---------------------------------------------
# -bcxx : back-end C++     -Wba : génère aussi le code Any/TypeCode (DynSK)
banque.hh $(SOURCES_GEN): $(IDL_FILE)
	omniidl -bcxx -Wba $(IDL_FILE)

# --- 2. Compilation --------------------------------------------------------
%.o: %.cc banque.hh
	$(CXX) $(CXXFLAGS) -c $< -o $@

BanqueServeur.o: BanqueServeur.cpp banque.hh
	$(CXX) $(CXXFLAGS) -c BanqueServeur.cpp -o BanqueServeur.o

$(CIBLE): $(OBJETS)
	$(CXX) $(OBJETS) -o $(CIBLE) $(LDLIBS)

clean:
	rm -f $(CIBLE) *.o banque.hh banqueSK.cc banqueDynSK.cc

.PHONY: all clean
