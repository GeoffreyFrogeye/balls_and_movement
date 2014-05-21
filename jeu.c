/*
Projet-ISN

Fichier: jeu.c

Contenu: Fonctions principales du jeu: la boucle, la gestion des collisions, l'affichage ...

Actions: C'est ici que sont effectu�es les t�ches principales du jeu, la boucle principale, le rendu � l'�cran, les collisions, ...

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
#include <SDL2_gfxPrimitives.h>
#include <fmod.h>
#include "IOoptions.h"
#include "IOmain.h"
#include "main.h"
#include "jeu.h"

extern int TailleBloc, TailleBoule, TailleMissileH, TailleMissileW, BMusique, BSons;		//Lien vers les variables globales d�clar�es dans main.c
extern double Volume, Largeur, Hauteur;
extern InfoDeJeu infos;

int BouclePrincipale(Joueur *pJoueur, Sprite images[], Animation anim[], SDL_Renderer *pMoteurRendu, FMOD_SYSTEM *pMoteurSon, Sons *pSons, TTF_Font *polices[])
{
	ClavierSouris entrees;     //Structure pour conna�tre l'�tat du clavier et de la souris
	unsigned char descente[10]= {false};	//Tableau pour savoir quand les missiles montent ou descendent
	unsigned char ajoutAnim=false;	//Contr�le de l'ex�cution d'une animation
	unsigned int tempsFPS=0, tempsAncienFPS=0;	//Diff�rents temps pour les calculs
	int differenceFPS=0, etat=0, control=JEU_EN_COURS, etatNiveau, i=0;
	FMOD_CHANNEL *pChannelEnCours=NULL;	//Contr�le des canaux audio
	Map *pMap=NULL;	//Pointeurs sur une structure Map
	SDL_Event evenementPoubelle;	//Structure event pour purger la file des �v�nements

	EntreesZero(&entrees);	//On initialise la structure

	InitialisationPositions(images, pJoueur, infos.niveau);	//On charge les positions des images pour le niveau
	pMap = ChargementNiveau(pMoteurRendu, pJoueur, infos.niveau, &etatNiveau);	//On charge la map du niveau

	if(pMap == NULL)	//On v�rifie que la map a bien �t� charg�e
	{
		if(etatNiveau == CHARGEMENT_ERREUR || etatNiveau == CHARGEMENT_FICHIER_CORROMPU)	//Erreur de chargement
		{
			control = JEU_FIN_ERREUR_CHARGEMENT;
		}
		else if(etatNiveau == CHARGEMENT_GAGNE)
		{
			control = JEU_FIN_GAGNE;	//Fin normale, on a tout fait, on a d�j� gagn�
		}
	}

	while (SDL_PollEvent(&evenementPoubelle));	//On purge la file des �v�nements

	/* Boucle principale du jeu */
	while (control == JEU_EN_COURS)
	{
		if (ajoutAnim)	//Tant qu'une animation est en cours on bloque les commandes en purgeant la file et en remettant � z�ro la structure
		{
			SDL_PollEvent(&evenementPoubelle);
			EntreesZero(&entrees);
		}
		else
		{
			GestionEvenements(&entrees);     //Traitement des �v�nements du clavier et de la souris normalement
		}

		if(entrees.clavier[ECHAP] || entrees.fermeture)	//Appuis sur �chap ou sur Entr�e, on demande � quitter
		{
			entrees.clavier[ECHAP] = false;

			if(MessageInformations("Voulez-vous vraiment retourner au menu principal?", polices, pMoteurRendu, &entrees) == 1)
			{
				control = JEU_FIN;      //En cas d'appuie sur Entr�e on coupe la boucle.
			}
		}

		if(entrees.clavier[F5])	//Appuis sur F5 on demande � recommencer le niveau
		{
			if(MessageInformations("Voulez-vous vraiment recommencer ce niveau ?", polices, pMoteurRendu, &entrees) == 1)
			{
				/* En cas d'appuis sur Entr�e on recharge le niveau */
				pMap = ChargementNiveau(pMoteurRendu, pJoueur, infos.niveau, &etatNiveau);
				InitialisationPositions(images, pJoueur, infos.niveau);
				infos.compteurTemps = 0;
				infos.bonus &= AUCUN_BONUS;

				for(i=0; i<10; i++)
				{
					descente[i] = false;
				}
			}
		}

		/* Maintenant on va mettre � jour les coordonn�es et faire l'affichage */

		tempsFPS = SDL_GetTicks();	//On prend le temps �coul� depuis l'initialisation de la SDL

		differenceFPS = tempsFPS - tempsAncienFPS;	//On soustrait

		if(differenceFPS > T_FPS)	//On regarde s'il s'est �coul� plus de T_FPS ms depuis le dernier affichage et la mise � jour des coordonn�es
		{
			if(!ajoutAnim)	//On ne met pas � jour les coordonn�es pendant l'animation (gravit�, ect)
			{
				MiseAJourCoordonnees(entrees, images, descente, pMap, pMoteurSon, pSons);
			}

			/* Ensuite on effectue l'affichage */
			Affichage(pMoteurRendu, images, polices, descente, pMap, anim, &ajoutAnim);

			tempsAncienFPS = tempsFPS;
		}

		/* On regarde si le joueur a gagn� le niveau ou s'il s'est fait tuer */
		etat = VerifierMortOUGagne(images, pMap, pMoteurSon, pSons);
		/* On r�agit en cons�quence */
		TraitementEtatDuNiveau(pMoteurRendu, pMoteurSon, pSons, &pMap, pJoueur, images, polices, anim, &entrees, descente, &etat, &ajoutAnim, &control);

		if(infos.vies == 0)	//S'il ne reste plus de vie, on a perdu
		{
			if(BMusique)
			{
				FMOD_System_GetChannel(pMoteurSon, M_JEU, &pChannelEnCours);	//On arr�te la musique du jeu
				FMOD_Channel_SetPaused(pChannelEnCours, true);

				FMOD_System_PlaySound(pMoteurSon, M_PERDU, pSons->music[M_PERDU], true, NULL);		//On joue la musique quand on a perdu
				FMOD_System_GetChannel(pMoteurSon, M_PERDU, &pChannelEnCours);
				FMOD_Channel_SetVolume(pChannelEnCours, Volume/100.0);
				FMOD_Channel_SetPaused(pChannelEnCours, false);
			}

			Perdu(pMoteurRendu, images, anim, pMap, polices, &ajoutAnim);	//On affiche l'�cran de fin quand on a perdu

			control = JEU_FIN;	//On coupe la boucle
		}

		DetectionBonus(images, BOULE_MAGENTA, pMap);	//Si le joueur prend un bonus

		/* On s'occupe d'ajouter de la vie ou du score selon les bonus pris, on d�sactive les bonus ensuite et on joue un son */
		TraitementBonus(pMoteurSon, pSons);

		if(!ajoutAnim)
		{
			Chrono();	//Mise � jour du compteur de temps du niveau (sauf pendant l'animation)
		}

		SDL_Delay(1);	//Petit d�lais d'au moins 1 ms pour �viter les bugs d'affichage et une surcharge du processeur
	}

	/* Si on a fini le jeu normalement (pas � cause d'une erreur) on lib�re la m�moire de la map */
	if (control != JEU_FIN_ERREUR_CHARGEMENT)
	{
		DestructionMap(pMap);
	}

	if (BMusique)
	{
		FMOD_System_GetChannel(pMoteurSon, M_PERDU, &pChannelEnCours);	//On arr�te la musique quand on a perdu
		FMOD_Channel_SetPaused(pChannelEnCours, true);

		FMOD_System_GetChannel(pMoteurSon, M_JEU, &pChannelEnCours);	//On arr�te la musique du jeu
		FMOD_Channel_SetPaused(pChannelEnCours, true);

		FMOD_System_GetChannel(pMoteurSon, M_GAGNE, &pChannelEnCours);	//On arr�te la musique quand on a gagn�
		FMOD_Channel_SetPaused(pChannelEnCours, true);

		FMOD_System_GetChannel(pMoteurSon, M_MENU, &pChannelEnCours);	//Et on remet celle du menu
		FMOD_Channel_SetPaused(pChannelEnCours, false);
	}

	pJoueur->niveau_max = infos.niveau;	// On stocke le niveau maximum que le joueur a pu atteindre

	return control;	//On renvoie le code d'erreur pour savoir pourquoi la boucle s'est termin�e
}

int MiseAJourCoordonnees(ClavierSouris entrees, Sprite images[], unsigned char descente[], Map *pMap, FMOD_SYSTEM *pMoteurSon, Sons *pSons)
{
	static unsigned char sautEnCoursBleue=false, sautEnCoursVerte=false;	//Variables de contr�le des sauts

	/* Maintenant on va changer les positions des boules */
	DeplacementBoules(images, pMap, &entrees, pMoteurSon, pSons);

	/* On les fait sauter */
	Sauts(images, pMap, &entrees, &sautEnCoursBleue, &sautEnCoursVerte);

	/* On d�place les missiles */
	DeplacementMissiles(images, descente);

	/* On applique la gravit� */
	Gravite(images, pMap, sautEnCoursBleue, sautEnCoursVerte);

	return 0;
}

int Sauts(Sprite images[], Map *pMap, ClavierSouris *pEntrees, unsigned char *pSautEnCoursBleue, unsigned char *pSautEnCoursVerte)
{
	static unsigned char timerBleue=false, timerVerte=false;	//Variables de contr�le des timers des sauts
	static unsigned int temps=0, tempsAncien=0;	//Variables de temps
	static int savePosBleue=0, savePosVerte=0, difference=0;	//Variables pour sauvegarder des positions et une diff�rence de temps
	Collision collision= {COLL_NONE, 0};	//Structure pour g�rer les collisions

	temps = SDL_GetTicks();	//On prend le temps �coul� depuis l'initialisation de la SDL

	difference = temps - tempsAncien;	//On calcul la diff�rence avec l'ancien temps

	/* Si il s'est �coul� au moins 250ms, on r�initialise les timers des sauts */
	if (difference >= 250)
	{
		timerBleue = timerVerte = false;
		tempsAncien = temps;
	}
	else if (difference < 0)	//Si la diff�rence de temps est n�gative, c'est � cause des variables statiques, on remet donc le temps ancien � 0
	{
		tempsAncien = 0;
	}

	/* Si le timer n'est pas actif et que l'on appuie sur Espace ou qu'un saut �tait d�j� en cours */
	if((pEntrees->clavier[ESPACE] || *pSautEnCoursBleue) && !timerBleue)
	{
		/* Si un saut �tait en cours, on le poursuit */
		if(*pSautEnCoursBleue)
		{
			savePosBleue = SautBleue(&images[BOULE_BLEUE].position[0], pSautEnCoursBleue);
			images[BOULE_BLEUE].position[0].y += savePosBleue;
		}
		/* Sinon c'est que l'on a appuy� sur Espace, on v�rifie donc que l'on peut commencer un saut (on doit �tre pos� sur le sol) */
		else if (pMap->plan[Arrondir(images[BOULE_BLEUE].position[0].x + images[BOULE_BLEUE].position[0].w/2.0 - TailleBoule*0.3) /TailleBloc][Arrondir(images[BOULE_BLEUE].position[0].y + images[BOULE_BLEUE].position[0].h +1) /TailleBloc] != VIDE
		         ||
		         pMap->plan[Arrondir(images[BOULE_BLEUE].position[0].x + images[BOULE_BLEUE].position[0].w/2.0 + TailleBoule*0.3) /TailleBloc][Arrondir(images[BOULE_BLEUE].position[0].y + images[BOULE_BLEUE].position[0].h +1) /TailleBloc] != VIDE)
		{
			savePosBleue = SautBleue(&images[BOULE_BLEUE].position[0], pSautEnCoursBleue);
			images[BOULE_BLEUE].position[0].y += savePosBleue;
		}

		/* Ensuite on d�tecte les collisions */
		CollisionDetect(images, BOULE_BLEUE, pMap, &collision);

		/* Si on est rentr� dans une des autres boules ou dans le d�cor, on annule le d�placement et on arr�te le saut et on active le timer*/
		if((collision.etatColl & COLL_BOULE_MAGENTA) || (collision.etatColl & COLL_BOULE_VERTE) || (collision.etatColl & COLL_DECOR))
		{
			images[BOULE_BLEUE].position[0].y -= savePosBleue;
			savePosBleue=0;
			*pSautEnCoursBleue=false;
			timerBleue=true;
		}
	}

	/* Il en va de m�me avec la boule verte */
	if((pEntrees->clavier[ESPACE] || *pSautEnCoursVerte) && !timerVerte)
	{
		if(*pSautEnCoursVerte)
		{
			savePosVerte = SautVerte(&images[BOULE_VERTE].position[0], pSautEnCoursVerte);
			images[BOULE_VERTE].position[0].y += savePosVerte;
		}
		else if (pMap->plan[Arrondir(images[BOULE_VERTE].position[0].x + images[BOULE_VERTE].position[0].w/2.0 - TailleBoule*0.3) /TailleBloc][Arrondir(images[BOULE_VERTE].position[0].y -1)/ TailleBloc] != VIDE || pMap->plan[Arrondir(images[BOULE_VERTE].position[0].x + images[BOULE_VERTE].position[0].w/2.0 + TailleBoule*0.3) /TailleBloc][Arrondir(images[BOULE_VERTE].position[0].y -1)/ TailleBloc] != VIDE)
		{
			savePosVerte = SautVerte(&images[BOULE_VERTE].position[0], pSautEnCoursVerte);
			images[BOULE_VERTE].position[0].y += savePosVerte;
		}

		CollisionDetect(images, BOULE_VERTE, pMap, &collision);

		if((collision.etatColl & COLL_BOULE_MAGENTA) || (collision.etatColl & COLL_BOULE_BLEUE) || (collision.etatColl & COLL_DECOR))
		{
			images[BOULE_VERTE].position[0].y -= savePosVerte;
			savePosVerte=0;
			*pSautEnCoursVerte=false;
			timerVerte=true;
		}
	}

	return 0;
}

int Gravite(Sprite images[], Map *pMap, unsigned char sautEnCoursBleue, unsigned char sautEnCoursVerte)
{
	int i=0, v_gx = Arrondir(0.0085*Hauteur); //Compteur + variable de vitesse de la gravit�
	Collision collision= {COLL_NONE, 0};	//Structure pour g�rer les collisions

	/* On applique la gravit� si la boule n'est pas en train de sauter */
	if(!sautEnCoursBleue)
	{
		images[BOULE_BLEUE].position[0].y += v_gx;	//On applique la gravit�

		CollisionDetect(images, BOULE_BLEUE, pMap, &collision);	//D�tection des collisions

		/* On annule le d�placement de gravit� si on touche le d�cor ou une des 2 autres boules */
		if((collision.etatColl & COLL_DECOR) || (collision.etatColl & COLL_BOULE_MAGENTA) || (collision.etatColl & COLL_BOULE_VERTE))
		{
			images[BOULE_BLEUE].position[0].y -= v_gx;

			/* Pour �viter que la boule ne flotte dans l'air, on r�applique la gravit� pixel par pixel */
			for (i=1; i<v_gx; i++)
			{
				images[BOULE_BLEUE].position[0].y += 1;

				CollisionDetect(images, BOULE_BLEUE, pMap, &collision);

				if((collision.etatColl & COLL_DECOR) || (collision.etatColl & COLL_BOULE_MAGENTA) || (collision.etatColl & COLL_BOULE_VERTE))
				{
					images[BOULE_BLEUE].position[0].y -= 1;
					break;
				}
			}
		}
	}

	if(!sautEnCoursVerte)
	{
		images[BOULE_VERTE].position[0].y -= v_gx;	//Gravit� invers�e

		CollisionDetect(images, BOULE_VERTE, pMap, &collision);	//D�tection des collisions

		/* On annule le d�placement de gravit� si on touche le d�cor ou une des 2 autres boules */
		if((collision.etatColl & COLL_DECOR) || (collision.etatColl & COLL_BOULE_MAGENTA) || (collision.etatColl & COLL_BOULE_BLEUE))
		{
			images[BOULE_VERTE].position[0].y += v_gx;

			/* Pour �viter que la boule ne flotte dans l'air, on r�applique la gravit� pixel par pixel */
			for (i=1; i<v_gx; i++)
			{
				images[BOULE_VERTE].position[0].y -= 1;

				CollisionDetect(images, BOULE_VERTE, pMap, &collision);

				if((collision.etatColl & COLL_DECOR) || (collision.etatColl & COLL_BOULE_MAGENTA) || (collision.etatColl & COLL_BOULE_BLEUE))
				{
					images[BOULE_VERTE].position[0].y += 1;
					break;
				}
			}
		}
	}

	return 0;
}

int DeplacementMissiles(Sprite images[], unsigned char descente[])
{
	int i=0;	//Compteur

	/* On d�place les 5 missiles verticaux */
	for(i=0; i<5; i++)
	{
		/* On v�rifie que, pour la descente, le missile ne sort pas de l'�cran */
		if(images[MISSILE].position[i].y < (Hauteur-images[MISSILE].position[i].h) && descente[i])
		{
			images[MISSILE].position[i].y +=  Arrondir(0.003*Largeur);

			/* S'il sort apr�s le d�placement, on bascule en mont�e */
			if (images[MISSILE].position[i].y >= (Hauteur-images[MISSILE].position[i].h))
			{
				images[MISSILE].position[i].y = (Hauteur-images[MISSILE].position[i].h);
				descente[i] = false;
			}
		}
		/* On v�rifie que, pour la mont�e, le missile ne sort pas de l'�cran */
		else if((images[MISSILE].position[i].y > 0) && !descente[i])
		{
			images[MISSILE].position[i].y -= Arrondir(0.003*Largeur);

			/* S'il sort apr�s le d�placement, on bascule en descente */
			if (images[MISSILE].position[i].y <= 0)
			{
				images[MISSILE].position[i].y = 0;
				descente[i] = true;
			}
		}
	}

	/* Puis on d�place les 5 missiles horizontaux */
	for(i=5; i<10; i++)
	{
		/* On v�rifie que, pour aller vers la droite, le missile ne sort pas de l'�cran */
		if(images[MISSILE].position[i].x < Largeur && descente[i])
		{
			images[MISSILE].position[i].x +=  Arrondir(0.003*Largeur);

			/* S'il sort apr�s le d�placement, on inverse le sens */
			if (images[MISSILE].position[i].x >= Largeur)
			{
				images[MISSILE].position[i].x = Largeur;
				descente[i] = false;
			}
		}
		/* On v�rifie que, pour aller vers la gauche, le missile ne sort pas de l'�cran */
		else if((images[MISSILE].position[i].x > 0) && !descente[i])
		{
			images[MISSILE].position[i].x -= Arrondir(0.003*Largeur);

			/* S'il sort apr�s le d�placement, on inverse le sens */
			if (images[MISSILE].position[i].x - images[MISSILE].position[i].h <= 0)
			{
				images[MISSILE].position[i].x = images[MISSILE].position[i].h;
				descente[i] = true;
			}
		}
	}

	return 0;
}

int DeplacementBoules(Sprite images[], Map *pMap, ClavierSouris *pEntrees, FMOD_SYSTEM *pMoteurSon, Sons *pSons)
{
	int i=0, enLecture=false;	//Compteur + variable pour contr�ler l'�tat de lecture
	FMOD_CHANNEL *pChannelEnCours=NULL;	//Pour contr�ler la musique
	Collision collision= {COLL_NONE, 0};	//Structure pour g�rer les collisions
	static int v_x1=0, v_y1=0, v_x2=0, v_x3=0;	// Variables de vitesse

	/* On augmente progressivement la vitesse de la boule magenta dans une certaine limite */
	if(pEntrees->clavier[HAUT] && v_y1 > Arrondir(-0.002*Largeur))
	{
		v_y1--;
	}
	else if(pEntrees->clavier[BAS] && v_y1 < Arrondir(0.002*Largeur))
	{
		v_y1++;
	}
	else if(!(pEntrees->clavier[HAUT] || pEntrees->clavier[BAS]))	//Si on appuie sur aucune touche on remet � z�ro la vitesse
	{
		v_y1 = 0;
	}

	/* On applique la vitesse */
	images[BOULE_MAGENTA].position[0].y += v_y1;

	CollisionDetect(images, BOULE_MAGENTA, pMap, &collision);	//On d�tecte les collisions

	/* S'il y a une collision avec une autre boule, le d�cor ou le bord de la fen�tre en haut ou en bas */
	if ((collision.etatColl & COLL_BOULE_BLEUE) || (collision.etatColl & COLL_BOULE_VERTE) || (collision.etatColl & COLL_DECOR) || (collision.etatColl & COLL_BORD_HAUT) || (collision.etatColl & COLL_BORD_BAS))
	{
		/* Si c'est avec une autre boules, on joue le son de collision entre boules */
		if((collision.etatColl & COLL_BOULE_BLEUE) || (collision.etatColl & COLL_BOULE_VERTE))
		{
			if(BSons)
			{
				FMOD_System_GetChannel(pMoteurSon, S_BOULE_BOULE+10, &pChannelEnCours);
				FMOD_Channel_IsPlaying(pChannelEnCours, &enLecture);

				if (!enLecture)
				{
					FMOD_System_PlaySound(pMoteurSon, S_BOULE_BOULE+10, pSons->bruits[S_BOULE_BOULE], false, NULL);
				}
			}
		}

		/* On annule le d�placement car il y eu collision */
		images[BOULE_MAGENTA].position[0].y -= v_y1;

		/* Si la vitesse �tait positive (vers le bas) */
		if (v_y1 > 0)
		{
			/* On tente de se coller � l'obstacle, pixel par pixel */
			for (i=1; i<v_y1; i++)
			{
				images[BOULE_MAGENTA].position[0].y += 1;

				CollisionDetect(images, BOULE_MAGENTA, pMap, &collision);

				if ((collision.etatColl & COLL_BOULE_BLEUE) || (collision.etatColl & COLL_BOULE_VERTE) || (collision.etatColl & COLL_DECOR) || (collision.etatColl & COLL_BORD_HAUT) || (collision.etatColl & COLL_BORD_BAS))
				{
					images[BOULE_MAGENTA].position[0].y -= 1;
					break;
				}
			}
		}
		else	//Sinon si la vitesse �tait n�gative (vers le haut)
		{
			/* On tente de se coller � l'obstacle, pixel par pixel */
			for (i=1; i>v_y1; i--)
			{
				images[BOULE_MAGENTA].position[0].y -= 1;

				CollisionDetect(images, BOULE_MAGENTA, pMap, &collision);

				if ((collision.etatColl & COLL_BOULE_BLEUE) || (collision.etatColl & COLL_BOULE_VERTE) || (collision.etatColl & COLL_DECOR) || (collision.etatColl & COLL_BORD_HAUT) || (collision.etatColl & COLL_BORD_BAS))
				{
					images[BOULE_MAGENTA].position[0].y += 1;
					break;
				}
			}
		}
	}

	/* On augmente la vitesse de la boule magenta progressivement (pour l'axe x cette fois) */
	if(pEntrees->clavier[GAUCHE] && v_x1 > Arrondir(-0.002*Largeur))
	{
		v_x1--;
	}
	else if(pEntrees->clavier[DROITE] && v_x1 < Arrondir(0.002*Largeur))
	{
		v_x1++;
	}
	else if(!(pEntrees->clavier[GAUCHE] || pEntrees->clavier[DROITE]))	//Si on appuie sur aucune touche on remet � z�ro la vitesse
	{
		v_x1 = 0;
	}

	/* On applique le d�placement */
	images[BOULE_MAGENTA].position[0].x += v_x1;

	CollisionDetect(images, BOULE_MAGENTA, pMap, &collision);	//D�tection des collisions

	/* S'il y a une collision � droite, � gauche, avec une des 2 autres boules ou avec le d�cor */
	if ((collision.etatColl & COLL_BORD_GAUCHE) || (collision.etatColl & COLL_BORD_DROIT) || (collision.etatColl & COLL_BOULE_BLEUE) || (collision.etatColl & COLL_BOULE_VERTE) || (collision.etatColl & COLL_DECOR))
	{
		/* Si c'est avec une des 2 autres boules, on joue le son de collision entre boules */
		if((collision.etatColl & COLL_BOULE_BLEUE) || (collision.etatColl & COLL_BOULE_VERTE))
		{
			if(BSons)
			{
				FMOD_System_GetChannel(pMoteurSon, S_BOULE_BOULE+10, &pChannelEnCours);
				FMOD_Channel_IsPlaying(pChannelEnCours, &enLecture);

				if (!enLecture)
				{
					FMOD_System_PlaySound(pMoteurSon, S_BOULE_BOULE+10, pSons->bruits[S_BOULE_BOULE], false, NULL);
				}
			}
		}

		/* On annule le d�placement */
		images[BOULE_MAGENTA].position[0].x -= v_x1;

		/* La vitesse est positive (vers la droite) */
		if (v_x1 > 0)
		{
			/* On tente de s'approcher pixel par pixel de l'obstacle */
			for (i=1; i<v_x1; i++)
			{
				images[BOULE_MAGENTA].position[0].x += 1;

				CollisionDetect(images, BOULE_MAGENTA, pMap, &collision);

				if ((collision.etatColl & COLL_BORD_GAUCHE) || (collision.etatColl & COLL_BORD_DROIT) || (collision.etatColl & COLL_BOULE_BLEUE) || (collision.etatColl & COLL_BOULE_VERTE) || (collision.etatColl & COLL_DECOR))
				{
					images[BOULE_MAGENTA].position[0].x -= 1;
					break;
				}
			}
		}
		else	//Sinon vitesse n�gative (vers la gauche)
		{
			/* On tente de s'approcher pixel par pixel de l'obstacle */
			for (i=1; i>v_x1; i--)
			{
				images[BOULE_MAGENTA].position[0].x -= 1;

				CollisionDetect(images, BOULE_MAGENTA, pMap, &collision);

				if ((collision.etatColl & COLL_BORD_GAUCHE) || (collision.etatColl & COLL_BORD_DROIT) || (collision.etatColl & COLL_BOULE_BLEUE) || (collision.etatColl & COLL_BOULE_VERTE) || (collision.etatColl & COLL_DECOR))
				{
					images[BOULE_MAGENTA].position[0].x += 1;
					break;
				}
			}
		}
	}

	/* On commence par augmenter progressivement la vitesse de la boule bleue (en x) dans la limite d�finie par la pr�sence de bonus */
	if (infos.bonus & BONUS_VITESSE_BLEUE_FORT)
	{
		if(pEntrees->clavier[Q] && v_x2 > Arrondir(-0.002*Largeur*1.8))
		{
			v_x2--;
		}
		else if(pEntrees->clavier[D] && v_x2 < Arrondir(0.002*Largeur*1.8))
		{
			v_x2++;
		}
		else if(!(pEntrees->clavier[Q] || pEntrees->clavier[D]))
		{
			v_x2 = 0;
		}
	}
	else if (infos.bonus & BONUS_VITESSE_BLEUE_FAIBLE)
	{
		if(pEntrees->clavier[Q] && v_x2 > Arrondir(-0.002*Largeur*1.5))
		{
			v_x2--;
		}
		else if(pEntrees->clavier[D] && v_x2 < Arrondir(0.002*Largeur*1.5))
		{
			v_x2++;
		}
		else if(!(pEntrees->clavier[Q] || pEntrees->clavier[D]))
		{
			v_x2 = 0;
		}
	}
	else
	{
		if(pEntrees->clavier[Q] && v_x2 > Arrondir(-0.002*Largeur))
		{
			v_x2--;
		}
		else if(pEntrees->clavier[D] && v_x2 < Arrondir(0.002*Largeur))
		{
			v_x2++;
		}
		else if(!(pEntrees->clavier[Q] || pEntrees->clavier[D]))
		{
			v_x2 = 0;
		}
	}

	/* D�placement */
	images[BOULE_BLEUE].position[0].x += v_x2;

	CollisionDetect(images, BOULE_BLEUE, pMap, &collision);	//D�tection des collisions

	/* S'il y a une collision avec le bord gauche, droit, le d�cor ou une des deux autres boules */
	if ((collision.etatColl & COLL_BORD_GAUCHE) || (collision.etatColl & COLL_BORD_DROIT) || (collision.etatColl & COLL_BOULE_MAGENTA) || (collision.etatColl & COLL_BOULE_VERTE) || (collision.etatColl & COLL_DECOR))
	{
		/* Si c'est avec une des 2 autres boules, on joue le son de collision entre boules */
		if((collision.etatColl & COLL_BOULE_MAGENTA) || (collision.etatColl & COLL_BOULE_VERTE))
		{
			if(BSons)
			{
				FMOD_System_GetChannel(pMoteurSon, S_BOULE_BOULE+10, &pChannelEnCours);
				FMOD_Channel_IsPlaying(pChannelEnCours, &enLecture);

				if (!enLecture)
				{
					FMOD_System_PlaySound(pMoteurSon, S_BOULE_BOULE+10, pSons->bruits[S_BOULE_BOULE], false, NULL);
				}
			}
		}

		/* On annule le d�placement */
		images[BOULE_BLEUE].position[0].x -= v_x2;

		/* Si la vitesse est positive (vers la droite) */
		if (v_x2 > 0)
		{
			/* On tente de s'approcher pixel par pixel de l'obstacle */
			for (i=1; i<v_x2; i++)
			{
				images[BOULE_BLEUE].position[0].x += 1;

				CollisionDetect(images, BOULE_BLEUE, pMap, &collision);

				if ((collision.etatColl & COLL_BORD_GAUCHE) || (collision.etatColl & COLL_BORD_DROIT) || (collision.etatColl & COLL_BOULE_MAGENTA) || (collision.etatColl & COLL_BOULE_VERTE) || (collision.etatColl & COLL_DECOR))
				{
					images[BOULE_BLEUE].position[0].x -= 1;
					break;
				}
			}
		}
		else	//Sinon la vitesse est n�gative (vers la gauche)
		{
			/* On tente de s'approcher pixel par pixel de l'obstacle */
			for (i=1; i>v_x2; i--)
			{
				images[BOULE_BLEUE].position[0].x -= 1;

				CollisionDetect(images, BOULE_BLEUE, pMap, &collision);

				if ((collision.etatColl & COLL_BORD_GAUCHE) || (collision.etatColl & COLL_BORD_DROIT) || (collision.etatColl & COLL_BOULE_MAGENTA) || (collision.etatColl & COLL_BOULE_VERTE) || (collision.etatColl & COLL_DECOR))
				{
					images[BOULE_BLEUE].position[0].x += 1;
					break;
				}
			}
		}
	}

	/* On commence par augmenter progressivement la vitesse de la boule verte (en x) dans la limite d�finie par la pr�sence de bonus */
	if (infos.bonus & BONUS_VITESSE_VERTE_FORT)
	{
		if(pEntrees->clavier[J] && v_x3 > Arrondir(-0.002*Largeur*1.8))
		{
			v_x3--;
		}
		else if(pEntrees->clavier[L] && v_x3 < Arrondir(0.002*Largeur*1.8))
		{
			v_x3++;
		}
		else if(!(pEntrees->clavier[J] || pEntrees->clavier[L]))
		{
			v_x3 = 0;
		}
	}
	else if (infos.bonus & BONUS_VITESSE_VERTE_FAIBLE)
	{
		if(pEntrees->clavier[J] && v_x3 > Arrondir(-0.002*Largeur*1.5))
		{
			v_x3--;
		}
		else if(pEntrees->clavier[L] && v_x3 < Arrondir(0.002*Largeur*1.5))
		{
			v_x3++;
		}
		else if(!(pEntrees->clavier[J] || pEntrees->clavier[L]))
		{
			v_x3 = 0;
		}
	}
	else
	{
		if(pEntrees->clavier[J] && v_x3 > Arrondir(-0.002*Largeur))
		{
			v_x3--;
		}
		else if(pEntrees->clavier[L] && v_x3 < Arrondir(0.002*Largeur))
		{
			v_x3++;
		}
		else if(!(pEntrees->clavier[J] || pEntrees->clavier[L]))
		{
			v_x3 = 0;
		}
	}

	/* D�placement */
	images[BOULE_VERTE].position[0].x += v_x3;

	CollisionDetect(images, BOULE_VERTE, pMap, &collision);	//D�tection des collisions

	/* S'il y a une collision avec le bord gauche, droit, le d�cor ou une des deux autres boules */
	if ((collision.etatColl & COLL_BORD_GAUCHE) || (collision.etatColl & COLL_BORD_DROIT) || (collision.etatColl & COLL_BOULE_MAGENTA) || (collision.etatColl & COLL_BOULE_BLEUE) || (collision.etatColl & COLL_DECOR))
	{
		/* Si c'est avec une des 2 autres boules, on joue le son de collision entre boules */
		if((collision.etatColl & COLL_BOULE_MAGENTA) || (collision.etatColl & COLL_BOULE_BLEUE))
		{
			if(BSons)
			{
				FMOD_System_GetChannel(pMoteurSon, S_BOULE_BOULE+10, &pChannelEnCours);
				FMOD_Channel_IsPlaying(pChannelEnCours, &enLecture);

				if (!enLecture)
				{
					FMOD_System_PlaySound(pMoteurSon, S_BOULE_BOULE+10, pSons->bruits[S_BOULE_BOULE], false, NULL);
				}
			}
		}

		/* On annule le d�placement */
		images[BOULE_VERTE].position[0].x -= v_x3;

		/* Si la vitesse est positive (vers la droite) */
		if (v_x3 > 0)
		{
			for (i=1; i<v_x3; i++)
			{
				images[BOULE_VERTE].position[0].x += 1;

				CollisionDetect(images, BOULE_VERTE, pMap, &collision);

				if ((collision.etatColl & COLL_BORD_GAUCHE) || (collision.etatColl & COLL_BORD_DROIT) || (collision.etatColl & COLL_BOULE_MAGENTA) || (collision.etatColl & COLL_BOULE_BLEUE) || (collision.etatColl & COLL_DECOR))
				{
					images[BOULE_VERTE].position[0].x -= 1;
					break;
				}
			}
		}
		else	//Sinon la vitesse est n�gative (vers la gauche)
		{
			for (i=1; i>v_x3; i--)
			{
				images[BOULE_VERTE].position[0].x -= 1;

				CollisionDetect(images, BOULE_VERTE, pMap, &collision);

				if ((collision.etatColl & COLL_BORD_GAUCHE) || (collision.etatColl & COLL_BORD_DROIT) || (collision.etatColl & COLL_BOULE_MAGENTA) || (collision.etatColl & COLL_BOULE_BLEUE) || (collision.etatColl & COLL_DECOR))
				{
					images[BOULE_VERTE].position[0].x += 1;
					break;
				}
			}
		}
	}

	return 0;
}

char VerifierMortOUGagne(Sprite images[], Map *pMap, FMOD_SYSTEM *pMoteurSon, Sons *pSons)
{
	/* Cette fonction permet de v�rifier si le niveau a �t� gagn� ou s'il le joueur est mort par un missile ou par une chute malheureuse */

	FMOD_CHANNEL *musicEnCours = NULL;
	Collision collision= {0, 0};
	int i=0;

	/* On v�rifie si pour toutes les boules on est rentr� dans un missile, si c'est le cas, on joue le son d'explosion du missile et on renvoie le num�ro du missile qui a explos� */
	for(i=BOULE_BLEUE; i<=BOULE_VERTE; i++)
	{
		CollisionDetect(images, i, pMap, &collision);

		if(collision.etatColl & COLL_MISSILE)
		{
			if(BSons)
			{
				FMOD_System_PlaySound(pMoteurSon, S_BOULE_BOUM+10, pSons->bruits[S_BOULE_BOUM], true, NULL);
				FMOD_System_GetChannel(pMoteurSon, S_BOULE_BOUM+10, &musicEnCours);
				FMOD_Channel_SetVolume(musicEnCours, Volume/150.0);
				FMOD_Channel_SetPaused(musicEnCours,  false);
			}

			return collision.numMissile;
		}
	}

	/* On v�rifie si la boule bleue n'est pas tomb�e en bas */
	CollisionDetect(images, BOULE_BLEUE, pMap, &collision);

	if (collision.etatColl & COLL_BORD_BAS)
	{
		if(BSons)
		{
			FMOD_System_PlaySound(pMoteurSon, S_TOMBE+10, pSons->bruits[S_TOMBE], true, NULL);
			FMOD_System_GetChannel(pMoteurSon, S_TOMBE+10, &musicEnCours);
			FMOD_Channel_SetVolume(musicEnCours, Volume/200.0);
			FMOD_Channel_SetPaused(musicEnCours,  false);
		}

		return MORT_BORDURE;	//On renvoie la mort
	}

	/* On v�rifie si la boule verte n'est pas tomb�e en haut */
	CollisionDetect(images, BOULE_VERTE, pMap, &collision);

	if (collision.etatColl & COLL_BORD_HAUT)
	{
		if(BSons)
		{
			FMOD_System_PlaySound(pMoteurSon, S_TOMBE+10, pSons->bruits[S_TOMBE], true, NULL);
			FMOD_System_GetChannel(pMoteurSon, S_TOMBE+10, &musicEnCours);
			FMOD_Channel_SetVolume(musicEnCours, Volume/200.0);
			FMOD_Channel_SetPaused(musicEnCours,  false);
		}

		return MORT_BORDURE;	//On renvoie la mort
	}

	/* Si la boule bleue est dans le vortex bleu, alors on v�rifie que la verte se trouve dans son vortex �galement */
	CollisionDetect(images, BOULE_BLEUE, pMap, &collision);

	if(collision.etatColl & COLL_VORTEX_BLEU)
	{
		CollisionDetect(images, BOULE_VERTE, pMap, &collision);

		if (collision.etatColl & COLL_VORTEX_VERT)
		{
			if(BSons)
			{
				FMOD_System_PlaySound(pMoteurSon, S_SORTIE+10, pSons->bruits[S_SORTIE], true, NULL);
				FMOD_System_GetChannel(pMoteurSon, S_SORTIE+10, &musicEnCours);
				FMOD_Channel_SetVolume(musicEnCours, Volume/140.0);
				FMOD_Channel_SetPaused(musicEnCours,  false);
			}

			return GAGNE;	//On renvoie qu'on a gagn� le niveau
		}
	}

	return RIEN;	//On renvoie qu'il ne se passe rien
}

int TraitementEtatDuNiveau(SDL_Renderer *pMoteurRendu, FMOD_SYSTEM *pMoteurSon, Sons *pSons, Map **ppMap, Joueur *pJoueur, Sprite images[], TTF_Font *polices[], Animation anim[], ClavierSouris *pEntrees, unsigned char descente[], int *pEtat, unsigned char *pAjoutAnim, int *pControl)
{
	/* Cette fonction va traiter la valeur renvoy�e par la fonction pr�c�dente et agir en cons�quence */

	char missileTouche =-1;	//Variable pour conna�tre le missile qui a explos�
	int etatNiveau, i=0;	//Variable pour stocker l'�tat du niveau qui vient d'�tre charg� (Chargement r�ussi, �chou�, �chou� car il n'y a plus de niveau)
	Map *pMapNew=NULL;	//Pointeur vers une structure Map
	FMOD_CHANNEL *pChannelEnCours=NULL;	//Pour g�rer les channels
	char chaine[100];	//Cha�ne pour travailler

	/* Si une des boules est tomb�e, on enl�ve de la vie, du score, on r�initialise le niveau et on d�sactive les bonus */
	if(*pEtat == MORT_BORDURE)
	{
		infos.vies--;	//On enl�ve une vie
		infos.score -= 150;	//On enl�ve du score
		infos.compteurTemps = 0;
		InitialisationPositions(images, pJoueur, infos.niveau);	//On recommence
		*ppMap = ChargementNiveau(pMoteurRendu, pJoueur, infos.niveau, &etatNiveau);
		infos.bonus &= AUCUN_BONUS;

		for(i=0; i<10; i++)
		{
			descente[i] = false;
		}
	}
	else if (*pEtat >= 0)	//Mort par un missile
	{
		/* On enl�ve de la vie, du score et on lance l'animation */
		infos.vies--;
		infos.score -= 150;
		infos.compteurTemps = 0;
		*pAjoutAnim = true;
		missileTouche = *pEtat;	//On r�cup�re le num�ro du missile qui a explos�

		/* On d�finit la taille et l'emplacement de l'animation */
		if (missileTouche >= 5)	//Missiles H
		{
			anim[ANIM_0].pos.x = Arrondir(images[MISSILE].position[missileTouche].x - (0.03 * Largeur) - images[MISSILE].position[missileTouche].h);
			anim[ANIM_0].pos.y = Arrondir(images[MISSILE].position[missileTouche].y - (0.03 * Largeur));
			anim[ANIM_0].pos.h = Arrondir(images[MISSILE].position[missileTouche].w + (0.06 * Largeur));
			anim[ANIM_0].pos.w = Arrondir(images[MISSILE].position[missileTouche].h + (0.03 * Largeur));
		}
		else	//Missile V
		{
			anim[ANIM_0].pos.x = Arrondir(images[MISSILE].position[missileTouche].x - (0.03 * Largeur));
			anim[ANIM_0].pos.y = Arrondir(images[MISSILE].position[missileTouche].y - (0.03 * Largeur));
			anim[ANIM_0].pos.h = Arrondir(images[MISSILE].position[missileTouche].h + (0.03 * Largeur));
			anim[ANIM_0].pos.w = Arrondir(images[MISSILE].position[missileTouche].w + (0.06 * Largeur));
		}

		/* On recharge le niveau, on d�sactive les bonus */
		InitialisationPositions(images, pJoueur, infos.niveau);	//On recommence
		*ppMap = ChargementNiveau(pMoteurRendu, pJoueur, infos.niveau, &etatNiveau);
		infos.bonus &= AUCUN_BONUS;

		for(i=0; i<10; i++)
		{
			descente[i] = false;
		}
	}
	else if (*pEtat == GAGNE)	//Si on a gagn� le niveau en cours
	{
		infos.score += 350; //On gagne 350 en score
		infos.score -= Arrondir(0.75*infos.compteurTemps);	//On perd 75% du temps mis pour effectuer le niveau

		/* On gagne 120 en score si on a toutes ses vies de d�part, sinon �a d�pend de combien il en reste par rapport � combien il y en avait au d�part */
		infos.score += Arrondir(infos.vies*(120.0/(double)infos.viesInitiales));
		infos.niveau++;	//Niveau suivant

		/* On charge le niveau suivant dans un autre pointeur */
		pMapNew = ChargementNiveau(pMoteurRendu, pJoueur, infos.niveau, &etatNiveau);

		if (pMapNew == NULL)	//Si on a pas r�ussi (c'�tait le dernier niveau ou il y a eu un probl�me)
		{
			if(etatNiveau == CHARGEMENT_GAGNE)	//Si c'est parce que c'�tait le dernier niveau, on a gagn�
			{
				if(BMusique)
				{
					FMOD_System_GetChannel(pMoteurSon, M_JEU, &pChannelEnCours);	//On arr�te la musique de jeu
					FMOD_Channel_SetPaused(pChannelEnCours, true);

					FMOD_System_PlaySound(pMoteurSon, M_GAGNE, pSons->music[M_GAGNE], true, NULL);	//On lance celle de fin
					FMOD_System_GetChannel(pMoteurSon, M_GAGNE, &pChannelEnCours);
					FMOD_Channel_SetVolume(pChannelEnCours, Volume/100.0);
					FMOD_Channel_SetPaused(pChannelEnCours, false);
				}

				Gagne(pMoteurRendu, images, *ppMap, polices);	//On affiche l'�cran de fin quand on a gagn�

				*pControl = JEU_FIN_GAGNE;		//On sort de la boucle principale
			}
			else if(etatNiveau == CHARGEMENT_FICHIER_CORROMPU || etatNiveau == CHARGEMENT_ERREUR) //Si c'est une erreur de chargement
			{
				*pControl = JEU_FIN_ERREUR_CHARGEMENT;	//On coupe la boucle du jeu depuis ici
			}
		}
		else	//On a charg� le niveau suivant correctement
		{
			sprintf(chaine, "Votre score est de: %ld, passage au niveau %d : %s", infos.score, infos.niveau, pMapNew->titre);
			MessageInformations(chaine, polices, pMoteurRendu, pEntrees);

			/* On ajoute le niveau pr�c�dent et son score � la liste en mode campagne */
			if(pJoueur->mode == MODE_CAMPAGNE)
			{
				sprintf(chaine, "%d:%ld;", infos.niveau-1, infos.score);
				strcat(pJoueur->autre, chaine);
			}

			DestructionMap(*ppMap);	//On lib�re la m�moire prise par l'ancienne structure Map
			*ppMap = pMapNew;	//On copie l'adresse de la nouvelle Map dans l'autre pointeur

			infos.compteurTemps = 0;	//On remet le compteur de temps � 0
			infos.score = 1000;	//On remet le score � 1000
			infos.vies = infos.viesInitiales;	//On remet la vie au maximum pour le prochain niveau

			InitialisationPositions(images, pJoueur, infos.niveau);	//On charge les positions

			*pEtat = RIEN;	//On r�initialise la valeur 'etat' pour que le nouveau niveau ne se termine pas instantan�ment

			/* On d�sactive tous les bonus, �quivalent � "infos.bonus = infos.bonus & AUCUN_BONUS". On fait un ET bit � bit avec AUCUN_BONUS = 00000000 */
			infos.bonus &= AUCUN_BONUS;

			/* On remet le tableau de bool�ens 'descente' � false pour le prochain niveau */
			for(i=0; i<10; i++)
			{
				descente[i] = false;
			}
		}
	}

	return 0;
}

void TraitementBonus(FMOD_SYSTEM *pMoteurSon, Sons *pSons)
{
	/* Cette fonction s'occupe des bonus � usage unique */

	FMOD_CHANNEL *pChannelEnCours=NULL;	//Gestion de la musique
	int enLecture=false, i=0;	//Pour savoir si la lecture d'un channel est en cours
	static int dejaActif = 0x0;

	if(BSons)
	{
		/* On v�rifie si le son des bonus n'est pas d�j� en lecture */
		FMOD_System_GetChannel(pMoteurSon, S_BONUS+10, &pChannelEnCours);
		FMOD_Channel_IsPlaying(pChannelEnCours, &enLecture);

		/* S'il ne l'est pas et que c'est un bonus � usage unique (pas de vitesse ni de saut) alors on le joue, sinon pour les bonus permanents on v�rifie, par des op�rations bit � bit, qu'ils ne sont pas d�j� actifs depuis la derni�re fois. Ca permet d'�viter de rejouer en boucle le son*/
		if(!enLecture)
		{
			if(infos.bonus & (BONUS_VIE|BONUS_SCORE_FORT|BONUS_SCORE_FAIBLE))	//Tous les bonus � usage unique
			{
				FMOD_System_PlaySound(pMoteurSon, S_BONUS+10, pSons->bruits[S_BONUS], true, NULL);
				FMOD_Channel_SetVolume(pChannelEnCours, Volume/200.0);
				FMOD_Channel_SetPaused(pChannelEnCours, false);
			}
			else if((infos.bonus & BONUS_VITESSE_BLEUE_FAIBLE) && !(dejaActif & BONUS_VITESSE_BLEUE_FAIBLE))	//Les autres
			{
				FMOD_System_PlaySound(pMoteurSon, S_BONUS+10, pSons->bruits[S_BONUS], true, NULL);
				FMOD_Channel_SetVolume(pChannelEnCours, Volume/200.0);
				FMOD_Channel_SetPaused(pChannelEnCours, false);
				dejaActif |= BONUS_VITESSE_BLEUE_FAIBLE;	//On marque qu'on a d�j� jou� le son pour ce bonus
			}
			else if((infos.bonus & BONUS_VITESSE_BLEUE_FORT) && !(dejaActif & BONUS_VITESSE_BLEUE_FORT))
			{
				FMOD_System_PlaySound(pMoteurSon, S_BONUS+10, pSons->bruits[S_BONUS], true, NULL);
				FMOD_Channel_SetVolume(pChannelEnCours, Volume/200.0);
				FMOD_Channel_SetPaused(pChannelEnCours, false);
				dejaActif |= BONUS_VITESSE_BLEUE_FORT;
			}
			else if((infos.bonus & BONUS_VITESSE_VERTE_FAIBLE) && !(dejaActif & BONUS_VITESSE_VERTE_FAIBLE))
			{
				FMOD_System_PlaySound(pMoteurSon, S_BONUS+10, pSons->bruits[S_BONUS], true, NULL);
				FMOD_Channel_SetVolume(pChannelEnCours, Volume/200.0);
				FMOD_Channel_SetPaused(pChannelEnCours, false);
				dejaActif |= BONUS_VITESSE_VERTE_FAIBLE;
			}
			else if((infos.bonus & BONUS_VITESSE_VERTE_FORT) && !(dejaActif & BONUS_VITESSE_VERTE_FORT))
			{
				FMOD_System_PlaySound(pMoteurSon, S_BONUS+10, pSons->bruits[S_BONUS], true, NULL);
				FMOD_Channel_SetVolume(pChannelEnCours, Volume/200.0);
				FMOD_Channel_SetPaused(pChannelEnCours, false);
				dejaActif |= BONUS_VITESSE_VERTE_FORT;
			}
			else if((infos.bonus & BONUS_SAUT_BLEUE_FAIBLE) && !(dejaActif & BONUS_SAUT_BLEUE_FAIBLE))
			{
				FMOD_System_PlaySound(pMoteurSon, S_BONUS+10, pSons->bruits[S_BONUS], true, NULL);
				FMOD_Channel_SetVolume(pChannelEnCours, Volume/200.0);
				FMOD_Channel_SetPaused(pChannelEnCours, false);
				dejaActif |= BONUS_SAUT_BLEUE_FAIBLE;
			}
			else if((infos.bonus & BONUS_SAUT_BLEUE_FORT) && !(dejaActif & BONUS_SAUT_BLEUE_FORT))
			{
				FMOD_System_PlaySound(pMoteurSon, S_BONUS+10, pSons->bruits[S_BONUS], true, NULL);
				FMOD_Channel_SetVolume(pChannelEnCours, Volume/200.0);
				FMOD_Channel_SetPaused(pChannelEnCours, false);
				dejaActif |= BONUS_SAUT_BLEUE_FORT;
			}
			else if((infos.bonus & BONUS_SAUT_VERTE_FAIBLE) && !(dejaActif & BONUS_SAUT_VERTE_FAIBLE))
			{
				FMOD_System_PlaySound(pMoteurSon, S_BONUS+10, pSons->bruits[S_BONUS], true, NULL);
				FMOD_Channel_SetVolume(pChannelEnCours, Volume/200.0);
				FMOD_Channel_SetPaused(pChannelEnCours, false);
				dejaActif |= BONUS_SAUT_VERTE_FAIBLE;
			}
			else if((infos.bonus & BONUS_SAUT_VERTE_FORT) && !(dejaActif & BONUS_SAUT_VERTE_FORT))
			{
				FMOD_System_PlaySound(pMoteurSon, S_BONUS+10, pSons->bruits[S_BONUS], true, NULL);
				FMOD_Channel_SetVolume(pChannelEnCours, Volume/200.0);
				FMOD_Channel_SetPaused(pChannelEnCours, false);
				dejaActif |= BONUS_SAUT_VERTE_FORT;
			}
		}

		/* Dans le cas o� il s'agirait d'un nouveau niveau, les bonus �tant remis � 0, on oublie tous ceux qu'on avait d�j� retenus. Ainsi les sons vont pouvoir �tre rejou�s dans ce nouveau niveau */
		for (i=BONUS_SAUT_VERTE_FAIBLE; i<=BONUS_VITESSE_VERTE_FORT; i <<= 1)
		{
			if (!(infos.bonus & i) && (dejaActif & i))	//Si le bonus n'est pas actuellement actif mais qu'il est marqu� comme d�j� actif, on le d�sactive
			{
				dejaActif &= ~i;
			}
		}
	}

	/* Ensuite on s'occupe des bonus � usage unique */
	if((infos.bonus & BONUS_VIE) && infos.vies < infos.viesInitiales)
	{
		infos.vies++;	//On ajoute de la vie si on est pas d�j� au maximum
		infos.bonus &= ~BONUS_VIE;
	}
	else if (infos.bonus & BONUS_VIE)
	{
		infos.score += 75;	//Sinon on gagne un peu de score
		infos.bonus &= ~BONUS_VIE;
	}

	/* Bonus de score */
	if (infos.bonus & BONUS_SCORE_FAIBLE)
	{
		infos.score += 100;
		infos.bonus &= ~BONUS_SCORE_FAIBLE;
	}

	if (infos.bonus & BONUS_SCORE_FORT)
	{
		infos.score += 220;
		infos.bonus &= ~BONUS_SCORE_FORT;
	}
}

void Chrono()
{
	/* Cette fonction augmente le compteur de temps du niveau toute les secondes */

	static unsigned int tempsChrono=0, tempsAncienChrono=0;	//Variables pour retenir les temps
	static int difference=0;	//Variable pour faire la soustraction

	tempsChrono = SDL_GetTicks();	//On prend le temps

	difference = tempsChrono - tempsAncienChrono;	//On calcul la diff�rence

	if(difference >= 1000)
	{
		infos.compteurTemps++;	// S'il s'est �coul� plus d'une seconde on augmente le compteur de 1
		tempsAncienChrono = tempsChrono;
	}
	else if(difference < 0)	//Si la diff�rence est n�gative on remet tempsAncien � z�ro (c'est � cause des variables statiques)
	{
		tempsAncienChrono = 0;
	}
}

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

int Affichage(SDL_Renderer *pMoteurRendu, Sprite images[], TTF_Font *polices[], unsigned char descente[], Map* pMap, Animation anim[], unsigned char *pAjoutAnim)
{
	SDL_Rect posFond;	//Position et taille de l'image de fond

	/* Affectation */
	posFond.h = (int)Hauteur;
	posFond.w = (int)Largeur;
	posFond.x = 0;
	posFond.y = 0;

	/* On efface l'�cran */
	SDL_SetRenderDrawColor(pMoteurRendu, 0, 0, 0, SDL_ALPHA_OPAQUE);
	SDL_RenderClear(pMoteurRendu);

	SDL_RenderCopy(pMoteurRendu, pMap->fond, NULL, &posFond);	//On copie le fond

	AffichageMap(pMoteurRendu, pMap);	//On copie la map

	AffichageBonus(pMoteurRendu, pMap, images);	//On copie les bonus

	AffichageImages(pMoteurRendu, images, anim, descente, pAjoutAnim);	//On copie les images (boules, vortex et missiles)

	AffichageVies(pMoteurRendu, images);	//On copie la vie

	AffichageTextes(pMoteurRendu, polices, images[FOND_TEXTES].pTextures[0]);	//On copie le fond pour le texte de temps et de score

	SDL_RenderPresent(pMoteurRendu);         //Mise � jour de l'�cran

	return 0;
}

int AffichageMap(SDL_Renderer *pMoteurRendu, Map *pMap)
{
	int i, j;	//Compteurs
	SDL_Rect pos;	//Position et taille

	/* Affectation de la taille */
	pos.h = TailleBloc;
	pos.w = TailleBloc;

	/* On parcourt la map � l'aide d'une double boucle */
	for(i=0; i< pMap->nbtiles_largeur_monde; i++)
		for(j=0; j< pMap->nbtiles_hauteur_monde; j++)
		{
			/* On affecte la position en fonction de i et j */
			pos.x = i*TailleBloc;
			pos.y = j*TailleBloc;

			/* On regarde ce qu'il y a � cet endroit et on copie ce qui correspond (il y a de nombreuses tiles inutilis�es car je n'arrive pas � g�rer leur collision pour l'instant) */
			switch(pMap->plan[i][j])
			{
			case SOL_PETITE_PENTE_G:
				SDL_RenderCopy(pMoteurRendu, pMap->tileset, &pMap->props[SOL_PETITE_PENTE_G].src, &pos);
				break;

			case SOL_PETITE_PENTE_D:
				SDL_RenderCopy(pMoteurRendu, pMap->tileset, &pMap->props[SOL_PETITE_PENTE_D].src, &pos);
				break;

			case SOL_COIN_D_1:
				SDL_RenderCopy(pMoteurRendu, pMap->tileset, &pMap->props[SOL_COIN_D_1].src, &pos);
				break;

			case SOL_COIN_D_2:
				SDL_RenderCopy(pMoteurRendu, pMap->tileset, &pMap->props[SOL_COIN_D_2].src, &pos);
				break;

			case SOL_COIN_G_1:
				SDL_RenderCopy(pMoteurRendu, pMap->tileset, &pMap->props[SOL_COIN_G_1].src, &pos);
				break;

			case SOL_COIN_G_2:
				SDL_RenderCopy(pMoteurRendu, pMap->tileset, &pMap->props[SOL_COIN_G_2].src, &pos);
				break;

			case SOL_GRANDE_PENTE_D:
				SDL_RenderCopy(pMoteurRendu, pMap->tileset, &pMap->props[SOL_GRANDE_PENTE_D].src, &pos);
				break;

			case SOL_GRANDE_PENTE_G:
				SDL_RenderCopy(pMoteurRendu, pMap->tileset, &pMap->props[SOL_GRANDE_PENTE_G].src, &pos);
				break;

			case SOL_FIN_D:
				SDL_RenderCopy(pMoteurRendu, pMap->tileset, &pMap->props[SOL_FIN_D].src, &pos);
				break;

			case SOL_FIN_G:
				SDL_RenderCopy(pMoteurRendu, pMap->tileset, &pMap->props[SOL_FIN_G].src, &pos);
				break;

			case SOL_NORMAL:
				SDL_RenderCopy(pMoteurRendu, pMap->tileset, &pMap->props[SOL_NORMAL].src, &pos);
				break;

			case SOL_PLEIN_1:
				SDL_RenderCopy(pMoteurRendu, pMap->tileset, &pMap->props[SOL_PLEIN_1].src, &pos);
				break;

			case SOL_PLEIN_2:
				SDL_RenderCopy(pMoteurRendu, pMap->tileset, &pMap->props[SOL_PLEIN_2].src, &pos);
				break;

			case SOL_PLEIN_3:
				SDL_RenderCopy(pMoteurRendu, pMap->tileset, &pMap->props[SOL_PLEIN_3].src, &pos);
				break;

			case SOL_PLEIN_4:
				SDL_RenderCopy(pMoteurRendu, pMap->tileset, &pMap->props[SOL_PLEIN_4].src, &pos);
				break;

			case SOL_PLEIN_5:
				SDL_RenderCopy(pMoteurRendu, pMap->tileset, &pMap->props[SOL_PLEIN_5].src, &pos);
				break;

			case SOL_PLEIN_UNI:
				SDL_RenderCopy(pMoteurRendu, pMap->tileset, &pMap->props[SOL_PLEIN_UNI].src, &pos);
				break;

			case VIDE:	//On ne met rien
				break;
			}
		}

	return 0;
}

int AffichageVies(SDL_Renderer *pMoteurRendu, Sprite images[])
{
	int i;	//Compteur

	/* On regarde d'abord combien il y a de vie au total, puis combien il en reste */
	switch(infos.viesInitiales)
	{
	case 1:
		switch(infos.vies)
		{
		case 1:
			SDL_RenderCopy(pMoteurRendu, images[VIE].pTextures[0], NULL, &images[VIE].position[0]);	//Une vie enti�re
			break;

		case 0:
			SDL_RenderCopy(pMoteurRendu, images[VIE].pTextures[1], NULL, &images[VIE].position[0]);	//Une vie vide
			break;
		}

		break;

	case 2:
		switch(infos.vies)
		{
		case 2:
			for (i=0; i<2; i++)
			{
				SDL_RenderCopy(pMoteurRendu, images[VIE].pTextures[0], NULL, &images[VIE].position[i]);	//2 vies enti�res
			}

			break;

		case 1:
			SDL_RenderCopy(pMoteurRendu, images[VIE].pTextures[1], NULL, &images[VIE].position[1]);	//Une vie vide
			SDL_RenderCopy(pMoteurRendu, images[VIE].pTextures[0], NULL, &images[VIE].position[0]);	//Une vie enti�re
			break;

		case 0:
			for (i=0; i<2; i++)
			{
				SDL_RenderCopy(pMoteurRendu, images[VIE].pTextures[1], NULL, &images[VIE].position[i]);	//2 vies vides
			}

			break;
		}

		break;

	case 3:
		switch(infos.vies)
		{
		case 3:
			for (i=0; i<3; i++)
			{
				SDL_RenderCopy(pMoteurRendu, images[VIE].pTextures[0], NULL, &images[VIE].position[i]);	//3 vies enti�res
			}

			break;

		case 2:
			for (i=0; i<2; i++)
			{
				SDL_RenderCopy(pMoteurRendu, images[VIE].pTextures[0], NULL, &images[VIE].position[i]);	//2 vies enti�res
			}

			SDL_RenderCopy(pMoteurRendu, images[VIE].pTextures[1], NULL, &images[VIE].position[2]);	//Une vie vide
			break;

		case 1:
			for (i=1; i<3; i++)
			{
				SDL_RenderCopy(pMoteurRendu, images[VIE].pTextures[1], NULL, &images[VIE].position[i]);	//2 vies vides
			}

			SDL_RenderCopy(pMoteurRendu, images[VIE].pTextures[0], NULL, &images[VIE].position[0]);	//Une vie enti�re
			break;

		case 0:
			for (i=0; i<3; i++)
			{
				SDL_RenderCopy(pMoteurRendu, images[VIE].pTextures[1], NULL, &images[VIE].position[i]);	//3 vies vides
			}

			break;
		}

		break;

	case 4:
		switch(infos.vies)
		{
		case 4:
			for (i=0; i<4; i++)
			{
				SDL_RenderCopy(pMoteurRendu, images[VIE].pTextures[0], NULL, &images[VIE].position[i]);	//4 vies enti�res
			}

			break;

		case 3:
			for (i=0; i<3; i++)
			{
				SDL_RenderCopy(pMoteurRendu, images[VIE].pTextures[0], NULL, &images[VIE].position[i]);	//3 vies enti�res
			}

			SDL_RenderCopy(pMoteurRendu, images[VIE].pTextures[1], NULL, &images[VIE].position[3]);	//Une vie vide
			break;

		case 2:
			for (i=0; i<2; i++)
			{
				SDL_RenderCopy(pMoteurRendu, images[VIE].pTextures[0], NULL, &images[VIE].position[i]);	//2 vies enti�res
			}

			for (i=2; i<4; i++)
			{
				SDL_RenderCopy(pMoteurRendu, images[VIE].pTextures[1], NULL, &images[VIE].position[i]);	//2 vies vides
			}

			break;

		case 1:
			for (i=1; i<4; i++)
			{
				SDL_RenderCopy(pMoteurRendu, images[VIE].pTextures[1], NULL, &images[VIE].position[i]);	//3 vies vides
			}

			SDL_RenderCopy(pMoteurRendu, images[VIE].pTextures[0], NULL, &images[VIE].position[0]);	//Une vie enti�re
			break;

		case 0:
			for (i=0; i<4; i++)
			{
				SDL_RenderCopy(pMoteurRendu, images[VIE].pTextures[1], NULL, &images[VIE].position[i]);	//4 vies vides
			}

			break;
		}

		break;

	case 5:
		switch(infos.vies)
		{
		case 5:
			for (i=0; i<5; i++)
			{
				SDL_RenderCopy(pMoteurRendu, images[VIE].pTextures[0], NULL, &images[VIE].position[i]);	//5 vies enti�res
			}

			break;

		case 4:
			for (i=0; i<4; i++)
			{
				SDL_RenderCopy(pMoteurRendu, images[VIE].pTextures[0], NULL, &images[VIE].position[i]);	//4 vies enti�res
			}

			SDL_RenderCopy(pMoteurRendu, images[VIE].pTextures[1], NULL, &images[VIE].position[4]);	//Une vie vide
			break;

		case 3:
			for (i=0; i<3; i++)
			{
				SDL_RenderCopy(pMoteurRendu, images[VIE].pTextures[0], NULL, &images[VIE].position[i]);	//3 vies enti�res
			}

			for (i=3; i<5; i++)
			{
				SDL_RenderCopy(pMoteurRendu, images[VIE].pTextures[1], NULL, &images[VIE].position[i]);	//2 vies vides
			}

			break;

		case 2:
			for (i=0; i<2; i++)
			{
				SDL_RenderCopy(pMoteurRendu, images[VIE].pTextures[0], NULL, &images[VIE].position[i]);	//2 vies enti�res
			}

			for (i=2; i<5; i++)
			{
				SDL_RenderCopy(pMoteurRendu, images[VIE].pTextures[1], NULL, &images[VIE].position[i]);	//3 vies vides
			}

			break;

		case 1:
			for (i=1; i<5; i++)
			{
				SDL_RenderCopy(pMoteurRendu, images[VIE].pTextures[1], NULL, &images[VIE].position[i]);	//4 vies vides
			}

			SDL_RenderCopy(pMoteurRendu, images[VIE].pTextures[0], NULL, &images[VIE].position[0]);	//Une vie enti�re
			break;

		case 0:
			for (i=0; i<5; i++)
			{
				SDL_RenderCopy(pMoteurRendu, images[VIE].pTextures[1], NULL, &images[VIE].position[i]);	//5 vies vides
			}

			break;
		}

		break;
	}

	return 0;
}

int AffichageBonus(SDL_Renderer *pMoteurRendu, Map *pMap, Sprite images[])
{
	int i, j;	//Compteurs

	/* On parcourt la map des bonus avec une double boucle */
	for (i=0; i<pMap->nbtiles_largeur_monde; i++)
	{
		for (j=0; j<pMap->nbtiles_hauteur_monde; j++)
		{
			/* On affecte la position en fonction de i et j et on centre le bonus dans la case */
			images[GEMMES].position[0].x = Arrondir(i*TailleBloc + (TailleBloc * 0.3)/2.0);
			images[GEMMES].position[0].y = Arrondir(j*TailleBloc + (TailleBloc * 0.3)/2.0);

			/* Ensuite on regarde ce qu'il y a dans la case et on copie ce qui correspond */
			switch(pMap->planObjets[i][j])
			{
			case DIA_VERTCLAIR:
				SDL_RenderCopy(pMoteurRendu, images[GEMMES].pTextures[0], &images[GEMMES].position[DIA_VERTCLAIR], &images[GEMMES].position[0]);
				break;

			case DIA_NOIR:
				SDL_RenderCopy(pMoteurRendu, images[GEMMES].pTextures[0], &images[GEMMES].position[DIA_NOIR], &images[GEMMES].position[0]);
				break;

			case DIA_TURQUOISE:
				SDL_RenderCopy(pMoteurRendu, images[GEMMES].pTextures[0], &images[GEMMES].position[DIA_TURQUOISE], &images[GEMMES].position[0]);
				break;

			case DIA_JAUNE:
				SDL_RenderCopy(pMoteurRendu, images[GEMMES].pTextures[0], &images[GEMMES].position[DIA_JAUNE], &images[GEMMES].position[0]);
				break;

			case DIA_ROSE:
				SDL_RenderCopy(pMoteurRendu, images[GEMMES].pTextures[0], &images[GEMMES].position[DIA_ROSE], &images[GEMMES].position[0]);
				break;

			case DIA_ROUGE:
				SDL_RenderCopy(pMoteurRendu, images[GEMMES].pTextures[0], &images[GEMMES].position[DIA_ROUGE], &images[GEMMES].position[0]);
				break;

			case DIA_ORANGE:
				SDL_RenderCopy(pMoteurRendu, images[GEMMES].pTextures[0], &images[GEMMES].position[DIA_ORANGE], &images[GEMMES].position[0]);
				break;

			case DIA_BLEUCLAIR:
				SDL_RenderCopy(pMoteurRendu, images[GEMMES].pTextures[0], &images[GEMMES].position[DIA_BLEUCLAIR], &images[GEMMES].position[0]);
				break;

			case DIA_BLEUMARINE:
				SDL_RenderCopy(pMoteurRendu, images[GEMMES].pTextures[0], &images[GEMMES].position[DIA_BLEUMARINE], &images[GEMMES].position[0]);
				break;

			case DIA_VERT:
				SDL_RenderCopy(pMoteurRendu, images[GEMMES].pTextures[0], &images[GEMMES].position[DIA_VERT], &images[GEMMES].position[0]);
				break;

			case DIA_MARRON:
				SDL_RenderCopy(pMoteurRendu, images[GEMMES].pTextures[0], &images[GEMMES].position[DIA_MARRON], &images[GEMMES].position[0]);
				break;

			case DIA_BLEU:
				SDL_RenderCopy(pMoteurRendu, images[GEMMES].pTextures[0], &images[GEMMES].position[DIA_BLEU], &images[GEMMES].position[0]);
				break;

			case DIA_ROSECLAIR:
				SDL_RenderCopy(pMoteurRendu, images[GEMMES].pTextures[0], &images[GEMMES].position[DIA_ROSECLAIR], &images[GEMMES].position[0]);
				break;

			case DIA_VIOLET:
				SDL_RenderCopy(pMoteurRendu, images[GEMMES].pTextures[0], &images[GEMMES].position[DIA_VIOLET], &images[GEMMES].position[0]);
				break;

			case DIA_FUSHIA:
				SDL_RenderCopy(pMoteurRendu, images[GEMMES].pTextures[0], &images[GEMMES].position[DIA_FUSHIA], &images[GEMMES].position[0]);
				break;

			case DIA_MARRONCLAIR:
				SDL_RenderCopy(pMoteurRendu, images[GEMMES].pTextures[0], &images[GEMMES].position[DIA_MARRONCLAIR], &images[GEMMES].position[0]);
				break;

			case DIA_GRIS:
				SDL_RenderCopy(pMoteurRendu, images[GEMMES].pTextures[0], &images[GEMMES].position[DIA_GRIS], &images[GEMMES].position[0]);
				break;

			case DIA_BLEUFONCE:
				SDL_RenderCopy(pMoteurRendu, images[GEMMES].pTextures[0], &images[GEMMES].position[DIA_BLEUFONCE], &images[GEMMES].position[0]);
				break;
			}
		}
	}

	return 0;
}

int AffichageImages(SDL_Renderer *pMoteurRendu, Sprite images[], Animation anim[], unsigned char descente[], unsigned char *pAjoutAnim)
{
	static unsigned int angleVortex=360;	//Angle pour faire tourner les vortex
	int i=0;	//Compteur
	static const SDL_Point pointOrigine= {0, 0};	//Point � partir duquel faire la rotation (0;0)

	angleVortex -= 10;	//On diminue l'angle du vortex pour le faire tourner

	/* S'il arrive � z�ro (ou bien au dessus de 360� car ici la variable est non-sign�e, soit 0-1 = 4�294�967�296) on le remet � 360 */
	if (angleVortex==0 || angleVortex>360)
	{
		angleVortex=360;
	}

	/* On copie les vortex */
	SDL_RenderCopyEx(pMoteurRendu, images[VORTEX_BLEU].pTextures[0], NULL, &images[VORTEX_BLEU].position[0], angleVortex, NULL, SDL_FLIP_NONE);
	SDL_RenderCopyEx(pMoteurRendu, images[VORTEX_VERT].pTextures[0], NULL, &images[VORTEX_VERT].position[0], angleVortex, NULL, SDL_FLIP_NONE);

	for(i=0; i < MISSILE; i++)
	{
		SDL_RenderCopy(pMoteurRendu, images[i].pTextures[0], NULL, &images[i].position[0]);        //Collage des surfaces autres que les missiles
	}

	/* S'il y a une animation, on la lit */
	if(*pAjoutAnim)
	{
		if (LectureAnimation(pMoteurRendu, anim, ANIM_0) == -1)
		{
			*pAjoutAnim = false;	//Puis on d�sactive quand elle est finie
		}
	}
	else	//Sans animation, on copie les missiles
	{
		for(i=0; i<5; i++)
		{
			if(descente[i])
			{
				SDL_RenderCopyEx(pMoteurRendu, images[MISSILE].pTextures[0], NULL, &images[MISSILE].position[i], 180, NULL, SDL_FLIP_NONE);	// On retourne le missile quand il descend
			}
			else
			{
				SDL_RenderCopyEx(pMoteurRendu, images[MISSILE].pTextures[0], NULL, &images[MISSILE].position[i], 0, NULL, SDL_FLIP_NONE);
			}
		}

		for(i=5; i<10; i++)
		{
			if(descente[i])
			{
				SDL_RenderCopyEx(pMoteurRendu, images[MISSILE].pTextures[0], NULL, &images[MISSILE].position[i], 90, &pointOrigine, SDL_FLIP_NONE);	// On retourne le missile quand il va vers la droite
			}
			else
			{
				SDL_RenderCopyEx(pMoteurRendu, images[MISSILE].pTextures[0], NULL, &images[MISSILE].position[i], 90, &pointOrigine, SDL_FLIP_VERTICAL);
			}
		}
	}

	return 0;
}

int AffichageTextes(SDL_Renderer *pMoteurRendu, TTF_Font *polices[], SDL_Texture *pTextureFond)
{
	/* Cette fonction affiche les textes de compteur de temps et de score */

	char chaineTemps[50]="", chaineScore[50]="";	//Cha�nes pour contenir
	SDL_Color blancOpaque= {255, 255, 255, 255};
	static SDL_Rect posTemps, posScore, posFond;	//Diff�rentes positions et tailles
	SDL_Surface *pSurfTemps=NULL, *pSurfScore=NULL;	//Pointeurs sur des surfaces
	SDL_Texture *pTextureTemps=NULL, *pTextureScore=NULL;	//Pointeurs sur des textures

	/* On d�finit les positions */
	posFond.x = posFond.y=0;
	posTemps.x = posScore.x = Arrondir(0.008*Largeur);
	posTemps.y=0;
	posScore.y = Arrondir(0.023*Largeur);

	/* On �crit les valeurs dans les cha�nes */
	sprintf(chaineTemps, "%d", infos.compteurTemps);
	sprintf(chaineScore, "%ld", infos.score);

	/* On cr�e les surfaces, on d�finit les tailles en fonction de la longueur du texte */
	pSurfTemps = TTF_RenderText_Solid(polices[POLICE_ARIAL], chaineTemps, blancOpaque);
	TTF_SizeText(polices[POLICE_ARIAL], chaineTemps, &posTemps.w, &posTemps.h);
	posTemps.w = Arrondir(((double)posTemps.w/1280.0) * Largeur);
	posTemps.h = Arrondir(((double)posTemps.h/1280.0) * Largeur);

	pSurfScore = TTF_RenderText_Solid(polices[POLICE_ARIAL], chaineScore, blancOpaque);
	TTF_SizeText(polices[POLICE_ARIAL], chaineScore, &posScore.w, &posScore.h);
	posScore.w = Arrondir(((double)posScore.w/1280.0) * Largeur);
	posScore.h = Arrondir(((double)posScore.h/1280.0) * Largeur);

	/* On d�finit la taille du fond des textes en fonction de celle de ceux ci */
	posFond.h = posScore.y + posScore.h + Arrondir(0.008*Largeur);
	posFond.w = posScore.x + posScore.w + Arrondir(0.008*Largeur);

	/* On convertit les surfaces en textures */
	pTextureTemps = SDL_CreateTextureFromSurface(pMoteurRendu, pSurfTemps);
	pTextureScore = SDL_CreateTextureFromSurface(pMoteurRendu, pSurfScore);

	/* On copie les textures */
	SDL_RenderCopy(pMoteurRendu, pTextureFond, NULL, &posFond);
	SDL_RenderCopy(pMoteurRendu, pTextureTemps, NULL, &posTemps);
	SDL_RenderCopy(pMoteurRendu, pTextureScore, NULL, &posScore);

	/* On d�truit les surfaces */
	SDL_FreeSurface(pSurfScore);
	SDL_FreeSurface(pSurfTemps);

	/* On d�truit les textures */
	SDL_DestroyTexture(pTextureScore);
	SDL_DestroyTexture(pTextureTemps);

	return 0;
}

int InitialisationPositions(Sprite images[], Joueur *pJoueur, int level)
{
	/* Cette fonction initialise les positions et les tailles des diff�rentes images selon le mode de jeu, l'�dition d'un niveau ou la cr�ation d'un niveau */
	int i=0, j=0, k=0;	//Compteurs
	double nb;	//Nombre d�cimal
	char *c = NULL;	//Pointeur sur un caract�re pour faire des recherches

	/* Si on est en mode campagne, en mode perso ou en mode �dition on va charger depuis un fichier */
	if (pJoueur->mode == MODE_CAMPAGNE || pJoueur->mode == MODE_PERSO || (pJoueur->mode == MODE_EDITEUR && level != -1))
	{
		FILE *pFichierNiveau = NULL;
		char ligne[20] = "";

		/* Mode campagne c'est le fichier de campagne (le md5 a d�j� �t� v�rifi� lors du chargement de la map, juste avant) */
		if(pJoueur->mode == MODE_CAMPAGNE)
		{
			pFichierNiveau = fopen("ressources/level.lvl", "r");
		}
		else if(pJoueur->mode == MODE_PERSO || pJoueur->mode == MODE_EDITEUR)	//Mode perso ou �dition d'un niveau
		{
			pFichierNiveau = fopen("ressources/levelUser.lvl", "r");
		}

		/* V�rification de l'ouverture du fichier */
		if (pFichierNiveau == NULL)
		{
			return -1;
		}

		/* On passe tous les niveaux jusqu'� atteindre la ligne de celui juste avant celui que l'on souhaite charger */
		for (i=0; i<level-1; i++)
		{
			fgets(ligne, 20, pFichierNiveau);

			do
			{
				fgets(ligne, 20, pFichierNiveau);
			}
			while (strcmp(ligne, "##--##\n") != 0);
		}

		k=0;	//on remet k � 0

		/* On recherche la ligne o� les premi�res positions sont enregistr�es (celles des missiles V) */
		while(k++, strcmp(ligne, "#missileV\n") != 0)
		{
			fgets(ligne, 20, pFichierNiveau);
		}

		/* On lit les coordonn�es des 5 missiles V */
		for(i=0; i<5; i++)
		{
			fgets(ligne, 20, pFichierNiveau);

			nb = strtod(ligne, NULL);	//On convertit la cha�ne en nombre d�cimal

			/* Selon le syst�me utilis�, il faut une virgule et non un point dans la cha�ne pour former un nombre d�cimal correct */
			if(nb == 0)
			{
				c = strstr(ligne, ".");
				*c = ',';
				nb = strtod(ligne, NULL);
			}

			images[MISSILE].position[i].x = Arrondir(nb*Largeur);	//Enfin on multiplie par la largeur et on arrondit au pixel

			/* On recommence pour la coordonn�e en y */
			fgets(ligne, 20, pFichierNiveau);

			nb = strtod(ligne, NULL);

			if(nb == 0)
			{
				c = strstr(ligne, ".");
				*c = ',';
				nb = strtod(ligne, NULL);
			}

			images[MISSILE].position[i].y = Arrondir(nb*Hauteur);
		}

		/* On lit la ligne quivante qui contient normalement "#missileH\n" */
		fgets(ligne, 20, pFichierNiveau);

		/* On lit les coordonn�es des 5 missiles H */
		for(i=5; i<10; i++)
		{
			fgets(ligne, 20, pFichierNiveau);

			nb = strtod(ligne, NULL);

			if(nb == 0)
			{
				c = strstr(ligne, ".");
				*c = ',';
				nb = strtod(ligne, NULL);
			}

			images[MISSILE].position[i].x = Arrondir(nb*Largeur);

			fgets(ligne, 20, pFichierNiveau);

			nb = strtod(ligne, NULL);

			if(nb == 0)
			{
				c = strstr(ligne, ".");
				*c = ',';
				nb = strtod(ligne, NULL);
			}

			images[MISSILE].position[i].y = Arrondir(nb*Hauteur);
		}

		/* Ensuite on v�rifie bien qu'on s'appr�te � lire les coordonn�es des vortex */
		if(fgets(ligne, 20, pFichierNiveau), strcmp(ligne, "#vortex\n") != 0)
		{
			return -1;
		}

		/* Coordonn�es du vortex bleu */
		fgets(ligne, 20, pFichierNiveau);

		nb = strtod(ligne, NULL);

		if(nb == 0)
		{
			c = strstr(ligne, ".");
			*c = ',';
			nb = strtod(ligne, NULL);
		}

		images[VORTEX_BLEU].position[0].x = Arrondir(nb*Largeur);

		fgets(ligne, 20, pFichierNiveau);

		nb = strtod(ligne, NULL);

		if(nb == 0)
		{
			c = strstr(ligne, ".");
			*c = ',';
			nb = strtod(ligne, NULL);
		}

		images[VORTEX_BLEU].position[0].y = Arrondir(nb*Hauteur);

		/* Coordonn�es du vortex vert */

		fgets(ligne, 20, pFichierNiveau);

		nb = strtod(ligne, NULL);

		if(nb == 0)
		{
			c = strstr(ligne, ".");
			*c = ',';
			nb = strtod(ligne, NULL);
		}

		images[VORTEX_VERT].position[0].x = Arrondir(nb*Largeur);

		fgets(ligne, 20, pFichierNiveau);

		nb = strtod(ligne, NULL);

		if(nb == 0)
		{
			c = strstr(ligne, ".");
			*c = ',';
			nb = strtod(ligne, NULL);
		}

		images[VORTEX_VERT].position[0].y = Arrondir(nb*Hauteur);

		/* Ensuite on v�rifie que c'est bien les boules */
		if (fgets(ligne, 20, pFichierNiveau), strcmp(ligne, "#boules\n") != 0)
		{
			return -1;
		}

		/* Coordonn�es de la boule bleue */
		fgets(ligne, 20, pFichierNiveau);

		nb = strtod(ligne, NULL);

		if(nb == 0)
		{
			c = strstr(ligne, ".");
			*c = ',';
			nb = strtod(ligne, NULL);
		}

		images[BOULE_BLEUE].position[0].x = Arrondir(nb*Largeur);

		fgets(ligne, 20, pFichierNiveau);

		nb = strtod(ligne, NULL);

		if(nb == 0)
		{
			c = strstr(ligne, ".");
			*c = ',';
			nb = strtod(ligne, NULL);
		}

		images[BOULE_BLEUE].position[0].y = Arrondir(nb*Hauteur);

		/* Coordonn�es de la boule magenta */
		fgets(ligne, 20, pFichierNiveau);

		nb = strtod(ligne, NULL);

		if(nb == 0)
		{
			c = strstr(ligne, ".");
			*c = ',';
			nb = strtod(ligne, NULL);
		}

		images[BOULE_MAGENTA].position[0].x = Arrondir(nb*Largeur);

		fgets(ligne, 20, pFichierNiveau);

		nb = strtod(ligne, NULL);

		if(nb == 0)
		{
			c = strstr(ligne, ".");
			*c = ',';
			nb = strtod(ligne, NULL);
		}

		images[BOULE_MAGENTA].position[0].y = Arrondir(nb*Hauteur);

		/* Coordonn�es de la boule verte */
		fgets(ligne, 20, pFichierNiveau);

		nb = strtod(ligne, NULL);

		if(nb == 0)
		{
			c = strstr(ligne, ".");
			*c = ',';
			nb = strtod(ligne, NULL);
		}

		images[BOULE_VERTE].position[0].x = Arrondir(nb*Largeur);

		fgets(ligne, 20, pFichierNiveau);

		nb = strtod(ligne, NULL);

		if(nb == 0)
		{
			c = strstr(ligne, ".");
			*c = ',';
			nb = strtod(ligne, NULL);
		}

		images[BOULE_VERTE].position[0].y = Arrondir(nb*Hauteur);

		/* On ferme le fichier */
		fclose(pFichierNiveau);

		/* On d�finit les positions des vies dans les deux modes de jeu */
		if(pJoueur->mode == MODE_CAMPAGNE || pJoueur->mode == MODE_PERSO)
		{
			for(i=0; i<infos.viesInitiales; i++)
			{
				images[VIE].position[i].x = Arrondir(Largeur - (double)(i+1)*0.05*Largeur);
				images[VIE].position[i].y = 0;
			}
		}

		/* Tailles des vies */
		for (i=0; i<infos.viesInitiales; i++)
		{
			images[VIE].position[i].h = images[VIE].position[i].w = Arrondir(0.04*Largeur);
		}
	}
	else if(pJoueur->mode == MODE_EDITEUR && level == -1)	//Sinon si on est en mode �diteur et qu'on ajoute un nouveau niveau
	{
		for(i=0; i<10; i++)	//On cache les missiles en dehors de la fen�tre
		{
			images[MISSILE].position[i].x =	images[MISSILE].position[i].y = -2000;
		}

		/* On d�finit les positions par d�faut des boules et des vortex */
		images[VORTEX_BLEU].position[0].x = Arrondir(0.925*Largeur);
		images[VORTEX_BLEU].position[0].y = Arrondir(images[AJOUTER_MISSILE_V].position[0].y + 0.08*Hauteur);
		images[VORTEX_VERT].position[0].x = Arrondir(0.925*Largeur);
		images[VORTEX_VERT].position[0].y = Arrondir(images[VORTEX_BLEU].position[0].y+ (double)TailleBoule*2.0 + 0.08*Hauteur);
		images[BOULE_BLEUE].position[0].x = Arrondir(0.925*Largeur);
		images[BOULE_BLEUE].position[0].y = Arrondir(images[VORTEX_VERT].position[0].y + (double)TailleBoule*2.0 + 0.08*Hauteur);
		images[BOULE_MAGENTA].position[0].x = Arrondir(0.925*Largeur);
		images[BOULE_MAGENTA].position[0].y = Arrondir(images[BOULE_BLEUE].position[0].y + (double)TailleBoule + 0.08*Hauteur);
		images[BOULE_VERTE].position[0].x = Arrondir(0.925*Largeur);
		images[BOULE_VERTE].position[0].y = Arrondir(images[BOULE_MAGENTA].position[0].y + (double)TailleBoule + 0.08*Hauteur);
	}

	if(pJoueur->mode == MODE_EDITEUR)	//Pour tous les �diteurs (ajout ou �dition) on place les boutons et les gemmes bonus
	{
		/* On d�finit les positions des boutons d'ajout de missiles */
		images[AJOUTER_MISSILE_H].position[0].x = Arrondir(0.900*Largeur);
		images[AJOUTER_MISSILE_H].position[0].y = Arrondir(0.055*Hauteur);
		images[AJOUTER_MISSILE_V].position[0].x = Arrondir(0.900*Largeur);
		images[AJOUTER_MISSILE_V].position[0].y = Arrondir(0.120*Hauteur);

		for (k=1, i=0; i<3; i++)
		{
			for (j=0; j<6; j++, k++)
			{
				images[GEMMES].position[k].x = 91*j;
				images[GEMMES].position[k].y = 78*i;
			}
		}

		/* Tailles du curseur */
		images[CURSEUR].position[0].w = Arrondir(0.02*Largeur);
		images[CURSEUR].position[0].h = Arrondir(0.027*Largeur);
	}

	/* Initialisation des tailles des boules */
	images[BOULE_BLEUE].position[0].h = TailleBoule;
	images[BOULE_BLEUE].position[0].w = TailleBoule;
	images[BOULE_MAGENTA].position[0].h = TailleBoule;
	images[BOULE_MAGENTA].position[0].w = TailleBoule;
	images[BOULE_VERTE].position[0].h = TailleBoule;
	images[BOULE_VERTE].position[0].w = TailleBoule;

	/* On initialise les tailles des missiles */
	for(i=0; i<11; i++)
	{
		images[MISSILE].position[i].h = TailleMissileH;
		images[MISSILE].position[i].w = TailleMissileW;
	}

	/* Celles des vortex */
	images[VORTEX_BLEU].position[0].w = Arrondir(TailleBloc*1.7);
	images[VORTEX_BLEU].position[0].h = Arrondir(TailleBloc*1.7);
	images[VORTEX_VERT].position[0].w = Arrondir(TailleBloc*1.7);
	images[VORTEX_VERT].position[0].h = Arrondir(TailleBloc*1.7);

	/* Celles des bonus */
	images[GEMMES].position[0].w = Arrondir(TailleBloc - (TailleBloc * 0.3));
	images[GEMMES].position[0].h = Arrondir(TailleBloc - (TailleBloc * 0.3));

	/* Tailles des bonus pour le d�coupage sur l'image originale */
	for (i=0; i<18; i++)
	{
		images[GEMMES].position[i+1].h = 78;
		images[GEMMES].position[i+1].w = 91;
	}

	return 0;
}

int SautBleue(SDL_Rect *pPosition, unsigned char *pSautEnCours)
{
	/* Cette fonction renvoie les valeurs successives de sauts pour la boule bleue d'apr�s une formule de physique */

	/* Les variables sont statiques pour que le saut se poursuive d'appel en appel */
	static int positionRelative=0, positionRelativeAncienne=0, positionFinale=0;
	static int positionInitiale=0, temps=0;
	double vitesse_initiale=0.0;

	/* La vitesse initiale de la boule est assign�e en fonction des bonus */
	if(infos.bonus & BONUS_SAUT_BLEUE_FORT)
	{
		vitesse_initiale=1.4;
	}
	else if(infos.bonus & BONUS_SAUT_BLEUE_FAIBLE)
	{
		vitesse_initiale=1.25;
	}
	else
	{
		vitesse_initiale=1.0;
	}

	/* Si c'est la premi�re fois (= commencement du saut), on retient la position � laquelle la boule doit revenir en retombant, et on met le bool�en � 'true' pour que le saut se poursuive ensuite de lui m�me */
	if(*pSautEnCours == false)
	{
		positionInitiale = pPosition->y;
		temps = positionRelativeAncienne = 0;
		*pSautEnCours = true;
	}

	/* On calcule la position relative � celle de base (sur l'axe y) par la formule : -(G/2)*t� + vx*t + 0, que l'on divise par 600 puis que l'on multiplie par la hauteur pour faire la mise � l'�chelle selon la r�solution choisie */
	positionRelative = Arrondir(((double)((vitesse_initiale * temps)-((_G_ * temps * temps)/2000)) /600.0)*Hauteur);

	/* On calcule de combien de pixels la boule doit finalement mont�e en soustrayant de combien elle est d�j� mont�e � l'appel pr�c�dent */
	positionFinale = positionRelative - positionRelativeAncienne;
	positionRelativeAncienne = positionRelative;	//On retient cette valeur pour la soustaire lors de l'appel suivant

	if(pPosition->y - positionRelative > positionInitiale)	//Si on est revenu � notre position de d�part (ou plus bas), on coupe le saut, on remet tout � 0
	{
		temps = 0;
		*pSautEnCours = false;
		positionInitiale = 0;
		return 0;
	}

	temps += 5;	//Sinon le temps augmente de 5

	return (-positionFinale);	//On retourne - la valeur calcul�e car le rep�re de l'�cran est invers� pour l'axe en y
}

int SautVerte(SDL_Rect *pPosition, unsigned char *pSautEnCours)
{
	/* Cette fonction fait la m�me chose que la pr�c�dente, mais pour la boule verte. Elle retourne donc une valeur positive */

	static int positionRelative, positionRelativeAncienne=0, positionFinale;
	static int positionInitiale=0, temps=0;
	double vitesse_initiale;

	if(infos.bonus & BONUS_SAUT_VERTE_FORT)
	{
		vitesse_initiale=1.4;
	}
	else if(infos.bonus & BONUS_SAUT_VERTE_FAIBLE)
	{
		vitesse_initiale=1.25;
	}
	else
	{
		vitesse_initiale=1.0;
	}

	if(*pSautEnCours == false)
	{
		positionInitiale = pPosition->y;
		temps = positionRelativeAncienne =0;
		*pSautEnCours = true;
	}

	positionRelative = Arrondir(((double)((vitesse_initiale * temps)-((_G_ * temps * temps)/2000)) /600.0)*Hauteur);

	positionFinale = positionRelative - positionRelativeAncienne;
	positionRelativeAncienne = positionRelative;

	if(pPosition->y + positionRelative < positionInitiale)
	{
		temps = 0;
		*pSautEnCours = false;
		positionInitiale = 0;
		return 0;
	}

	temps += 5;

	return positionFinale;	//Valeur positive
}

int Arrondir(double nombre)
{
	/* Cette fonction arrondit (pour les pixels), elle ajoute 0.5 aux nombres positifs (-0.5 aux n�gatifs), puis tronque la partie d�cimale */
	if (nombre >= 0)
	{
		return (int)(nombre+0.5);
	}
	else
	{
		return (int)(nombre-0.5);
	}
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

	int i=0;	//Compteur

	if (indiceImage < VORTEX_BLEU)	//On v�rifie que l'on s'appr�te � tester une des 3 boules
	{
		for(i=0; i < VORTEX_BLEU; i++)	//Si c'est le cas on la teste avec les 2 autres
		{
			if (i == indiceImage)	//On ne la teste pas avec elle m�me
			{
				continue;
			}

			/* On v�rifie y */
			if((images[indiceImage].position[0].y + images[indiceImage].position[0].h -1> images[i].position[0].y) && (images[indiceImage].position[0].y < images[i].position[0].y + images[i].position[0].h -1))
			{
				/* On v�rifie x */
				if(((images[indiceImage].position[0].x + images[indiceImage].position[0].w -1) > images[i].position[0].x) && (images[indiceImage].position[0].x < images[i].position[0].x + images[i].position[0].w -1))
				{
					switch(i)	//On renvoie le flag correspondant
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
		}
	}

	/* Ensuite on v�rifie si elles touchent un missile V */
	for(i=0; i < 5; i++)
	{
		if((images[indiceImage].position[0].y + images[indiceImage].position[0].h - (0.007*Largeur) > images[MISSILE].position[i].y) && (images[indiceImage].position[0].y < images[MISSILE].position[i].y + images[MISSILE].position[i].h - (0.007*Largeur)))
		{
			if(((images[indiceImage].position[0].x + images[indiceImage].position[0].w - (0.007*Largeur)) > images[MISSILE].position[i].x) && (images[indiceImage].position[0].x < images[MISSILE].position[i].x + images[MISSILE].position[i].w - (0.007*Largeur)))
			{
				pCollision->numMissile = i;
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
				pCollision->numMissile = i;
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

void DetectionBonus (Sprite images[], int indiceImage, Map* pMap)
{
	/* Cette fonction fait une boucle sur tous les bonus, puis regarde si un des 4 coins de l'image pass�e en param�tre est sur une case avec un bonus */

	int i;	//Compteur

	for (i=DIA_VERTCLAIR; i<=DIA_BLEUFONCE; i++)
	{
		if((pMap->planObjets[((images[indiceImage].position[0].x + images[indiceImage].position[0].w) / TailleBloc)][((images[indiceImage].position[0].y + images[indiceImage].position[0].h) / TailleBloc)]) == i)
		{
			switch(i)
			{
			case DIA_VERTCLAIR:
				infos.bonus |= BONUS_VITESSE_VERTE_FAIBLE;
				pMap->planObjets[((images[indiceImage].position[0].x + images[indiceImage].position[0].w) / TailleBloc)][((images[indiceImage].position[0].y + images[indiceImage].position[0].h) / TailleBloc)] = AUCUN_BONUS;
				break;

			case DIA_VERT:
				infos.bonus |= BONUS_VITESSE_VERTE_FORT;
				pMap->planObjets[((images[indiceImage].position[0].x + images[indiceImage].position[0].w) / TailleBloc)][((images[indiceImage].position[0].y + images[indiceImage].position[0].h) / TailleBloc)] = AUCUN_BONUS;
				break;

			case DIA_MARRONCLAIR:
				infos.bonus |= BONUS_SAUT_VERTE_FAIBLE;
				pMap->planObjets[((images[indiceImage].position[0].x + images[indiceImage].position[0].w) / TailleBloc)][((images[indiceImage].position[0].y + images[indiceImage].position[0].h) / TailleBloc)] = AUCUN_BONUS;
				break;

			case DIA_MARRON:
				infos.bonus |= BONUS_SAUT_VERTE_FORT;
				pMap->planObjets[((images[indiceImage].position[0].x + images[indiceImage].position[0].w) / TailleBloc)][((images[indiceImage].position[0].y + images[indiceImage].position[0].h) / TailleBloc)] = AUCUN_BONUS;
				break;

			case DIA_JAUNE:
				infos.bonus |= BONUS_SCORE_FAIBLE;
				pMap->planObjets[((images[indiceImage].position[0].x + images[indiceImage].position[0].w) / TailleBloc)][((images[indiceImage].position[0].y + images[indiceImage].position[0].h) / TailleBloc)] = AUCUN_BONUS;
				break;

			case DIA_ORANGE:
				infos.bonus |= BONUS_SCORE_FORT;
				pMap->planObjets[((images[indiceImage].position[0].x + images[indiceImage].position[0].w) / TailleBloc)][((images[indiceImage].position[0].y + images[indiceImage].position[0].h) / TailleBloc)] = AUCUN_BONUS;
				break;

			case DIA_ROUGE:
				infos.bonus |= BONUS_VIE;
				pMap->planObjets[((images[indiceImage].position[0].x + images[indiceImage].position[0].w) / TailleBloc)][((images[indiceImage].position[0].y + images[indiceImage].position[0].h) / TailleBloc)] = AUCUN_BONUS;
				break;

			case DIA_BLEUCLAIR:
				infos.bonus |= BONUS_SAUT_BLEUE_FAIBLE;
				pMap->planObjets[((images[indiceImage].position[0].x + images[indiceImage].position[0].w) / TailleBloc)][((images[indiceImage].position[0].y + images[indiceImage].position[0].h) / TailleBloc)] = AUCUN_BONUS;
				break;

			case DIA_BLEUFONCE:
				infos.bonus |= BONUS_SAUT_BLEUE_FORT;
				pMap->planObjets[((images[indiceImage].position[0].x + images[indiceImage].position[0].w) / TailleBloc)][((images[indiceImage].position[0].y + images[indiceImage].position[0].h) / TailleBloc)] = AUCUN_BONUS;
				break;

			case DIA_BLEU:
				infos.bonus |= BONUS_VITESSE_BLEUE_FAIBLE;
				pMap->planObjets[((images[indiceImage].position[0].x + images[indiceImage].position[0].w) / TailleBloc)][((images[indiceImage].position[0].y + images[indiceImage].position[0].h) / TailleBloc)] = AUCUN_BONUS;
				break;

			case DIA_BLEUMARINE:
				infos.bonus |= BONUS_VITESSE_BLEUE_FORT;
				pMap->planObjets[((images[indiceImage].position[0].x + images[indiceImage].position[0].w) / TailleBloc)][((images[indiceImage].position[0].y + images[indiceImage].position[0].h) / TailleBloc)] = AUCUN_BONUS;
				break;
			}
		}

		if((pMap->planObjets[((images[indiceImage].position[0].x + images[indiceImage].position[0].w) / TailleBloc)][(images[indiceImage].position[0].y / TailleBloc)]) == i)
		{
			switch(i)
			{
			case DIA_VERTCLAIR:
				infos.bonus |= BONUS_VITESSE_VERTE_FAIBLE;
				pMap->planObjets[((images[indiceImage].position[0].x + images[indiceImage].position[0].w) / TailleBloc)][(images[indiceImage].position[0].y / TailleBloc)] = AUCUN_BONUS;
				break;

			case DIA_VERT:
				infos.bonus |= BONUS_VITESSE_VERTE_FORT;
				pMap->planObjets[((images[indiceImage].position[0].x + images[indiceImage].position[0].w) / TailleBloc)][(images[indiceImage].position[0].y / TailleBloc)] = AUCUN_BONUS;
				break;

			case DIA_MARRONCLAIR:
				infos.bonus |= BONUS_SAUT_VERTE_FAIBLE;
				pMap->planObjets[((images[indiceImage].position[0].x + images[indiceImage].position[0].w) / TailleBloc)][(images[indiceImage].position[0].y / TailleBloc)] = AUCUN_BONUS;
				break;

			case DIA_MARRON:
				infos.bonus |= BONUS_SAUT_VERTE_FORT;
				pMap->planObjets[((images[indiceImage].position[0].x + images[indiceImage].position[0].w) / TailleBloc)][(images[indiceImage].position[0].y / TailleBloc)] = AUCUN_BONUS;
				break;

			case DIA_JAUNE:
				infos.bonus |= BONUS_SCORE_FAIBLE;
				pMap->planObjets[((images[indiceImage].position[0].x + images[indiceImage].position[0].w) / TailleBloc)][(images[indiceImage].position[0].y / TailleBloc)] = AUCUN_BONUS;
				break;

			case DIA_ORANGE:
				infos.bonus |= BONUS_SCORE_FORT;
				pMap->planObjets[((images[indiceImage].position[0].x + images[indiceImage].position[0].w) / TailleBloc)][(images[indiceImage].position[0].y / TailleBloc)] = AUCUN_BONUS;
				break;

			case DIA_ROUGE:
				infos.bonus |= BONUS_VIE;
				pMap->planObjets[((images[indiceImage].position[0].x + images[indiceImage].position[0].w) / TailleBloc)][(images[indiceImage].position[0].y / TailleBloc)] = AUCUN_BONUS;
				break;

			case DIA_BLEUCLAIR:
				infos.bonus |= BONUS_SAUT_BLEUE_FAIBLE;
				pMap->planObjets[((images[indiceImage].position[0].x + images[indiceImage].position[0].w) / TailleBloc)][(images[indiceImage].position[0].y / TailleBloc)] = AUCUN_BONUS;
				break;

			case DIA_BLEUFONCE:
				infos.bonus |= BONUS_SAUT_BLEUE_FORT;
				pMap->planObjets[((images[indiceImage].position[0].x + images[indiceImage].position[0].w) / TailleBloc)][(images[indiceImage].position[0].y / TailleBloc)] = AUCUN_BONUS;
				break;

			case DIA_BLEU:
				infos.bonus |= BONUS_VITESSE_BLEUE_FAIBLE;
				pMap->planObjets[((images[indiceImage].position[0].x + images[indiceImage].position[0].w) / TailleBloc)][(images[indiceImage].position[0].y / TailleBloc)] = AUCUN_BONUS;
				break;

			case DIA_BLEUMARINE:
				infos.bonus |= BONUS_VITESSE_BLEUE_FORT;
				pMap->planObjets[((images[indiceImage].position[0].x + images[indiceImage].position[0].w) / TailleBloc)][(images[indiceImage].position[0].y / TailleBloc)] = AUCUN_BONUS;
				break;
			}
		}

		if((pMap->planObjets[(images[indiceImage].position[0].x / TailleBloc)][((images[indiceImage].position[0].y + images[indiceImage].position[0].h) / TailleBloc)]) == i)
		{
			switch(i)
			{
			case DIA_VERTCLAIR:
				infos.bonus |= BONUS_VITESSE_VERTE_FAIBLE;
				pMap->planObjets[(images[indiceImage].position[0].x / TailleBloc)][((images[indiceImage].position[0].y + images[indiceImage].position[0].h) / TailleBloc)] = false;
				break;

			case DIA_VERT:
				infos.bonus |= BONUS_VITESSE_VERTE_FORT;
				pMap->planObjets[(images[indiceImage].position[0].x / TailleBloc)][((images[indiceImage].position[0].y + images[indiceImage].position[0].h) / TailleBloc)] = AUCUN_BONUS;
				break;

			case DIA_MARRONCLAIR:
				infos.bonus |= BONUS_SAUT_VERTE_FAIBLE;
				pMap->planObjets[(images[indiceImage].position[0].x / TailleBloc)][((images[indiceImage].position[0].y + images[indiceImage].position[0].h) / TailleBloc)] = AUCUN_BONUS;
				break;

			case DIA_MARRON:
				infos.bonus |= BONUS_SAUT_VERTE_FORT;
				pMap->planObjets[(images[indiceImage].position[0].x / TailleBloc)][((images[indiceImage].position[0].y + images[indiceImage].position[0].h) / TailleBloc)] = AUCUN_BONUS;
				break;

			case DIA_JAUNE:
				infos.bonus |= BONUS_SCORE_FAIBLE;
				pMap->planObjets[(images[indiceImage].position[0].x / TailleBloc)][((images[indiceImage].position[0].y + images[indiceImage].position[0].h) / TailleBloc)] = AUCUN_BONUS;
				break;

			case DIA_ORANGE:
				infos.bonus |= BONUS_SCORE_FORT;
				pMap->planObjets[(images[indiceImage].position[0].x / TailleBloc)][((images[indiceImage].position[0].y + images[indiceImage].position[0].h) / TailleBloc)] = AUCUN_BONUS;
				break;

			case DIA_ROUGE:
				infos.bonus |= BONUS_VIE;
				pMap->planObjets[(images[indiceImage].position[0].x / TailleBloc)][((images[indiceImage].position[0].y + images[indiceImage].position[0].h) / TailleBloc)] = AUCUN_BONUS;
				break;

			case DIA_BLEUCLAIR:
				infos.bonus |= BONUS_SAUT_BLEUE_FAIBLE;
				pMap->planObjets[(images[indiceImage].position[0].x / TailleBloc)][((images[indiceImage].position[0].y + images[indiceImage].position[0].h) / TailleBloc)] = AUCUN_BONUS;
				break;

			case DIA_BLEUFONCE:
				infos.bonus |= BONUS_SAUT_BLEUE_FORT;
				pMap->planObjets[(images[indiceImage].position[0].x / TailleBloc)][((images[indiceImage].position[0].y + images[indiceImage].position[0].h) / TailleBloc)] = AUCUN_BONUS;
				break;

			case DIA_BLEU:
				infos.bonus |= BONUS_VITESSE_BLEUE_FAIBLE;
				pMap->planObjets[(images[indiceImage].position[0].x / TailleBloc)][((images[indiceImage].position[0].y + images[indiceImage].position[0].h) / TailleBloc)] = AUCUN_BONUS;
				break;

			case DIA_BLEUMARINE:
				infos.bonus |= BONUS_VITESSE_BLEUE_FORT;
				pMap->planObjets[(images[indiceImage].position[0].x / TailleBloc)][((images[indiceImage].position[0].y + images[indiceImage].position[0].h) / TailleBloc)] = AUCUN_BONUS;
				break;
			}
		}

		if((pMap->planObjets[(images[indiceImage].position[0].x / TailleBloc)][(images[indiceImage].position[0].y / TailleBloc)]) == i)
		{
			switch(i)
			{
			case DIA_VERTCLAIR:
				infos.bonus |= BONUS_VITESSE_VERTE_FAIBLE;
				pMap->planObjets[(images[indiceImage].position[0].x / TailleBloc)][(images[indiceImage].position[0].y / TailleBloc)] = AUCUN_BONUS;
				break;

			case DIA_VERT:
				infos.bonus |= BONUS_VITESSE_VERTE_FORT;
				pMap->planObjets[(images[indiceImage].position[0].x / TailleBloc)][(images[indiceImage].position[0].y / TailleBloc)] = AUCUN_BONUS;
				break;

			case DIA_MARRONCLAIR:
				infos.bonus |= BONUS_SAUT_VERTE_FAIBLE;
				pMap->planObjets[(images[indiceImage].position[0].x / TailleBloc)][(images[indiceImage].position[0].y / TailleBloc)] = AUCUN_BONUS;
				break;

			case DIA_MARRON:
				infos.bonus |= BONUS_SAUT_VERTE_FORT;
				pMap->planObjets[(images[indiceImage].position[0].x / TailleBloc)][(images[indiceImage].position[0].y / TailleBloc)] = AUCUN_BONUS;
				break;

			case DIA_JAUNE:
				infos.bonus |= BONUS_SCORE_FAIBLE;
				pMap->planObjets[(images[indiceImage].position[0].x / TailleBloc)][(images[indiceImage].position[0].y / TailleBloc)] = AUCUN_BONUS;
				break;

			case DIA_ORANGE:
				infos.bonus |= BONUS_SCORE_FORT;
				pMap->planObjets[(images[indiceImage].position[0].x / TailleBloc)][(images[indiceImage].position[0].y / TailleBloc)] = AUCUN_BONUS;
				break;

			case DIA_ROUGE:
				infos.bonus |= BONUS_VIE;
				pMap->planObjets[(images[indiceImage].position[0].x / TailleBloc)][(images[indiceImage].position[0].y / TailleBloc)] = AUCUN_BONUS;
				break;

			case DIA_BLEUCLAIR:
				infos.bonus |= BONUS_SAUT_BLEUE_FAIBLE;
				pMap->planObjets[(images[indiceImage].position[0].x / TailleBloc)][(images[indiceImage].position[0].y / TailleBloc)] = AUCUN_BONUS;
				break;

			case DIA_BLEUFONCE:
				infos.bonus |= BONUS_SAUT_BLEUE_FORT;
				pMap->planObjets[(images[indiceImage].position[0].x / TailleBloc)][(images[indiceImage].position[0].y / TailleBloc)] = AUCUN_BONUS;
				break;

			case DIA_BLEU:
				infos.bonus |= BONUS_VITESSE_BLEUE_FAIBLE;
				pMap->planObjets[(images[indiceImage].position[0].x / TailleBloc)][(images[indiceImage].position[0].y / TailleBloc)] = AUCUN_BONUS;
				break;

			case DIA_BLEUMARINE:
				infos.bonus |= BONUS_VITESSE_BLEUE_FORT;
				pMap->planObjets[(images[indiceImage].position[0].x / TailleBloc)][(images[indiceImage].position[0].y / TailleBloc)] = AUCUN_BONUS;
				break;
			}
		}
	}
}

int Perdu(SDL_Renderer *pMoteurRendu, Sprite images[], Animation anim[], Map* pMap, TTF_Font *polices[], unsigned char *pAjoutAnim)
{
	/* Cette fonction est appel�e quand on a plus de vie, elle fait sa propre boucle d'affichage */

	SDL_Surface *psFondPerdu = NULL;	//Pointeur sur la surface de fond
	SDL_Texture *pFondPerdu = NULL;	//Pointeur sur la texture de fond
	Texte information;	//Structure qui contient des cha�nes, des textures, des positions et des tailles

	ClavierSouris entrees;
	SDL_Color color = {255, 255, 255, SDL_ALPHA_OPAQUE};	//Couleur blanche
	int rmask, gmask, bmask, amask, differenceFPS=0, i=0;
	unsigned int tempsFPS=0, tempsAncienFPS=0;	//Temps

	/* On remplie les cha�nes avec ce que l'on veut afficher */
	sprintf(information.chaines[0], "PERDU");
	sprintf(information.chaines[1], "Niveau: %d", infos.niveau);
	sprintf(information.chaines[2], "Temps mis pour ce niveau: %d", infos.compteurTemps);
	sprintf(information.chaines[3], "Score: %ld", infos.score);

	EntreesZero(&entrees);	//On initialise la structure avec les entr�es

	/* Masques RGBA */
	rmask = 0x000000ff;
	gmask = 0x0000ff00;
	bmask = 0x00ff0000;
	amask = 0xff000000;

	/* On cr�e le fond orange transparent et on le convertit en texture */
	psFondPerdu = SDL_CreateRGBSurface(0 , (int)Largeur, (int)Hauteur, 32, rmask, gmask, bmask, amask);
	SDL_FillRect(psFondPerdu, NULL, SDL_MapRGBA(psFondPerdu->format, 200, 125, 75, 128));
	pFondPerdu = SDL_CreateTextureFromSurface(pMoteurRendu, psFondPerdu);

	SDL_FreeSurface(psFondPerdu);	//Lib�ration

	/* On cr�e des surfaces � partir des cha�nes, on d�finit les tailles, les positions (on centre), on cr�e la texture correspondante et on lib�re la m�moire */
	for (i=0; i<4; i++)
	{
		information.surface = TTF_RenderText_Blended(polices[POLICE_SNICKY_GRAND], information.chaines[i], color);
		TTF_SizeText(polices[POLICE_SNICKY_GRAND], information.chaines[i], &information.positions[i].w, &information.positions[i].h);
		information.positions[i].w = Arrondir(((double)information.positions[i].w / 1280.0) * Largeur);
		information.positions[i].h = Arrondir(((double)information.positions[i].h / 1280.0) * Largeur);

		information.positions[i].x = Arrondir((Largeur/2.0) - ((double)information.positions[i].w/2.0));
		information.positions[i].y = Arrondir((0.063*Largeur + 0.141*Largeur*(double)i));

		information.pTextures[i] = SDL_CreateTextureFromSurface(pMoteurRendu, information.surface);

		SDL_FreeSurface(information.surface);
	}

	/* On lance la boucle d'affichage tant qu'on appuie pas sur ESPACE, ENTREE, ECHAP et que l'on ne ferme pas */
	while(!entrees.clavier[ECHAP] && !entrees.clavier[ESPACE] && !entrees.clavier[ENTREE] && !entrees.fermeture)
	{
		GestionEvenements(&entrees);

		tempsFPS = SDL_GetTicks();

		differenceFPS = tempsFPS - tempsAncienFPS;

		if (differenceFPS > T_FPS)
		{
			PerduAffichage(pMoteurRendu, images, anim, pFondPerdu, pMap, &information, pAjoutAnim);
			tempsAncienFPS = tempsFPS;
		}
	}

	entrees.clavier[ECHAP] = entrees.clavier[ESPACE] = entrees.clavier[ENTREE] = false;	//On remet � 0 pour �viter que le message ne saute tout de suite

	MessageInformations("Oh non, les boules ont p�ri, bris�es par les missiles !", polices, pMoteurRendu, &entrees);	//Message (boucle d'affichage interne)

	return 0;
}

int PerduAffichage(SDL_Renderer *pMoteurRendu, Sprite images[], Animation anim[], SDL_Texture *pFondPerdu, Map *pMap, Texte *pInformation, unsigned char *pAjoutAnim)
{
	/* Affichage de la fonction Perdu */

	static SDL_Rect pos, posFond;	//Positions du fond et des blocs pour la map en arri�re plan
	int i=0;	//Compteur

	pos.h = pos.w = TailleBloc;	//Taille est celle des blocs

	/* Taille de l'�cran */
	posFond.h = (int)Hauteur;
	posFond.w = (int)Largeur;
	posFond.x = posFond.y = 0;	//(0;0)

	SDL_SetRenderDrawColor(pMoteurRendu, 0, 0, 0, SDL_ALPHA_OPAQUE); //Remplissage en noir pour effacer
	SDL_RenderClear(pMoteurRendu);

	SDL_RenderCopy(pMoteurRendu, pMap->fond, NULL, &posFond);	//On copie le fond

	SDL_RenderCopy(pMoteurRendu, images[VORTEX_BLEU].pTextures[0], NULL, &images[VORTEX_BLEU].position[0]);        //Collage des surfaces des vortex
	SDL_RenderCopy(pMoteurRendu, images[VORTEX_VERT].pTextures[0], NULL, &images[VORTEX_VERT].position[0]);

	AffichageMap(pMoteurRendu, pMap);	//On affiche la map

	AffichageBonus(pMoteurRendu, pMap, images);	//On affiche les bonus

	AffichageVies(pMoteurRendu, images);	//Les vies (0)

	/* Si la derni�re mort est d�e � un missile, on lit l'animation */
	if(*pAjoutAnim)
	{
		if (LectureAnimation(pMoteurRendu, anim, ANIM_0) == -1)
		{
			*pAjoutAnim = false;
		}
	}
	else	//Sinon on affiche notre fond orange transparent avec ses informations
	{
		SDL_RenderCopy(pMoteurRendu, pFondPerdu, NULL, &posFond);

		for(i=0; i<4; i++)
		{
			SDL_RenderCopy(pMoteurRendu, pInformation->pTextures[i], NULL, &pInformation->positions[i]);
		}
	}

	SDL_RenderPresent(pMoteurRendu);         //Mise � jour de l'�cran

	return 0;
}

int LectureAnimation(SDL_Renderer *pMoteurRendu, Animation anim[], int animNB)
{
	/* Cette fonction est appel�e pour lire une succession d'images */

	static int temps=0, tempsAncien=0, k=0;	//Temps + compteur d'images

	temps = SDL_GetTicks();	//On prend le temps

	/* Si le temps depuis le dernier changement d'image est suffisant et qu'il y a une image valide apr�s */
	if (anim[animNB].img[k] != NULL && temps - tempsAncien > T_ANIM)
	{
		SDL_RenderCopy(pMoteurRendu, anim[animNB].img[k], NULL, &anim[animNB].pos);	//On affiche l'image suivante
		k++;	//On passe � celle d'apr�s
		tempsAncien = temps;
	}
	else if (anim[animNB].img[k] != NULL)	//Sinon si l'image suivante est valide mais que le temps requis ne s'est pas �coul�, on r�affiche la m�me
	{
		SDL_RenderCopy(pMoteurRendu, anim[animNB].img[k], NULL, &anim[animNB].pos);
	}
	else /* Si l'image suivante n'est pas correcte, l'animation est termin�e, on remet k et les temps � 0 pour la prochaine fois et on signale la fin de l'animation */
	{
		k=0;
		temps = tempsAncien = 0;
		return -1;
	}

	return 0;
}

int Gagne(SDL_Renderer *pMoteurRendu, Sprite images[], Map *pMap, TTF_Font *polices[])
{
	/* Cette fonction est appel�e quand on a gagn� tous les niveaux */

	SDL_Surface *pSurfFondGagne = NULL;	//Surface
	SDL_Texture *pTextureFondGagne = NULL;	//Texture

	Texte information;	//Cha�nes + textures + positions

	ClavierSouris entrees;
	SDL_Color color = {255, 255, 255, SDL_ALPHA_OPAQUE};
	int rmask, gmask, bmask, amask, i=0, differenceFPS=0;
	unsigned int tempsFPS=0, tempsAncienFPS=0;	//Temps

	/* On �crit les infos dans les cha�nes */
	sprintf(information.chaines[0], "GAGN�");
	sprintf(information.chaines[1], "Niveau: %d", infos.niveau -1);
	sprintf(information.chaines[2], "Temps mis pour ce niveau: %d", infos.compteurTemps);
	sprintf(information.chaines[3], "Score: %ld", infos.score);
	sprintf(information.chaines[4], "Vies: %d sur %d", infos.vies, infos.viesInitiales);

	EntreesZero(&entrees);	//Mise � 0

	/* Masques RGBA */
	rmask = 0x000000ff;
	gmask = 0x0000ff00;
	bmask = 0x00ff0000;
	amask = 0xff000000;

	/* On cr�e la surface de fond orang� et on la convertie en texture */
	pSurfFondGagne = SDL_CreateRGBSurface(0 , (int)Largeur, (int)Hauteur, 32, rmask, gmask, bmask, amask);
	SDL_FillRect(pSurfFondGagne, NULL, SDL_MapRGBA(pSurfFondGagne->format, 200, 125, 75, 128));
	pTextureFondGagne = SDL_CreateTextureFromSurface(pMoteurRendu, pSurfFondGagne);

	SDL_FreeSurface(pSurfFondGagne);	//Lib�ration

	/* On cr�e les surfaces, puis les textures � partir des cha�nes */
	for (i=0; i<5; i++)
	{
		information.surface = TTF_RenderText_Blended(polices[POLICE_SNICKY_GRAND], information.chaines[i], color);
		TTF_SizeText(polices[POLICE_SNICKY_GRAND], information.chaines[i], &information.positions[i].w, &information.positions[i].h);
		information.positions[i].w = Arrondir(((double)information.positions[i].w / 1280.0) * Largeur);
		information.positions[i].h = Arrondir(((double)information.positions[i].h / 1280.0) * Largeur);

		information.positions[i].x = ((Largeur/2.0) - (information.positions[i].w/2));
		information.positions[i].y = Arrondir(0.05*Largeur + 0.125*Largeur*(double)i);

		information.pTextures[i] = SDL_CreateTextureFromSurface(pMoteurRendu, information.surface);

		SDL_FreeSurface(information.surface);
	}

	/* Affichage */
	while(!entrees.clavier[ECHAP] && !entrees.clavier[ESPACE] && !entrees.clavier[ENTREE] && !entrees.fermeture)
	{
		GestionEvenements(&entrees);

		tempsFPS = SDL_GetTicks();

		differenceFPS = tempsFPS - tempsAncienFPS;

		if (differenceFPS > T_FPS)
		{
			GagneAffichage(pMoteurRendu, images, pTextureFondGagne, pMap, &information);
			tempsAncienFPS = tempsFPS;
		}
	}

	entrees.clavier[ECHAP] = entrees.clavier[ENTREE] = entrees.clavier[ESPACE] = false;

	MessageInformations("Bravo! Vous avez sauv� les boules.", polices, pMoteurRendu, &entrees);	//Message

	return 0;
}

int GagneAffichage(SDL_Renderer *pMoteurRendu, Sprite images[], SDL_Texture *pTextureFondGagne, Map *pMap, Texte *pInformation)
{
	/* Affichage de la fonction Gagne */

	static SDL_Rect pos, posFond;	//Positions
	int i=0;	//Compteur

	/* On affecte les tailles et les positions */
	pos.h = pos.w = TailleBloc;

	posFond.h = Hauteur;
	posFond.w = Largeur;
	posFond.x = posFond.y = 0;

	SDL_SetRenderDrawColor(pMoteurRendu, 0, 0, 0, SDL_ALPHA_OPAQUE); //Remplissage en noir pour effacer le fond
	SDL_RenderClear(pMoteurRendu);

	SDL_RenderCopy(pMoteurRendu, pMap->fond, NULL, &posFond);	//On copie le fond

	SDL_RenderCopy(pMoteurRendu, images[VORTEX_BLEU].pTextures[0], NULL, &images[VORTEX_BLEU].position[0]);        //Collage des surfaces des vortex
	SDL_RenderCopy(pMoteurRendu, images[VORTEX_VERT].pTextures[0], NULL, &images[VORTEX_VERT].position[0]);

	AffichageMap(pMoteurRendu, pMap);	//Affichage de la map

	AffichageBonus(pMoteurRendu, pMap, images);	//Des bonus

	AffichageVies(pMoteurRendu, images);	//Des vies (>0)

	SDL_RenderCopy(pMoteurRendu, pTextureFondGagne, NULL, &posFond);	//Du fond orang�

	for(i=0; i<5; i++)	//Affichage des informations
	{
		SDL_RenderCopy(pMoteurRendu, pInformation->pTextures[i], NULL, &pInformation->positions[i]);
	}

	SDL_RenderPresent(pMoteurRendu);         //Mise � jour de l'�cran

	return 0;
}
//Fin du fichier jeu.c
