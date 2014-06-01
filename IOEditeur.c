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
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <gtk/gtk.h>
#include <fmod.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include "IOoptions.h"
#include "IOmain.h"
#include "main.h"
#include "callback.h"
#include "jeu.h"
#include "IOediteur.h"
#include "editeur.h"

extern int TailleBloc, TailleBoule, TailleMissileH, TailleMissileW, BMusique, BSons;		//Lien vers les variables globales d�clar�es dans main.c
extern double Volume, Largeur, Hauteur;

int SauvegardeNiveau(Map* pMap, Sprite images[], Joueur *pJoueur)
{
    FILE *pFichierNiveau = NULL;	//Pointeur sur le fichier de niveau
    char tile[5] = "2\n";	//Cha�ne pour enregistrer chaque bloc, par d�faut c'est le bloc vide '2' avec un retour � la ligne '\n'
    char chaine[200] = "", *bufferAvant=NULL, *bufferApres=NULL;	//Une cha�ne pour �crire les autres informations et 2 buffers
    char *c = NULL;		//Pointeur sur un caract�re pour faire des recherches dans les cha�nes
    int i=0, j=0, taille=0;	//Compteurs
    int level = pJoueur->niveauEditeur;

    if(level == -1)
    {
        pFichierNiveau = fopen("ressources/levelUser.lvl", "a");	//On ouvre le fichier en mode ajout si on doit juste rajouter un niveau

        if (pFichierNiveau == NULL)	//V�rification
        {
            return -1;
        }
    }
    else
    {
        pFichierNiveau = fopen("ressources/levelUser.lvl", "r");	//On ouvre le fichier au d�but en lecture

        if (pFichierNiveau == NULL)	//V�rification
        {
            return -1;
        }

        i=j=0;

        /* On lit tous ce qu'il y a avant le niveau � �diter et on le sauvegarde dans un buffer(comme avec la fonction de suppression) */
        while(i<level-1)
        {
            fgets(chaine, 200, pFichierNiveau);
            taille += strlen(chaine);
            bufferAvant= realloc(bufferAvant, taille*sizeof(char)+1);
            strcpy(bufferAvant+j, chaine);
            j=taille;

            if (strcmp(chaine, "##--##\n") == 0)
            {
                i++;
            }
        }

        /* On saute le niveau � �diter */
        do
        {
            fgets(chaine, 200, pFichierNiveau);
        }
        while (strcmp(chaine, "##--##\n") != 0);

        /* On lit le reste jusqu'� la fin du fichier et on le sauvegarde dans un autre buffer */
        taille=j=0;

        while(!feof(pFichierNiveau))
        {
            if(fgets(chaine, 200, pFichierNiveau) != NULL)
            {
                taille += strlen(chaine);
                bufferApres= realloc(bufferApres, taille*sizeof(char)+1);
                strcpy(bufferApres+j, chaine);
                j=taille;
            }
        }

        /* On ferme le fichier */
        fclose(pFichierNiveau);

        pFichierNiveau = fopen("ressources/levelUser.lvl", "w");	//On l'ouvre en �criture et on le vide

        /* On met ce qu'il y avait avant le niveau � �diter s'il y avait quelque chose */
        if (bufferAvant != NULL)
        {
            fputs(bufferAvant, pFichierNiveau);
        }
    }

    /* On �crit le niveau que l'on a modifi� */
    fputs("#titre\n", pFichierNiveau);

    fputs(pMap->titre, pFichierNiveau);
    fputs("\n", pFichierNiveau);

    fputs("#taille\n", pFichierNiveau);

    /* On met la taille de la map dans la cha�ne et on �crit dans le fichier */
    sprintf(chaine, "%d\n", pMap->nbtiles_hauteur_monde);
    fputs(chaine, pFichierNiveau);

    sprintf(chaine, "%d\n", pMap->nbtiles_largeur_monde);
    fputs(chaine, pFichierNiveau);

    fputs("#missileV\n", pFichierNiveau);

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

    fputs("#missileH\n", pFichierNiveau);

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

    /* On �crit les niveaux qu'il y avait apr�s celui � �diter sauf s'il n'y en avait pas ou si c'�tait un simple ajout de nouveau niveau */
    if (bufferApres != NULL)
    {
        fputs(bufferApres, pFichierNiveau);
    }

    fclose(pFichierNiveau);	//On ferme le fichier

    return 0;
}

gboolean DemandeNomNiveau(gpointer *pData)
{
    /* Cette fonction cr�e la fen�tre de demande du nom du niveau */

    GtkWidget *pFenetre, *pBoutonOK, *pLabelBoutonOK, *pEntry, *pVBox;
    GdkRGBA couleurFond= {0.610, 0.805, 0.920, 1}, couleurBoutons= {0.650, 0.850, 0.925, 1}, couleurBoutonsEnfonce= {0.550, 0.655, 7.000, 1};	//Couleur
    GtkEntryBuffer *pBuffer;	//Pointeur sur un buffer
    GSList *pListe = NULL;	//On cr�e une liste cha�n�e
    NomBooleen *pNomNiveau = (NomBooleen *)pData;	//On r�cup�re la structure NomBool�en, qui contient une cha�ne de caract�res et un bool�en

    /* On cr�e la fen�tre */
    pFenetre = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_position(GTK_WINDOW(pFenetre), GTK_WIN_POS_CENTER_ALWAYS);
    gtk_window_set_decorated(GTK_WINDOW(pFenetre), false);
    gtk_window_set_title(GTK_WINDOW(pFenetre), "Nom");
    gtk_window_set_icon_from_file(GTK_WINDOW(pFenetre), "ressources/img/z.png", NULL);
    gtk_widget_set_size_request(pFenetre, 100, 120);
    gtk_window_set_resizable(GTK_WINDOW(pFenetre), false);
    gtk_widget_override_background_color(pFenetre, GTK_STATE_FLAG_NORMAL, &couleurFond);

    /* On cr�e le champ de saisie avec son buffer */
    pBuffer = gtk_entry_buffer_new(pNomNiveau->nom, strlen(pNomNiveau->nom));
    pEntry = gtk_entry_new_with_buffer(GTK_ENTRY_BUFFER(pBuffer));
    gtk_widget_set_margin_top(pEntry, 10);
    gtk_widget_set_margin_left(pEntry, 15);
    gtk_widget_set_margin_right(pEntry, 15);

    /* On ajoute � la liste le buffer et la structure NomBooleen */
    pListe = g_slist_append(pListe, pBuffer);
    g_slist_append(pListe, pNomNiveau);

    /* On cr�e le bouton OK */
    pBoutonOK = gtk_button_new();
    pLabelBoutonOK = gtk_label_new("OK");
    gtk_widget_override_background_color(pLabelBoutonOK, GTK_STATE_FLAG_NORMAL, &couleurBoutons);
    gtk_widget_override_background_color(pLabelBoutonOK, GTK_STATE_FLAG_ACTIVE, &couleurBoutonsEnfonce);
    gtk_container_add(GTK_CONTAINER(pBoutonOK), pLabelBoutonOK);
    gtk_widget_override_background_color(pBoutonOK, GTK_STATE_FLAG_NORMAL, &couleurBoutons);
    gtk_widget_override_background_color(pBoutonOK, GTK_STATE_FLAG_ACTIVE, &couleurBoutonsEnfonce);
    gtk_widget_set_size_request(pLabelBoutonOK, 40, 20);
    gtk_widget_set_margin_left(pBoutonOK, 30);
    gtk_widget_set_margin_right(pBoutonOK, 30);
    gtk_widget_set_margin_top(pBoutonOK, 20);
    g_signal_connect(G_OBJECT(pBoutonOK), "clicked", G_CALLBACK(RenvoiContenuEntry), pListe);	//Connection du signal "clicked"

    /* On met tout dans une bo�te V */
    pVBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_box_pack_start(GTK_BOX(pVBox), pEntry, false, false, 0);
    gtk_box_pack_start(GTK_BOX(pVBox), pBoutonOK, false, false, 0);

    /* On met la bo�te dans la fen�tre */
    gtk_container_add(GTK_CONTAINER(pFenetre), pVBox);

    gtk_widget_show_all(pFenetre);

    /* On renvoie 'false' pour que cette fonction, qui a �t� inject�e dans la boucle de GTK dans le thread principal, ne se r�p�te pas tout le temps */
    return false;
}

void RenvoiContenuEntry(GtkWidget *pWidget, gpointer *pData)
{
    /* Cette fonction est appel�e quand on clique sur OK dans la fen�tre de demande du nom du niveau */

    /* On r�cup�re les �l�ments dont on a besoin depuis la liste cha�n�e */
    NomBooleen *pNomNiveau = (NomBooleen*)g_slist_nth_data((GSList*)pData, 1);
    GtkEntryBuffer *pBuffer = (GtkEntryBuffer*)g_slist_nth_data((GSList*)pData, 0);

    /* On r�cup�re le texte du buffer, on le place dans la structure et on met le bool�en � 'true' pour que l'�diteur puisse continuer sa sauvegarde */
    sprintf(pNomNiveau->nom, gtk_entry_buffer_get_text(pBuffer));
    pNomNiveau->poursuite = true;

    /* On lib�re la m�moire */
    g_object_unref(pBuffer);
    g_slist_free((GSList*)pData);

    FermerFenetre(pWidget, NULL);	//On ferme la fen�tre
}

//Fin du fichier IOEditeur.c
