/*
Projet-ISN

Fichier: IOoptions.h

Contenu: Prototypes des fonctions contenues dans IOoptions.c

Actions: Permet � l'ordinateur de conna�tre toutes les fonctions pr�sentes dans le programme ainsi que leurs arguments.

Jean-Loup BEAUSSART & Dylan GUERVILLE
*/

#ifndef IOOPTIONS_H_INCLUDED		//Protection contre les inclusons infinies
#define IOOPTIONS_H_INCLUDED

/* Structures */
typedef struct Options		//Structure pour stocker les options
{
	char vies;
	int fullScreen;
	int musique;
	int nbLigne;
	int sons;
	int largeur;
	int hauteur;
	float volume;
} Options;

/* Prototypes des fonctions */
Options* DecouperOptions(char options[][50]);
int LectureOptions(char options[][50]);
int ValiderChangement(char options[][50]);

#endif // IOOPTIONS_H_INCLUDED

//Fin du fichier IOoptions.h
