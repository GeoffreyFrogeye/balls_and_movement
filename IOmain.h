/*
Projet-ISN

Fichier: IOmain.h

Contenu: Prototypes des fonctions contenues dans IOmain.c, et structures.

Actions: Permet � l'ordinateur de conna�tre toutes les fonctions pr�sentes dans le programme ainsi que leurs arguments.

Jean-Loup BEAUSSART & Dylan GUERVILLE
*/

#ifndef IOMAIN_H_INCLUDED
#define IOMAIN_H_INCLUDED	//Protection contre les inclusions infinies

/* Structures */
typedef struct Souris       //Structure pour g�rer les �v�nements de la souris
{
	char touches[3];
	SDL_Rect position;
	int scroll;
} Souris;

typedef struct ClavierSouris        //Structure pour g�rer les �v�nements du clavier
{
	int fermeture;
	char clavier[200];
	Souris souris;
} ClavierSouris;

typedef struct Texte        //Structure pour afficher du texte avec la SDL
{
	SDL_Texture *pTextures[100];
	SDL_Surface *surface;
	SDL_Rect positions[100];
	char chaines[100][200];
} Texte;

typedef struct Sprite           //Structure pour les images � afficher dans le jeu
{
	SDL_Texture *pTextures[20];
	SDL_Rect position[20];
} Sprite;

typedef struct TileProp		//Structure pour les propri�t�s des tiles du d�cor
{
	SDL_Rect src;
} TileProp;

typedef struct Map		//Structure pour la map d'un niveau
{
	int LARGEUR_TILE, HAUTEUR_TILE;
	int nbtilesX,nbtilesY;
	SDL_Texture *tileset, *fond;
	TileProp *props;
	int nbtiles_largeur_monde, nbtiles_hauteur_monde;
	int **plan;
	int **planObjets;
} Map;

typedef struct Sons		//Structure pour les sons du jeu
{
	FMOD_SOUND *music[10];
	FMOD_SOUND *bruits[20];
} Sons;

typedef struct Animation		//Structure pour les images qui forment une animation
{
	SDL_Texture *img[200];
	SDL_Rect pos;
} Animation;

typedef struct Collision		//Structure pour g�rer les collisions
{
	unsigned int etatColl;
	unsigned char numMissile;
} Collision;

typedef struct Joueur	//Structure avec les informations relatives au joueur
{
	char pseudo[255];
	char mdp[100];
	int score_max;
	int niveau_max;
	int connexion;
	int mode;
	char autre[9000];
} Joueur;


typedef struct InfoDeJeu	//Structure avec les infos de la partie
{
	char vies;
	char viesInitiales;
	long score;
	int niveau;
	int compteurTemps;
	unsigned int bonus;
} InfoDeJeu;


/* Prototypes des fonctions */
int Initialisation(SDL_Renderer **ppMoteurRendu, FILE *pFichierErreur, SDL_Window **ppFenetre, Options *pOptions);
int Chargements (Sprite images[], SDL_Renderer *pMoteurRendu, TTF_Font *polices[], Animation anim[]);
int GestionEvenements(ClavierSouris *entrees);
int ChargementTextures(SDL_Renderer *pMoteurRendu, Sprite images[]);
int DestructionSurfaces(SDL_Surface *sImages[]);
int ChargementPolices(TTF_Font *polices[]);
int ChargementMusic (Sons *pSons, FMOD_SYSTEM *pMoteurSon);
int InitialisationSon(FMOD_SYSTEM **ppMoteurSon, FILE *pFichierErreur, Sons *pSons);
Map* ChargementNiveau(SDL_Renderer *pMoteurRendu, Joueur *pJoueur, int level);
int DestructionMap(Map *pMap);
void EntreesZero(ClavierSouris *pEntrees);
int ChargementAnimations(SDL_Renderer *pMoteurRendu, Animation anim[]);
int MessageInformations(const char messageInfos[], TTF_Font *polices[], SDL_Renderer *pMoteurRendu, ClavierSouris *pEntrees);
int VerificationMD5(char empreinte[], char nomFichier[]);

#endif // IOMAIN_H_INCLUDED

//Fin du fichier IOmain.h
