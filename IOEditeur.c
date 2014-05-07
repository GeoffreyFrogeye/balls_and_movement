/*
Projet-ISN

Fichier: IOEditeur.c

Contenu: Fonctions relatives aux chargements et aux �critures de l'�diteur de niveau.

Actions: C'est ici que se trouve les fonctions qui g�rent les entr�es/sorties de l'�diteur.

Biblioth�ques utilis�es: Biblioth�ques standards, SDL, SDL_image, SDL_ttf, FMOD, GTK

Jean-Loup BEAUSSART & Dylan GUERVILLE
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <gtk/gtk.h>
#include <fmod.h>
#include <SDL2_gfxPrimitives.h>
#include "IOoptions.h"
#include "IOmain.h"
#include "main.h"
#include "jeu.h"
#include "IOediteur.h"
#include "editeur.h"

extern int TailleBloc, TailleBoule, TailleMissileH, TailleMissileW, BMusique, BSons;		//Lien vers les variables globales d�clar�es dans main.c
extern double Volume, Largeur, Hauteur;

int SauvegardeNiveau(Map* pMap, Sprite images[])
{
	FILE *pFichierNiveau = NULL;	//Pointeur sur le fichier de niveau
	char tile[5] = "2\n";	//Cha�ne pour enregistrer chaque bloc, par d�faut c'est le bloc vide '2' avec un retour � la ligne '\n'
	char chaine[20] = "";	//Une cha�ne pour �crire les autres informations
	char *c = NULL;		//Pointeur sur un caract�re pour faire des recherches dans les cha�nes
	int i=0, j=0;	//Compteurs

	pFichierNiveau = fopen("ressources/levelUser.lvl", "a");	//On ouvre le fichier en mode ajout

	if (pFichierNiveau == NULL)	//V�rification
	{
		return -1;
	}

	fputs("#taille\n", pFichierNiveau);	//On �crit

	/* On met la taille de la map dans la cha�ne et on �crit dans le fichier */
	sprintf(chaine, "%d\n", pMap->nbtiles_hauteur_monde);
	fputs(chaine, pFichierNiveau);

	sprintf(chaine, "%d\n", pMap->nbtiles_largeur_monde);
	fputs(chaine, pFichierNiveau);

	fputs("#missileV\n", pFichierNiveau);	//On �crit

	/* On �crit les coordonn�es des missile V en pourcentage de la largeur */
	for(i=0; i<5; i++)
	{
		if(images[MISSILE].position[i].x < 0)	//On �crit -1 si le missile n'est pas plac�
		{
			fputs("-1.0\n", pFichierNiveau);
			fputs("-1.0\n", pFichierNiveau);
			continue;	//On passe directement au tour de boucle suivant
		}

		/* On calcul les coordonn�es en pourcentage de la largeur, on recherche la virgule si il y en a une et on la remplace par un point */
		/* Selon le syst�me d'exploitation on peut avoir une virgule � la place du point */
		sprintf(chaine, "%f\n", (float)images[MISSILE].position[i].x/(float)Largeur);
		c = strstr(chaine, ",");

		if(c != NULL)
		{
			*c = '.';
		}

		fputs(chaine, pFichierNiveau);

		sprintf(chaine, "%f\n", (float)images[MISSILE].position[i].y/(float)Hauteur);
		c = strstr(chaine, ",");

		if(c != NULL)
		{
			*c = '.';
		}

		fputs(chaine, pFichierNiveau);
	}


	fputs("#missileH\n", pFichierNiveau);	//On �crit

	/* On �crit les coordonn�es des missile H en pourcentage de la largeur */
	for(i=5; i<10; i++)
	{
		if(images[MISSILE].position[i].x < 0)	//On �crit -1 si le missile n'est pas plac�
		{
			fputs("-1.0\n", pFichierNiveau);
			fputs("-1.0\n", pFichierNiveau);
			continue;
		}

		/* On calcul les coordonn�es en pourcentage de la largeur, on recherche la virgule si il y en a une et on la remplace par un point */
		/* Selon le syst�me d'exploitation on peut avoir une virgule � la place du point */
		sprintf(chaine, "%f\n", (float)images[MISSILE].position[i].x/(float)Largeur);
		c = strstr(chaine, ",");

		if(c != NULL)
		{
			*c = '.';
		}

		fputs(chaine, pFichierNiveau);

		sprintf(chaine, "%f\n", (float)images[MISSILE].position[i].y/(float)Hauteur);
		c = strstr(chaine, ",");

		if(c != NULL)
		{
			*c = '.';
		}

		fputs(chaine, pFichierNiveau);
	}

	/* On proc�de de la m�me fa�on avec les deux vortex */
	fputs("#vortex\n", pFichierNiveau);

	/* Le bleu */
	sprintf(chaine, "%f\n", (float)images[VORTEX_BLEU].position[0].x/(float)Largeur);
	c = strstr(chaine, ",");

	if(c != NULL)
	{
		*c = '.';
	}

	fputs(chaine, pFichierNiveau);

	sprintf(chaine, "%f\n", (float)images[VORTEX_BLEU].position[0].y/(float)Hauteur);
	c = strstr(chaine, ",");

	if(c != NULL)
	{
		*c = '.';
	}

	fputs(chaine, pFichierNiveau);

	/* Le vert */
	sprintf(chaine, "%f\n", (float)images[VORTEX_VERT].position[0].x/(float)Largeur);
	c = strstr(chaine, ",");

	if(c != NULL)
	{
		*c = '.';
	}

	fputs(chaine, pFichierNiveau);

	sprintf(chaine, "%f\n", (float)images[VORTEX_VERT].position[0].y/(float)Hauteur);
	c = strstr(chaine, ",");

	if(c != NULL)
	{
		*c = '.';
	}

	fputs(chaine, pFichierNiveau);

	/* Maintenant on s'occupe des boules */
	fputs("#boules\n", pFichierNiveau);

	/* Boule bleue */
	sprintf(chaine, "%f\n", (float)images[BOULE_BLEUE].position[0].x/(float)Largeur);
	c = strstr(chaine, ",");

	if(c != NULL)
	{
		*c = '.';
	}

	fputs(chaine, pFichierNiveau);

	sprintf(chaine, "%f\n", (float)images[BOULE_BLEUE].position[0].y/(float)Hauteur);
	c = strstr(chaine, ",");

	if(c != NULL)
	{
		*c = '.';
	}

	fputs(chaine, pFichierNiveau);

	/* Boule magenta */
	sprintf(chaine, "%f\n", (float)images[BOULE_MAGENTA].position[0].x/(float)Largeur);
	c = strstr(chaine, ",");

	if(c != NULL)
	{
		*c = '.';
	}

	fputs(chaine, pFichierNiveau);

	sprintf(chaine, "%f\n", (float)images[BOULE_MAGENTA].position[0].y/(float)Hauteur);
	c = strstr(chaine, ",");

	if(c != NULL)
	{
		*c = '.';
	}

	fputs(chaine, pFichierNiveau);

	/* Boule verte */
	sprintf(chaine, "%f\n", (float)images[BOULE_VERTE].position[0].x/(float)Largeur);
	c = strstr(chaine, ",");

	if(c != NULL)
	{
		*c = '.';
	}

	fputs(chaine, pFichierNiveau);

	sprintf(chaine, "%f\n", (float)images[BOULE_VERTE].position[0].y/(float)Hauteur);
	c = strstr(chaine, ",");

	if(c != NULL)
	{
		*c = '.';
	}

	fputs(chaine, pFichierNiveau);

	/* Enfin on s'occupe des blocs de la map */
	fputs("#map\n", pFichierNiveau);

	for (i=0; i< pMap->nbtiles_largeur_monde; i++)
	{
		for(j=0; j< pMap->nbtiles_hauteur_monde; j++)
		{
			sprintf(tile, "%d\n", pMap->plan[i][j]);	//On passe du chiffre 2 au caract�re '2'
			fputs(tile, pFichierNiveau);			//On �crit un par un chaque chiffre qui correspond � un bloc sur une ligne
		}
	}

	/* On �crit la map des bonus */
	fputs("#mapObjets\n", pFichierNiveau);

	for (i=0; i< pMap->nbtiles_largeur_monde; i++)
	{
		for(j=0; j< pMap->nbtiles_hauteur_monde; j++)
		{
			sprintf(tile, "%d\n", pMap->planObjets[i][j]);
			fputs(tile, pFichierNiveau);
		}
	}

	/* On place un s�parateur entre chaque niveau */
	fputs("##--##\n", pFichierNiveau);

	fclose(pFichierNiveau);	//On ferme le fichier

	return 0;
}

//Fin du fichier IOEditeur.c
