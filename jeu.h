/*
Projet-ISN

Fichier: jeu.h

Contenu: Prototypes des fonctions contenues dans jeu.c

Actions: Permet � l'ordinateur de conna�tre toutes les fonctions pr�sentes dans le programme ainsi que leurs arguments.

Jean-Loup BEAUSSART & Dylan GUERVILLE
*/

#ifndef JEU_H_INCLUDED		//Protection contre les inclusons infinies
#define JEU_H_INCLUDED

/* Prototypes des fonctions */
int MiseAJourCoordonnees(ClavierSouris entrees, Sprite images[], unsigned char descente[], Map *pMap, FMOD_SYSTEM *pMoteurSon, Sons *pSons);
int DeplacementBoules(Sprite images[], Map *pMap, ClavierSouris *pEntrees, FMOD_SYSTEM *pMoteurSon, Sons *pSons);
int DeplacementMissiles(Sprite images[], unsigned char descente[]);
int Gravite(Sprite images[], Map *pMap, unsigned char sautEnCoursBleue, unsigned char sautEnCoursVerte);
void Chrono();
void CollisionDetect(Sprite images[], int indiceImage, Map *pMap, Collision *pCollision);
unsigned int CollisionBordure (Sprite images[], int indiceImage);
unsigned int CollisionImage (Sprite images[], int indiceImage, Collision *pCollision);
unsigned int CollisionDecor (Sprite images[], int indiceImage, Map* pMap);
void DetectionBonus (Sprite images[], int indiceImage, Map* pMap);
int Affichage(SDL_Renderer *pMoteurRendu, Sprite images[], TTF_Font *polices[], unsigned char descente[], Map* pMap, Animation anim[], unsigned char *ajoutAnim);
int AffichageVies(SDL_Renderer *pMoteurRendu, Sprite images[]);
int AffichageMap(SDL_Renderer *pMoteurRendu, Map *pMap);
int AffichageBonus(SDL_Renderer *pMoteurRendu, Map *pMap, Sprite images[]);
int AffichageImages(SDL_Renderer *pMoteurRendu, Sprite images[], Animation anim[], unsigned char descente[], unsigned char *pAjoutAnim);
int AffichageTextes(SDL_Renderer *pMoteurRendu, TTF_Font *polices[], SDL_Texture *pTextureFond);
int InitialisationPositions(Sprite images[], Joueur *pJoueur, int level);
int SautBleue(SDL_Rect *pPosition, unsigned char *pSautEnCours);
int SautVerte(SDL_Rect *pPosition, unsigned char *pSautEnCours);
int Sauts(Sprite images[], Map *pMap, ClavierSouris *pEntrees, unsigned char *pSautEnCoursBleue, unsigned char *pSautEnCoursVerte);
int Boucle_principale(Joueur *pJoueur, Sprite images[], Animation anim[], SDL_Renderer *pMoteurRendu, FMOD_SYSTEM *pMoteurSon, Sons *pSons, TTF_Font *polices[]);
int Arrondir(double nombre);
int Perdu(SDL_Renderer *pMoteurRendu, Sprite images[], Animation anim[], Map* pMap, TTF_Font *polices[]);
int PerduAffichage(SDL_Renderer *pMoteurRendu, Sprite images[], Animation anim[], SDL_Texture *pFondPerdu, Map *pMap, Texte *pInformation, unsigned char *pAjoutAnim);
int LectureAnimation(SDL_Renderer *pMoteurRendu, Animation anim[], int animNB);
char VerifierMortOUGagne(Sprite images[], Map *pMap, FMOD_SYSTEM *pMoteurSon, Sons *pSons);
int TraitementEtatDuNiveau(SDL_Renderer *pMoteurRendu, FMOD_SYSTEM *pMoteurSon, Sons *pSons, Map **ppMap, Joueur *pJoueur, Sprite images[], TTF_Font *polices[], Animation anim[], int *pEtat, unsigned char *pAjoutAnim, int *pControl);
void TraitementBonus(FMOD_SYSTEM *pMoteurSon, Sons *pSons);
int Gagne(SDL_Renderer *pMoteurRendu, Sprite images[], Map *pMap, TTF_Font *polices[]);
int GagneAffichage(SDL_Renderer *pMoteurRendu, Sprite images[], SDL_Texture *pTextureFondGagne, Map *pMap, Texte *information);

#endif // JEU_H_INCLUDED

//Fin du fichier jeu.h
