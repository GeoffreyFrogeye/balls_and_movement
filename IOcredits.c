/*
Projet-ISN

Fichier: IOcredits.c

Contenu: Fonctions pour lire le fichier de cr�dit

Actions: C'est ici que se trouve les fonctions qui lisent ou �crivent sur le disque et qui concerne les cr�dits.

Biblioth�ques utilis�es: Biblioth�ques standards, SDL, SDL_image, SDL_ttf, FMOD, GTK

Jean-Loup BEAUSSART & Dylan GUERVILLE
*/

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <fmod.h>
#include <gtk/gtk.h>
#include "IOoptions.h"
#include "IOmain.h"
#include "main.h"
#include "IOcredits.h"

int ChargementCredits(char credit[][100])
{
	int i=0, j=0;
	FILE* pFichierCredits = NULL;       //Pointeur sur le fichier credits.cit
	char caractere=0;

	pFichierCredits = fopen("ressources/credits.cit", "r");    //On ouvre le fichier de cr�dit

	if(pFichierCredits == NULL)     //On v�rifie si on a r�ussi � l'ouvrir
	{
		return -1;
	}

	do    //On lit le fichier ligne par ligne tant qu'on est pas � la fin du fichier
	{
		/* On lit les lignes caract�re par caract�re tant que la ligne n'est pas finie ou qu'on est pas � la fin du fichier */
		do
		{
			caractere = fgetc(pFichierCredits);
			credit[i][j] = caractere;
			j++;
		}
		while(caractere != '\n' && caractere != EOF);

		credit[i][j-1] = '\0';   //On applique le caract�re de fin de chaine
		j=0;	//On revient au premier caract�re dans le tableau
		i++;	//On passe � ligne suivante dans le tableau
	}
	while(caractere != EOF);

	fclose(pFichierCredits);        //On ferme le fichier

	return i-1; //On renvoie le nombre de lignes du fichier pour l'affichage
}

//Fin du fichier IOcredits.c
