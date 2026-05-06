#ifndef ENTRAINEUR_H
#define ENTRAINEUR_H

#include <stdio.h>
#include "common.h"

#define MAX_ENTRAINEURS 100
#define MAX_COURS 50
#define MAX_ENTRAINEURS_PAR_COURS 100

// Fonctions de gestion des entraîneurs
int ajouter_entraineur(char *filename, Entraineur e);
int modifier_entraineur(char *filename, int id, Entraineur nouv);
int supprimer_entraineur(char *filename, int id);
Entraineur chercher_entraineur(char *filename, int id);
void afficher_entraineurs(char *filename);

void inscrire_entraineur_a_cours(Cours *c, Entraineur *e);
void afficher_entraineurs_du_cours(Cours *c);

int nombre_entraineurs(char *filename);
float experience_moyenne(char *filename);
int entraineurs_par_specialite(char *filename, char *specialite);
Entraineur plus_experimente(char *filename);
Entraineur moins_experimente(char *filename);
void statistiques_entraineurs();

void modifier_partiellement_entraineur(int id);

#endif
