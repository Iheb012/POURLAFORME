#ifndef CALLBACKS_H
#define CALLBACKS_H

#include <gtk/gtk.h>
// Déclarer le type DiagrammeData
typedef struct {
    // Statistiques membres
    int jeunes;
    int adultes;
    int seniors;
    int total_membres;
    
    // Statistiques cours
    int niveau1;
    int niveau2;
    int niveau3;
    int total_cours;
    
    // Statistiques entraîneurs
    int musculation;
    int gymnastique;
    int yoga;
    int total_entraineurs;
    
    // Statistiques équipements
    int mobilites;
    int musculation_eq;
    int cardio;
    int total_equipements;
    
    // Statistiques centres
    int tunis;
    int ariana;
    int aouina;
    int total_centres;
} DiagrammeData;
void start_refresh();
//treeview
void treeview_vider(GtkWidget *treeview);
void treeview_init_columns(GtkWidget *treeview, 
                          const char *column_titles[], 
                          int num_columns);
void treeview_ajouter_ligne(GtkWidget *treeview, gchar *valeurs[], int num_columns);
gboolean treeview_get_selected_row(GtkWidget *treeview, gchar *values[], int num_columns);
void treeview_supprimer_selection(GtkWidget *treeview);

//genereaux
void afficher_message(GtkWidget *widget, const char *type, const char *message);
void vider_formulaire(GtkWidget *parent);
void fermer_fenetre(GtkWidget *widget);
void retour_vers_login(GtkWidget *widget);
gboolean demander_confirmation(GtkWidget *parent, const char *titre, const char *message);

//chargement treeview
void charger_membres_dans_treeview(GtkWidget *treeview);
void charger_entraineurs_dans_treeview(GtkWidget *treeview);
void charger_cours_dans_treeview(GtkWidget *treeview);
void charger_equipements_dans_treeview(GtkWidget *treeview);
void charger_centres_dans_treeview(GtkWidget *treeview);

//login
void on_btn_login_clicked(GtkWidget *objet, gpointer user_data);
void on_btn_register_clicked(GtkWidget *objet, gpointer user_data);

//membre
void on_btn_admin_rechercher_membre_clicked(GtkWidget *objet, gpointer user_data);
void on_btn_admin_supprimer_membre_clicked(GtkWidget *objet, gpointer user_data);
void on_btn_admin_modifier_membre_clicked(GtkWidget *objet, gpointer user_data);
void on_btn_admin_ajouter_membre_clicked(GtkWidget *objet, gpointer user_data);
void on_btn_admin_membre_back_login_clicked(GtkWidget *objet, gpointer user_data);

//cours
void on_btn_admin_rechercher_cours_clicked(GtkWidget *objet, gpointer user_data);
void on_btn_admin_supprimer_cours_clicked(GtkWidget *objet, gpointer user_data);
void on_btn_admin_modifier_cours_clicked(GtkWidget *objet, gpointer user_data);
void on_btn_admin_ajouter_cours_clicked(GtkWidget *objet, gpointer user_data);
void on_btn_admin_cours_back_login_clicked(GtkWidget *objet, gpointer user_data);

//entraineur
void on_btn_admin_rechercher_entraineur_clicked(GtkWidget *objet, gpointer user_data);
void on_btn_admin_supprimer_entraineur_clicked(GtkWidget *objet, gpointer user_data);
void on_btn_admin_modifier_entraineur_clicked(GtkWidget *objet, gpointer user_data);
void on_btn_admin_ajouter_entraineur_clicked(GtkWidget *objet, gpointer user_data);
void on_btn_admin_entraineure_back_login_clicked(GtkWidget *objet, gpointer user_data);

//equipement
void on_btn_admin_rechercher_equipement_clicked(GtkWidget *objet, gpointer user_data);
void on_btn_admin_supprimer_equipement_clicked(GtkWidget *objet, gpointer user_data);
void on_btn_admin_modifier_equipement_clicked(GtkWidget *objet, gpointer user_data);
void on_btn_admin_ajouter_equipement_clicked(GtkWidget *objet, gpointer user_data);
void on_btn_admin_back_login_clicked(GtkWidget *objet, gpointer user_data);

//centre
void on_btn_admin_rechercher_centre_clicked(GtkWidget *objet, gpointer user_data);
void on_btn_admin_supprimer_centre_clicked(GtkWidget *objet, gpointer user_data);
void on_btn_admin_modifier_centre_clicked(GtkWidget *objet, gpointer user_data);
void on_btn_admin_ajouter_centre_clicked(GtkWidget *objet, gpointer user_data);
void on_btn_admin_centres_back_login_clicked(GtkWidget *objet, gpointer user_data);

//row
void on_treeview_membres_row_activated(GtkTreeView *treeview,
                                       GtkTreePath *path,
                                       GtkTreeViewColumn *column,
                                       gpointer user_data);

void on_treeview_entraineurs_row_activated(GtkTreeView *treeview,
                                           GtkTreePath *path,
                                           GtkTreeViewColumn *column,
                                           gpointer user_data);

void on_treeview_cours_row_activated(GtkTreeView *treeview,
                                     GtkTreePath *path,
                                     GtkTreeViewColumn *column,
                                     gpointer user_data);

void on_treeview_equipements_row_activated(GtkTreeView *treeview,
                                           GtkTreePath *path,
                                           GtkTreeViewColumn *column,
                                           gpointer user_data);

void on_treeview_centres_row_activated(GtkTreeView *treeview,
                                       GtkTreePath *path,
                                       GtkTreeViewColumn *column,
                                       gpointer user_data);



//ajout
void on_btn_ajouter_membre_enregistrer_clicked(GtkWidget *objet, gpointer user_data);
void on_btn_ajouter_membre_annuler_clicked(GtkWidget *objet, gpointer user_data);
void on_btn_ajouter_cours_enregistrer_clicked(GtkWidget *objet, gpointer user_data);
void on_btn_ajouter_cours_annuler_clicked(GtkWidget *objet, gpointer user_data);
void on_btn_ajouter_entraineur_enregistrer_clicked(GtkWidget *objet, gpointer user_data);
void on_btn_ajouter_entraineur_annuler_clicked(GtkWidget *objet, gpointer user_data);
void on_btn_ajouter_equipement_enregistrer_clicked(GtkWidget *objet, gpointer user_data);
void on_btn_ajouter_equipement_annuler_clicked(GtkWidget *objet, gpointer user_data);
void on_btn_ajouter_centre_enregistrer_clicked(GtkWidget *objet, gpointer user_data);
void on_btn_ajouter_centre_annuler_clicked(GtkWidget *objet, gpointer user_data);

//modification
void on_btn_modifier_membre_chercher_clicked(GtkWidget *objet, gpointer user_data);
void on_btn_modifier_membre_enregistrer_clicked(GtkWidget *objet, gpointer user_data);
void on_btn_modifier_membre_annuler_clicked(GtkWidget *objet, gpointer user_data);
void on_btn_modifier_cours_chercher_clicked(GtkWidget *objet, gpointer user_data);
void on_btn_modifier_cours_enregistrer_clicked(GtkWidget *objet, gpointer user_data);
void on_btn_modifier_cours_annuler_clicked(GtkWidget *objet, gpointer user_data);
void on_btn_modifier_entraineur_chercher_clicked(GtkWidget *objet, gpointer user_data);
void on_btn_modifier_entraineur_enregistrer_clicked(GtkWidget *objet, gpointer user_data);
void on_btn_modifier_entraineur_annuler_clicked(GtkWidget *objet, gpointer user_data);
void on_btn_modifier_equipement_chercher_clicked(GtkWidget *objet, gpointer user_data);
void on_btn_modifier_equipement_enregistrer_clicked(GtkWidget *objet, gpointer user_data);
void on_btn_modifier_equipement_annuler_clicked(GtkWidget *objet, gpointer user_data);
void on_btn_modifier_centre_chercher_clicked(GtkWidget *objet, gpointer user_data);
void on_btn_modifier_centre_enregistrer_clicked(GtkWidget *objet, gpointer user_data);
void on_btn_modifier_centre_annuler_clicked(GtkWidget *objet, gpointer user_data);


//espace membre
void on_btn_membre_deconnexion_clicked(GtkWidget *objet, gpointer user_data);
void on_btn_membre_ask_clicked(GtkWidget *objet, gpointer user_data);
void on_inscrire__cours_clicked(GtkButton *button, gpointer user_data);
void on_reserver__coach_clicked(GtkButton *button, gpointer user_data);


//filtrage
void on_check_equipement_disponible_only_toggled(GtkWidget *widget, gpointer user_data);
void on_combo_filtre_type_equipement_changed(GtkComboBox *combobox, gpointer user_data);

//my gg
void on_boutton_activate(GtkMenuItem *menuitem, gpointer user_data);

//supprimer treeview
void supprimer_ligne_treeview(GtkWidget *treeview, const char *fichier, int id);
//menu
void on_menuitem7_activate(GtkMenuItem *menuitem, gpointer user_data);
void on_new1_activate(GtkMenuItem *menuitem, gpointer user_data);
void on_open1_activate(GtkMenuItem *menuitem, gpointer user_data);
void on_save1_activate(GtkMenuItem *menuitem, gpointer user_data);
void on_save_as1_activate(GtkMenuItem *menuitem, gpointer user_data);
void on_quit1_activate(GtkMenuItem *menuitem, gpointer user_data);
void on_menuitem8_activate(GtkMenuItem *menuitem, gpointer user_data);
void on_cut1_activate(GtkMenuItem *menuitem, gpointer user_data);
void on_copy1_activate(GtkMenuItem *menuitem, gpointer user_data);
void on_paste1_activate(GtkMenuItem *menuitem, gpointer user_data);
void on_delete1_activate(GtkMenuItem *menuitem, gpointer user_data);
void on_menuitem9_activate(GtkMenuItem *menuitem, gpointer user_data);
void on_menuitem10_activate(GtkMenuItem *menuitem, gpointer user_data);
void on_about1_activate(GtkMenuItem *menuitem, gpointer user_data);
void on_menuitem11_activate(GtkMenuItem *menuitem, gpointer user_data);
//sign up
void on_btn_enregistrer_signup_clicked(GtkWidget *widget, gpointer user_data);
void on_btn_annuler_signup_clicked(GtkWidget *widget, gpointer user_data);
void on_btn_enregistrer_coach_signup_clicked(GtkWidget *widget, gpointer user_data);
void on_btn_annuler_coach_signup_clicked(GtkWidget *widget, gpointer user_data);

void
on_equip__pouruncour_clicked           (GtkButton       *button,
                                        gpointer         user_data);

void
on_inscrit__cour_clicked               (GtkButton       *button,
                                        gpointer         user_data);

void on_inscrit__centre__sportif_clicked(GtkButton *button, gpointer user_data);
//stats

DiagrammeData lire_donnees_pour_diagramme(void);
void initialiser_diagramme(GtkWidget *fenetre_admin);
void rafraichir_diagramme(GtkWidget *fenetre_admin);
gboolean 
on_drawingarea_diagramme_expose_event  (GtkWidget       *widget,
                                        GdkEventExpose  *event,
                                        gpointer         user_data);
gboolean
on_drawingarea_stat_cours_expose_event (GtkWidget       *widget,
                                        GdkEventExpose  *event,
                                        gpointer         user_data);

                                      
gboolean
on_drawingarea_stat_entraineurs_expose_event
                                        (GtkWidget       *widget,
                                        GdkEventExpose  *event,
                                        gpointer         user_data);

gboolean
on_drawingarea_stat_equipements_expose_event
                                        (GtkWidget       *widget,
                                        GdkEventExpose  *event,
                                        gpointer         user_data);

gboolean
on_drawingarea_stat_centres_expose_event
                                        (GtkWidget       *widget,
                                        GdkEventExpose  *event,
                                        gpointer         user_data);
void
on_button_website_clicked              (GtkButton       *button,
                                        gpointer         user_data);
#endif 




