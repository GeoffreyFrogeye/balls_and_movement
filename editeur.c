/*
Projet-ISN

Fichier: editeur.c

Contenu: Fonctions relatives � l'�diteur de niveau.

Actions: C'est ici que se trouve les fonctions qui g�rent l'�diteur de niveau.

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
#include <fmod.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include "IOoptions.h"
#include "IOmain.h"
#include "main.h"
#include "jeu.h"
#include "IOediteur.h"
#include "editeur.h"
#include "collision.h"

extern int TailleBloc, TailleBoule, TailleMissileH, TailleMissileW, BMusique, BSons;		//Lien vers les variables globales d�clar�es dans main.c
extern double Volume, Hauteur, Largeur;

int Editeur (SDL_Renderer *pMoteurRendu, Sprite images[], FMOD_SYSTEM *pMoteurSon, Sons *pSons, TTF_Font *polices[], Joueur *pJoueur)
{
    Map *pMap = NULL;	//Pointeur sur une structure Map
    int continuer=true, etat=0, objetPris=AUCUN_BONUS, differenceFPS=0;
    unsigned int tempsFPS=0, tempsAncienFPS=0;	//Les temps sont forc�ment positifs
    ClavierSouris entrees;	//Structure pour g�rer les entr�es clavier et souris
    FMOD_CHANNEL *pChannelEnCours=NULL;
    SDL_Event evenementPoubelle;	//Une structure pour prendre les �v�nements dans la file d'attente sans les utiliser (purge de la file des �v�nements)
    NomBooleen nomNiveau= {"", false};

    EntreesZero(&entrees);	//On initialise la structure des entr�es

    /* On d�finit les positions et les tailles des images, on charge une map vierge et on cr�e les boutons pour ajouter des missiles */
    pMap = InitialisationEditeur(pMoteurRendu, images, polices, pJoueur, &etat);

    while(SDL_PollEvent(&evenementPoubelle));	//On purge la file d'attente des �v�nements (si des touches ont �t� appuy�es pendant le chargement)

    while(continuer)	//Tant que continuer ne vaut pas 0
    {
        tempsFPS = SDL_GetTicks();	//On prend le temps �coul� depuis le lancement de la SDL

        differenceFPS = tempsFPS - tempsAncienFPS;

        /* On effectue l'affichage s'il s'est �coul� T_FPS depuis le dernier affichage, pour ne pas saturer le GPU */
        if(differenceFPS > T_FPS)
        {
            AffichageEditeur(pMoteurRendu, images, pMap, entrees, objetPris);
            tempsAncienFPS = tempsFPS;	//On garde le temps actuel dans une autre variable pour pouvoir faire la soustraction
        }

        /* On met � jour la map de l'�diteur et on r�cup�re l'�ventuel objet qui est s�lectionn� et qui suit le curseur pour l'affichage */
        objetPris = MiseAJourMap(pMap, images, &entrees, pMoteurSon, pSons);

        /* On met � jour la structure qui donne l'�tat du clavier et de la souris en lisant le prochain �v�nement en file d'attente */
        GestionEvenements(&entrees);

        /* Si il y a eu appuis sur la touche Echap ou demande de fermeture (croix ou Alt + F4), on affiche un message de confirmation */
        if (entrees.clavier[ECHAP] || entrees.fermeture)
        {
            entrees.clavier[ECHAP] = false;

            /* Si on appuit sur entr�e alors on quitte vraiment l'�diteur */
            if(MessageInformations("Voulez-vous vraiment retourner au menu sans sauvegarder ?", polices, pMoteurRendu, &entrees) == 1)
            {
                continuer = false;
            }
        }

        /* Si on appuit sur S on v�rifie la validit� de la map et on la sauvegarde */
        if (entrees.clavier[S])
        {
            etat = VerifierEmplacements(images, pMap);	//On v�rifie

            /* Si un missile et une boule sont sur la m�me trajectoire */
            if(etat == -1)
            {
                if(BSons)	//On joue un son d'alarme en boucle
                {
                    FMOD_System_PlaySound(pMoteurSon, S_ALARME+10, pSons->bruits[S_ALARME], false, NULL);
                    FMOD_System_GetChannel(pMoteurSon, S_ALARME+10, &pChannelEnCours);
                    FMOD_Channel_SetLoopCount(pChannelEnCours, -1);
                }

                /* On affiche un message */
                MessageInformations("Le missile et une des boules sont sur la m�me trajectoire.", polices, pMoteurRendu, &entrees);

                if(BSons)	// On arr�te l'alarme
                {
                    FMOD_Channel_Stop(pChannelEnCours);
                }
            }
            else if (etat == -2)	//Si une boule est dans le sol d�s le d�part
            {
                if(BSons)	//Alarme
                {
                    FMOD_System_PlaySound(pMoteurSon, S_ALARME+10, pSons->bruits[S_ALARME], false, NULL);
                    FMOD_System_GetChannel(pMoteurSon, S_ALARME+10, &pChannelEnCours);
                    FMOD_Channel_SetLoopCount(pChannelEnCours, 10);
                }

                /* Message */
                MessageInformations("Une des boules est en collision.", polices, pMoteurRendu, &entrees);

                if(BSons)	//Fin de l'alarme
                {
                    FMOD_Channel_Stop(pChannelEnCours);
                }
            }
            else if (etat == -3)	//Si le vortex bleu est dans le sol
            {
                if(BSons)	//Alarme
                {
                    FMOD_System_PlaySound(pMoteurSon, S_ALARME+10, pSons->bruits[S_ALARME], false, NULL);
                    FMOD_System_GetChannel(pMoteurSon, S_ALARME+10, &pChannelEnCours);
                    FMOD_Channel_SetLoopCount(pChannelEnCours, 10);
                }

                /* Message */
                MessageInformations("Le vortex bleu est en collision.", polices, pMoteurRendu, &entrees);

                if(BSons)	//Fin de l'alarme
                {
                    FMOD_Channel_Stop(pChannelEnCours);
                }
            }
            else if (etat == -4)	//Si le vortex vert est dans le sol
            {
                if(BSons)	//Alarme
                {
                    FMOD_System_PlaySound(pMoteurSon, S_ALARME+10, pSons->bruits[S_ALARME], false, NULL);
                    FMOD_System_GetChannel(pMoteurSon, S_ALARME+10, &pChannelEnCours);
                    FMOD_Channel_SetLoopCount(pChannelEnCours, 10);
                }

                /* Message */
                MessageInformations("Le vortex vert est en collision.", polices, pMoteurRendu, &entrees);

                if(BSons)	// Fin de l'alarme
                {
                    FMOD_Channel_Stop(pChannelEnCours);
                }
            }
            else	//Sinon si la map est correcte
            {
                /* On demande son nom en injectant dans le thread principal GTK, la fonction DemandeNomNiveau, tant que elle n'a pas chang� le titre du niveau, on fait une boucle infinie dans l'�diteur pour attendre */
                strcpy(nomNiveau.nom, pMap->titre);
                g_idle_add((GSourceFunc)DemandeNomNiveau, &nomNiveau);

                while(!nomNiveau.poursuite);

                /* Ensuite on copie le nom entr� dans la structure Map */
                sprintf(pMap->titre, nomNiveau.nom);

                if(BSons)	//Petit son de sauvegarde
                {
                    FMOD_System_PlaySound(pMoteurSon, S_SAVE+10, pSons->bruits[S_SAVE], true, NULL);
                    FMOD_System_GetChannel(pMoteurSon, S_SAVE+10, &pChannelEnCours);
                    FMOD_Channel_SetVolume(pChannelEnCours, (float)(Volume/110.0));
                    FMOD_Channel_SetPaused(pChannelEnCours, false);
                }

                /* On enregistre dans le fichier levelUser.lvl */
                if(SauvegardeNiveau(pMap, images, pJoueur) == -1)
                {
                    /* Message erreur */
                    MessageInformations("Erreur lors de la sauvegarde !", polices, pMoteurRendu, &entrees);
                }
                else
                {
                    /* Message */
                    MessageInformations("Niveau sauvegard� !", polices, pMoteurRendu, &entrees);
                }

                continuer = false;	// On retourne au menu
            }
        }

        SDL_Delay(1);	//D�lais d'au moins 1 ms pour emp�cher des bugs et une surcharge du processeur pour les autres t�ches que l'affichage
    }

    /* On d�truit la map apr�s la boucle de l'�diteur pour �viter les fuites de m�moire */
    DestructionMap(pMap);

    /* On remet la musique du menu */
    if(BMusique)
    {
        FMOD_System_GetChannel(pMoteurSon, M_JEU, &pChannelEnCours);
        FMOD_Channel_SetPaused(pChannelEnCours, true);
        FMOD_System_GetChannel(pMoteurSon, M_MENU, &pChannelEnCours);
        FMOD_Channel_SetPaused(pChannelEnCours, false);
    }

    /* On remet -1 en valeur de niveau � �diter pour �viter que lors d'un futur clic sur le bouton ajout de niveau, on ne charge un niveau au lieu d'en cr�er un nouveau */
    pJoueur->niveauEditeur = -1;

    return 0;	// On retourne au menu principal
}

Map* InitialisationEditeur (SDL_Renderer *pMoteurRendu, Sprite images[], TTF_Font *polices[], Joueur *pJoueur, int *pEtat)
{
    Map* pMap = NULL;	//Pointeur vers une structure Map
    SDL_Surface *pSurfMissileH, *pSurfMissileV;		//Pointeurs vers des surfaces
    SDL_Color blancOpaque = {255, 255, 255, SDL_ALPHA_OPAQUE};

    /* On cr�e une nouvelle surface avec le texte sp�cifi�, on met � la bonne taille, on la transforme en texture et on lib�re cette surface */
    pSurfMissileH = TTF_RenderText_Blended(polices[POLICE_SNICKY], "Ajout missile H", blancOpaque);
    images[AJOUTER_MISSILE_H].pTextures[0] = SDL_CreateTextureFromSurface(pMoteurRendu, pSurfMissileH);
    TTF_SizeText(polices[POLICE_SNICKY], "Ajout missile H", &images[AJOUTER_MISSILE_H].position[0].w, &images[AJOUTER_MISSILE_H].position[0].h);
    SDL_FreeSurface(pSurfMissileH);

    /* On cr�e une nouvelle surface avec le texte sp�cifi�, on met � la bonne taille, on la transforme en texture et on lib�re cette surface */
    pSurfMissileV = TTF_RenderText_Blended(polices[POLICE_SNICKY], "Ajout missile V", blancOpaque);
    images[AJOUTER_MISSILE_V].pTextures[0] = SDL_CreateTextureFromSurface(pMoteurRendu, pSurfMissileV);
    TTF_SizeText(polices[POLICE_SNICKY], "Ajout missile V", &images[AJOUTER_MISSILE_V].position[0].w, &images[AJOUTER_MISSILE_V].position[0].h);
    SDL_FreeSurface(pSurfMissileV);

    /* On charge une map en mode '�diteur' donc vierge ou alors on charge un niveau pour �dition */
    pMap = ChargementNiveau(pMoteurRendu, pJoueur, pJoueur->niveauEditeur, pEtat);

    InitialisationPositions(images, pJoueur, pJoueur->niveauEditeur);		//On initialise les positions et les tailles des images

    return pMap;	//On renvoie l'adresse de la map
}

int AffichageEditeur(SDL_Renderer *pMoteurRendu, Sprite images[], Map* pMap, ClavierSouris entrees, int objetPris)
{
    /* Cette fonction s'occupe de l'affichage */

    int i=0;     //Compteur
    SDL_Point pointOrigine= {0, 0};	//Coordonn�es d'un point (0;0) pour faire les rotations

    /* On efface l'�cran avec du noir */
    SDL_SetRenderDrawColor(pMoteurRendu, 0, 0, 0, SDL_ALPHA_OPAQUE);
    SDL_RenderClear(pMoteurRendu);

    /* On dessine un fond bleu sur toute la surface dans laquelle on peut travailler (si l'�cran n'est pas au format 16/10, il y aura des bandes en haut et en bas ou sur les c�t�s, elles resteront noires) */
    boxRGBA(pMoteurRendu, 0, 0, Largeur, Hauteur, 85, 120, 180, SDL_ALPHA_OPAQUE);

    /* On affiche les blocs de la map */
    AffichageMap(pMoteurRendu, pMap);

    /* On affiche les bonus */
    AffichageBonus(pMoteurRendu, pMap, images);

    /* Puis les images de boules, de vortex et les boutons "Ajout de missiles" */
    SDL_RenderCopy(pMoteurRendu, images[MISSILE].pTextures[0], NULL, &images[MISSILE].position[0]);
    SDL_RenderCopy(pMoteurRendu, images[VORTEX_BLEU].pTextures[0], NULL, &images[VORTEX_BLEU].position[0]);
    SDL_RenderCopy(pMoteurRendu, images[VORTEX_VERT].pTextures[0], NULL, &images[VORTEX_VERT].position[0]);
    SDL_RenderCopy(pMoteurRendu, images[BOULE_BLEUE].pTextures[0], NULL, &images[BOULE_BLEUE].position[0]);
    SDL_RenderCopy(pMoteurRendu, images[BOULE_MAGENTA].pTextures[0], NULL, &images[BOULE_MAGENTA].position[0]);
    SDL_RenderCopy(pMoteurRendu, images[BOULE_VERTE].pTextures[0], NULL, &images[BOULE_VERTE].position[0]);
    SDL_RenderCopy(pMoteurRendu, images[AJOUTER_MISSILE_V].pTextures[0], NULL, &images[AJOUTER_MISSILE_V].position[0]);
    SDL_RenderCopy(pMoteurRendu, images[AJOUTER_MISSILE_H].pTextures[0], NULL, &images[AJOUTER_MISSILE_H].position[0]);

    /* On affiche ensuite les missiles */
    for(i=0; i<5; i++)
    {
        SDL_RenderCopy(pMoteurRendu, images[MISSILE].pTextures[0], NULL, &images[MISSILE].position[i]);
    }

    /* Les 5 derniers sont horizontales donc rotation de 90� */
    for (i=5; i<10; i++)
    {
        SDL_RenderCopyEx(pMoteurRendu, images[MISSILE].pTextures[0], NULL, &images[MISSILE].position[i], 90, &pointOrigine, SDL_FLIP_NONE);
    }

    /* On affiche les bonus dans leur cadre de s�lection */
    AffichageBonusEditeur(pMoteurRendu, images);

    /* Le cadre en question */
    AffichageBoxEditeur(pMoteurRendu, &entrees);

    /* On affiche le curseur et l'�ventuel objet s�lectionn� qui suit ce dernier */
    AffichageObjetCurseurEditeur(pMoteurRendu, &entrees, images, objetPris);

    SDL_RenderPresent(pMoteurRendu);    //Mise � jour de l'�cran

    return 0; //Affichage termin�
}

int MiseAJourMap (Map *pMap, Sprite images[], ClavierSouris *pEntrees, FMOD_SYSTEM *pMoteurSon, Sons *pSons)
{
    /* On retient ce qui est s�lectionn� m�me lorsque la fonction est rappel�e plusieurs fois, ces variables sont statiques */
    static int diaPris=AUCUN_BONUS, missilePris=0;

    /* On d�place l'objets qui est s�lectionn� par exemple une boule */
    DeplacementObjetEditeur(pMoteurSon, pSons, images, pEntrees);

    /* On place ou on retire un bloc ou un bonus selon l'�tat des entr�es */
    MiseAjourMapEtBonusEditeur(pEntrees, pMoteurSon, pSons, pMap, &diaPris, &missilePris);

    /* On ajoute un missile si on clique sur un bouton ou si on en a un de s�lectionner et que l'on clique sur la map */
    MiseAJourMapMissileEditeur(pMoteurSon, pSons, pEntrees, images, &missilePris);

    /* On change le type de chaque bloc de la map selon ceux qui se trouve au-dessus */
    AmeliorationMap(pMap);

    /* S'il y a un bonus de s�lectionn� on renvoie lequel */
    if(diaPris != AUCUN_BONUS)
    {
        return diaPris;
    }
    else	// Sinon on renvoie 100 ou 101 pour un �ventuel missile H ou V, ou 0 s'il n 'y a aucun objet s�lectionn�
    {
        return missilePris;
    }
}

int VerifierEmplacements(Sprite images[], Map *pMap)
{
    /* Cette fonction v�rifie les collisions et les alignements interdits, comme une boule dans le sol ou sur la trajectoire d'un missile */

    int i=0, j=0;
    Collision collision= {COLL_NONE, 0};	//Structure pour retenir les collisions et l'indice du missile incrimin� s'il y en a un

    /* On teste chaque boule une par une */
    for(i=BOULE_BLEUE; i<=BOULE_VERTE; i++)
    {
        /* On regarde si chaque missile V va toucher la boule que l'on teste  au cours de son d�placement */
        for(j=0; j<5; j++)
        {
            if(images[i].position[0].x + images[i].position[0].w > images[MISSILE].position[j].x && images[i].position[0].x < images[MISSILE].position[j].x + images[MISSILE].position[j].w)
            {
                return -1;
            }
        }

        /* On fait la m�me chose avec chaque missile H */
        for(j=5; j<10; j++)
        {
            if(images[i].position[0].y + images[i].position[0].h > images[MISSILE].position[j].y && images[i].position[0].y < images[MISSILE].position[j].y + images[MISSILE].position[j].w)
            {
                return -1;
            }
        }

        /* On v�rifie si la boule que l'on teste est en collision avec quoi que ce soit */
        CollisionDetect(images, i, pMap, &collision);

        if(collision.etatColl & ~COLL_NONE)	//Si un seul bit est � 1 c'est qu'il y a une collision
        {
            return -2;
        }
    }

    /* On v�rifie ensuite les collisions pour les 2 vortex */
    CollisionDetect(images, VORTEX_BLEU, pMap, &collision);

    if(collision.etatColl & ~COLL_NONE)
    {
        return -3;
    }

    CollisionDetect(images, VORTEX_VERT, pMap, &collision);

    if(collision.etatColl & ~COLL_NONE)
    {
        return -4;
    }

    return 0;	//On renvoie 0 s'il n'y a aucun probl�me
}

void DeplacementObjetEditeur(FMOD_SYSTEM *pMoteurSon, Sons *pSons, Sprite images[], ClavierSouris *pEntrees)
{
    int i=0,j=0;
    static int deplacement = -1;	//On retient dans une variable statique si on a s�lectionn� un objet et lequel (-1 pas d'objet)

    /* On s'occupe de chaque boule et des 2 vortex l'un apr�s l'autre, les missiles sont trait�s dans une boucle � l'int�rieur de l'autre */
    for (i=BOULE_BLEUE; i<=VORTEX_VERT; i++)
    {
        if(i == MISSILE || deplacement >= 100)	//Si on s'occupe des missiles ou que l'objet s�lectionn� est un missile (>=100)
        {
            /* On fait les 5 missiles V */
            for (j=0; j<5; j++)
            {
                /* On regarde si on clique sur un missile ou si il y en avait d�j� un s�lectionn� */
                if((pEntrees->souris.touches[C_MOLETTE] && pEntrees->souris.position.x > images[MISSILE].position[j].x && pEntrees->souris.position.x < (images[MISSILE].position[j].x + images[MISSILE].position[j].w) && pEntrees->souris.position.y > images[MISSILE].position[j].y && pEntrees->souris.position.y < (images[MISSILE].position[j].y + images[MISSILE].position[j].h)) || (deplacement >= 100 && deplacement < 105))
                {
                    /* Si il n'y avait pas de d�placement en cours c'est une s�lection d'objet */
                    if (deplacement == -1)
                    {
                        if(BSons)
                        {
                            FMOD_System_PlaySound(pMoteurSon, S_CLICK+10, pSons->bruits[S_CLICK], false, NULL);	//Bruit de clic
                        }

                        deplacement = 100+j;	//On retient l'indice de l'image que l'on est en train de s�lectionner
                        pEntrees->souris.touches[C_MOLETTE] = false;	//On remet � z�ro le clic de molette
                    }

                    /* Si un d�placement est en cours, un objet est d�j� s�lectionn�, deplacement compris entre 100 et 104 ; si on fait un clic de molette on d�pose alors l'image l� o� elle est */
                    if (deplacement >= 100 && deplacement < 105)
                    {
                        images[MISSILE].position[deplacement-100].x = pEntrees->souris.position.x - images[MISSILE].position[deplacement-100].w /2;
                        images[MISSILE].position[deplacement-100].y = pEntrees->souris.position.y - images[MISSILE].position[deplacement-100].h /2;

                        if (pEntrees->souris.touches[C_MOLETTE])
                        {
                            if(BSons)
                            {
                                FMOD_System_PlaySound(pMoteurSon, S_CLICK+10, pSons->bruits[S_CLICK], false, NULL);	//Bruit de clic
                            }

                            deplacement = -1;	//On remet d�placement � -1
                            pEntrees->souris.touches[C_MOLETTE] = false;
                        }
                    }
                }
            }

            /* On s'occupe de m�me des missiles H */
            for (j=5; j<10; j++)
            {
                if((pEntrees->souris.touches[C_MOLETTE] && pEntrees->souris.position.x > images[MISSILE].position[j].x - images[MISSILE].position[j].h && pEntrees->souris.position.x < images[MISSILE].position[j].x && pEntrees->souris.position.y > images[MISSILE].position[j].y && pEntrees->souris.position.y < (images[MISSILE].position[j].y + images[MISSILE].position[j].w)) || (deplacement >= 105 && deplacement < 110))
                {
                    /* Si il n'y avait pas de d�placement en cours c'est une s�lection d'objet */
                    if (deplacement == -1)
                    {
                        if(BSons)
                        {
                            FMOD_System_PlaySound(pMoteurSon, S_CLICK+10, pSons->bruits[S_CLICK], false, NULL);	//Bruit de clic
                        }

                        deplacement = 100+j;	//On retient l'indice de l'image que l'on est en train de s�lectionner
                        pEntrees->souris.touches[C_MOLETTE] = false;	//On remet � z�ro le clic de molette
                    }

                    /* Si un d�placement est en cours, un objet est d�j� s�lectionn�, deplacement compris entre 105 et 109 ; si on fait un clic de molette on d�pose alors l'image l� o� elle est */
                    if (deplacement >= 105 && deplacement < 110)
                    {
                        images[MISSILE].position[deplacement-100].x = pEntrees->souris.position.x + images[MISSILE].position[deplacement-100].h /2;
                        images[MISSILE].position[deplacement-100].y = pEntrees->souris.position.y - images[MISSILE].position[deplacement-100].w /2;

                        if (pEntrees->souris.touches[C_MOLETTE])
                        {
                            if(BSons)
                            {
                                FMOD_System_PlaySound(pMoteurSon, S_CLICK+10, pSons->bruits[S_CLICK], false, NULL);	//Bruit de clic
                            }

                            deplacement = -1;	//On remet d�placement � -1
                            pEntrees->souris.touches[C_MOLETTE] = false;
                        }
                    }
                }
            }
        }

        /* On regarde si la souris est sur l'image et si on appuie sur la molette OU si un d�placement est en cours */
        if((pEntrees->souris.touches[C_MOLETTE] && pEntrees->souris.position.x > images[i].position[0].x && pEntrees->souris.position.x < (images[i].position[0].x + images[i].position[0].w) && pEntrees->souris.position.y > images[i].position[0].y && pEntrees->souris.position.y < (images[i].position[0].y + images[i].position[0].h)) || (deplacement != -1 && deplacement < 100))
        {
            /* Si il n'y avait pas de d�placement en cours c'est une s�lection d'objet */
            if (deplacement == -1)
            {
                if(BSons)
                {
                    FMOD_System_PlaySound(pMoteurSon, S_CLICK+10, pSons->bruits[S_CLICK], false, NULL);	//Bruit de clic
                }

                deplacement = i;	//On retient l'indice de l'image que l'on est en train de s�lectionner
                pEntrees->souris.touches[C_MOLETTE] = false;	//On remet � z�ro le clic de molette
            }

            /* La position de l'image suit alors celle de la souris mais elle est centr�e par rapport au curseur */
            images[deplacement].position[0].x = pEntrees->souris.position.x - images[deplacement].position[0].w /2;
            images[deplacement].position[0].y = pEntrees->souris.position.y - images[deplacement].position[0].h /2;

            /* Si un d�placement est en cours, un objet est d�j� s�lectionn�, deplacement != -1 ; si on fait un clic de molette on d�pose alors l'image l� o� elle est */
            if (deplacement != -1 && deplacement < 100 && pEntrees->souris.touches[C_MOLETTE])
            {
                if(BSons)
                {
                    FMOD_System_PlaySound(pMoteurSon, S_CLICK+10, pSons->bruits[S_CLICK], false, NULL);	//Bruit de clic
                }

                deplacement = -1;	//On remet d�placement � -1
                pEntrees->souris.touches[C_MOLETTE] = false;
            }
        }
    }
}

void MiseAjourMapEtBonusEditeur(ClavierSouris *pEntrees, FMOD_SYSTEM *pMoteurSon, Sons *pSons, Map *pMap, int *pDiaPris, int *pMissilePris)
{
    SDL_Rect pos;	//Variable de position et de taille (x; y; h; w)
    int i=0, j=0, k=0;	//Compteurs

    /* On prend les coordonn�es de la souris */
    pos.x = pEntrees->souris.position.x;
    pos.y = pEntrees->souris.position.y;

    /* On fait bien attention � ce que la souris soit dans la zone et non sur les bandes noires qui pourraient appara�tre sur les c�t�s, sinon plantage assur� */
    if(pos.x > 0 && pos.x < Largeur && pos.y > 0 && pos.y < Hauteur)
    {
        /* Si on fait un clic gauche sans appuyer sur SHIFT et qu'il n'y a aucun objet s�lectionn� on met un bloc */
        if(pEntrees->souris.touches[C_GAUCHE] && !pEntrees->clavier[SHIFT] && *pDiaPris == AUCUN_BONUS && !(*pMissilePris))
        {
            if(BSons)	//Bruit de clic
            {
                FMOD_System_PlaySound(pMoteurSon, S_CLICK+10, pSons->bruits[S_CLICK], false, NULL);
            }

            pMap->plan[pos.x /TailleBloc][pos.y /TailleBloc] = SOL_NORMAL; //On place un bloc de sol normal dans la case sur laquelle se trouve la souris
        }
        else if(pEntrees->souris.touches[C_DROIT] && !pEntrees->clavier[SHIFT] && *pDiaPris == AUCUN_BONUS && !(*pMissilePris))
        {
            pMap->plan[pos.x /TailleBloc][pos.y /TailleBloc] = VIDE;	//Si c'est un clic droit dans les m�mes conditions on efface la case
        }
        else if (*pDiaPris && pEntrees->souris.touches[C_GAUCHE])	//S'il y a un bonus s�lectionn� et que l'on fait un clic gauche on d�pose le bonus
        {
            if(BSons)	//Bruit de clic
            {
                FMOD_System_PlaySound(pMoteurSon, S_CLICK+10, pSons->bruits[S_CLICK], false, NULL);
            }

            pMap->planObjets[pos.x /TailleBloc][pos.y /TailleBloc] = *pDiaPris;	//On d�pose le bonus s�lectionn�
            *pDiaPris = AUCUN_BONUS;	//Plus rien n'est s�lectionn�
            pEntrees->souris.touches[C_GAUCHE] = false;	//Clic gauche d�sactiv� pour �viter que des blocs ne soit pos�s dans la foul�
        }
        else if (pEntrees->souris.touches[C_DROIT] && pEntrees->clavier[SHIFT])	//Si on appuie sur SHIFT et que l'on fait un clic droit alors on efface le bonus
        {
            pMap->planObjets[pos.x /TailleBloc][pos.y /TailleBloc] = AUCUN_BONUS;	//Plus de bonus ici
        }

        /* Maintenant on s'occupe de s�lectionner un bonus dans le cadre des bonus */
        if(pEntrees->clavier[SHIFT] && pEntrees->souris.touches[C_GAUCHE])	//Si on appuie sur SHIFT et que l'on fait un clic gauche
        {
            /* On va balayer tous les bonus (3 lignes avec i et 9 bonus par ligne avec j), k donne le rang du bonus (de 1 jusqu'� 18) */
            for (k=1, i=0; i<2; i++)
            {
                for (j=0; j<9; j++, k++)
                {
                    /* On regarde si la souris est sur le bonus actuel */
                    if(pEntrees->souris.position.x > 0.075*Largeur*(double)j +0.081*Largeur && pEntrees->souris.position.x < 0.075*Largeur*(double)(j+1) +0.081*Largeur && pEntrees->souris.position.y > 0.075*Hauteur*(double)i +0.83*Hauteur && pEntrees->souris.position.y < 0.075*Largeur*(double)(i+1) +0.83*Hauteur)
                    {
                        if(BSons)
                        {
                            FMOD_System_PlaySound(pMoteurSon, S_CLICK+10, pSons->bruits[S_CLICK], false, NULL);	//Bruit de clic
                        }

                        *pDiaPris = k;	//Le bonus s�lectionn� est retenu
                        pEntrees->souris.touches[C_GAUCHE] = false;	//Le clic gauche est d�sactiv� pour �viter que le bonus ne soit d�pos� instantan�ment
                    }
                }
            }
        }
    }
}

void MiseAJourMapMissileEditeur(FMOD_SYSTEM *pMoteurSon, Sons *pSons, ClavierSouris *pEntrees, Sprite images[], int *pMissilePris)
{
    int i=0, j=0;	//Compteurs

    /* Si on clique sur le bouton Ajouter Missile H */
    if(pEntrees->souris.touches[C_MOLETTE] && pEntrees->souris.position.x > images[AJOUTER_MISSILE_H].position[0].x && pEntrees->souris.position.x < images[AJOUTER_MISSILE_H].position[0].x + images[AJOUTER_MISSILE_H].position[0].w)
    {
        if(pEntrees->souris.position.y > images[AJOUTER_MISSILE_H].position[0].y && pEntrees->souris.position.y < images[AJOUTER_MISSILE_H].position[0].y + images[AJOUTER_MISSILE_H].position[0].h)
        {
            if(BSons)
            {
                FMOD_System_PlaySound(pMoteurSon, S_CLICK+10, pSons->bruits[S_CLICK], false, NULL);	//Bruit de clic
            }

            *pMissilePris=101;	//On s�lectionne un missile H
            pEntrees->souris.touches[C_MOLETTE] = false;	//On d�sactive le clic de molette pour �viter que le missile ne soit d�pos� instantan�ment
        }
    }

    /* On fait la m�me chose avec le bouton pour les missiles V */
    if(pEntrees->souris.touches[C_MOLETTE] && pEntrees->souris.position.x > images[AJOUTER_MISSILE_V].position[0].x && pEntrees->souris.position.x < images[AJOUTER_MISSILE_V].position[0].x + images[AJOUTER_MISSILE_V].position[0].w)
    {
        if(pEntrees->souris.position.y > images[AJOUTER_MISSILE_V].position[0].y && pEntrees->souris.position.y < images[AJOUTER_MISSILE_V].position[0].y + images[AJOUTER_MISSILE_V].position[0].h)
        {
            if(BSons)
            {
                FMOD_System_PlaySound(pMoteurSon, S_CLICK+10, pSons->bruits[S_CLICK], false, NULL);	//Bruit de clic
            }

            *pMissilePris=100;	//On s�lectionne un missile V
            pEntrees->souris.touches[C_MOLETTE] = false;	//On d�sactive le clic de molette
        }
    }

    /* Si un missile est s�lectionn� et que l'on fait un clic de molette on va le d�poser*/
    if (*pMissilePris && pEntrees->souris.touches[C_MOLETTE])
    {
        /* On regarde si c'est un missile H ou V */
        switch(*pMissilePris)
        {
        case 100:	//Missile V
            for(i=0; i<5; i++)	//On regarde si parmis les missiles V il en reste qui ne sont pas plac�s
            {
                if(images[MISSILE].position[i].x < 0)
                {
                    j=1;	//Si oui on met j � 1 et on quitte la boucle tout de suite 'break;'
                    break;
                }
                else
                {
                    j=-1;	//Sinon on met -1 dans j pour dire que ce missile V est d�j� plac�
                }
            }

            if (j != -1)	//Si on a trouv� un missile V pas encore plac� on le place sinon il ne se passe rien
            {
                images[MISSILE].position[i].x = pEntrees->souris.position.x;
                images[MISSILE].position[i].y = pEntrees->souris.position.y;
            }

            pEntrees->souris.touches[C_MOLETTE] = false;	//On d�sactive le clic de molette
            break;

        case 101:	//Missile H
            for(i=5; i<10; i++)	//On v�rifie s'ils ne sont pas tous plac�s
            {
                if(images[MISSILE].position[i].x < 0)
                {
                    j=1;	//On en a trouv� un pas encore plac�
                    break;
                }
                else
                {
                    j=-1;
                }
            }

            if (j != -1)	//On place le missile H s'ils ne sont pas tous d�j� plac�s
            {
                images[MISSILE].position[i].x = pEntrees->souris.position.x;
                images[MISSILE].position[i].y = pEntrees->souris.position.y;
            }

            pEntrees->souris.touches[C_MOLETTE] = false;	//On d�sactive le clic de molette
            break;
        }

        if(BSons)
        {
            FMOD_System_PlaySound(pMoteurSon, S_CLICK+10, pSons->bruits[S_CLICK], false, NULL);	//Bruit de clic
        }

        *pMissilePris = 0;	//Plus de missile s�lectionn�
    }
}

void AmeliorationMap(Map *pMap)
{
    /* Cette fonction change automatiquement le type de bloc de la map en fonction de ceux au-dessus, elle �vite ainsi que plusieurs couches d'herbe ne soient superpos�es */

    int i=0, j=0;	//Compteurs

    /* On parcourt la map colonne par colonne */
    for (i=0; i< pMap->nbtiles_largeur_monde; i++)
    {
        /* Dans chaque colonne on descent bloc par bloc */
        for (j=0; j< pMap->nbtiles_hauteur_monde; j++)
        {
            /* On v�rifie si il y a du sol avec de l'herbe et en dessous autre chose que du vide alors on met le sol de transition qui doit �tre juste en dessous de l'herbe, on v�rifie �galement que le bloc avec de l'herbe n'est pas tout en bas de la colonne sinon plantage */
            if(pMap->plan[i][j] == SOL_NORMAL && pMap->plan[i][j+1] != VIDE && j < (pMap->nbtiles_hauteur_monde-1))
            {
                pMap->plan[i][j+1] = SOL_PLEIN_3;

                /* On v�rifie ensuite si en dessous de ce second bloc il y autre chose que du vide et on place le bloc de sol uniforme, on v�rifie �galement que le bloc avec de l'herbe n'est pas l'avant dernier de la colonne */
                if (pMap->plan[i][j+2] != VIDE && j < (pMap->nbtiles_hauteur_monde-2))
                {
                    pMap->plan[i][j+2] = SOL_PLEIN_UNI;
                }
            }

            /* Si on a un bloc non vide et qu'au dessus c'est un sol uniforme alors ce bloc est aussi un sol uniforme */
            if (pMap->plan[i][j] != VIDE && pMap->plan[i][j-1] == SOL_PLEIN_UNI)
            {
                pMap->plan[i][j] = SOL_PLEIN_UNI;
            }

            /* Si on a un bloc non vide et qu'au dessus le bloc est vide alors ce bloc est un bloc d'herbe */
            if (pMap->plan[i][j] != VIDE && pMap->plan[i][j-1] == VIDE)
            {
                pMap->plan[i][j] = SOL_NORMAL;
            }
        }
    }
}

void AffichageBonusEditeur(SDL_Renderer *pMoteurRendu, Sprite images[])
{
    /* Cette fonction affiche les bonus dans leur cadre de s�lection */

    int i=0, j=0, k=0;	//Compteurs

    /* On parcourt les 18 bonus gr�ce � une double boucle, k compte les bonus de 1 � 18 */
    for (k=1, i=0; i<2; i++)
    {
        for (j=0; j<9; j++, k++)
        {
            /* On colle les bonus � une position qui d�pend de leur ordre sur l'image d'origine */
            images[GEMMES].position[0].x = Arrondir(0.075*Largeur*j + 0.081*Largeur);
            images[GEMMES].position[0].y = Arrondir(0.075*Hauteur*i + 0.83*Hauteur);
            SDL_RenderCopy(pMoteurRendu, images[GEMMES].pTextures[0], &images[GEMMES].position[k], &images[GEMMES].position[0]);
        }
    }
}

void AffichageBoxEditeur(SDL_Renderer *pMoteurRendu, ClavierSouris *pEntrees)
{
    /* Cette fonction affiche les cadres o� sont plac�s les objets au d�part, on en dessine 4 d�cal�s d'un pixel pour plus d'�paisseur */

    int i=0;	//Compteur

    if(pEntrees->clavier[SHIFT])	//Si on appuie sur SHIFT on dessine des cadres plus clairs pour les bonus et des normaux pour les images
    {
        for (i=0; i<=4; i++)
        {
            roundedRectangleRGBA(pMoteurRendu, (0.89*Largeur +i), i, (Largeur-10-i), (Hauteur-10-i), 10, 255, 255, 255, 128);
            roundedRectangleRGBA(pMoteurRendu, (10+i), (0.8*Hauteur +i), (0.8*Largeur -i), (Hauteur-10-i), 10, 255, 255, 255, 200);
        }
    }
    else	//Sinon des cadres normaux pour tous
    {
        for (i=0; i<=4; i++)
        {
            roundedRectangleRGBA(pMoteurRendu, (0.89*Largeur +i), i, (Largeur-10-i), (Hauteur-10-i), 10, 255, 255, 255, 128);
            roundedRectangleRGBA(pMoteurRendu, (10+i), (0.8*Hauteur +i), (0.8*Largeur -i), (Hauteur-10-i), 10, 255, 255, 255, 100);
        }
    }
}

void AffichageObjetCurseurEditeur(SDL_Renderer *pMoteurRendu, ClavierSouris *pEntrees, Sprite images[], int objetPris)
{
    SDL_Point pointOrigine= {0, 0};	//Coordonn�es du point d'origine de l'image pour faire la rotation
    int angleCurseur=335;

    /* On d�finit les coordonn�es des diff�rentes images que l'on pourrait avoir � coller sur celle de la souris */
    images[MISSILE].position[10].x = images[GEMMES].position[0].x = images[CURSEUR].position[0].x = pEntrees->souris.position.x;
    images[MISSILE].position[10].y = images[GEMMES].position[0].y = images[CURSEUR].position[0].y = pEntrees->souris.position.y;

    /* Si il y a un bonus mais pas de missile on colle le bonus pr�s de la souris */
    if(objetPris != AUCUN_BONUS && objetPris != 100 && objetPris != 101)
    {
        SDL_RenderCopy(pMoteurRendu, images[GEMMES].pTextures[0], &images[GEMMES].position[objetPris], &images[GEMMES].position[0]);
    }
    else if (objetPris == 100)	//Sinon s'il y a un missile V on colle un missile V pr�s de la souris
    {
        SDL_RenderCopy(pMoteurRendu, images[MISSILE].pTextures[0], NULL, &images[MISSILE].position[10]);
    }
    else if (objetPris == 101)	//Sinon s'il y a un missile H on colle un missile H pr�s de la souris
    {
        SDL_RenderCopyEx(pMoteurRendu, images[MISSILE].pTextures[0], NULL, &images[MISSILE].position[10], 90, &pointOrigine, SDL_FLIP_NONE);
    }

    /* Enfin on colle le curseur */
    SDL_RenderCopyEx(pMoteurRendu, images[CURSEUR].pTextures[0], NULL, &images[CURSEUR].position[0], angleCurseur, NULL, SDL_FLIP_NONE);
}

//Fin du fichier editeur.c
