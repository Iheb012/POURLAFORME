#ifndef MEMBRE_H_INCLUDED
#define MEMBRE_H_INCLUDED

#include <stdio.h>
#include "common.h"

// Fonctions de gestion des membres
int ajouter_membre(char *filename, Membre m);
int modifier_membre(char *filename, int id, Membre nouv);
int supprimer_membre(char *filename, int id);
Membre chercher_membre(char *filename, int id);
int nombre_membres(char *filename);
int membres_par_Typeabonnement(char *filename, char *Typeabonnement);
int membres_par_sexe(char *filename, char *sexe);
void statistiques_completes_membre(char *filename);
// Rechercher un membre par nom
Membre chercher_membre_par_nom(char *filename, const char *nom);
// Fonctions pour la réservation de coach
Entraineur chercher_entraineur_disponible(char *filename);
int reserver_entraineur_membre(int id_membre, int id_entraineur, char *date, char *creneau);
int parse_membre_ligne(char *ligne, Membre *m);
#endif
