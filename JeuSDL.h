/*
Projet-ISN

Fichier: jeuSDL.h

Contenu: Prototypes des fonctions contenues dans jeuSDL.c

Actions: Permet � l'ordinateur de conna�tre toutes les fonctions pr�sentes dans le programme ainsi que leurs arguments.

Jean-Loup BEAUSSART & Dylan GUERVILLE
*/

#ifndef JEU_SDL_H_INCLUDED		//Protection contre les inclusons infinies
#define JEU_SDL_H_INCLUDED

/* Prototypes des fonctions */
void InitialiserInfos(Options *pOptions, Joueur *pJoueur);
int LancerJeu(gpointer *pData);
void LibererMemoire(SDL_Renderer *pMoteurRendu, Sprite images[], Animation anim[], TTF_Font *polices[], SDL_Window *pFenetre, Options *pOptions);
int SauverMySql(Joueur *pJoueur);

#endif // JEU_SDL_H_INCLUDED

//Fin du fichier jeuSDL.h
