/*
Projet-ISN

Fichier: collision.c

Contenu: Fonctions de gestion des collisions.

Actions: C'est ici que sont effectu�es les t�ches pour d�tecter les collisions entre deux boules, entre une boule et le sol ...

Biblioth�ques utilis�es: Biblioth�ques standards, SDL, SDL_image, SDL_ttf, FMOD, GTK

Jean-Loup BEAUSSART & Dylan GUERVILLE
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <gtk/gtk.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include <fmod.h>
#include "IOoptions.h"
#include "IOmain.h"
#include "main.h"
#include "jeu.h"
#include "collision.h"

extern int TailleBloc, TailleBoule, TailleMissileH, TailleMissileW, BMusique, BSons;		//Lien vers les variables globales d�clar�es dans main.c
extern double Volume, Largeur, Hauteur;
extern InfoDeJeu infos;

void CollisionDetect(Sprite images[], int indiceImage, Map *pMap, Collision *pCollision)
{
    /* Cette fonction r�initialise la variable pCollision->etatColl et active les bits correspondants aux collisions gr�ce � des OU bits � bits */

    /* Mise � z�ro */
    pCollision->etatColl &= COLL_NONE;

    /* S'il y a une collision avec une autre image, la structure est remplie en interne */
    CollisionImage(images, indiceImage, pCollision);

    /* S'il y a une collision avec un des bords de la fen�tre */
    pCollision->etatColl |= CollisionBordure(images, indiceImage);

    /*S'il y a une collision avec le d�cor*/
    pCollision->etatColl |= CollisionDecor(images, indiceImage, pMap);
}

unsigned int CollisionBordure (Sprite images[], int indiceImage)
{
    /* S'il y a une collision avec un des bords de la fen�tre, retourne un flag */

    if(images[indiceImage].position[0].y <= 0)
    {
        return  COLL_BORD_HAUT;
    }

    if(images[indiceImage].position[0].y + images[indiceImage].position[0].h >= Hauteur)
    {
        return COLL_BORD_BAS;
    }

    if(images[indiceImage].position[0].x <= 0)
    {
        return COLL_BORD_GAUCHE;
    }

    if(images[indiceImage].position[0].x + images[indiceImage].position[0].w >= Largeur)
    {
        return COLL_BORD_DROIT;
    }

    return COLL_NONE;
}

int CollisionImage (Sprite images[], int indiceImage, Collision *pCollision)
{
    /* Permet de d�tecter les collisions entre images, d�finie les bits de la structure Collision avec les flags, et �ventuellement place le num�ro du missile touch� dans la structure */

    int i=0, distance=0;	//Compteur
    SDL_Rect centre1, centre2;  //Centres des boules

    for(i=0; i <= BOULE_VERTE; i++)	//On teste les boules entre elles
    {
        if (i == indiceImage)	//On ne la teste pas avec elle m�me
        {
            continue;
        }

        /* On calcule les centres des boules */
        centre1.x = (images[indiceImage].position[0].x*2 + images[indiceImage].position[0].w)/2;
        centre1.y = (images[indiceImage].position[0].y*2 + images[indiceImage].position[0].h)/2;

        centre2.x = (images[i].position[0].x*2 + images[i].position[0].w)/2;
        centre2.y = (images[i].position[0].y*2 + images[i].position[0].h)/2;

        /* On calcule la distance centre � centre (racine de la somme des carr�s des soustractions des x et des y) */
        distance = Arrondir(sqrtf(pow((float)centre1.x - centre2.x, 2) + pow((float)centre1.y - centre2.y, 2)));

        if(distance < TailleBoule)  //Distance centre � centre inf�rieure au diam�tre
        {
            switch(i)	//On active le flag correspondant
            {
            case BOULE_BLEUE:
                pCollision->etatColl |= COLL_BOULE_BLEUE;
                break;

            case BOULE_MAGENTA:
                pCollision->etatColl |= COLL_BOULE_MAGENTA;
                break;

            case BOULE_VERTE:
                pCollision->etatColl |= COLL_BOULE_VERTE;
                break;
            }
        }
    }

    /* Ensuite on v�rifie si elles touchent un missile V */
    for(i=0; i < 5; i++)
    {
        if((images[indiceImage].position[0].y + images[indiceImage].position[0].h - (0.007*Largeur) > images[MISSILE].position[i].y) && (images[indiceImage].position[0].y < images[MISSILE].position[i].y + images[MISSILE].position[i].h - (0.007*Largeur)))
        {
            if(((images[indiceImage].position[0].x + images[indiceImage].position[0].w - (0.007*Largeur)) > images[MISSILE].position[i].x) && (images[indiceImage].position[0].x < images[MISSILE].position[i].x + images[MISSILE].position[i].w - (0.007*Largeur)))
            {
                pCollision->numMissile = (char)i;
                pCollision->etatColl |= COLL_MISSILE;
            }
        }
    }

    /* Un missile H */
    for(i=5; i < 10; i++)
    {
        if((images[indiceImage].position[0].y + images[indiceImage].position[0].h - (0.007*Largeur) > images[MISSILE].position[i].y) && (images[indiceImage].position[0].y < images[MISSILE].position[i].y - (0.007*Largeur) + images[MISSILE].position[i].w))
        {
            if((images[indiceImage].position[0].x + images[indiceImage].position[0].w - (0.007*Largeur) > images[MISSILE].position[i].x - images[MISSILE].position[i].h) && (images[indiceImage].position[0].x < images[MISSILE].position[i].x - (0.007*Largeur)))
            {
                pCollision->numMissile = (char)i;
                pCollision->etatColl |= COLL_MISSILE;
            }
        }
    }

    /* Enfin on regarde si les deux boules sont dans leur vortex respectif (on v�rifie que ce n'est pas le vortex que l'on est en train de tester)*/
    if (indiceImage != VORTEX_BLEU)
    {
        if((images[indiceImage].position[0].y + images[indiceImage].position[0].h > images[VORTEX_BLEU].position[0].y) && (images[indiceImage].position[0].y < images[VORTEX_BLEU].position[0].y + images[VORTEX_BLEU].position[0].h))
        {
            if(((images[indiceImage].position[0].x + images[indiceImage].position[0].w) > images[VORTEX_BLEU].position[0].x) && (images[indiceImage].position[0].x < images[VORTEX_BLEU].position[0].x + images[VORTEX_BLEU].position[0].w))
            {
                pCollision->etatColl |= COLL_VORTEX_BLEU;
            }
        }
    }

    if (indiceImage != VORTEX_VERT)
    {
        if((images[indiceImage].position[0].y + images[indiceImage].position[0].h > images[VORTEX_VERT].position[0].y) && (images[indiceImage].position[0].y < images[VORTEX_VERT].position[0].y + images[VORTEX_VERT].position[0].h))
        {
            if(((images[indiceImage].position[0].x + images[indiceImage].position[0].w) > images[VORTEX_VERT].position[0].x) && (images[indiceImage].position[0].x < images[VORTEX_VERT].position[0].x + images[VORTEX_VERT].position[0].w))
            {
                pCollision->etatColl |= COLL_VORTEX_VERT;
            }
        }
    }

    return 0;
}

unsigned int CollisionDecor (Sprite images[], int indiceImage, Map* pMap)
{
    /* Cette fonction d�tecte les collisions avec le d�cor et renvoie le flag correspondant */

    if((pMap->plan[((images[indiceImage].position[0].x + images[indiceImage].position[0].w) / TailleBloc)][((images[indiceImage].position[0].y + images[indiceImage].position[0].h) / TailleBloc)]) != VIDE)
    {
        return COLL_DECOR;
    }

    if((pMap->plan[((images[indiceImage].position[0].x + images[indiceImage].position[0].w) / TailleBloc)][(images[indiceImage].position[0].y / TailleBloc)]) != VIDE)
    {
        return COLL_DECOR;
    }

    if((pMap->plan[(images[indiceImage].position[0].x / TailleBloc)][((images[indiceImage].position[0].y + images[indiceImage].position[0].h) / TailleBloc)]) != VIDE)
    {
        return COLL_DECOR;
    }

    if((pMap->plan[(images[indiceImage].position[0].x / TailleBloc)][(images[indiceImage].position[0].y / TailleBloc)]) != VIDE)
    {
        return COLL_DECOR;
    }

    return COLL_NONE;
}

//Fin du fichier collision.c