# Variables
## Pour make
.PHONY: clean, mrproper
## Inclusions
INC_GTK = $(shell pkg-config gtk+-2.0 --cflags --libs)
INC_SDL = $(shell sdl2-config --cflags --libs) -lSDL2_ttf -lSDL2_gfxPrimitives
## Compilation
CXX = gcc
CXXFLAGS = $(INC_SDL) $(INC_GTK)
CXXFLAGSDEBUG = $(CXXFLAGS) -DDEBUG
## Chemins
EXEPATH = ./
OBJPATH = ./
SRCPATH = ./
## Dépendances
DEP_MAIN = IOoptions IOmain main interfaceMain callback jeu JeuSDL IOcredits


main: $(SRCPATH)main.o

$(OBJPATH)%.o: $(SRCPATH)%.c $(SRCPATH)%.h
	$(CXX) -c $< -o $@ $(CXXFLAGS)

clean:
	rm -rf $(OBJPATH)*.o
