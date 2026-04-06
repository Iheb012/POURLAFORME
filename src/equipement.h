#ifndef EQUIPEMENT_H_INCLUDED
#define EQUIPEMENT_H_INCLUDED

#include "common.h"

// Fonctions de gestion des équipements
int ajouter_equipement(char *filename, Equipement e);
int modifier_equipement(char *filename, char *reference, Equipement nouv);
int supprimer_equipement(char *filename, const char *reference);  // AJOUTÉ const
Equipement chercher_equipement(char *filename, const char *reference);  // AJOUTÉ const
int compter_total_equipements(char *filename);
int compter_equipements_par_type(char *filename, char *type);
void afficher_equipements_disponibles(char *filename);
void filtrer_equipements_par_capacite(char *filename, int capacite_min);
void statistiques_equipements(char *filename);

// Fonctions de réservation
int reserver_equipement(char *filename_equipements, char *reference, 
                       char *date, int id_entraineur, char *nom_entraineur);
void mes_reservations_equipements(int id_entraineur);
// Fonction pour remplir un TreeView
void afficher_equipements_treeview(GtkWidget *treeview, const char *filename);
void afficher_statistiques_equipements(GtkLabel *label_total, GtkLabel *label_mobilite, 
 			GtkLabel *label_musculation, GtkLabel *label_cardio, 
                      	const char *filename);
void configurer_treeview_equipements(GtkWidget *treeview);

#endif
