#ifndef CALLBACKS_H
#define CALLBACKS_H

#include <gtk/gtk.h>
#include "common.h"
#include "membre.h"
#include "entraineur.h"
#include "cours.h"
#include "equipement.h"
#include "centre.h"

// ============ FONCTIONS UTILITAIRES ============
GtkWidget* lookup_widget(GtkWidget *widget, const char *name);
void treeview_vider(GtkWidget *treeview);
void treeview_init_columns(GtkWidget *treeview, const char *column_titles[], int num_columns);
void treeview_ajouter_ligne(GtkWidget *treeview, gchar *valeurs[], int num_columns);
gboolean treeview_get_selected_row(GtkWidget *treeview, gchar *values[], int num_columns);
void treeview_supprimer_selection(GtkWidget *treeview);
void afficher_message(GtkWidget *widget, const char *type, const char *message);
void vider_formulaire(GtkWidget *parent);
void fermer_fenetre(GtkWidget *widget);
void retour_vers_login(GtkWidget *widget);
gboolean demander_confirmation(GtkWidget *parent, const char *titre, const char *message);

// ============ FONCTIONS DE GESTION DES FICHIERS ============

gboolean verifier_login(const char *username, const char *password, char *user_type, int *user_id);
void start_refresh(void);

// ============ CHARGEMENT DES TREEVIEW ============
void charger_membres_dans_treeview(GtkWidget *treeview);
void charger_entraineurs_dans_treeview(GtkWidget *treeview);
void charger_cours_dans_treeview(GtkWidget *treeview);
void charger_equipements_dans_treeview(GtkWidget *treeview);
void charger_centres_dans_treeview(GtkWidget *treeview);
void charger_demandes_entraineur_dans_treeviewcoach(GtkWidget *treeview);

// ============ CALLBACKS LOGIN (window_login) ============
void on_btn_se_connecter_clicked(GtkWidget *objet, gpointer user_data);
void on_btn_inscrire_clicked(GtkWidget *objet, gpointer user_data);
void on_chk_robot_toggled(GtkToggleButton *togglebutton, gpointer user_data);
void on_entry_email_focus_in(GtkWidget *widget, GdkEventFocus *event, gpointer user_data);
void on_window_login_delete_event(GtkWidget *widget, GdkEvent *event, gpointer user_data);

// ============ CALLBACKS HOMEPAGE (window_homepage) ============
void on_window_homepage_delete_event(GtkWidget *widget, GdkEvent *event, gpointer user_data);
void on_btn_clubs_clicked(GtkWidget *objet, gpointer user_data);
void on_btn_activites_clicked(GtkWidget *objet, gpointer user_data);
void on_btn_offres_nav_clicked(GtkWidget *objet, gpointer user_data);
void on_btn_planning_clicked(GtkWidget *objet, gpointer user_data);
void on_btn_about_clicked(GtkWidget *objet, gpointer user_data);
void on_btn_entreprise_clicked(GtkWidget *objet, gpointer user_data);
void on_btn_rejoindre_clicked(GtkWidget *objet, gpointer user_data);
void on_btn_cta_offres_clicked(GtkWidget *objet, gpointer user_data);
void on_offre_standard_clicked(GtkWidget *item, gpointer user_data);
void on_offre_premium_clicked(GtkWidget *item, gpointer user_data);
void on_offre_vip_clicked(GtkWidget *item, gpointer user_data);
void on_offre_annuelle_clicked(GtkWidget *item, gpointer user_data);
// ============ CALLBACKS ADMIN (window_admin) ============
void on_window_admin_delete_event(GtkWidget *widget, GdkEvent *event, gpointer user_data);
void on_btn_admin_deconnexion_clicked(GtkWidget *objet, gpointer user_data);

// Membres
void on_btn_admin_rechercher_membre_clicked(GtkWidget *objet, gpointer user_data);
void on_btn_admin_supprimer_membre_clicked(GtkWidget *objet, gpointer user_data);
void on_btn_admin_modifier_membre_clicked(GtkWidget *objet, gpointer user_data);
void on_btn_admin_ajouter_membre_clicked(GtkWidget *objet, gpointer user_data);
void on_treeview_membres_row_activated(GtkTreeView *treeview, GtkTreePath *path, GtkTreeViewColumn *column, gpointer user_data);

// Cours
void on_btn_admin_rechercher_cours_clicked(GtkWidget *objet, gpointer user_data);
void on_btn_admin_supprimer_cours_clicked(GtkWidget *objet, gpointer user_data);
void on_btn_admin_modifier_cours_clicked(GtkWidget *objet, gpointer user_data);
void on_btn_admin_ajouter_cours_clicked(GtkWidget *objet, gpointer user_data);
void on_treeview_cours_row_activated(GtkTreeView *treeview, GtkTreePath *path, GtkTreeViewColumn *column, gpointer user_data);

// Entraineurs
void on_btn_admin_rechercher_entraineur_clicked(GtkWidget *objet, gpointer user_data);
void on_btn_admin_supprimer_entraineur_clicked(GtkWidget *objet, gpointer user_data);
void on_btn_admin_modifier_entraineur_clicked(GtkWidget *objet, gpointer user_data);
void on_btn_admin_ajouter_entraineur_clicked(GtkWidget *objet, gpointer user_data);
void on_treeview_entraineurs_row_activated(GtkTreeView *treeview, GtkTreePath *path, GtkTreeViewColumn *column, gpointer user_data);

// Equipements
void on_btn_admin_rechercher_equipement_clicked(GtkWidget *objet, gpointer user_data);
void on_btn_admin_supprimer_equipement_clicked(GtkWidget *objet, gpointer user_data);
void on_btn_admin_modifier_equipement_clicked(GtkWidget *objet, gpointer user_data);
void on_btn_admin_ajouter_equipement_clicked(GtkWidget *objet, gpointer user_data);
void on_treeview_equipements_row_activated(GtkTreeView *treeview, GtkTreePath *path, GtkTreeViewColumn *column, gpointer user_data);

// Centres
void on_btn_admin_rechercher_centre_clicked(GtkWidget *objet, gpointer user_data);
void on_btn_admin_supprimer_centre_clicked(GtkWidget *objet, gpointer user_data);
void on_btn_admin_modifier_centre_clicked(GtkWidget *objet, gpointer user_data);
void on_btn_admin_ajouter_centre_clicked(GtkWidget *objet, gpointer user_data);
void on_treeview_centres_row_activated(GtkTreeView *treeview, GtkTreePath *path, GtkTreeViewColumn *column, gpointer user_data);

// ============ CALLBACKS DASHBOARD MEMBER (window_dashboard_member) ============
void on_window_member_delete_event(GtkWidget *widget, GdkEvent *event, gpointer user_data);
void on_eb_m_avatar_clicked(GtkWidget *objet, gpointer user_data);
void on_btn_inscrire_cour_clicked(GtkWidget *objet, gpointer user_data);
void on_btn_reserver_coach_clicked(GtkWidget *objet, gpointer user_data);
void on_btn_deconnexion_m_clicked(GtkWidget *objet, gpointer user_data);
void on_btn_chatbot_send_clicked(GtkWidget *objet, gpointer user_data);

// ============ CALLBACKS DASHBOARD COACH (window_dashboard_coach) ============
void on_window_coach_delete_event(GtkWidget *widget, GdkEvent *event, gpointer user_data);
void on_eb_c_avatar_clicked(GtkWidget *objet, gpointer user_data);
void on_btn_valider_presence_clicked(GtkWidget *objet, gpointer user_data);
void on_btn_demander_equip_clicked(GtkWidget *objet, gpointer user_data);
void on_btn_inscrire_centre_clicked(GtkWidget *objet, gpointer user_data);

// ============ STATISTIQUES ============
DiagrammeData lire_toutes_donnees(void);
void initialiser_diagramme(GtkWidget *fenetre_admin);
void rafraichir_diagramme(GtkWidget *fenetre_admin);
gboolean on_drawingarea_diagramme_expose_event(GtkWidget *widget, GdkEventExpose *event, gpointer user_data);

// ============ FONCTIONS POUR IMAGE DE FOND ============
void load_background_image(void);
void appliquer_background_a_fenetre(GtkWidget *fenetre);
gboolean on_eventbox_generic_expose(GtkWidget *widget, GdkEventExpose *event, gpointer data);
// ============ FONCTIONS POUR IMAGE DE FOND ============
void load_background_image(void);
void appliquer_background_a_fenetre(GtkWidget *fenetre);
gboolean on_eventbox_generic_expose(GtkWidget *widget, GdkEventExpose *event, gpointer data);

// ============ FONCTIONS POUR COULEURS DES FRAMES ============
void definir_couleur_frame(GtkWidget *frame, const char *couleur_label);
void appliquer_couleurs_tous_frames(GtkWidget *fenetre);

// ============ CALLBACKS POUR window_homepage ============
void on_window_homepage_delete_event(GtkWidget *widget, GdkEvent *event, gpointer user_data);
void on_btn_clubs_clicked(GtkWidget *objet, gpointer user_data);
void on_btn_activites_clicked(GtkWidget *objet, gpointer user_data);
void on_btn_offres_nav_clicked(GtkWidget *objet, gpointer user_data);
void on_btn_planning_clicked(GtkWidget *objet, gpointer user_data);
void on_btn_about_clicked(GtkWidget *objet, gpointer user_data);
void on_btn_entreprise_clicked(GtkWidget *objet, gpointer user_data);
void on_btn_rejoindre_clicked(GtkWidget *objet, gpointer user_data);
void on_btn_cta_offres_clicked(GtkWidget *objet, gpointer user_data);

// ============ CALLBACKS POUR window_login ============
void on_btn_se_connecter_clicked(GtkWidget *objet, gpointer user_data);
void on_btn_inscrire_clicked(GtkWidget *objet, gpointer user_data);
void on_chk_robot_toggled(GtkToggleButton *togglebutton, gpointer user_data);
void on_entry_email_focus_in(GtkWidget *widget, GdkEventFocus *event, gpointer user_data);
void on_window_login_delete_event(GtkWidget *widget, GdkEvent *event, gpointer user_data);

// ============ CALLBACKS POUR dashboard member ============
void on_window_member_delete_event(GtkWidget *widget, GdkEvent *event, gpointer user_data);
void on_eb_m_avatar_clicked(GtkWidget *objet, gpointer user_data);
void on_btn_inscrire_cour_clicked(GtkWidget *objet, gpointer user_data);
void on_btn_reserver_coach_clicked(GtkWidget *objet, gpointer user_data);
void on_btn_deconnexion_m_clicked(GtkWidget *objet, gpointer user_data);
void on_btn_chatbot_send_clicked(GtkWidget *objet, gpointer user_data);

// ============ CALLBACKS POUR dashboard coach ============
void on_window_coach_delete_event(GtkWidget *widget, GdkEvent *event, gpointer user_data);
void on_eb_c_avatar_clicked(GtkWidget *objet, gpointer user_data);
void on_btn_valider_presence_clicked(GtkWidget *objet, gpointer user_data);
void on_btn_demander_equip_clicked(GtkWidget *objet, gpointer user_data);
void on_btn_inscrire_centre_clicked(GtkWidget *objet, gpointer user_data);
// ==================== DÉCLARATIONS POUR LA FENÊTRE ADMIN ====================

// Callbacks pour les expose events
gboolean on_eb_topbar_expose(GtkWidget *widget, GdkEventExpose *event, gpointer data);
gboolean on_eb_login_form_expose(GtkWidget *widget, GdkEventExpose *event, gpointer data);
gboolean on_eb_stat_card_expose(GtkWidget *widget, GdkEventExpose *event, gpointer data);
gboolean on_eb_dark_panel_expose(GtkWidget *widget, GdkEventExpose *event, gpointer data);
gboolean on_eb_sidebar_expose(GtkWidget *widget, GdkEventExpose *event, gpointer data);

// Callbacks pour les drawing areas
gboolean on_da_mini_bars_expose(GtkWidget *widget, GdkEventExpose *event, gpointer data);
gboolean on_da_repartition_age_expose(GtkWidget *widget, GdkEventExpose *event, gpointer data);
gboolean on_da_abonnement_expose(GtkWidget *widget, GdkEventExpose *event, gpointer data);
gboolean on_da_evolution_expose(GtkWidget *widget, GdkEventExpose *event, gpointer data);
gboolean on_da_repartition_horaire_expose(GtkWidget *widget, GdkEventExpose *event, gpointer data);
gboolean on_da_cours_niveaux_expose(GtkWidget *widget, GdkEventExpose *event, gpointer data);
gboolean on_da_top5_cours_expose(GtkWidget *widget, GdkEventExpose *event, gpointer data);
gboolean on_da_ent_experience_expose(GtkWidget *widget, GdkEventExpose *event, gpointer data);
gboolean on_da_ent_specialites_expose(GtkWidget *widget, GdkEventExpose *event, gpointer data);
gboolean on_da_ent_top_coachs_expose(GtkWidget *widget, GdkEventExpose *event, gpointer data);
gboolean on_da_equip_dispo_expose(GtkWidget *widget, GdkEventExpose *event, gpointer data);
gboolean on_da_equip_etat_expose(GtkWidget *widget, GdkEventExpose *event, gpointer data);
gboolean on_da_equip_utilisation_expose(GtkWidget *widget, GdkEventExpose *event, gpointer data);
gboolean on_da_ctr_taux_occ_expose(GtkWidget *widget, GdkEventExpose *event, gpointer data);
gboolean on_da_ctr_par_ville_expose(GtkWidget *widget, GdkEventExpose *event, gpointer data);
gboolean on_da_ctr_capacite_expose(GtkWidget *widget, GdkEventExpose *event, gpointer data);


// ==================== DÉCLARATIONS POUR LA FENÊTRE ADMIN ====================

// Expose events
gboolean on_eb_topbar_expose(GtkWidget *widget, GdkEventExpose *event, gpointer data);
gboolean on_eb_login_form_expose(GtkWidget *widget, GdkEventExpose *event, gpointer data);
gboolean on_eb_stat_card_expose(GtkWidget *widget, GdkEventExpose *event, gpointer data);
gboolean on_eb_dark_panel_expose(GtkWidget *widget, GdkEventExpose *event, gpointer data);
gboolean on_eb_sidebar_expose(GtkWidget *widget, GdkEventExpose *event, gpointer data);
gboolean on_eventbox1_expose_event(GtkWidget *widget, GdkEventExpose *event, gpointer data);
gboolean on_eventbox2_expose_event(GtkWidget *widget, GdkEventExpose *event, gpointer data);
gboolean on_eventbox3_expose_event(GtkWidget *widget, GdkEventExpose *event, gpointer data);
gboolean on_eventbox4_expose_event(GtkWidget *widget, GdkEventExpose *event, gpointer data);
gboolean on_eventbox5_expose_event(GtkWidget *widget, GdkEventExpose *event, gpointer data);




// Ajoutez ces lignes à la fin de callbacks.h
void on_treeview_membres_row_activated(GtkTreeView *treeview, GtkTreePath *path,
                                       GtkTreeViewColumn *column, gpointer user_data);
void on_treeview_cours_row_activated(GtkTreeView *treeview, GtkTreePath *path,
                                     GtkTreeViewColumn *column, gpointer user_data);
void on_treeview_entraineurs_row_activated(GtkTreeView *treeview, GtkTreePath *path,
                                          GtkTreeViewColumn *column, gpointer user_data);
void on_treeview_equipements_row_activated(GtkTreeView *treeview, GtkTreePath *path,
                                          GtkTreeViewColumn *column, gpointer user_data);
void on_treeview_centres_row_activated(GtkTreeView *treeview, GtkTreePath *path,
                                      GtkTreeViewColumn *column, gpointer user_data);
// ==================== FONCTION POUR COLORER TOUS LES WIDGETS ====================

void appliquer_couleurs_sombres(GtkWidget *fenetre);
void forcer_couleurs_frames_dashboard(GtkWidget *fenetre);
// ==================== FONCTION POUR COLORER TOUS LES WIDGETS ====================

void appliquer_couleurs_sombres(GtkWidget *fenetre);
void forcer_couleurs_frames_dashboard(GtkWidget *fenetre);
// ==================== FONCTIONS UTILITAIRES POUR DIALOGUES ====================
static gchar* afficher_dialog_id(GtkWidget *parent, const char *titre, const char *label);
static gchar* afficher_dialog_reference(GtkWidget *parent, const char *titre, const char *label);

// ==================== DIALOGUES POUR MEMBRES ====================
static void dialog_ajouter_membre(GtkWidget *parent);
static void dialog_modifier_membre(GtkWidget *parent);
static void dialog_supprimer_membre(GtkWidget *parent);
static void dialog_rechercher_membre(GtkWidget *parent, GtkWidget *treeview);

// ==================== DIALOGUES POUR COURS ====================
static void dialog_ajouter_cours(GtkWidget *parent);
static void dialog_modifier_cours(GtkWidget *parent);
static void dialog_supprimer_cours(GtkWidget *parent);
static void dialog_rechercher_cours(GtkWidget *parent, GtkWidget *treeview);

// ==================== DIALOGUES POUR ENTRAINEURS ====================
static void dialog_ajouter_entraineur(GtkWidget *parent);
static void dialog_modifier_entraineur(GtkWidget *parent);
static void dialog_supprimer_entraineur(GtkWidget *parent);
static void dialog_rechercher_entraineur(GtkWidget *parent, GtkWidget *treeview);

// ==================== DIALOGUES POUR EQUIPEMENTS ====================
static void dialog_ajouter_equipement(GtkWidget *parent);
static void dialog_modifier_equipement(GtkWidget *parent);
static void dialog_supprimer_equipement(GtkWidget *parent);
static void dialog_rechercher_equipement(GtkWidget *parent, GtkWidget *treeview);

// ==================== DIALOGUES POUR CENTRES ====================
static void dialog_ajouter_centre(GtkWidget *parent);
static void dialog_modifier_centre(GtkWidget *parent);
static void dialog_supprimer_centre(GtkWidget *parent);
static void dialog_rechercher_centre(GtkWidget *parent, GtkWidget *treeview);

// ==================== BOUTONS ADMIN (MEMBRES) ====================
void on_btn_add_clicked(GtkButton *button, gpointer data);
void on_btn_edit_clicked(GtkButton *button, gpointer data);
void on_btn_delete_clicked(GtkButton *button, gpointer data);
void on_btn_search_side_clicked(GtkButton *button, gpointer data);

// ==================== BOUTONS COURS ====================
void on_btn_cours_add_clicked(GtkButton *button, gpointer data);
void on_btn_cours_edit_clicked(GtkButton *button, gpointer data);
void on_btn_cours_delete_clicked(GtkButton *button, gpointer data);
void on_btn_cours_search_clicked(GtkButton *button, gpointer data);

// ==================== BOUTONS ENTRAINEURS ====================
void on_btn_ent_add_clicked(GtkButton *button, gpointer data);
void on_btn_ent_edit_clicked(GtkButton *button, gpointer data);
void on_btn_ent_delete_clicked(GtkButton *button, gpointer data);
void on_btn_ent_search_clicked(GtkButton *button, gpointer data);

// ==================== BOUTONS EQUIPEMENTS ====================
void on_btn_equip_add_clicked(GtkButton *button, gpointer data);
void on_btn_equip_edit_clicked(GtkButton *button, gpointer data);
void on_btn_equip_delete_clicked(GtkButton *button, gpointer data);
void on_btn_equip_search_clicked(GtkButton *button, gpointer data);

// ==================== BOUTONS CENTRES ====================
void on_btn_ctr_add_clicked(GtkButton *button, gpointer data);
void on_btn_ctr_edit_clicked(GtkButton *button, gpointer data);
void on_btn_ctr_delete_clicked(GtkButton *button, gpointer data);
void on_btn_ctr_search_clicked(GtkButton *button, gpointer data);

// ==================== GESTION DES INSCRIPTIONS (NOUVEAU) ====================
// Pour les membres
void charger_cours_membre_dans_treeview(GtkWidget *treeview, int membre_id);
int ajouter_inscription_membre(int membre_id, int cours_id);
int supprimer_inscription_membre(int membre_id, int cours_id);

// Pour les entraineurs (avec noms différents pour éviter conflit)
void charger_cours_entraineur_dans_treeview(GtkWidget *treeview, int entraineur_id);
int ajouter_inscription_entraineur(int entraineur_id, int cours_id);
int supprimer_inscription_entraineur(int entraineur_id, int cours_id);
// ============ FONCTIONS PROFIL ============
void afficher_profil_membre(GtkWidget *label, int user_id);
void afficher_profil_entraineur(GtkWidget *label, int user_id);

// ============ FONCTIONS CHARGEMENT COURS ============
void charger_cours_entraineur_dans_treeview(GtkWidget *treeview, int entraineur_id);
void charger_cours_membre_dans_treeview(GtkWidget *treeview, int membre_id);

// ============ FONCTIONS INSCRIPTIONS ============
int inscrire_membre_a_cours(int membre_id, int cours_id);
int desinscrire_membre_dun_cours(int membre_id, int cours_id);
int ajouter_inscription_entraineur(int entraineur_id, int cours_id);
int supprimer_inscription_entraineur(int entraineur_id, int cours_id);

// ============ FONCTIONS ADMIN ============
void on_btn_avatar_clicked(GtkButton *button, gpointer data);
void on_btn_deconnexion_clicked(GtkButton *button, gpointer user_data);
void on_window_admin_destroy(GtkWidget *widget, gpointer data);
// Expose event handlers pour les drawing areas
gboolean on_da_mini_bars_expose(GtkWidget *widget, GdkEventExpose *event, gpointer user_data);
gboolean on_da_repartition_age_expose(GtkWidget *widget, GdkEventExpose *event, gpointer user_data);
gboolean on_da_abonnement_expose(GtkWidget *widget, GdkEventExpose *event, gpointer user_data);
gboolean on_da_evolution_expose(GtkWidget *widget, GdkEventExpose *event, gpointer user_data);
gboolean on_da_repartition_horaire_expose(GtkWidget *widget, GdkEventExpose *event, gpointer user_data);
gboolean on_da_cours_niveaux_expose(GtkWidget *widget, GdkEventExpose *event, gpointer user_data);
gboolean on_da_top5_cours_expose(GtkWidget *widget, GdkEventExpose *event, gpointer user_data);
gboolean on_da_ent_experience_expose(GtkWidget *widget, GdkEventExpose *event, gpointer user_data);
gboolean on_da_ent_specialites_expose(GtkWidget *widget, GdkEventExpose *event, gpointer user_data);
gboolean on_da_ent_top_coachs_expose(GtkWidget *widget, GdkEventExpose *event, gpointer user_data);
gboolean on_da_equip_dispo_expose(GtkWidget *widget, GdkEventExpose *event, gpointer user_data);
gboolean on_da_equip_etat_expose(GtkWidget *widget, GdkEventExpose *event, gpointer user_data);
gboolean on_da_equip_utilisation_expose(GtkWidget *widget, GdkEventExpose *event, gpointer user_data);
gboolean on_da_ctr_taux_occ_expose(GtkWidget *widget, GdkEventExpose *event, gpointer user_data);
gboolean on_da_ctr_par_ville_expose(GtkWidget *widget, GdkEventExpose *event, gpointer user_data);
gboolean on_da_ctr_capacite_expose(GtkWidget *widget, GdkEventExpose *event, gpointer user_data);
static double read_stat_from_file(const char *filename, const char *key);
static void load_distribution_data(const char *filename, char ***labels, double **values, int *count);
static double* load_series_data(const char *filename, int *count);

#endif
