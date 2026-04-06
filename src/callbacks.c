#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif
#ifndef MIN
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#include <gtk/gtk.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <cairo.h>
#include <math.h>
#include "callbacks.h"
#include "interface.h"
#include "support.h"
#include "common.h"

#include "membre.h"
#include "entraineur.h"
#include "cours.h"
#include "equipement.h"
#include "centre.h"

//utilitaire
void treeview_vider(GtkWidget *treeview)
{
    GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(treeview));
    if (model != NULL) {
        if (GTK_IS_LIST_STORE(model)) {
            gtk_list_store_clear(GTK_LIST_STORE(model));
        } else if (GTK_IS_TREE_STORE(model)) {
            gtk_tree_store_clear(GTK_TREE_STORE(model));
        }
    }
}

void treeview_init_columns(GtkWidget *treeview, 
                          const char *column_titles[], 
                          int num_columns)
{
    GtkCellRenderer *renderer;
    GtkTreeViewColumn *column;
    GtkListStore *store;
    int i;
    
    GType *column_types = g_new(GType, num_columns);
    for (i = 0; i < num_columns; i++) {
        column_types[i] = G_TYPE_STRING;
    }
    
    store = gtk_list_store_newv(num_columns, column_types);
    g_free(column_types);
    
    for (i = 0; i < num_columns; i++) {
        renderer = gtk_cell_renderer_text_new();
        column = gtk_tree_view_column_new_with_attributes(
            column_titles[i], 
            renderer, 
            "text", i, 
            NULL);
        gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);
    }
    
    gtk_tree_view_set_model(GTK_TREE_VIEW(treeview), GTK_TREE_MODEL(store));
    g_object_unref(store);
}

void treeview_ajouter_ligne(GtkWidget *treeview, gchar *valeurs[], int num_columns)
{
    GtkTreeIter iter;
    GtkListStore *store;
    int i;
    
    store = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(treeview)));
    gtk_list_store_append(store, &iter);
    
    for (i = 0; i < num_columns; i++) {
        gtk_list_store_set(store, &iter, i, valeurs[i], -1);
    }
}
gboolean demander_confirmation(GtkWidget *parent, const char *titre, const char *message)
{
    GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(gtk_widget_get_toplevel(parent)),
                                               GTK_DIALOG_MODAL,
                                               GTK_MESSAGE_QUESTION,
                                               GTK_BUTTONS_YES_NO,
                                               "%s", message);
    
    gtk_window_set_title(GTK_WINDOW(dialog), titre);
    gint response = gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
    
    return (response == GTK_RESPONSE_YES);
}
gboolean treeview_get_selected_row(GtkWidget *treeview, gchar *values[], int num_columns)
{
    GtkTreeSelection *selection;
    GtkTreeModel *model;
    GtkTreeIter iter;
    int i;
    
    selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(treeview));
    if (gtk_tree_selection_get_selected(selection, &model, &iter)) {
        for (i = 0; i < num_columns; i++) {
            gchar *value = NULL;
            gtk_tree_model_get(model, &iter, i, &value, -1);
            values[i] = value;
        }
        return TRUE;
    }
    return FALSE;
}

void treeview_supprimer_selection(GtkWidget *treeview)
{
    GtkTreeSelection *selection;
    GtkTreeModel *model;
    GtkTreeIter iter;
    
    selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(treeview));
    if (gtk_tree_selection_get_selected(selection, &model, &iter)) {
        gtk_list_store_remove(GTK_LIST_STORE(model), &iter);
    }
}

void charger_demandes_entraineur_dans_treeviewcoach(GtkWidget *treeview)
{
    FILE *f;
    gchar *row_values[6];
    char buffer[256];
    
    // Clear the treeview first
    treeview_vider(treeview);
    
    f = fopen("demandes.txt", "r");
    if (f != NULL) {
        // Read each line from the file
        while (fgets(buffer, sizeof(buffer), f)) {
            buffer[strcspn(buffer, "\n")] = 0; // Remove newline
            
            // Parse the line using semicolon as delimiter
            char *p = buffer;
            int fields = 0;
            char *field_ptrs[10];
            
            field_ptrs[fields++] = p;
            while (*p && fields < 10) {
                if (*p == ';') {
                    *p = '\0';
                    p++;
                    if (*p && *p != ';') {
                        field_ptrs[fields++] = p;
                    }
                } else {
                    p++;
                }
            }
            
            // We expect at least 5 fields: Type;Coach;Date/Heure;Durée;Note
            if (fields >= 5) {
                // Allocate memory for each column value
                row_values[0] = g_strdup(field_ptrs[0]); // Type (e.g., "COACH")
                row_values[1] = g_strdup(field_ptrs[1]); // Coach name
                row_values[2] = g_strdup(field_ptrs[2]); // Date/Heure
                row_values[3] = g_strdup(field_ptrs[3]); // Durée
                row_values[4] = g_strdup(field_ptrs[4]); // Note
                
                // If there are additional fields (like status or user ID), add them
                if (fields > 5) {
                    row_values[5] = g_strdup(field_ptrs[5]); // Additional info (e.g., member ID)
                } else {
                    row_values[5] = g_strdup("N/A"); // No additional info
                }
                
                // Add the row to the treeview (assuming 6 columns)
                treeview_ajouter_ligne(treeview, row_values, 6);
                
                // Free the allocated memory
                for (int i = 0; i < 6; i++) {
                    g_free(row_values[i]);
                }
            }
        }
        fclose(f);
    } else {
        // Create the file if it doesn't exist
        f = fopen("demandes.txt", "w");
        if (f) fclose(f);
        
        // Optional: Add a placeholder row to show the file is empty
        gchar *no_data[6] = {"Aucune", "donnée", "disponible", "dans", "demandes.txt", ""};
        treeview_ajouter_ligne(treeview, no_data, 6);
    }
}

// Function to filter demands for a specific trainer and load into treeviewcoach
void charger_demandes_entraineur_specifique_dans_treeviewcoach(GtkWidget *treeview, const char *nom_entraineur)
{
    FILE *f;
    gchar *row_values[6];
    char buffer[256];
    int demande_count = 0;
    
    // Clear the treeview first
    treeview_vider(treeview);
    
    f = fopen("demandes.txt", "r");
    if (f != NULL) {
        // Read each line from the file
        while (fgets(buffer, sizeof(buffer), f)) {
            buffer[strcspn(buffer, "\n")] = 0;
            
            // Parse the line
            char *p = buffer;
            int fields = 0;
            char *field_ptrs[10];
            
            field_ptrs[fields++] = p;
            while (*p && fields < 10) {
                if (*p == ';') {
                    *p = '\0';
                    p++;
                    if (*p && *p != ';') {
                        field_ptrs[fields++] = p;
                    }
                } else {
                    p++;
                }
            }
            
            // Check if this demand is for the current trainer
            if (fields >= 5) {
                // Extract coach name from field_ptrs[1]
                // Format in demandes.txt: "Nom Entraineur - Spécialité"
                char coach_info[100];
                strncpy(coach_info, field_ptrs[1], sizeof(coach_info) - 1);
                coach_info[sizeof(coach_info) - 1] = '\0';
                
                // Check if the coach name contains the trainer's name
                if (strstr(coach_info, nom_entraineur) != NULL) {
                    row_values[0] = g_strdup("Demande Coach"); // Type
                    row_values[1] = g_strdup(coach_info);     // Coach info
                    row_values[2] = g_strdup(field_ptrs[2]);  // Date/Heure
                    row_values[3] = g_strdup(field_ptrs[3]);  // Durée
                    row_values[4] = g_strdup(field_ptrs[4]);  // Note
                    
                    // Status column - default to "En attente"
                    if (fields > 5) {
                        row_values[5] = g_strdup(field_ptrs[5]); // Status
                    } else {
                        row_values[5] = g_strdup("En attente");
                    }
                    
                    treeview_ajouter_ligne(treeview, row_values, 6);
                    demande_count++;
                    
                    for (int i = 0; i < 6; i++) {
                        g_free(row_values[i]);
                    }
                }
            }
        }
        fclose(f);
        
        // If no demands found, show a message
        if (demande_count == 0) {
            gchar *no_demandes[6] = {"Aucune", "demande", "trouvée", "pour", "ce", "coach"};
            treeview_ajouter_ligne(treeview, no_demandes, 6);
        }
    } else {
        // Create the file if it doesn't exist
        f = fopen("demandes.txt", "w");
        if (f) fclose(f);
        
        // Add a placeholder row
        gchar *no_file[6] = {"Fichier", "demandes.txt", "vide", "ou", "inexistant", ""};
        treeview_ajouter_ligne(treeview, no_file, 6);
    }
}

// Function to refresh treeviewcoach
void rafraichir_treeviewcoach(GtkWidget *widget)
{
    GtkWidget *treeviewcoach = lookup_widget(widget, "treeviewcoach");
    if (treeviewcoach != NULL && GTK_IS_TREE_VIEW(treeviewcoach)) {
        charger_demandes_entraineur_dans_treeviewcoach(treeviewcoach);
    }
}


void afficher_message(GtkWidget *widget, const char *type, const char *message)
{
    GtkWidget *toplevel = gtk_widget_get_toplevel(widget);
    GtkMessageType msg_type;
    
    if (strcmp(type, "erreur") == 0)
        msg_type = GTK_MESSAGE_ERROR;
    else if (strcmp(type, "avertissement") == 0)
        msg_type = GTK_MESSAGE_WARNING;
    else
        msg_type = GTK_MESSAGE_INFO;
    
    GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(toplevel),
                                               GTK_DIALOG_MODAL,
                                               msg_type,
                                               GTK_BUTTONS_OK,
                                               "%s", message);
    
    gtk_window_set_title(GTK_WINDOW(dialog), "Message");
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
}

void vider_formulaire(GtkWidget *parent)
{
    if (!parent) return;
    
    GList *children = gtk_container_get_children(GTK_CONTAINER(parent));
    GList *iter;
    
    for (iter = children; iter != NULL; iter = iter->next)
    {
        GtkWidget *widget = GTK_WIDGET(iter->data);
        if (GTK_IS_ENTRY(widget))
        {
            gtk_entry_set_text(GTK_ENTRY(widget), "");
        }
        else if (GTK_IS_COMBO_BOX(widget))
        {
            gtk_combo_box_set_active(GTK_COMBO_BOX(widget), 0);
        }
        else if (GTK_IS_SPIN_BUTTON(widget))
        {
            gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget), 0);
        }
        else if (GTK_IS_TOGGLE_BUTTON(widget))
        {
            gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget), FALSE);
        }
        else if (GTK_IS_TEXT_VIEW(widget))
        {
            GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(widget));
            gtk_text_buffer_set_text(buffer, "", 0);
        }
    }
    
    g_list_free(children);
}

void fermer_fenetre(GtkWidget *widget)
{
    GtkWidget *window = gtk_widget_get_toplevel(widget);
    if (GTK_IS_WINDOW(window))
    {
        gtk_widget_destroy(window);
    }
}

void retour_vers_login(GtkWidget *widget)
{
    GtkWidget *current_window = gtk_widget_get_toplevel(widget);
    GtkWidget *login_window = create_Login();
    
    if (login_window != NULL)
    {
        gtk_widget_show_all(login_window);
        if (current_window != NULL && GTK_IS_WINDOW(current_window))
        {
            gtk_widget_destroy(current_window);
        }
    }
}

void charger_membres_dans_treeview(GtkWidget *treeview)
{
    FILE *f;
    Membre m;
    gchar *row_values[7];
    char id_str[20], age_str[20], tarif_str[20];
    
    treeview_vider(treeview);
    
    f = fopen("membres.txt", "r");
    if (f != NULL) {
        char buffer[256];
        while (fgets(buffer, sizeof(buffer), f)) {
            buffer[strcspn(buffer, "\n")] = 0;
            
            char *p = buffer;
            int fields = 0;
            char *field_ptrs[10];
            
            field_ptrs[fields++] = p;
            while (*p && fields < 10) {
                if (*p == ' ') {
                    *p = '\0';
                    p++;
                    if (*p && *p != ' ') {
                        field_ptrs[fields++] = p;
                    }
                } else {
                    p++;
                }
            }
            
            if (fields >= 7) {
                m.id = atoi(field_ptrs[0]);
                strcpy(m.nom, field_ptrs[1]);
                strcpy(m.email, field_ptrs[2]);
                m.age = atoi(field_ptrs[3]);
                
                if (fields > 7 && strcmp(field_ptrs[4], "Non") == 0) {
                    strcpy(m.sexe, "Non spécifié");
                    strcpy(m.Typeabonnement, field_ptrs[6]);
                    m.Tarif = atof(field_ptrs[7]);
                } else {
                    strcpy(m.sexe, field_ptrs[4]);
                    strcpy(m.Typeabonnement, field_ptrs[5]);
                    m.Tarif = atof(field_ptrs[6]);
                }
                
                sprintf(id_str, "%d", m.id);
                sprintf(age_str, "%d", m.age);
                sprintf(tarif_str, "%.2f DT", m.Tarif);
                
                row_values[0] = g_strdup(id_str);
                row_values[1] = g_strdup(m.nom);
                row_values[2] = g_strdup(m.email);
                row_values[3] = g_strdup(age_str);
                row_values[4] = g_strdup(m.sexe);
                row_values[5] = g_strdup(m.Typeabonnement);
                row_values[6] = g_strdup(tarif_str);
                
                treeview_ajouter_ligne(treeview, row_values, 7);
                
                for (int i = 0; i < 7; i++) {
                    g_free(row_values[i]);
                }
            }
        }
        fclose(f);
    } else {
        f = fopen("membres.txt", "w");
        if (f) fclose(f);
    }
}

//login
void
on_btn_login_clicked (GtkWidget *objet,
                      gpointer   user_data)
{
    GtkWidget *entry_username;
    GtkWidget *entry_password;
    GtkWidget *checkbutton_robot;
    const char *username;
    const char *password;
    gboolean is_robot;
    char user_type[20];
    int user_id;
    GtkWidget *login_window;
    
    entry_username = lookup_widget(objet, "entry2");
    entry_password = lookup_widget(objet, "entry3");
    checkbutton_robot = lookup_widget(objet, "checkbutton33");
    
    if (!entry_username || !entry_password || !checkbutton_robot) {
        afficher_message(objet, "erreur", "Erreur interne: widgets non trouvés");
        return;
    }
    
    username = gtk_entry_get_text(GTK_ENTRY(entry_username));
    password = gtk_entry_get_text(GTK_ENTRY(entry_password));
    is_robot = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(checkbutton_robot));
    
    if (strlen(username) == 0 || strlen(password) == 0)
    {
        afficher_message(objet, "erreur", "Veuillez remplir tous les champs!");
        return;
    }
    
    if (!is_robot)
    {
        afficher_message(objet, "erreur", "Veuillez confirmer que vous n'êtes pas un robot!");
        return;
    }
    
    if (verifier_login(username, password, user_type, &user_id))
    {
        login_window = lookup_widget(objet, "Login");
        if (login_window) {
            gtk_widget_hide(login_window);
        }
        
        if (strcmp(user_type, "admin") == 0)
        {
            GtkWidget *admin_window = create_Admin();
            if (admin_window != NULL) {
                gtk_widget_show_all(admin_window);
                start_refresh();
                initialiser_diagramme(admin_window);
                GtkWidget *treeview_membres = lookup_widget(admin_window, "treeview_membres");
                if (treeview_membres != NULL && GTK_IS_TREE_VIEW(treeview_membres)) {
                    const char *titles[] = {"ID", "Nom", "Email", "Age", "Sexe", "Abonnement", "Tarif"};
                    treeview_init_columns(treeview_membres, titles, 7);
                    charger_membres_dans_treeview(treeview_membres);
                }
                
                GtkWidget *treeview_entraineurs = lookup_widget(admin_window, "treeview_entraineurs");
                if (treeview_entraineurs != NULL && GTK_IS_TREE_VIEW(treeview_entraineurs)) {
                    const char *titles[] = {"ID", "Nom", "Spécialité", "Expérience", "Age", "Sexe"};
                    treeview_init_columns(treeview_entraineurs, titles, 6);
                    charger_entraineurs_dans_treeview(treeview_entraineurs);
                }
                
                GtkWidget *treeview_cours = lookup_widget(admin_window, "treeview_cours");
                if (treeview_cours != NULL && GTK_IS_TREE_VIEW(treeview_cours)) {
                    const char *titles[] = {"ID", "Cours", "Coach", "Salle", "Date/Heure", "Niveau"};
                    treeview_init_columns(treeview_cours, titles, 6);
                    charger_cours_dans_treeview(treeview_cours);
                }
                
                GtkWidget *treeview_equipements = lookup_widget(admin_window, "treeview_equipements");
                if (treeview_equipements != NULL && GTK_IS_TREE_VIEW(treeview_equipements)) {
                    const char *titles[] = {"Référence", "Type", "Centre", "Capacité", "Disponibilité", "État"};
                    treeview_init_columns(treeview_equipements, titles, 6);
                    charger_equipements_dans_treeview(treeview_equipements);
                }
                
                GtkWidget *treeview_centres = lookup_widget(admin_window, "treeview_centres");
                if (treeview_centres != NULL && GTK_IS_TREE_VIEW(treeview_centres)) {
                    const char *titles[] = {"ID", "Nom", "Adresse", "Ville", "Capacité"};
                    treeview_init_columns(treeview_centres, titles, 5);
                    charger_centres_dans_treeview(treeview_centres);
                }
            }
        }
        else if (strcmp(user_type, "membre") == 0)
        {
            GtkWidget *accueil_membre = create_Accueil_Membre();
            if (accueil_membre != NULL) {
                gtk_widget_show_all(accueil_membre);
            }
        }
        else if (strcmp(user_type, "entraineur") == 0)
	{
    	GtkWidget *entraineur_window = create_Accueil_Entraineur();
    	if (entraineur_window != NULL) {
        	gtk_widget_show_all(entraineur_window);
        
        // Load demands into treeviewcoach
        GtkWidget *treeviewcoach = lookup_widget(entraineur_window, "treeviewcoach");
        if (treeviewcoach != NULL && GTK_IS_TREE_VIEW(treeviewcoach)) {
            const char *titles[] = {"Type", "Coach", "Date/Heure", "Durée", "Note", "Statut"};
            treeview_init_columns(treeviewcoach, titles, 6);
            charger_demandes_entraineur_dans_treeviewcoach(treeviewcoach);
        	}
    	    }
	}
        
        if (login_window) {
            vider_formulaire(login_window);
        }
    }
    else
    {
        afficher_message(objet, "erreur", "Identifiants incorrects!");
    }
}

void on_btn_register_clicked(GtkWidget *objet, gpointer user_data) {
    GtkWidget *dialog = gtk_dialog_new_with_buttons(
        "Choisir le type d'inscription",
        GTK_WINDOW(gtk_widget_get_toplevel(objet)),
        GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
        "Membre", GTK_RESPONSE_YES,
        "Entraineur", GTK_RESPONSE_NO,
        "Annuler", GTK_RESPONSE_CANCEL,
        NULL);
    
    GtkWidget *label = gtk_label_new("Choisissez le type de compte que vous souhaitez créer:");
    GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    gtk_container_add(GTK_CONTAINER(content_area), label);
    gtk_widget_show(label);
    
    gint response = gtk_dialog_run(GTK_DIALOG(dialog));
    
    if (response == GTK_RESPONSE_YES) {
        GtkWidget *login_window = lookup_widget(objet, "Login");
        GtkWidget *signup_window = create_sign_up_Membre();
        
        if (signup_window != NULL) {
            gtk_widget_show_all(signup_window);
            if (login_window != NULL) {
                gtk_widget_hide(login_window);
            }
        }
    } else if (response == GTK_RESPONSE_NO) {
        GtkWidget *login_window = lookup_widget(objet, "Login");
        GtkWidget *coach_window = create_sign_up_Coach();
        
        if (coach_window != NULL) {
            gtk_widget_show_all(coach_window);
            if (login_window != NULL) {
                gtk_widget_hide(login_window);
            }
        }
    }
    
    gtk_widget_destroy(dialog);
}
//membre
void
on_treeview_membres_row_activated (GtkTreeView       *treeview,
                                   GtkTreePath       *path,
                                   GtkTreeViewColumn *column,
                                   gpointer           user_data)
{
    GtkTreeIter iter;
    GtkTreeModel *model = gtk_tree_view_get_model(treeview);
    gchar *id_str, *nom, *email, *age_str, *sexe, *abonnement, *tarif;
    
    if (gtk_tree_model_get_iter(model, &iter, path)) {
        gtk_tree_model_get(model, &iter, 
                          0, &id_str,
                          1, &nom,
                          2, &email,
                          3, &age_str,
                          4, &sexe,
                          5, &abonnement,
                          6, &tarif,
                          -1);
        
        GtkWidget *dialog = gtk_dialog_new_with_buttons(
            "Action sur le membre",
            GTK_WINDOW(gtk_widget_get_toplevel(GTK_WIDGET(treeview))),
            GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
            "Modifier", GTK_RESPONSE_YES,
            "Supprimer", GTK_RESPONSE_NO,
            NULL);
        
        char message[100];
        sprintf(message, "Membre: %s\nID: %s", nom, id_str);
        
        GtkWidget *label = gtk_label_new(message);
        GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
        gtk_container_add(GTK_CONTAINER(content_area), label);
        gtk_widget_show(label);
        
        gint response = gtk_dialog_run(GTK_DIALOG(dialog));
        
        if (response == GTK_RESPONSE_YES) {
            GtkWidget *modifier_window = create_ModifierMembre();
            if (modifier_window != NULL) {
                GtkWidget *entry_nom = lookup_widget(modifier_window, "entry_nom_membre_modif");
                if (entry_nom) {
                    gtk_entry_set_text(GTK_ENTRY(entry_nom), nom);
                }
                gtk_widget_show_all(modifier_window);
            }
            
        } else if (response == GTK_RESPONSE_NO) {
            char confirm_msg[150];
            sprintf(confirm_msg, "Supprimer le membre %s (ID: %s) ?", nom, id_str);
            
            if (demander_confirmation(GTK_WIDGET(treeview), "Confirmer suppression", confirm_msg)) {
                int id = atoi(id_str);
                if (supprimer_membre("membres.txt", id)) {
                    GtkTreeSelection *selection = gtk_tree_view_get_selection(treeview);
                    if (gtk_tree_selection_get_selected(selection, &model, &iter)) {
                        gtk_list_store_remove(GTK_LIST_STORE(model), &iter);
                    }
                    afficher_message(GTK_WIDGET(treeview), "info", "Membre supprimé!");
                } else {
                    afficher_message(GTK_WIDGET(treeview), "erreur", "Erreur suppression!");
                }
            }
        }
        
        gtk_widget_destroy(dialog);
        
        g_free(id_str);
        g_free(nom);
        g_free(email);
        g_free(age_str);
        g_free(sexe);
        g_free(abonnement);
        g_free(tarif);
    }
}
void on_btn_admin_rechercher_membre_clicked (GtkWidget *objet,
                                        gpointer   user_data)
{
    GtkWidget *entry_id;
    GtkWidget *treeview_membres;
    const char *id_text;
    int id;
    Membre m;
    
    entry_id = lookup_widget(objet, "entry11");
    treeview_membres = lookup_widget(objet, "treeview_membres");
    
    if (!treeview_membres) {
        afficher_message(objet, "erreur", "TreeView non trouvé!");
        return;
    }
    
    id_text = gtk_entry_get_text(GTK_ENTRY(entry_id));
    
    if (strlen(id_text) > 0)
    {
        id = atoi(id_text);
        m = chercher_membre("membres.txt", id);
        
        if (m.id != -1)
        {
            treeview_vider(treeview_membres);
            
            gchar *row_values[7];
            char id_str[20], age_str[20], tarif_str[20];
            
            sprintf(id_str, "%d", m.id);
            sprintf(age_str, "%d", m.age);
            sprintf(tarif_str, "%.2f DT", m.Tarif);
            
            row_values[0] = g_strdup(id_str);
            row_values[1] = g_strdup(m.nom);
            row_values[2] = g_strdup(m.email);
            row_values[3] = g_strdup(age_str);
            row_values[4] = g_strdup(m.sexe);
            row_values[5] = g_strdup(m.Typeabonnement);
            row_values[6] = g_strdup(tarif_str);
            
            treeview_ajouter_ligne(treeview_membres, row_values, 7);
            
            for (int i = 0; i < 7; i++) {
                g_free(row_values[i]);
            }
        }
        else
        {
            afficher_message(objet, "avertissement", "Membre non trouvé!");
        }
    }
    else
    {
        charger_membres_dans_treeview(treeview_membres);
    }
}

void on_btn_admin_supprimer_membre_clicked(GtkWidget *widget, gpointer user_data) {
    GtkWidget *entry_id = lookup_widget(widget, "entry11");
    
    if (!entry_id) {
        afficher_message(widget, "erreur", "Champ ID non trouvé!");
        return;
    }
    
    const char *id_text = gtk_entry_get_text(GTK_ENTRY(entry_id));
    
    if (strlen(id_text) == 0) {
        afficher_message(widget, "erreur", "Veuillez entrer un ID!");
        return;
    }
    
    int id = atoi(id_text);
    
    if (supprimer_membre("membres.txt", id)) {
        afficher_message(widget, "info", "Membre supprimé!");
        gtk_entry_set_text(GTK_ENTRY(entry_id), "");
        
        GtkWidget *treeview_membres = lookup_widget(widget, "treeview_membres");
        if (treeview_membres) {
            charger_membres_dans_treeview(treeview_membres);
        }
    } else {
        afficher_message(widget, "erreur", "Membre non trouvé!");
    }
}
void
on_btn_modifier_membre_annuler_clicked (GtkWidget *objet,
                                        gpointer   user_data)
{
    fermer_fenetre(objet);
}
void
on_btn_admin_modifier_membre_clicked (GtkWidget *objet,
                                      gpointer   user_data)
{
    GtkWidget *modifier_membre_window = create_ModifierMembre();
    if (modifier_membre_window != NULL)
    {
        gtk_widget_show_all(modifier_membre_window);
    }
}

void
on_btn_admin_ajouter_membre_clicked (GtkWidget *objet,
                                     gpointer   user_data)
{
    GtkWidget *ajouter_membre_window = create_AjouterMembre();
    if (ajouter_membre_window != NULL)
    {
        gtk_widget_show_all(ajouter_membre_window);
    }
}

void
on_btn_admin_membre_back_login_clicked (GtkWidget *objet,
                                        gpointer   user_data)
{
    retour_vers_login(objet);
}
void
on_btn_modifier_membre_enregistrer_clicked (GtkWidget *objet,
                                            gpointer   user_data)
{
    GtkWidget *entry_recherche = lookup_widget(objet, "entry_recherche_membre");
    if (!entry_recherche) {
        entry_recherche = lookup_widget(objet, "entry_recherche");
    }
    if (!entry_recherche) {
        entry_recherche = lookup_widget(objet, "entry_nom_membre_modif");
    }
    
    if (!entry_recherche) {
        afficher_message(objet, "erreur", "Champ de recherche non trouvé!");
        return;
    }
    
    const char *nom_recherche = gtk_entry_get_text(GTK_ENTRY(entry_recherche));
    
    if (strlen(nom_recherche) == 0) {
        afficher_message(objet, "erreur", "Veuillez d'abord rechercher un membre!");
        return;
    }
    
    Membre m = chercher_membre_par_nom("membres.txt", nom_recherche);
    
    if (m.id == -1) {
        afficher_message(objet, "erreur", "Membre non trouvé!");
        return;
    }
    
    int current_editing_member_id = m.id;
    
    GtkWidget *entry_nom_prenom = lookup_widget(objet, "entry_nom_membre_modif");
    if (!entry_nom_prenom) {
        entry_nom_prenom = lookup_widget(objet, "entry_nom_prenom_modif");
    }
    
    GtkWidget *entry_email = lookup_widget(objet, "entry_email_membre_modif");
    if (!entry_email) {
        entry_email = lookup_widget(objet, "entry_email");
    }
    
    GtkWidget *spin_age = lookup_widget(objet, "spin_age_membre_modif");
    if (!spin_age) {
        spin_age = lookup_widget(objet, "spin_age");
    }
    
    GtkWidget *radio_mensuelle = lookup_widget(objet, "radio_mensuelle_membre_modif");
    if (!radio_mensuelle) {
        radio_mensuelle = lookup_widget(objet, "radio_mensuelle");
    }
    
    GtkWidget *radio_annuelle = lookup_widget(objet, "radio_annuelle_membre_modif");
    if (!radio_annuelle) {
        radio_annuelle = lookup_widget(objet, "radio_annuelle");
    }
    
    if (!entry_nom_prenom || !entry_email || !spin_age || !radio_mensuelle || !radio_annuelle) {
        afficher_message(objet, "erreur", "Erreur: widgets non trouvés!");
        return;
    }
    
    const char *nom_prenom = gtk_entry_get_text(GTK_ENTRY(entry_nom_prenom));
    const char *email = gtk_entry_get_text(GTK_ENTRY(entry_email));
    int age = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(spin_age));
    gboolean annuelle = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio_annuelle));
    
    if (strlen(nom_prenom) == 0 || strlen(email) == 0) {
        afficher_message(objet, "erreur", "Veuillez remplir tous les champs!");
        return;
    }
    
    if (age <= 0 || age > 120) {
        afficher_message(objet, "erreur", "Âge invalide!");
        return;
    }
    
    Membre nouv;
    nouv.id = current_editing_member_id;
    strcpy(nouv.nom, nom_prenom);
    strcpy(nouv.email, email);
    nouv.age = age;
    strcpy(nouv.sexe, "Non spécifié");
    strcpy(nouv.Typeabonnement, annuelle ? "Annuelle" : "Mensuelle");
    nouv.Tarif = annuelle ? 500.0 : 50.0;
    
    if (modifier_membre("membres.txt", current_editing_member_id, nouv)) {
        afficher_message(objet, "info", "Membre modifié avec succès!");
        fermer_fenetre(objet);
    } else {
        afficher_message(objet, "erreur", "Erreur lors de la modification!");
    }
}
void
on_btn_ajouter_membre_enregistrer_clicked (GtkWidget *objet,
                                           gpointer   user_data)
{
    GtkWidget *entry_nom_prenom;
    GtkWidget *entry_email;
    GtkWidget *combo_gouvernerat;
    GtkWidget *spin_age;
    GtkWidget *radio_mensuelle;
    GtkWidget *radio_annuelle;
    const char *nom_prenom;
    const char *email;
    const char *gouvernerat;
    int age;
    const char *abonnement;
    Membre m;
    
    entry_nom_prenom = lookup_widget(objet, "entry_nom_prenom");
    entry_email = lookup_widget(objet, "entry_email");
    combo_gouvernerat = lookup_widget(objet, "combo_gouvernerat");
    spin_age = lookup_widget(objet, "spin_age");
    radio_mensuelle = lookup_widget(objet, "radio_mensuelle");
    radio_annuelle = lookup_widget(objet, "radio_annuelle");
    
    if (entry_nom_prenom == NULL || entry_email == NULL || combo_gouvernerat == NULL || 
        spin_age == NULL || radio_mensuelle == NULL || radio_annuelle == NULL)
    {
        afficher_message(objet, "erreur", "Erreur: Widgets non trouvés!");
        return;
    }
    
    nom_prenom = gtk_entry_get_text(GTK_ENTRY(entry_nom_prenom));
    email = gtk_entry_get_text(GTK_ENTRY(entry_email));
    gouvernerat = gtk_combo_box_get_active_text(GTK_COMBO_BOX(combo_gouvernerat));
    age = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(spin_age));
    
    if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio_annuelle)))
        abonnement = "Annuelle";
    else
        abonnement = "Mensuelle";
    
    if (strlen(nom_prenom) == 0 || strlen(email) == 0)
    {
        afficher_message(objet, "erreur", "Veuillez remplir le nom et l'email!");
        return;
    }
    
    if (gouvernerat == NULL || strlen(gouvernerat) == 0)
    {
        afficher_message(objet, "erreur", "Veuillez sélectionner un gouvernerat!");
        return;
    }
    
    if (age <= 0 || age > 120)
    {
        afficher_message(objet, "erreur", "Âge invalide! Doit être entre 1 et 120 ans.");
        return;
    }
    
    if (strchr(email, '@') == NULL)
    {
        afficher_message(objet, "erreur", "Email invalide! L'email doit contenir '@'");
        return;
    }
    
    FILE *f = fopen("membres.txt", "r");
    int max_id = 0;
    Membre temp;
    if (f != NULL) {
       while (fscanf(f, "%d %49s %49s %d %9s %19s %f", 
             &temp.id, temp.nom, temp.email, &temp.age, temp.sexe, 
             temp.Typeabonnement, &temp.Tarif) == 7) {
            if (temp.id > max_id) {
                max_id = temp.id;
            }
        }
        fclose(f);
    }
    
    m.id = max_id + 1;
    strncpy(m.nom, nom_prenom, sizeof(m.nom) - 1);
    m.nom[sizeof(m.nom) - 1] = '\0';
    
    strncpy(m.email, email, sizeof(m.email) - 1);
    m.email[sizeof(m.email) - 1] = '\0';
    
    m.age = age;
    strncpy(m.sexe, "Non spécifié", sizeof(m.sexe) - 1);
    m.sexe[sizeof(m.sexe) - 1] = '\0';
    
    strncpy(m.Typeabonnement, abonnement, sizeof(m.Typeabonnement) - 1);
    m.Typeabonnement[sizeof(m.Typeabonnement) - 1] = '\0';
    
    m.Tarif = (strcmp(abonnement, "Annuelle") == 0) ? 500.0 : 50.0;
    
    if (ajouter_membre("membres.txt", m))
    {
        afficher_message(objet, "info", "Membre ajouté avec succès!");
        fermer_fenetre(objet);
    }
    else
    {
        afficher_message(objet, "erreur", "Erreur lors de l'ajout du membre!");
    }
}

void
on_btn_ajouter_membre_annuler_clicked (GtkWidget *objet,
                                       gpointer   user_data)
{
    fermer_fenetre(objet);
}
void
on_btn_modifier_membre_chercher_clicked (GtkWidget *objet,
                                         gpointer   user_data)
{
    GtkWidget *entry_recherche = lookup_widget(objet, "entry_nom_membre_modif");
    if (!entry_recherche) {
        entry_recherche = lookup_widget(objet, "entry_recherche_membre");
    }
    if (!entry_recherche) {
        entry_recherche = lookup_widget(objet, "entry_nom_prenom_modif");
    }
    if (!entry_recherche) {
        entry_recherche = lookup_widget(objet, "entry_recherche");
    }
    
    if (!entry_recherche) {
        afficher_message(objet, "erreur", "Champ de recherche non trouvé!");
        return;
    }
    
    const char *nom_recherche = gtk_entry_get_text(GTK_ENTRY(entry_recherche));
    
    if (strlen(nom_recherche) == 0) {
        afficher_message(objet, "erreur", "Veuillez entrer un nom à rechercher!");
        return;
    }
    
    Membre m = chercher_membre_par_nom("membres.txt", nom_recherche);
    
    if (m.id != -1) {
        GtkWidget *entry_email = lookup_widget(objet, "entry_email_membre_modif");
        if (!entry_email) {
            entry_email = lookup_widget(objet, "entry_email");
        }
        
        GtkWidget *spin_age = lookup_widget(objet, "spin_age_membre_modif");
        if (!spin_age) {
            spin_age = lookup_widget(objet, "spin_age");
        }
        
        GtkWidget *combo_gouvernerat = lookup_widget(objet, "combo_gouvernerat_membre_modif");
        if (!combo_gouvernerat) {
            combo_gouvernerat = lookup_widget(objet, "combo_gouvernerat");
        }
        
        GtkWidget *radio_mensuelle = lookup_widget(objet, "radio_mensuelle_membre_modif");
        if (!radio_mensuelle) {
            radio_mensuelle = lookup_widget(objet, "radio_mensuelle");
        }
        
        GtkWidget *radio_annuelle = lookup_widget(objet, "radio_annuelle_membre_modif");
        if (!radio_annuelle) {
            radio_annuelle = lookup_widget(objet, "radio_annuelle");
        }
        
        if (entry_email) {
            gtk_entry_set_text(GTK_ENTRY(entry_email), m.email);
        }
        
        if (spin_age) {
            gtk_spin_button_set_value(GTK_SPIN_BUTTON(spin_age), m.age);
        }
        
        if (combo_gouvernerat) {
            gtk_combo_box_set_active(GTK_COMBO_BOX(combo_gouvernerat), 0);
        }
        
        if (radio_mensuelle && radio_annuelle) {
            if (strcmp(m.Typeabonnement, "Annuelle") == 0) {
                gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(radio_annuelle), TRUE);
                gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(radio_mensuelle), FALSE);
            } else {
                gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(radio_mensuelle), TRUE);
                gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(radio_annuelle), FALSE);
            }
        }
        
        afficher_message(objet, "info", "Membre trouvé! Modifiez les champs et cliquez sur Enregistrer.");
    } else {
        afficher_message(objet, "erreur", "Membre non trouvé!");
    }
}
//cours
void charger_cours_dans_treeview(GtkWidget *treeview)
{
    FILE *f;
    Cours c;
    gchar *row_values[6];
    char id_str[20];
    
    treeview_vider(treeview);
    
    f = fopen("cours.txt", "r");
    if (f != NULL) {
        while (fscanf(f, "%d %49s %49s %49s %49s %49s", 
                     &c.id, c.cours, c.coach, c.salle, 
                     c.date_heure, c.niveau) == 6) {
            
            sprintf(id_str, "%d", c.id);
            
            row_values[0] = g_strdup(id_str);
            row_values[1] = g_strdup(c.cours);
            row_values[2] = g_strdup(c.coach);
            row_values[3] = g_strdup(c.salle);
            row_values[4] = g_strdup(c.date_heure);
            row_values[5] = g_strdup(c.niveau);
            
            treeview_ajouter_ligne(treeview, row_values, 6);
            
            for (int i = 0; i < 6; i++) {
                g_free(row_values[i]);
            }
        }
        fclose(f);
    } else {
        f = fopen("cours.txt", "w");
        if (f) fclose(f);
    }
}
void on_btn_admin_rechercher_cours_clicked (GtkWidget *objet,
                                       gpointer   user_data)
{
    GtkWidget *entry_id;
    GtkWidget *treeview_cours;
    const char *id_text;
    int id;
    Cours c;
    
    entry_id = lookup_widget(objet, "entry12");
    treeview_cours = lookup_widget(objet, "treeview_cours");
    
    if (!treeview_cours) {
        afficher_message(objet, "erreur", "TreeView non trouvé!");
        return;
    }
    
    id_text = gtk_entry_get_text(GTK_ENTRY(entry_id));
    
    if (strlen(id_text) > 0)
    {
        id = atoi(id_text);
        c = chercher_cours("cours.txt", id);
        
        if (c.id != -1)
        {
            treeview_vider(treeview_cours);
            
            gchar *row_values[6];
            char id_str[20];
            
            sprintf(id_str, "%d", c.id);
            
            row_values[0] = g_strdup(id_str);
            row_values[1] = g_strdup(c.cours);
            row_values[2] = g_strdup(c.coach);
            row_values[3] = g_strdup(c.salle);
            row_values[4] = g_strdup(c.date_heure);
            row_values[5] = g_strdup(c.niveau);
            
            treeview_ajouter_ligne(treeview_cours, row_values, 6);
            
            for (int i = 0; i < 6; i++) {
                g_free(row_values[i]);
            }
        }
        else
        {
            afficher_message(objet, "avertissement", "Cours non trouvé!");
        }
    }
    else
    {
        charger_cours_dans_treeview(treeview_cours);
    }
}

void on_btn_admin_supprimer_cours_clicked(GtkWidget *widget, gpointer user_data) {
    GtkWidget *entry_id = lookup_widget(widget, "entry12");
    
    if (!entry_id) {
        afficher_message(widget, "erreur", "Champ ID non trouvé!");
        return;
    }
    
    const char *id_text = gtk_entry_get_text(GTK_ENTRY(entry_id));
    
    if (strlen(id_text) == 0) {
        afficher_message(widget, "erreur", "Veuillez entrer un ID!");
        return;
    }
    
    int id = atoi(id_text);
    
    if (supprimer_cours("cours.txt", id)) {
        afficher_message(widget, "info", "Cours supprimé!");
        gtk_entry_set_text(GTK_ENTRY(entry_id), "");
        
        GtkWidget *treeview_cours = lookup_widget(widget, "treeview_cours");
        if (treeview_cours) {
            charger_cours_dans_treeview(treeview_cours);
        }
    } else {
        afficher_message(widget, "erreur", "Cours non trouvé!");
    }
}

void
on_btn_admin_modifier_cours_clicked (GtkWidget *objet,
                                     gpointer   user_data)
{
    GtkWidget *modifier_cours_window = create_Modifiercours();
    if (modifier_cours_window != NULL)
    {
        gtk_widget_show_all(modifier_cours_window);
    }
}

void
on_btn_admin_ajouter_cours_clicked (GtkWidget *objet,
                                    gpointer   user_data)
{
    GtkWidget *ajouter_cours_window = create_Ajoutercours();
    if (ajouter_cours_window != NULL)
    {
        gtk_widget_show_all(ajouter_cours_window);
    }
}

void
on_btn_admin_cours_back_login_clicked (GtkWidget *objet,
                                       gpointer   user_data)
{
    retour_vers_login(objet);
}
void
on_btn_modifier_cours_chercher_clicked (GtkWidget *objet,
                                        gpointer   user_data)
{
    GtkWidget *entry_recherche = lookup_widget(objet, "entry_id_cours_modif");
    if (!entry_recherche) {
        entry_recherche = lookup_widget(objet, "entry_recherche_cours");
    }
    if (!entry_recherche) {
        entry_recherche = lookup_widget(objet, "entry_recherche");
    }
    
    if (!entry_recherche) {
        afficher_message(objet, "erreur", "Champ de recherche non trouvé!");
        return;
    }
    
    const char *id_text = gtk_entry_get_text(GTK_ENTRY(entry_recherche));
    
    if (strlen(id_text) == 0) {
        afficher_message(objet, "erreur", "Veuillez entrer un ID à rechercher!");
        return;
    }
    
    int id = atoi(id_text);
    Cours c = chercher_cours("cours.txt", id);
    
    if (c.id != -1) {
        GtkWidget *entry_nom = lookup_widget(objet, "entry_nom_cours_modif");
        GtkWidget *combo_salle = lookup_widget(objet, "combo_salle_cours_modif");
        GtkWidget *entry_date = lookup_widget(objet, "entry_date_cours_modif");
        GtkWidget *spin_niveau = lookup_widget(objet, "spin_niveau_cours_modif");
        GtkWidget *radio_avec_coach = lookup_widget(objet, "radio_avec_coach_cours_modif");
        GtkWidget *radio_sans_coach = lookup_widget(objet, "radio_sans_coach_cours_modif");
        
        if (entry_nom) {
            gtk_entry_set_text(GTK_ENTRY(entry_nom), c.cours);
        }
        
        if (combo_salle && GTK_IS_COMBO_BOX(combo_salle)) {
            gtk_combo_box_set_active(GTK_COMBO_BOX(combo_salle), 0);
            GtkWidget *entry = gtk_bin_get_child(GTK_BIN(combo_salle));
            if (entry) {
                gtk_entry_set_text(GTK_ENTRY(entry), c.salle);
            }
        }
        
        if (entry_date) {
            gtk_entry_set_text(GTK_ENTRY(entry_date), c.date_heure);
        }
        
        if (spin_niveau) {
            int niveau_value = 1;
            if (strstr(c.niveau, "Débutant") != NULL) niveau_value = 1;
            else if (strstr(c.niveau, "Intermédiaire") != NULL) niveau_value = 2;
            else if (strstr(c.niveau, "Avancé") != NULL) niveau_value = 3;
            gtk_spin_button_set_value(GTK_SPIN_BUTTON(spin_niveau), niveau_value);
        }
        
        if (radio_avec_coach && radio_sans_coach) {
            if (strstr(c.coach, "Avec coach") != NULL) {
                gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(radio_avec_coach), TRUE);
                gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(radio_sans_coach), FALSE);
            } else {
                gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(radio_sans_coach), TRUE);
                gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(radio_avec_coach), FALSE);
            }
        }
        
        afficher_message(objet, "info", "Cours trouvé! Modifiez les champs et cliquez sur Enregistrer.");
    } else {
        afficher_message(objet, "erreur", "Cours non trouvé!");
    }
}

void
on_btn_modifier_cours_enregistrer_clicked (GtkWidget *objet,
                                           gpointer   user_data)
{
    GtkWidget *entry_recherche = lookup_widget(objet, "entry_id_cours_modif");
    if (!entry_recherche) {
        afficher_message(objet, "erreur", "Champ de recherche non trouvé!");
        return;
    }
    
    const char *id_text = gtk_entry_get_text(GTK_ENTRY(entry_recherche));
    
    if (strlen(id_text) == 0) {
        afficher_message(objet, "erreur", "Veuillez d'abord rechercher un cours!");
        return;
    }
    
    int id = atoi(id_text);
    Cours c = chercher_cours("cours.txt", id);
    
    if (c.id == -1) {
        afficher_message(objet, "erreur", "Cours non trouvé!");
        return;
    }
    
    int current_editing_cours_id = c.id;
    
    GtkWidget *entry_nom = lookup_widget(objet, "entry_nom_cours_modif");
    GtkWidget *combo_salle = lookup_widget(objet, "combo_salle_cours_modif");
    GtkWidget *entry_date = lookup_widget(objet, "entry_date_cours_modif");
    GtkWidget *spin_niveau = lookup_widget(objet, "spin_niveau_cours_modif");
    GtkWidget *radio_avec_coach = lookup_widget(objet, "radio_avec_coach_cours_modif");
    GtkWidget *radio_sans_coach = lookup_widget(objet, "radio_sans_coach_cours_modif");
    
    if (!entry_nom || !combo_salle || !entry_date || !spin_niveau || !radio_avec_coach || !radio_sans_coach) {
        afficher_message(objet, "erreur", "Erreur: widgets non trouvés!");
        return;
    }
    
    const char *nom = gtk_entry_get_text(GTK_ENTRY(entry_nom));
    const char *date = gtk_entry_get_text(GTK_ENTRY(entry_date));
    int niveau = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(spin_niveau));
    gboolean avec_coach = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio_avec_coach));
    
    gchar *salle_text = NULL;
    if (GTK_IS_COMBO_BOX(combo_salle)) {
        GtkWidget *entry = gtk_bin_get_child(GTK_BIN(combo_salle));
        if (entry) {
            salle_text = g_strdup(gtk_entry_get_text(GTK_ENTRY(entry)));
        }
    }
    
    if (strlen(nom) == 0 || !salle_text || strlen(date) == 0) {
        afficher_message(objet, "erreur", "Veuillez remplir tous les champs!");
        if (salle_text) g_free(salle_text);
        return;
    }
    
    if (niveau < 1 || niveau > 3) {
        afficher_message(objet, "erreur", "Niveau invalide! Doit être entre 1 et 3.");
        if (salle_text) g_free(salle_text);
        return;
    }
    
    Cours nouv;
    nouv.id = current_editing_cours_id;
    strcpy(nouv.cours, nom);
    strcpy(nouv.salle, salle_text);
    strcpy(nouv.date_heure, date);
    strcpy(nouv.coach, avec_coach ? "Avec coach" : "Sans coach");
    
    char niveau_text[20];
    if (niveau == 1) strcpy(niveau_text, "Débutant");
    else if (niveau == 2) strcpy(niveau_text, "Intermédiaire");
    else if (niveau == 3) strcpy(niveau_text, "Avancé");
    else sprintf(niveau_text, "Niveau %d", niveau);
    strcpy(nouv.niveau, niveau_text);
    
    if (modifier_cours("cours.txt", current_editing_cours_id, nouv)) {
        afficher_message(objet, "info", "Cours modifié avec succès!");
        fermer_fenetre(objet);
    } else {
        afficher_message(objet, "erreur", "Erreur lors de la modification!");
    }
    
    if (salle_text) g_free(salle_text);
}


void
on_btn_modifier_cours_annuler_clicked (GtkWidget *objet,
                                       gpointer   user_data)
{
    fermer_fenetre(objet);
}

void
on_btn_ajouter_cours_enregistrer_clicked (GtkWidget *objet,
                                          gpointer   user_data)
{
    GtkWidget *entry_id;
    GtkWidget *entry_nom;
    GtkWidget *combo_salle;
    GtkWidget *entry_date;
    GtkWidget *spin_niveau;
    GtkWidget *radio_avec_coach;
    GtkWidget *radio_sans_coach;
    const char *id_text;
    const char *nom;
    const char *date;
    const char *salle;
    int niveau;
    gboolean avec_coach;
    int id;
    Cours c;
    char niveau_text[20];
    
    entry_id = lookup_widget(objet, "entry_ajouter_cours_id");
    entry_nom = lookup_widget(objet, "entry_ajouter_cours_nom");
    combo_salle = lookup_widget(objet, "combo_ajouter_cours_salle");
    entry_date = lookup_widget(objet, "entry_ajouter_cours_date");
    spin_niveau = lookup_widget(objet, "spin_ajouter_cours_niveau");
    radio_avec_coach = lookup_widget(objet, "radio_ajouter_cours_avec_coach");
    radio_sans_coach = lookup_widget(objet, "radio_ajouter_cours_sans_coach");
    
    if (entry_id == NULL || entry_nom == NULL || combo_salle == NULL || 
        entry_date == NULL || spin_niveau == NULL || radio_avec_coach == NULL || 
        radio_sans_coach == NULL)
    {
        afficher_message(objet, "erreur", "Erreur: Widgets non trouvés!");
        return;
    }
    
    id_text = gtk_entry_get_text(GTK_ENTRY(entry_id));
    nom = gtk_entry_get_text(GTK_ENTRY(entry_nom));
    date = gtk_entry_get_text(GTK_ENTRY(entry_date));
    salle = gtk_combo_box_get_active_text(GTK_COMBO_BOX(combo_salle));
    niveau = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(spin_niveau));
    avec_coach = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio_avec_coach));
    
    if (strlen(id_text) == 0)
    {
        afficher_message(objet, "erreur", "Veuillez entrer un ID!");
        return;
    }
    
    if (strlen(nom) == 0)
    {
        afficher_message(objet, "erreur", "Veuillez entrer un nom de cours!");
        return;
    }
    
    if (salle == NULL || strlen(salle) == 0)
    {
        afficher_message(objet, "erreur", "Veuillez sélectionner une salle!");
        return;
    }
    
    if (strlen(date) == 0)
    {
        afficher_message(objet, "erreur", "Veuillez entrer une date/heure!");
        return;
    }
    
    id = atoi(id_text);
    if (id <= 0)
    {
        afficher_message(objet, "erreur", "L'ID doit être un nombre positif!");
        return;
    }
    
    c.id = id;
    strncpy(c.cours, nom, sizeof(c.cours) - 1);
    c.cours[sizeof(c.cours) - 1] = '\0';
    
    if (avec_coach)
        strcpy(c.coach, "Avec_coach");
    else
        strcpy(c.coach, "Sans_coach");
    
    strncpy(c.date_heure, date, sizeof(c.date_heure) - 1);
    c.date_heure[sizeof(c.date_heure) - 1] = '\0';
    
    strncpy(c.salle, salle, sizeof(c.salle) - 1);
    c.salle[sizeof(c.salle) - 1] = '\0';
    
    if (niveau == 1)
        strcpy(niveau_text, "Débutant");
    else if (niveau == 2)
        strcpy(niveau_text, "Intermédiaire");
    else if (niveau == 3)
        strcpy(niveau_text, "Avancé");
    else
        sprintf(niveau_text, "Niveau_%d", niveau);
    
    strncpy(c.niveau, niveau_text, sizeof(c.niveau) - 1);
    c.niveau[sizeof(c.niveau) - 1] = '\0';
    
    if (ajouter_cours("cours.txt", c))
    {
        afficher_message(objet, "info", "Cours ajouté avec succès!");
        fermer_fenetre(objet);
    }
    else
    {
        afficher_message(objet, "erreur", "Erreur lors de l'ajout du cours!");
    }
}
void
on_treeview_cours_row_activated (GtkTreeView       *treeview,
                                 GtkTreePath       *path,
                                 GtkTreeViewColumn *column,
                                 gpointer           user_data)
{
    GtkTreeIter iter;
    GtkTreeModel *model = gtk_tree_view_get_model(treeview);
    gchar *id_str, *cours, *coach, *salle, *date_heure, *niveau;
    
    if (gtk_tree_model_get_iter(model, &iter, path)) {
        gtk_tree_model_get(model, &iter, 
                          0, &id_str,
                          1, &cours,
                          2, &coach,
                          3, &salle,
                          4, &date_heure,
                          5, &niveau,
                          -1);
        

        GtkWidget *dialog = gtk_dialog_new_with_buttons(
            "Action sur le cours",
            GTK_WINDOW(gtk_widget_get_toplevel(GTK_WIDGET(treeview))),
            GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
            "Modifier", GTK_RESPONSE_YES,
            "Supprimer", GTK_RESPONSE_NO,
            NULL);
        
        char message[100];
        sprintf(message, "Cours: %s\nID: %s", cours, id_str);
        
        GtkWidget *label = gtk_label_new(message);
        GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
        gtk_container_add(GTK_CONTAINER(content_area), label);
        gtk_widget_show(label);
        
        gint response = gtk_dialog_run(GTK_DIALOG(dialog));
        
        if (response == GTK_RESPONSE_YES) {

            GtkWidget *modifier_window = create_Modifiercours();
            if (modifier_window != NULL) {
                GtkWidget *entry_id = lookup_widget(modifier_window, "entry_id_cours_modif");
                if (entry_id) {
                    gtk_entry_set_text(GTK_ENTRY(entry_id), id_str);
                }
                gtk_widget_show_all(modifier_window);
            }
            
        } else if (response == GTK_RESPONSE_NO) {

            char confirm_msg[150];
            sprintf(confirm_msg, "Supprimer le cours %s (ID: %s) ?", cours, id_str);
            
            if (demander_confirmation(GTK_WIDGET(treeview), "Confirmer suppression", confirm_msg)) {
                int id = atoi(id_str);
                if (supprimer_cours("cours.txt", id)) {

                    GtkTreeSelection *selection = gtk_tree_view_get_selection(treeview);
                    if (gtk_tree_selection_get_selected(selection, &model, &iter)) {
                        gtk_list_store_remove(GTK_LIST_STORE(model), &iter);
                    }
                    afficher_message(GTK_WIDGET(treeview), "info", "Cours supprimé!");
                } else {
                    afficher_message(GTK_WIDGET(treeview), "erreur", "Erreur suppression!");
                }
            }
        }
        
        gtk_widget_destroy(dialog);
        

        g_free(id_str);
        g_free(cours);
        g_free(coach);
        g_free(salle);
        g_free(date_heure);
        g_free(niveau);
    }
}
void
on_btn_ajouter_cours_annuler_clicked (GtkWidget *objet,
                                      gpointer   user_data)
{
    fermer_fenetre(objet);
}
//entraineur
void charger_entraineurs_dans_treeview(GtkWidget *treeview)
{
    FILE *f;
    Entraineur e;
    gchar *row_values[6];
    char id_str[20], exp_str[20], age_str[20], sexe_str[2];
    
    treeview_vider(treeview);
    
    f = fopen("entraineurs.txt", "r");
    if (f != NULL) {
        while (fscanf(f, "%d %49s %49s %d %d %c", 
                     &e.id, e.nometprenom, e.specialite, 
                     &e.experience, &e.age, &e.sexe) == 6) {
            
            sprintf(id_str, "%d", e.id);
            sprintf(exp_str, "%d ans", e.experience);
            sprintf(age_str, "%d", e.age);
            sexe_str[0] = e.sexe;
            sexe_str[1] = '\0';
            
            row_values[0] = g_strdup(id_str);
            row_values[1] = g_strdup(e.nometprenom);
            row_values[2] = g_strdup(e.specialite);
            row_values[3] = g_strdup(exp_str);
            row_values[4] = g_strdup(age_str);
            row_values[5] = g_strdup(sexe_str);
            
            treeview_ajouter_ligne(treeview, row_values, 6);
            
            for (int i = 0; i < 6; i++) {
                g_free(row_values[i]);
            }
        }
        fclose(f);
    } else {
        f = fopen("entraineurs.txt", "w");
        if (f) fclose(f);
    }
}
void on_btn_admin_rechercher_entraineur_clicked (GtkWidget *objet,
                                            gpointer   user_data)
{
    GtkWidget *entry_id;
    GtkWidget *treeview_entraineurs;
    const char *id_text;
    int id;
    Entraineur e;
    
    entry_id = lookup_widget(objet, "entry13");
    treeview_entraineurs = lookup_widget(objet, "treeview_entraineurs");
    
    if (!treeview_entraineurs) {
        afficher_message(objet, "erreur", "TreeView non trouvé!");
        return;
    }
    
    id_text = gtk_entry_get_text(GTK_ENTRY(entry_id));
    
    if (strlen(id_text) > 0)
    {
        id = atoi(id_text);
        e = chercher_entraineur("entraineurs.txt", id);
        
        if (e.id != -1)
        {
            treeview_vider(treeview_entraineurs);
            
            gchar *row_values[6];
            char id_str[20], exp_str[20], age_str[20], sexe_str[2];
            
            sprintf(id_str, "%d", e.id);
            sprintf(exp_str, "%d ans", e.experience);
            sprintf(age_str, "%d", e.age);
            sexe_str[0] = e.sexe;
            sexe_str[1] = '\0';
            
            row_values[0] = g_strdup(id_str);
            row_values[1] = g_strdup(e.nometprenom);
            row_values[2] = g_strdup(e.specialite);
            row_values[3] = g_strdup(exp_str);
            row_values[4] = g_strdup(age_str);
            row_values[5] = g_strdup(sexe_str);
            
            treeview_ajouter_ligne(treeview_entraineurs, row_values, 6);
            
            for (int i = 0; i < 6; i++) {
                g_free(row_values[i]);
            }
        }
        else
        {
            afficher_message(objet, "avertissement", "Entraineur non trouvé!");
        }
    }
    else
    {
        charger_entraineurs_dans_treeview(treeview_entraineurs);
    }
}

void on_btn_admin_supprimer_entraineur_clicked(GtkWidget *widget, gpointer user_data) {
    GtkWidget *entry_id = lookup_widget(widget, "entry13");
    
    if (!entry_id) {
        afficher_message(widget, "erreur", "Champ ID non trouvé!");
        return;
    }
    
    const char *id_text = gtk_entry_get_text(GTK_ENTRY(entry_id));
    
    if (strlen(id_text) == 0) {
        afficher_message(widget, "erreur", "Veuillez entrer un ID!");
        return;
    }
    
    int id = atoi(id_text);
    
    if (supprimer_entraineur("entraineurs.txt", id)) {
        afficher_message(widget, "info", "Entraineur supprimé!");
        gtk_entry_set_text(GTK_ENTRY(entry_id), "");
        
        GtkWidget *treeview_entraineurs = lookup_widget(widget, "treeview_entraineurs");
        if (treeview_entraineurs) {
            charger_entraineurs_dans_treeview(treeview_entraineurs);
        }
    } else {
        afficher_message(widget, "erreur", "Entraineur non trouvé!");
    }
}
void
on_btn_admin_modifier_entraineur_clicked (GtkWidget *objet,
                                          gpointer   user_data)
{
    GtkWidget *modifier_entraineur_window = create_ModifierEntraineur();
    if (modifier_entraineur_window != NULL)
    {
        gtk_widget_show_all(modifier_entraineur_window);
    }
}

void
on_btn_admin_ajouter_entraineur_clicked (GtkWidget *objet,
                                         gpointer   user_data)
{
    GtkWidget *ajouter_entraineur_window = create_AjouterEntraineur();
    if (ajouter_entraineur_window != NULL)
    {
        gtk_widget_show_all(ajouter_entraineur_window);
    }
}

void
on_btn_admin_entraineure_back_login_clicked (GtkWidget *objet,
                                             gpointer   user_data)
{
    retour_vers_login(objet);
}
void
on_btn_modifier_entraineur_chercher_clicked (GtkWidget *objet,
                                             gpointer   user_data)
{
    GtkWidget *entry_recherche = lookup_widget(objet, "entry_id_entraineur_modif");
    if (!entry_recherche) {
        entry_recherche = lookup_widget(objet, "entry_recherche_entraineur");
    }
    if (!entry_recherche) {
        entry_recherche = lookup_widget(objet, "entry_recherche");
    }
    
    if (!entry_recherche) {
        afficher_message(objet, "erreur", "Champ de recherche non trouvé!");
        return;
    }
    
    const char *id_text = gtk_entry_get_text(GTK_ENTRY(entry_recherche));
    
    if (strlen(id_text) == 0) {
        afficher_message(objet, "erreur", "Veuillez entrer un ID à rechercher!");
        return;
    }
    
    int id = atoi(id_text);
    Entraineur e = chercher_entraineur("entraineurs.txt", id);
    
    if (e.id != -1) {
        GtkWidget *entry_nom_prenom = lookup_widget(objet, "entry_nom_prenom_entraineur_modif");
        GtkWidget *combo_specialite = lookup_widget(objet, "combo_specialite_entraineur_modif");
        GtkWidget *combo_experience = lookup_widget(objet, "combo_experience_entraineur_modif");
        GtkWidget *spin_age = lookup_widget(objet, "spin_age_entraineur_modif");
        GtkWidget *radio_homme = lookup_widget(objet, "radio_homme_entraineur_modif");
        GtkWidget *radio_femme = lookup_widget(objet, "radio_femme_entraineur_modif");
        
        if (entry_nom_prenom) {
            gtk_entry_set_text(GTK_ENTRY(entry_nom_prenom), e.nometprenom);
        }
        
        if (combo_specialite && GTK_IS_COMBO_BOX(combo_specialite)) {
            gtk_combo_box_set_active(GTK_COMBO_BOX(combo_specialite), 0);
            GtkWidget *entry = gtk_bin_get_child(GTK_BIN(combo_specialite));
            if (entry) {
                gtk_entry_set_text(GTK_ENTRY(entry), e.specialite);
            }
        }
        
        if (combo_experience && GTK_IS_COMBO_BOX(combo_experience)) {
            gtk_combo_box_set_active(GTK_COMBO_BOX(combo_experience), 0);
            GtkWidget *entry = gtk_bin_get_child(GTK_BIN(combo_experience));
            if (entry) {
                char experience_str[20];
                sprintf(experience_str, "%d", e.experience);
                gtk_entry_set_text(GTK_ENTRY(entry), experience_str);
            }
        }
        
        if (spin_age) {
            gtk_spin_button_set_value(GTK_SPIN_BUTTON(spin_age), e.age);
        }
        
        if (radio_homme && radio_femme) {
            if (e.sexe == 'H' || e.sexe == 'M') {
                gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(radio_homme), TRUE);
                gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(radio_femme), FALSE);
            } else {
                gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(radio_femme), TRUE);
                gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(radio_homme), FALSE);
            }
        }
        
        afficher_message(objet, "info", "Entraineur trouvé! Modifiez les champs et cliquez sur Enregistrer.");
    } else {
        afficher_message(objet, "erreur", "Entraineur non trouvé!");
    }
}

void
on_btn_modifier_entraineur_enregistrer_clicked (GtkWidget *objet,
                                                gpointer   user_data)
{
    GtkWidget *entry_recherche = lookup_widget(objet, "entry_id_entraineur_modif");
    if (!entry_recherche) {
        afficher_message(objet, "erreur", "Champ de recherche non trouvé!");
        return;
    }
    
    const char *id_text = gtk_entry_get_text(GTK_ENTRY(entry_recherche));
    
    if (strlen(id_text) == 0) {
        afficher_message(objet, "erreur", "Veuillez d'abord rechercher un entraineur!");
        return;
    }
    
    int id = atoi(id_text);
    Entraineur e = chercher_entraineur("entraineurs.txt", id);
    
    if (e.id == -1) {
        afficher_message(objet, "erreur", "Entraineur non trouvé!");
        return;
    }
    
    int current_editing_entraineur_id = e.id;
    
    GtkWidget *entry_nom_prenom = lookup_widget(objet, "entry_nom_prenom_entraineur_modif");
    GtkWidget *combo_specialite = lookup_widget(objet, "combo_specialite_entraineur_modif");
    GtkWidget *combo_experience = lookup_widget(objet, "combo_experience_entraineur_modif");
    GtkWidget *spin_age = lookup_widget(objet, "spin_age_entraineur_modif");
    GtkWidget *radio_homme = lookup_widget(objet, "radio_homme_entraineur_modif");
    GtkWidget *radio_femme = lookup_widget(objet, "radio_femme_entraineur_modif");
    
    if (!entry_nom_prenom || !combo_specialite || !combo_experience || 
        !spin_age || !radio_homme || !radio_femme) {
        afficher_message(objet, "erreur", "Erreur: widgets non trouvés!");
        return;
    }
    
    const char *nom_prenom = gtk_entry_get_text(GTK_ENTRY(entry_nom_prenom));
    int age = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(spin_age));
    gboolean homme = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio_homme));
    
    gchar *specialite_text = NULL;
    gchar *experience_text = NULL;
    
    if (GTK_IS_COMBO_BOX(combo_specialite)) {
        GtkWidget *entry = gtk_bin_get_child(GTK_BIN(combo_specialite));
        if (entry) {
            specialite_text = g_strdup(gtk_entry_get_text(GTK_ENTRY(entry)));
        }
    }
    
    if (GTK_IS_COMBO_BOX(combo_experience)) {
        GtkWidget *entry = gtk_bin_get_child(GTK_BIN(combo_experience));
        if (entry) {
            experience_text = g_strdup(gtk_entry_get_text(GTK_ENTRY(entry)));
        }
    }
    
    if (strlen(nom_prenom) == 0 || !specialite_text || !experience_text) {
        afficher_message(objet, "erreur", "Veuillez remplir tous les champs!");
        if (specialite_text) g_free(specialite_text);
        if (experience_text) g_free(experience_text);
        return;
    }
    
    if (age < 18 || age > 70) {
        afficher_message(objet, "erreur", "Âge invalide! Doit être entre 18 et 70 ans.");
        if (specialite_text) g_free(specialite_text);
        if (experience_text) g_free(experience_text);
        return;
    }
    
    Entraineur nouv;
    nouv.id = current_editing_entraineur_id;
    strcpy(nouv.nometprenom, nom_prenom);
    strcpy(nouv.specialite, specialite_text);
    nouv.experience = atoi(experience_text);
    nouv.age = age;
    nouv.sexe = homme ? 'H' : 'F';
    
    if (modifier_entraineur("entraineurs.txt", current_editing_entraineur_id, nouv)) {
        afficher_message(objet, "info", "Entraineur modifié avec succès!");
        fermer_fenetre(objet);
    } else {
        afficher_message(objet, "erreur", "Erreur lors de la modification!");
    }
    
    if (specialite_text) g_free(specialite_text);
    if (experience_text) g_free(experience_text);
}
void
on_btn_modifier_entraineur_annuler_clicked (GtkWidget *objet,
                                            gpointer   user_data)
{
    fermer_fenetre(objet);
}
void
on_treeview_entraineurs_row_activated (GtkTreeView       *treeview,
                                       GtkTreePath       *path,
                                       GtkTreeViewColumn *column,
                                       gpointer           user_data)
{
    GtkTreeIter iter;
    GtkTreeModel *model = gtk_tree_view_get_model(treeview);
    gchar *id_str, *nom, *specialite, *experience, *age_str, *sexe;
    
    if (gtk_tree_model_get_iter(model, &iter, path)) {
        gtk_tree_model_get(model, &iter, 
                          0, &id_str,
                          1, &nom,
                          2, &specialite,
                          3, &experience,
                          4, &age_str,
                          5, &sexe,
                          -1);
        

        GtkWidget *dialog = gtk_dialog_new_with_buttons(
            "Action sur l'entraineur",
            GTK_WINDOW(gtk_widget_get_toplevel(GTK_WIDGET(treeview))),
            GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
            "Modifier", GTK_RESPONSE_YES,
            "Supprimer", GTK_RESPONSE_NO,
            NULL);
        
        char message[100];
        sprintf(message, "Entraineur: %s\nID: %s", nom, id_str);
        
        GtkWidget *label = gtk_label_new(message);
        GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
        gtk_container_add(GTK_CONTAINER(content_area), label);
        gtk_widget_show(label);
        
        gint response = gtk_dialog_run(GTK_DIALOG(dialog));
        
        if (response == GTK_RESPONSE_YES) {

            GtkWidget *modifier_window = create_ModifierEntraineur();
            if (modifier_window != NULL) {
                GtkWidget *entry_id = lookup_widget(modifier_window, "entry_id_entraineur_modif");
                if (entry_id) {
                    gtk_entry_set_text(GTK_ENTRY(entry_id), id_str);
                }
                gtk_widget_show_all(modifier_window);
            }
            
        } else if (response == GTK_RESPONSE_NO) {

            char confirm_msg[150];
            sprintf(confirm_msg, "Supprimer l'entraineur %s (ID: %s) ?", nom, id_str);
            
            if (demander_confirmation(GTK_WIDGET(treeview), "Confirmer suppression", confirm_msg)) {
                int id = atoi(id_str);
                if (supprimer_entraineur("entraineurs.txt", id)) {

                    GtkTreeSelection *selection = gtk_tree_view_get_selection(treeview);
                    if (gtk_tree_selection_get_selected(selection, &model, &iter)) {
                        gtk_list_store_remove(GTK_LIST_STORE(model), &iter);
                    }
                    afficher_message(GTK_WIDGET(treeview), "info", "Entraineur supprimé!");
                } else {
                    afficher_message(GTK_WIDGET(treeview), "erreur", "Erreur suppression!");
                }
            }
        }
        
        gtk_widget_destroy(dialog);
        

        g_free(id_str);
        g_free(nom);
        g_free(specialite);
        g_free(experience);
        g_free(age_str);
        g_free(sexe);
    }
}
void
on_btn_ajouter_entraineur_enregistrer_clicked (GtkWidget *objet,
                                               gpointer   user_data)
{
    GtkWidget *entry_nom_prenom;
    GtkWidget *combo_specialites;
    GtkWidget *combo_experience;
    GtkWidget *spin_age;
    GtkWidget *radio_homme;
    GtkWidget *radio_femme;
    const char *nom_prenom;
    const char *specialites;
    const char *experience;
    int age;
    char sexe;
    Entraineur e;
    
    entry_nom_prenom = lookup_widget(objet, "entry_ajouter_entraineur_nom_prenom");
    combo_specialites = lookup_widget(objet, "combobox_ajouter_entraineur_specialites");
    combo_experience = lookup_widget(objet, "combobox_ajouter_entraineur_experience");
    spin_age = lookup_widget(objet, "spin_ajouter_entraineur_age");
    radio_homme = lookup_widget(objet, "radio_ajouter_entraineur_homme");
    radio_femme = lookup_widget(objet, "radio_ajouter_entraineur_femme");
    
    if (entry_nom_prenom == NULL || combo_specialites == NULL || combo_experience == NULL || 
        spin_age == NULL || radio_homme == NULL || radio_femme == NULL)
    {
        afficher_message(objet, "erreur", "Erreur: Widgets non trouvés!");
        return;
    }
    
    nom_prenom = gtk_entry_get_text(GTK_ENTRY(entry_nom_prenom));
    specialites = gtk_combo_box_get_active_text(GTK_COMBO_BOX(combo_specialites));
    experience = gtk_combo_box_get_active_text(GTK_COMBO_BOX(combo_experience));
    age = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(spin_age));
    
    if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio_femme)))
        sexe = 'F';
    else
        sexe = 'H';
    
    if (strlen(nom_prenom) == 0 || specialites == NULL || strlen(specialites) == 0 || 
        experience == NULL || strlen(experience) == 0)
    {
        afficher_message(objet, "erreur", "Veuillez remplir tous les champs!");
        return;
    }
    
    if (age < 18 || age > 70)
    {
        afficher_message(objet, "erreur", "L'âge doit être entre 18 et 70 ans!");
        return;
    }
    
    FILE *f = fopen("entraineurs.txt", "r");
    int max_id = 0;
    Entraineur temp;
    if (f != NULL) {
        while (fscanf(f, "%d %49s %49s %d %d %c", 
                     &temp.id, temp.nometprenom, temp.specialite, 
                     &temp.experience, &temp.age, &temp.sexe) == 6) {
            if (temp.id > max_id) {
                max_id = temp.id;
            }
        }
        fclose(f);
    }
    
    e.id = max_id + 1;
    strcpy(e.nometprenom, nom_prenom);
    strcpy(e.specialite, specialites);
    e.experience = atoi(experience);
    e.age = age;
    e.sexe = sexe;
    
    if (ajouter_entraineur("entraineurs.txt", e))
    {
        afficher_message(objet, "info", "Entraîneur ajouté avec succès!");
        fermer_fenetre(objet);
    }
    else
    {
        afficher_message(objet, "erreur", "Erreur lors de l'ajout de l'entraîneur!");
    }
}

void
on_btn_ajouter_entraineur_annuler_clicked (GtkWidget *objet,
                                           gpointer   user_data)
{
    fermer_fenetre(objet);
}
//équipement
void charger_equipements_dans_treeview(GtkWidget *treeview)
{
    FILE *f;
    Equipement e;
    gchar *row_values[6];
    char cap_str[20], disp_str[20], etat_str[20];
    
    treeview_vider(treeview);
    
    f = fopen("equipements.txt", "r");
    if (f != NULL) {
        while (fscanf(f, "%19s %49s %49s %d %c %c", 
                     e.reference, e.type, e.centre, 
                     &e.capacite, &e.disponibilite, &e.etat) == 6) {
            
            sprintf(cap_str, "%d", e.capacite);
            sprintf(disp_str, "%c", e.disponibilite);
            sprintf(etat_str, "%c", e.etat);
            
            row_values[0] = g_strdup(e.reference);
            row_values[1] = g_strdup(e.type);
            row_values[2] = g_strdup(e.centre);
            row_values[3] = g_strdup(cap_str);
            row_values[4] = g_strdup(disp_str);
            row_values[5] = g_strdup(etat_str);
            
            treeview_ajouter_ligne(treeview, row_values, 6);
            
            for (int i = 0; i < 6; i++) {
                g_free(row_values[i]);
            }
        }
        fclose(f);
    } else {
        f = fopen("equipements.txt", "w");
        if (f) fclose(f);
    }
}
void on_btn_admin_rechercher_equipement_clicked (GtkWidget *objet,
                                            gpointer   user_data)
{
    GtkWidget *entry_reference;
    GtkWidget *treeview_equipements;
    const char *reference;
    Equipement e;
    
    entry_reference = lookup_widget(objet, "entry10");
    treeview_equipements = lookup_widget(objet, "treeview_equipements");
    
    if (!treeview_equipements) {
        afficher_message(objet, "erreur", "TreeView non trouvé!");
        return;
    }
    
    reference = gtk_entry_get_text(GTK_ENTRY(entry_reference));
    
    if (strlen(reference) > 0)
    {
        e = chercher_equipement("equipements.txt", reference);
        
        if (strcmp(e.reference, "") != 0)
        {
            treeview_vider(treeview_equipements);
            
            gchar *row_values[6];
            char cap_str[20], disp_str[2], etat_str[2];
            
            sprintf(cap_str, "%d", e.capacite);
            disp_str[0] = e.disponibilite;
            disp_str[1] = '\0';
            etat_str[0] = e.etat;
            etat_str[1] = '\0';
            
            row_values[0] = g_strdup(e.reference);
            row_values[1] = g_strdup(e.type);
            row_values[2] = g_strdup(e.centre);
            row_values[3] = g_strdup(cap_str);
            row_values[4] = g_strdup(disp_str);
            row_values[5] = g_strdup(etat_str);
            
            treeview_ajouter_ligne(treeview_equipements, row_values, 6);
            
            for (int i = 0; i < 6; i++) {
                g_free(row_values[i]);
            }
        }
        else
        {
            afficher_message(objet, "avertissement", "Équipement non trouvé!");
        }
    }
    else
    {
        charger_equipements_dans_treeview(treeview_equipements);
    }
}
void on_btn_admin_supprimer_equipement_clicked(GtkWidget *widget, gpointer user_data) {
    GtkWidget *entry_ref = lookup_widget(widget, "entry10");
    
    if (!entry_ref) {
        afficher_message(widget, "erreur", "Champ référence non trouvé!");
        return;
    }
    
    const char *reference = gtk_entry_get_text(GTK_ENTRY(entry_ref));
    
    if (strlen(reference) == 0) {
        afficher_message(widget, "erreur", "Veuillez entrer une référence!");
        return;
    }
    
    if (supprimer_equipement("equipements.txt", reference)) {
        afficher_message(widget, "info", "Équipement supprimé!");
        gtk_entry_set_text(GTK_ENTRY(entry_ref), "");
        
        GtkWidget *treeview_equipements = lookup_widget(widget, "treeview_equipements");
        if (treeview_equipements) {
            charger_equipements_dans_treeview(treeview_equipements);
        }
    } else {
        afficher_message(widget, "erreur", "Équipement non trouvé!");
    }
}
void
on_btn_admin_modifier_equipement_clicked (GtkWidget *objet,
                                          gpointer   user_data)
{
    GtkWidget *modifier_equipement_window = create_Modifierequipement();
    if (modifier_equipement_window != NULL)
    {
        gtk_widget_show_all(modifier_equipement_window);
    }
}

void
on_btn_admin_ajouter_equipement_clicked (GtkWidget *objet,
                                         gpointer   user_data)
{
    GtkWidget *ajouter_equipement_window = create_Ajouterequipement();
    if (ajouter_equipement_window != NULL)
    {
        gtk_widget_show_all(ajouter_equipement_window);
    }
}


void on_btn_modifier_equipement_chercher_clicked(GtkWidget *widget, gpointer user_data) {
    GtkWidget *window = gtk_widget_get_toplevel(widget);
    
    GtkWidget *entry_ref = lookup_widget(window, "entry_ref_equip_modif");
    
    if (!entry_ref || !GTK_IS_ENTRY(entry_ref)) {
        afficher_message(widget, "erreur", "Erreur interne: widget non trouvé!");
        return;
    }
    
    const char *reference = gtk_entry_get_text(GTK_ENTRY(entry_ref));
    
    if(strlen(reference) == 0) {
        afficher_message(widget, "erreur", "Veuillez entrer une référence!");
        return;
    }
    
    Equipement e = chercher_equipement("equipements.txt", reference);
    
    if(strcmp(e.reference, "") != 0) {
        GtkWidget *combo_type = lookup_widget(window, "combo_type_equip_modif");
        GtkWidget *combo_centre = lookup_widget(window, "combo_centre_equip_modif__");
        GtkWidget *spin_capacite = lookup_widget(window, "spin_capacite_equip_modif");
        GtkWidget *check_dispo = lookup_widget(window, "check_dispo_equip_modif");
        GtkWidget *radio_actif = lookup_widget(window, "radio_actif_equip_modif");
        GtkWidget *radio_inactif = lookup_widget(window, "radio_non_actif_equip_modif");
        
        if (combo_type && GTK_IS_COMBO_BOX(combo_type)) {
            GtkWidget *entry = gtk_bin_get_child(GTK_BIN(combo_type));
            if (entry) gtk_entry_set_text(GTK_ENTRY(entry), e.type);
        }
        
        if (combo_centre && GTK_IS_COMBO_BOX(combo_centre)) {
            GtkWidget *entry = gtk_bin_get_child(GTK_BIN(combo_centre));
            if (entry) gtk_entry_set_text(GTK_ENTRY(entry), e.centre);
        } else if (combo_centre && GTK_IS_COMBO(combo_centre)) {
            GtkWidget *entry = GTK_COMBO(combo_centre)->entry;
            if (entry) gtk_entry_set_text(GTK_ENTRY(entry), e.centre);
        }
        
        if (spin_capacite) {
            gtk_spin_button_set_value(GTK_SPIN_BUTTON(spin_capacite), e.capacite);
        }
        
        if (check_dispo) {
            gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_dispo), e.disponibilite == 'D');
        }
        
        if (radio_actif && radio_inactif) {
            if (e.etat == 'A') {
                gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(radio_actif), TRUE);
                gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(radio_inactif), FALSE);
            } else {
                gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(radio_actif), FALSE);
                gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(radio_inactif), TRUE);
            }
        }
        
        afficher_message(widget, "info", "Équipement trouvé avec succès!");
    } else {
        afficher_message(widget, "erreur", "Équipement non trouvé!");
    }
}

void on_btn_modifier_equipement_enregistrer_clicked(GtkWidget *widget, gpointer user_data) {
    GtkWidget *window = gtk_widget_get_toplevel(widget);
    
    GtkWidget *entry_ref = lookup_widget(window, "entry_ref_equip_modif");
    GtkWidget *combo_type = lookup_widget(window, "combo_type_equip_modif");
    GtkWidget *combo_centre = lookup_widget(window, "combo_centre_equip_modif__");
    GtkWidget *spin_capacite = lookup_widget(window, "spin_capacite_equip_modif");
    GtkWidget *check_dispo = lookup_widget(window, "check_dispo_equip_modif");
    GtkWidget *radio_actif = lookup_widget(window, "radio_actif_equip_modif");
    
    if (!entry_ref || !combo_type || !combo_centre || !spin_capacite || 
        !check_dispo || !radio_actif) {
        afficher_message(widget, "erreur", "Erreur: Widgets non trouvés!");
        return;
    }
    
    const char *reference = gtk_entry_get_text(GTK_ENTRY(entry_ref));
    
    char type_text[50] = "";
    char centre_text[50] = "";
    
    if (GTK_IS_COMBO_BOX(combo_type)) {
        GtkWidget *entry = gtk_bin_get_child(GTK_BIN(combo_type));
        if (entry && GTK_IS_ENTRY(entry)) {
            const char *text = gtk_entry_get_text(GTK_ENTRY(entry));
            if (text) strncpy(type_text, text, sizeof(type_text)-1);
        }
    }
    
    if (GTK_IS_COMBO_BOX(combo_centre)) {
        GtkWidget *entry = gtk_bin_get_child(GTK_BIN(combo_centre));
        if (entry && GTK_IS_ENTRY(entry)) {
            const char *text = gtk_entry_get_text(GTK_ENTRY(entry));
            if (text) strncpy(centre_text, text, sizeof(centre_text)-1);
        }
    } else if (GTK_IS_COMBO(combo_centre)) {
        GtkWidget *entry = GTK_COMBO(combo_centre)->entry;
        if (entry && GTK_IS_ENTRY(entry)) {
            const char *text = gtk_entry_get_text(GTK_ENTRY(entry));
            if (text) strncpy(centre_text, text, sizeof(centre_text)-1);
        }
    }
    
    int capacite = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(spin_capacite));
    gboolean dispo = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_dispo));
    gboolean actif = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio_actif));
    
    if (strlen(reference) == 0 || strlen(type_text) == 0 || strlen(centre_text) == 0) {
        afficher_message(widget, "erreur", "Veuillez remplir tous les champs!");
        return;
    }
    
    if (capacite <= 0) {
        afficher_message(widget, "erreur", "Capacité invalide!");
        return;
    }
    
    Equipement nouv;
    strncpy(nouv.reference, reference, sizeof(nouv.reference)-1);
    strncpy(nouv.type, type_text, sizeof(nouv.type)-1);
    strncpy(nouv.centre, centre_text, sizeof(nouv.centre)-1);
    nouv.capacite = capacite;
    nouv.disponibilite = dispo ? 'D' : 'R';
    nouv.etat = actif ? 'A' : 'I';
    
    char ref_copy[50];
    strncpy(ref_copy, reference, sizeof(ref_copy)-1);
    
    if(modifier_equipement("equipements.txt", ref_copy, nouv)) {
        afficher_message(widget, "info", "Équipement modifié avec succès!");
        fermer_fenetre(window);
    } else {
        afficher_message(widget, "erreur", "Erreur modification!");
    }
}
void
on_btn_admin_back_login_clicked (GtkWidget *objet,
                                 gpointer   user_data)
{
    retour_vers_login(objet);
}


void
on_btn_modifier_equipement_annuler_clicked (GtkWidget *objet,
                                            gpointer   user_data)
{
    fermer_fenetre(objet);
}
void
on_treeview_equipements_row_activated (GtkTreeView       *treeview,
                                       GtkTreePath       *path,
                                       GtkTreeViewColumn *column,
                                       gpointer           user_data)
{
    GtkTreeIter iter;
    GtkTreeModel *model = gtk_tree_view_get_model(treeview);
    gchar *ref, *type, *centre, *capacite, *dispo, *etat;
    
    if (gtk_tree_model_get_iter(model, &iter, path)) {
        gtk_tree_model_get(model, &iter, 
                          0, &ref,
                          1, &type,
                          2, &centre,
                          3, &capacite,
                          4, &dispo,
                          5, &etat,
                          -1);
        

        GtkWidget *dialog = gtk_dialog_new_with_buttons(
            "Action sur l'équipement",
            GTK_WINDOW(gtk_widget_get_toplevel(GTK_WIDGET(treeview))),
            GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
            "Modifier", GTK_RESPONSE_YES,
            "Supprimer", GTK_RESPONSE_NO,
            NULL);
        

        char message[100];
        sprintf(message, "Équipement: %s\nType: %s", ref, type);
        
        GtkWidget *label = gtk_label_new(message);
        GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
        gtk_container_add(GTK_CONTAINER(content_area), label);
        gtk_widget_show(label);
        

        gint response = gtk_dialog_run(GTK_DIALOG(dialog));
        
        if (response == GTK_RESPONSE_YES) {

            GtkWidget *modifier_window = create_Modifierequipement();
            if (modifier_window != NULL) {

                GtkWidget *entry_ref = lookup_widget(modifier_window, "entry_ref_equip_modif");
                if (entry_ref) {
                    gtk_entry_set_text(GTK_ENTRY(entry_ref), ref);
                }
                gtk_widget_show_all(modifier_window);
            }
            
        } else if (response == GTK_RESPONSE_NO) {

            char confirm_msg[150];
            sprintf(confirm_msg, "Supprimer l'équipement %s ?", ref);
            
            if (demander_confirmation(GTK_WIDGET(treeview), "Confirmer suppression", confirm_msg)) {
                if (supprimer_equipement("equipements.txt", ref)) {

                    GtkTreeSelection *selection = gtk_tree_view_get_selection(treeview);
                    if (gtk_tree_selection_get_selected(selection, &model, &iter)) {
                        gtk_list_store_remove(GTK_LIST_STORE(model), &iter);
                    }
                    afficher_message(GTK_WIDGET(treeview), "info", "Équipement supprimé!");
                } else {
                    afficher_message(GTK_WIDGET(treeview), "erreur", "Erreur suppression!");
                }
            }
        }
        

        gtk_widget_destroy(dialog);
        

        g_free(ref);
        g_free(type);
        g_free(centre);
        g_free(capacite);
        g_free(dispo);
        g_free(etat);
    }
}
void
on_btn_ajouter_equipement_enregistrer_clicked (GtkWidget *objet,
                                               gpointer   user_data)
{
    GtkWidget *window = gtk_widget_get_toplevel(objet);
    Equipement e;
    
    GtkWidget *entry_reference = lookup_widget(window, "entry_ajouter_ref");
    GtkWidget *combo_type = lookup_widget(window, "combo_ajouter_type");
    GtkWidget *combo_centre = lookup_widget(window, "combo_ajouter_centre");
    GtkWidget *spin_capacite = lookup_widget(window, "spin_ajouter_capacite");
    GtkWidget *check_disponibilite = lookup_widget(window, "check_ajouter_dispo");
    GtkWidget *radio_etat_actif = lookup_widget(window, "radio_ajouter_etat_actif");
    GtkWidget *radio_etat_inactif = lookup_widget(window, "radio_ajouter_etat_inactif");
    
    if (entry_reference == NULL || combo_type == NULL || combo_centre == NULL || 
        spin_capacite == NULL || check_disponibilite == NULL || radio_etat_actif == NULL || 
        radio_etat_inactif == NULL)
    {
        afficher_message(objet, "erreur", "Erreur interne: Widgets non trouvés");
        return;
    }
    
    const char *reference = gtk_entry_get_text(GTK_ENTRY(entry_reference));
    const char *type = NULL;
    const char *centre = NULL;
    int capacite = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(spin_capacite));
    gboolean dispo_checked = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_disponibilite));
    gboolean etat_actif = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio_etat_actif));
    char disponibilite = dispo_checked ? 'D' : 'R';
    char etat = etat_actif ? 'A' : 'I';
    
    if (GTK_IS_COMBO_BOX(combo_type)) {
        type = gtk_combo_box_get_active_text(GTK_COMBO_BOX(combo_type));
    } else if (GTK_IS_COMBO(combo_type)) {
        GtkWidget *entry = GTK_COMBO(combo_type)->entry;
        if (entry && GTK_IS_ENTRY(entry)) {
            type = gtk_entry_get_text(GTK_ENTRY(entry));
        }
    }
    
    if (GTK_IS_COMBO_BOX(combo_centre)) {
        centre = gtk_combo_box_get_active_text(GTK_COMBO_BOX(combo_centre));
    } else if (GTK_IS_COMBO(combo_centre)) {
        GtkWidget *entry = GTK_COMBO(combo_centre)->entry;
        if (entry && GTK_IS_ENTRY(entry)) {
            centre = gtk_entry_get_text(GTK_ENTRY(entry));
        }
    }
    
    if (strlen(reference) == 0) {
        afficher_message(objet, "erreur", "La référence est obligatoire");
        return;
    }
    
    if (type == NULL || strlen(type) == 0) {
        afficher_message(objet, "erreur", "Le type est obligatoire");
        return;
    }
    
    if (centre == NULL || strlen(centre) == 0) {
        afficher_message(objet, "erreur", "Le centre est obligatoire");
        return;
    }
    
    if (capacite <= 0) {
        afficher_message(objet, "erreur", "La capacité doit être positive");
        return;
    }
    
    strcpy(e.reference, reference);
    strcpy(e.type, type);
    strcpy(e.centre, centre);
    e.capacite = capacite;
    e.disponibilite = disponibilite;
    e.etat = etat;
    
    if (ajouter_equipement("equipements.txt", e))
    {
        afficher_message(objet, "info", "Équipement ajouté avec succès");
        fermer_fenetre(objet);
    }
    else
    {
        afficher_message(objet, "erreur", "Erreur lors de l'ajout de l'équipement");
    }
}

void
on_btn_ajouter_equipement_annuler_clicked (GtkWidget *objet,
                                           gpointer   user_data)
{
    fermer_fenetre(objet);
}
//centre
void charger_centres_dans_treeview(GtkWidget *treeview)
{
    FILE *f;
    Centre c;
    gchar *row_values[5];
    char id_str[20], cap_str[20];
    
    treeview_vider(treeview);
    
    f = fopen("centres.txt", "r");
    if (f != NULL) {
        char buffer[256];
        while (fgets(buffer, sizeof(buffer), f)) {
            buffer[strcspn(buffer, "\n")] = 0;
            
            char *p = buffer;
            int fields = 0;
            char *field_ptrs[10];
            
            field_ptrs[fields++] = p;
            while (*p && fields < 10) {
                if (*p == ';') {
                    *p = '\0';
                    p++;
                    if (*p && *p != ';') {
                        field_ptrs[fields++] = p;
                    }
                } else {
                    p++;
                }
            }
            
            if (fields >= 6) {
                c.id = atoi(field_ptrs[0]);
                strcpy(c.nom, field_ptrs[1]);
                strcpy(c.adresse, field_ptrs[2]);
                strcpy(c.ville, field_ptrs[3]);
                c.capacite = atoi(field_ptrs[4]);
                c.etat = field_ptrs[5][0];
                
                sprintf(id_str, "%d", c.id);
                sprintf(cap_str, "%d", c.capacite);
                
                row_values[0] = g_strdup(id_str);
                row_values[1] = g_strdup(c.nom);
                row_values[2] = g_strdup(c.adresse);
                row_values[3] = g_strdup(c.ville);
                row_values[4] = g_strdup(cap_str);
                
                treeview_ajouter_ligne(treeview, row_values, 5);
                
                for (int i = 0; i < 5; i++) {
                    g_free(row_values[i]);
                }
            }
        }
        fclose(f);
    } else {
        f = fopen("centres.txt", "w");
        if (f) fclose(f);
    }
}
void on_btn_admin_rechercher_centre_clicked (GtkWidget *objet,
                                        gpointer   user_data)
{
    GtkWidget *entry_id;
    GtkWidget *treeview_centres;
    const char *id_text;
    int id;
    Centre c;
    
    entry_id = lookup_widget(objet, "entry9");
    treeview_centres = lookup_widget(objet, "treeview_centres");
    
    if (!treeview_centres) {
        afficher_message(objet, "erreur", "TreeView non trouvé!");
        return;
    }
    
    id_text = gtk_entry_get_text(GTK_ENTRY(entry_id));
    
    if (strlen(id_text) > 0)
    {
        id = atoi(id_text);
        c = chercher_centre("centres.txt", id);
        
        if (c.id != -1)
        {
            treeview_vider(treeview_centres);
            
            gchar *row_values[5];
            char id_str[20], cap_str[20];
            
            sprintf(id_str, "%d", c.id);
            sprintf(cap_str, "%d", c.capacite);
            
            row_values[0] = g_strdup(id_str);
            row_values[1] = g_strdup(c.nom);
            row_values[2] = g_strdup(c.adresse);
            row_values[3] = g_strdup(c.ville);
            row_values[4] = g_strdup(cap_str);
            
            treeview_ajouter_ligne(treeview_centres, row_values, 5);
            
            for (int i = 0; i < 5; i++) {
                g_free(row_values[i]);
            }
        }
        else
        {
            afficher_message(objet, "avertissement", "Centre non trouvé!");
        }
    }
    else
    {
        charger_centres_dans_treeview(treeview_centres);
    }
}

void on_btn_admin_supprimer_centre_clicked(GtkWidget *widget, gpointer user_data) {
    GtkWidget *entry_id = lookup_widget(widget, "entry9");
    
    if (!entry_id) {
        afficher_message(widget, "erreur", "Champ ID non trouvé!");
        return;
    }
    
    const char *id_text = gtk_entry_get_text(GTK_ENTRY(entry_id));
    
    if (strlen(id_text) == 0) {
        afficher_message(widget, "erreur", "Veuillez entrer un ID!");
        return;
    }
    
    int id = atoi(id_text);
    
    if (supprimer_centre("centres.txt", id)) {
        afficher_message(widget, "info", "Centre supprimé!");
        gtk_entry_set_text(GTK_ENTRY(entry_id), "");
        
        GtkWidget *treeview_centres = lookup_widget(widget, "treeview_centres");
        if (treeview_centres) {
            charger_centres_dans_treeview(treeview_centres);
        }
    } else {
        afficher_message(widget, "erreur", "Centre non trouvé!");
    }
}
void
on_btn_admin_modifier_centre_clicked (GtkWidget *objet,
                                      gpointer   user_data)
{
    GtkWidget *modifier_centre_window = create_ModifierCentre();
    if (modifier_centre_window != NULL)
    {
        gtk_widget_show_all(modifier_centre_window);
    }
}

void
on_btn_admin_ajouter_centre_clicked (GtkWidget *objet,
                                     gpointer   user_data)
{
    GtkWidget *ajouter_centre_window = create_AjouterCentre();
    if (ajouter_centre_window != NULL)
    {
        gtk_widget_show_all(ajouter_centre_window);
    }
}
void
on_btn_modifier_centre_chercher_clicked (GtkWidget *objet,
                                         gpointer   user_data)
{
    GtkWidget *entry_recherche = lookup_widget(objet, "entry_id_centre_modif");
    if (!entry_recherche) {
        entry_recherche = lookup_widget(objet, "entry35");
    }
    if (!entry_recherche) {
        entry_recherche = lookup_widget(objet, "entry1");
    }
    
    if (!entry_recherche) {
        afficher_message(objet, "erreur", "Champ de recherche non trouvé!");
        return;
    }
    
    const char *id_text = gtk_entry_get_text(GTK_ENTRY(entry_recherche));
    
    if (strlen(id_text) == 0) {
        afficher_message(objet, "erreur", "Veuillez entrer un ID à rechercher!");
        return;
    }
    
    int id = atoi(id_text);
    Centre c = chercher_centre("centres.txt", id);
    
    if (c.id != -1) {
        GtkWidget *entry_nom = lookup_widget(objet, "entry_nom_centre_modif");
        GtkWidget *combo_ville = lookup_widget(objet, "combo_ville_centre_modif");
        GtkWidget *spin_capacite = lookup_widget(objet, "spin_capacite_centre_modif");
        GtkWidget *radio_actif = lookup_widget(objet, "radio_actif_centre_modif");
        GtkWidget *radio_inactif = lookup_widget(objet, "radio_inactif_centre_modif");
        
        if (entry_nom) {
            gtk_entry_set_text(GTK_ENTRY(entry_nom), c.nom);
        }
        
        if (combo_ville && GTK_IS_COMBO_BOX(combo_ville)) {
            gtk_combo_box_set_active(GTK_COMBO_BOX(combo_ville), 0);
            GtkWidget *entry = gtk_bin_get_child(GTK_BIN(combo_ville));
            if (entry) {
                gtk_entry_set_text(GTK_ENTRY(entry), c.ville);
            }
        }
        
        if (spin_capacite) {
            gtk_spin_button_set_value(GTK_SPIN_BUTTON(spin_capacite), c.capacite);
        }
        
        if (radio_actif && radio_inactif) {
            if (c.etat == 'A') {
                gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(radio_actif), TRUE);
                gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(radio_inactif), FALSE);
            } else {
                gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(radio_inactif), TRUE);
                gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(radio_actif), FALSE);
            }
        }
        
        afficher_message(objet, "info", "Centre trouvé! Modifiez les champs et cliquez sur Enregistrer.");
    } else {
        afficher_message(objet, "erreur", "Centre non trouvé!");
    }
}

void
on_btn_modifier_centre_enregistrer_clicked (GtkWidget *objet,
                                            gpointer   user_data)
{
    GtkWidget *entry_recherche = lookup_widget(objet, "entry_id_centre_modif");
    if (!entry_recherche) {
        afficher_message(objet, "erreur", "Champ de recherche non trouvé!");
        return;
    }
    
    const char *id_text = gtk_entry_get_text(GTK_ENTRY(entry_recherche));
    
    if (strlen(id_text) == 0) {
        afficher_message(objet, "erreur", "Veuillez d'abord rechercher un centre!");
        return;
    }
    
    int id = atoi(id_text);
    Centre c = chercher_centre("centres.txt", id);
    
    if (c.id == -1) {
        afficher_message(objet, "erreur", "Centre non trouvé!");
        return;
    }
    
    int current_editing_centre_id = c.id;
    
    GtkWidget *entry_nom = lookup_widget(objet, "entry_nom_centre_modif");
    GtkWidget *combo_ville = lookup_widget(objet, "combo_ville_centre_modif");
    GtkWidget *spin_capacite = lookup_widget(objet, "spin_capacite_centre_modif");
    GtkWidget *radio_actif = lookup_widget(objet, "radio_actif_centre_modif");
    GtkWidget *radio_inactif = lookup_widget(objet, "radio_inactif_centre_modif");
    
    if (!entry_nom || !combo_ville || !spin_capacite || !radio_actif || !radio_inactif) {
        afficher_message(objet, "erreur", "Erreur: widgets non trouvés!");
        return;
    }
    
    const char *nom = gtk_entry_get_text(GTK_ENTRY(entry_nom));
    int capacite = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(spin_capacite));
    gboolean actif = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio_actif));
    
    gchar *ville_text = NULL;
    if (GTK_IS_COMBO_BOX(combo_ville)) {
        GtkWidget *entry = gtk_bin_get_child(GTK_BIN(combo_ville));
        if (entry) {
            ville_text = g_strdup(gtk_entry_get_text(GTK_ENTRY(entry)));
        }
    }
    
    if (strlen(nom) == 0 || !ville_text) {
        afficher_message(objet, "erreur", "Veuillez remplir tous les champs!");
        if (ville_text) g_free(ville_text);
        return;
    }
    
    if (capacite <= 0) {
        afficher_message(objet, "erreur", "Capacité invalide! Doit être un nombre positif.");
        if (ville_text) g_free(ville_text);
        return;
    }
    
    Centre nouv;
    nouv.id = current_editing_centre_id;
    strcpy(nouv.nom, nom);
    strcpy(nouv.ville, ville_text);
    strcpy(nouv.adresse, c.adresse);
    nouv.capacite = capacite;
    nouv.etat = actif ? 'A' : 'I';
    
    if (modifier_centre("centres.txt", current_editing_centre_id, nouv)) {
        afficher_message(objet, "info", "Centre modifié avec succès!");
        fermer_fenetre(objet);
    } else {
        afficher_message(objet, "erreur", "Erreur lors de la modification!");
    }
    
    if (ville_text) g_free(ville_text);
}


void
on_btn_modifier_centre_annuler_clicked (GtkWidget *objet,
                                        gpointer   user_data)
{
    fermer_fenetre(objet);
}
void
on_btn_admin_centres_back_login_clicked (GtkWidget *objet,
                                         gpointer   user_data)
{
    retour_vers_login(objet);
}
void
on_treeview_centres_row_activated (GtkTreeView       *treeview,
                                   GtkTreePath       *path,
                                   GtkTreeViewColumn *column,
                                   gpointer           user_data)
{
    GtkTreeIter iter;
    GtkTreeModel *model = gtk_tree_view_get_model(treeview);
    gchar *id_str, *nom, *adresse, *ville, *capacite;
    
    if (gtk_tree_model_get_iter(model, &iter, path)) {
        gtk_tree_model_get(model, &iter, 
                          0, &id_str,
                          1, &nom,
                          2, &adresse,
                          3, &ville,
                          4, &capacite,
                          -1);
        

        GtkWidget *dialog = gtk_dialog_new_with_buttons(
            "Action sur le centre",
            GTK_WINDOW(gtk_widget_get_toplevel(GTK_WIDGET(treeview))),
            GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
            "Modifier", GTK_RESPONSE_YES,
            "Supprimer", GTK_RESPONSE_NO,
            NULL);
        
        char message[100];
        sprintf(message, "Centre: %s\nID: %s", nom, id_str);
        
        GtkWidget *label = gtk_label_new(message);
        GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
        gtk_container_add(GTK_CONTAINER(content_area), label);
        gtk_widget_show(label);
        
        gint response = gtk_dialog_run(GTK_DIALOG(dialog));
        
        if (response == GTK_RESPONSE_YES) {

            GtkWidget *modifier_window = create_ModifierCentre();
            if (modifier_window != NULL) {
                GtkWidget *entry_id = lookup_widget(modifier_window, "entry_id_centre_modif");
                if (entry_id) {
                    gtk_entry_set_text(GTK_ENTRY(entry_id), id_str);
                }
                gtk_widget_show_all(modifier_window);
            }
            
        } else if (response == GTK_RESPONSE_NO) {

            char confirm_msg[150];
            sprintf(confirm_msg, "Supprimer le centre %s (ID: %s) ?", nom, id_str);
            
            if (demander_confirmation(GTK_WIDGET(treeview), "Confirmer suppression", confirm_msg)) {
                int id = atoi(id_str);
                if (supprimer_centre("centres.txt", id)) {

                    GtkTreeSelection *selection = gtk_tree_view_get_selection(treeview);
                    if (gtk_tree_selection_get_selected(selection, &model, &iter)) {
                        gtk_list_store_remove(GTK_LIST_STORE(model), &iter);
                    }
                    afficher_message(GTK_WIDGET(treeview), "info", "Centre supprimé!");
                } else {
                    afficher_message(GTK_WIDGET(treeview), "erreur", "Erreur suppression!");
                }
            }
        }
        
        gtk_widget_destroy(dialog);
        

        g_free(id_str);
        g_free(nom);
        g_free(adresse);
        g_free(ville);
        g_free(capacite);
    }
}
void
on_btn_ajouter_centre_enregistrer_clicked (GtkWidget *objet,
                                           gpointer   user_data)
{
    GtkWidget *entry_nom;
    GtkWidget *entry_id;
    GtkWidget *combo_gouvernerat;
    GtkWidget *spin_capacite;
    GtkWidget *radio_ouvert;
    GtkWidget *radio_ferme;
    const char *nom;
    const char *id_text;
    const char *gouvernerat;
    int capacite;
    char etat;
    int id;
    Centre c;
    
    entry_nom = lookup_widget(objet, "entry_nom");
    entry_id = lookup_widget(objet, "entry_id");
    combo_gouvernerat = lookup_widget(objet, "combo_gouvernerat");
    spin_capacite = lookup_widget(objet, "spin_capacite");
    radio_ouvert = lookup_widget(objet, "radio_ouvert");
    radio_ferme = lookup_widget(objet, "radio_ferme");
    
    if (entry_nom == NULL || entry_id == NULL || combo_gouvernerat == NULL || 
        spin_capacite == NULL || radio_ouvert == NULL || radio_ferme == NULL)
    {
        afficher_message(objet, "erreur", "Erreur: Widgets non trouvés!");
        return;
    }
    
    nom = gtk_entry_get_text(GTK_ENTRY(entry_nom));
    id_text = gtk_entry_get_text(GTK_ENTRY(entry_id));
    gouvernerat = gtk_combo_box_get_active_text(GTK_COMBO_BOX(combo_gouvernerat));
    capacite = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(spin_capacite));
    
    if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio_ouvert)))
        etat = 'O';
    else
        etat = 'F';
    
    if (strlen(nom) == 0 || strlen(id_text) == 0)
    {
        afficher_message(objet, "erreur", "Veuillez remplir le nom et l'ID!");
        return;
    }
    
    if (gouvernerat == NULL || strlen(gouvernerat) == 0)
    {
        afficher_message(objet, "erreur", "Veuillez sélectionner un gouvernerat!");
        return;
    }
    
    id = atoi(id_text);
    if (id <= 0)
    {
        afficher_message(objet, "erreur", "ID invalide! Doit être un nombre positif.");
        return;
    }
    
    if (capacite <= 0)
    {
        afficher_message(objet, "erreur", "Capacité invalide! Doit être un nombre positif.");
        return;
    }
    
    c.id = id;
    strncpy(c.nom, nom, sizeof(c.nom) - 1);
    c.nom[sizeof(c.nom) - 1] = '\0';
    
    strncpy(c.ville, gouvernerat, sizeof(c.ville) - 1);
    c.ville[sizeof(c.ville) - 1] = '\0';
    
    strncpy(c.adresse, "Non spécifié", sizeof(c.adresse) - 1);
    c.adresse[sizeof(c.adresse) - 1] = '\0';
    
    c.capacite = capacite;
    c.etat = etat;
    
    if (ajouter_centre("centres.txt", c))
    {
        afficher_message(objet, "info", "Centre ajouté avec succès!");
        fermer_fenetre(objet);
    }
    else
    {
        afficher_message(objet, "erreur", "Erreur lors de l'ajout du centre!");
    }
}

void
on_btn_ajouter_centre_annuler_clicked (GtkWidget *objet,
                                       gpointer   user_data)
{
    fermer_fenetre(objet);
}
//espace membre
void
on_btn_membre_deconnexion_clicked (GtkWidget *objet,
                                   gpointer   user_data)
{
    retour_vers_login(objet);
}

void
on_btn_membre_ask_clicked (GtkWidget *objet,
                           gpointer   user_data)
{
}




//filtrage
void
on_check_equipement_disponible_only_toggled (GtkWidget *widget,
                                             gpointer   user_data)
{
    gboolean active;
    
    active = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
}

void
on_combo_filtre_type_equipement_changed (GtkComboBox *combobox,
                                         gpointer     user_data)
{
}
//my gg
void
on_boutton_activate (GtkMenuItem *menuitem,
                     gpointer     user_data)
{
    retour_vers_login(GTK_WIDGET(menuitem));
}
//supprimer
void supprimer_ligne_treeview(GtkWidget *treeview, const char *fichier, int id){}
//menu
void on_menuitem7_activate(GtkMenuItem *menuitem, gpointer user_data){}
void on_new1_activate(GtkMenuItem *menuitem, gpointer user_data){}
void on_open1_activate(GtkMenuItem *menuitem, gpointer user_data){}
void on_save1_activate(GtkMenuItem *menuitem, gpointer user_data){}
void on_save_as1_activate(GtkMenuItem *menuitem, gpointer user_data){}
void on_quit1_activate(GtkMenuItem *menuitem, gpointer user_data){}
void on_menuitem8_activate(GtkMenuItem *menuitem, gpointer user_data){}
void on_cut1_activate(GtkMenuItem *menuitem, gpointer user_data){}
void on_copy1_activate(GtkMenuItem *menuitem, gpointer user_data){}
void on_paste1_activate(GtkMenuItem *menuitem, gpointer user_data){}
void on_delete1_activate(GtkMenuItem *menuitem, gpointer user_data){}
void on_menuitem9_activate(GtkMenuItem *menuitem, gpointer user_data){}
void on_menuitem10_activate(GtkMenuItem *menuitem, gpointer user_data){}
void on_about1_activate(GtkMenuItem *menuitem, gpointer user_data){}
void on_menuitem11_activate(GtkMenuItem *menuitem, gpointer user_data){}
//sign up
void on_btn_enregistrer_signup_clicked(GtkWidget *widget, gpointer user_data) {
    GtkWidget *window = gtk_widget_get_toplevel(widget);
    
    GtkWidget *entry_nom = lookup_widget(window, "entry_nom_signup");
    GtkWidget *entry_email = lookup_widget(window, "entry_email_signup");
    GtkWidget *entry_login = lookup_widget(window, "entry_login_signup");
    GtkWidget *entry_mdp = lookup_widget(window, "entry_mdp_signup");
    GtkWidget *spin_age = lookup_widget(window, "spin_age_signup");
    GtkWidget *radio_mensuelle = lookup_widget(window, "radio_mensuelle_signup");
    GtkWidget *radio_annuelle = lookup_widget(window, "radio_annuelle_signup");
    
    if (!entry_nom || !entry_email || !entry_login || !entry_mdp || 
        !spin_age || !radio_mensuelle || !radio_annuelle) {
        afficher_message(widget, "erreur", "Widgets non trouvés!");
        return;
    }
    
    const char *nom = gtk_entry_get_text(GTK_ENTRY(entry_nom));
    const char *email = gtk_entry_get_text(GTK_ENTRY(entry_email));
    const char *login = gtk_entry_get_text(GTK_ENTRY(entry_login));
    const char *mdp = gtk_entry_get_text(GTK_ENTRY(entry_mdp));
    int age = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(spin_age));
    gboolean annuelle = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio_annuelle));
    
    if (strlen(nom) == 0 || strlen(email) == 0 || strlen(login) == 0 || strlen(mdp) == 0) {
        afficher_message(widget, "erreur", "Veuillez remplir tous les champs!");
        return;
    }
    
    if (age < 6 || age > 120) {
        afficher_message(widget, "erreur", "Âge invalide!");
        return;
    }
    
    if (strchr(email, '@') == NULL) {
        afficher_message(widget, "erreur", "Email invalide!");
        return;
    }
    
    FILE *f = fopen("login.txt", "r");
    if (f) {
        char buffer[100];
        while (fgets(buffer, sizeof(buffer), f)) {
            char existing_login[50];
            sscanf(buffer, "%s", existing_login);
            if (strcmp(existing_login, login) == 0) {
                fclose(f);
                afficher_message(widget, "erreur", "Ce login existe déjà!");
                return;
            }
        }
        fclose(f);
    }
    
    int nouveau_id = 1;
    f = fopen("membres.txt", "r");
    if (f) {
        Membre m;
        while (fscanf(f, "%d %49s %49s %d %9s %19s %f", 
                     &m.id, m.nom, m.email, &m.age, m.sexe, 
                     m.Typeabonnement, &m.Tarif) == 7) {
            if (m.id >= nouveau_id) {
                nouveau_id = m.id + 1;
            }
        }
        fclose(f);
    }
    
    Membre nouv_membre;
    nouv_membre.id = nouveau_id;
    strncpy(nouv_membre.nom, nom, sizeof(nouv_membre.nom)-1);
    strncpy(nouv_membre.email, email, sizeof(nouv_membre.email)-1);
    nouv_membre.age = age;
    strncpy(nouv_membre.sexe, "Non spécifié", sizeof(nouv_membre.sexe)-1);
    
    if (annuelle) {
        strncpy(nouv_membre.Typeabonnement, "Annuelle", sizeof(nouv_membre.Typeabonnement)-1);
        nouv_membre.Tarif = 500.0;
    } else {
        strncpy(nouv_membre.Typeabonnement, "Mensuelle", sizeof(nouv_membre.Typeabonnement)-1);
        nouv_membre.Tarif = 50.0;
    }
    
    if (ajouter_membre("membres.txt", nouv_membre)) {
        f = fopen("login.txt", "a");
        if (f) {
            fprintf(f, "%s %s membre %d\n", login, mdp, nouveau_id);
            fclose(f);
            afficher_message(widget, "info", "Inscription réussie!");
            retour_vers_login(widget);
        } else {
            afficher_message(widget, "erreur", "Erreur création compte!");
        }
    } else {
        afficher_message(widget, "erreur", "Erreur ajout membre!");
    }
}

void on_btn_annuler_signup_clicked(GtkWidget *widget, gpointer user_data) {
    retour_vers_login(widget);
}

void on_btn_enregistrer_coach_signup_clicked(GtkWidget *widget, gpointer user_data) {
    GtkWidget *window = gtk_widget_get_toplevel(widget);
    
    GtkWidget *entry_nom = lookup_widget(window, "entry_nom_coach_signup");
    GtkWidget *entry_specialite = lookup_widget(window, "entry_specialite_coach_signup");
    GtkWidget *entry_login = lookup_widget(window, "entry_login_coach_signup");
    GtkWidget *entry_mdp = lookup_widget(window, "entry_mdp_coach_signup");
    GtkWidget *spin_age = lookup_widget(window, "spin_age_coach_signup");
    GtkWidget *spin_experience = lookup_widget(window, "spin_experience_coach_signup");
    GtkWidget *radio_homme = lookup_widget(window, "radio_homme_coach_signup");
    GtkWidget *radio_femme = lookup_widget(window, "radio_femme_coach_signup");
    
    if (!entry_nom || !entry_specialite || !entry_login || !entry_mdp || 
        !spin_age || !spin_experience || !radio_homme || !radio_femme) {
        afficher_message(widget, "erreur", "Widgets non trouvés!");
        return;
    }
    
    const char *nom = gtk_entry_get_text(GTK_ENTRY(entry_nom));
    const char *specialite = gtk_entry_get_text(GTK_ENTRY(entry_specialite));
    const char *login = gtk_entry_get_text(GTK_ENTRY(entry_login));
    const char *mdp = gtk_entry_get_text(GTK_ENTRY(entry_mdp));
    int age = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(spin_age));
    int experience = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(spin_experience));
    gboolean homme = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio_homme));
    
    if (strlen(nom) == 0 || strlen(specialite) == 0 || strlen(login) == 0 || strlen(mdp) == 0) {
        afficher_message(widget, "erreur", "Veuillez remplir tous les champs!");
        return;
    }
    
    if (age < 18 || age > 70) {
        afficher_message(widget, "erreur", "Âge invalide! (18-70 ans)");
        return;
    }
    
    if (experience < 0 || experience > 50) {
        afficher_message(widget, "erreur", "Expérience invalide! (0-50 ans)");
        return;
    }
    
    FILE *f = fopen("login.txt", "r");
    if (f) {
        char buffer[100];
        while (fgets(buffer, sizeof(buffer), f)) {
            char existing_login[50];
            sscanf(buffer, "%s", existing_login);
            if (strcmp(existing_login, login) == 0) {
                fclose(f);
                afficher_message(widget, "erreur", "Ce login existe déjà!");
                return;
            }
        }
        fclose(f);
    }
    
    int nouveau_id = 1;
    f = fopen("entraineurs.txt", "r");
    if (f) {
        Entraineur e;
        while (fscanf(f, "%d %49s %49s %d %d %c", 
                     &e.id, e.nometprenom, e.specialite, 
                     &e.experience, &e.age, &e.sexe) == 6) {
            if (e.id >= nouveau_id) {
                nouveau_id = e.id + 1;
            }
        }
        fclose(f);
    }
    
    Entraineur nouv_entraineur;
    nouv_entraineur.id = nouveau_id;
    strncpy(nouv_entraineur.nometprenom, nom, sizeof(nouv_entraineur.nometprenom)-1);
    strncpy(nouv_entraineur.specialite, specialite, sizeof(nouv_entraineur.specialite)-1);
    nouv_entraineur.experience = experience;
    nouv_entraineur.age = age;
    nouv_entraineur.sexe = homme ? 'H' : 'F';
    
    if (ajouter_entraineur("entraineurs.txt", nouv_entraineur)) {
        f = fopen("login.txt", "a");
        if (f) {
            fprintf(f, "%s %s entraineur %d\n", login, mdp, nouveau_id);
            fclose(f);
            afficher_message(widget, "info", "Inscription entraineur réussie!");
            retour_vers_login(widget);
        } else {
            afficher_message(widget, "erreur", "Erreur création compte!");
        }
    } else {
        afficher_message(widget, "erreur", "Erreur ajout entraineur!");
    }
}

void on_btn_annuler_coach_signup_clicked(GtkWidget *widget, gpointer user_data) {
    retour_vers_login(widget);
}

void
on_inscrire__cours_clicked (GtkButton *button,
                           gpointer   user_data)
{
    GtkWidget *dialog;
    GtkWidget *content_area;
    GtkWidget *treeview_cours;
    GtkWidget *scrolled_window;
    GtkListStore *store;
    GtkCellRenderer *renderer;
    GtkTreeViewColumn *column;
    gint response;
    
    dialog = gtk_dialog_new_with_buttons(
        "Inscription à un Cours",
        GTK_WINDOW(gtk_widget_get_toplevel(GTK_WIDGET(button))),
        GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
        "S'inscrire", GTK_RESPONSE_ACCEPT,
        "Annuler", GTK_RESPONSE_CANCEL,
        NULL);
    
    content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    
    scrolled_window = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window),
                                   GTK_POLICY_AUTOMATIC,
                                   GTK_POLICY_AUTOMATIC);
    gtk_widget_set_size_request(scrolled_window, 500, 300);
    
    treeview_cours = gtk_tree_view_new();
    
    store = gtk_list_store_new(6, G_TYPE_STRING, G_TYPE_STRING, 
                               G_TYPE_STRING, G_TYPE_STRING, 
                               G_TYPE_STRING, G_TYPE_STRING);
    
    FILE *f_cours = fopen("cours.txt", "r");
    
    if (f_cours != NULL) {
        Cours c;
        char ligne[200];
        while (fgets(ligne, sizeof(ligne), f_cours)) {
            if (sscanf(ligne, "%d %49s %49s %49s %49s %49s", 
                      &c.id, c.cours, c.coach, c.salle, 
                      c.date_heure, c.niveau) == 6) {
                
                GtkTreeIter iter;
                gtk_list_store_append(store, &iter);
                
                char id_str[20];
                sprintf(id_str, "%d", c.id);
                
                gtk_list_store_set(store, &iter,
                                  0, id_str,
                                  1, c.cours,
                                  2, c.coach,
                                  3, c.salle,
                                  4, c.date_heure,
                                  5, c.niveau,
                                  -1);
            }
        }
        fclose(f_cours);
    }
    
    gtk_tree_view_set_model(GTK_TREE_VIEW(treeview_cours), GTK_TREE_MODEL(store));
    g_object_unref(store);
    
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("ID", renderer, "text", 0, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeview_cours), column);
    
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("Cours", renderer, "text", 1, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeview_cours), column);
    
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("Coach", renderer, "text", 2, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeview_cours), column);
    
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("Salle", renderer, "text", 3, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeview_cours), column);
    
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("Date/Heure", renderer, "text", 4, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeview_cours), column);
    
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("Niveau", renderer, "text", 5, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeview_cours), column);
    
    gtk_container_add(GTK_CONTAINER(scrolled_window), treeview_cours);
    gtk_container_add(GTK_CONTAINER(content_area), scrolled_window);
    gtk_widget_show_all(dialog);
    
    response = gtk_dialog_run(GTK_DIALOG(dialog));
    
    if (response == GTK_RESPONSE_ACCEPT) {
        GtkTreeSelection *selection;
        GtkTreeModel *model;
        GtkTreeIter iter;
        gchar *id_cours_str;
        gchar *nom_cours;
        gchar *coach;
        gchar *salle;
        gchar *date_heure;
        gchar *niveau;
        
        selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(treeview_cours));
        if (gtk_tree_selection_get_selected(selection, &model, &iter)) {
            gtk_tree_model_get(model, &iter, 
                              0, &id_cours_str,
                              1, &nom_cours,
                              2, &coach,
                              3, &salle,
                              4, &date_heure,
                              5, &niveau,
                              -1);
            
            FILE *f_inscriptions = fopen("inscriptions_membres.txt", "a");
            if (f_inscriptions != NULL) {
                char date_actuelle[20];
                get_date_actuelle(date_actuelle);
                
                fprintf(f_inscriptions, "%s;%s;%s;%s;%s;%s\n", 
                       id_cours_str, nom_cours, coach, salle, date_heure, date_actuelle);
                fclose(f_inscriptions);
                
                afficher_message(GTK_WIDGET(button), "info", "Inscription au cours enregistrée!");
            } else {
                afficher_message(GTK_WIDGET(button), "erreur", "Erreur d'enregistrement!");
            }
            
            g_free(id_cours_str);
            g_free(nom_cours);
            g_free(coach);
            g_free(salle);
            g_free(date_heure);
            g_free(niveau);
        } else {
            afficher_message(GTK_WIDGET(button), "erreur", "Veuillez sélectionner un cours!");
        }
    }
    
    gtk_widget_destroy(dialog);
}

void
on_reserver__coach_clicked (GtkButton *button,
                           gpointer   user_data)
{
    GtkWidget *dialog;
    GtkWidget *content_area;
    GtkWidget *combo_entraineurs;
    GtkWidget *entry_date;
    GtkWidget *entry_heure;
    GtkWidget *spin_duree;
    GtkWidget *entry_note;
    GtkWidget *vbox;
    gint response;
    
    dialog = gtk_dialog_new_with_buttons(
        "Demander un Coach Privé",
        GTK_WINDOW(gtk_widget_get_toplevel(GTK_WIDGET(button))),
        GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
        "Envoyer Demande", GTK_RESPONSE_ACCEPT,
        "Annuler", GTK_RESPONSE_CANCEL,
        NULL);
    
    content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    
    vbox = gtk_vbox_new(FALSE, 5);
    gtk_container_set_border_width(GTK_CONTAINER(vbox), 10);
    
    GtkWidget *label_entraineur = gtk_label_new("Sélectionner un entraîneur:");
    gtk_box_pack_start(GTK_BOX(vbox), label_entraineur, FALSE, FALSE, 0);
    
    combo_entraineurs = gtk_combo_box_new_text();
    FILE *f_entraineurs = fopen("entraineurs.txt", "r");
    if (f_entraineurs != NULL) {
        Entraineur e;
        char ligne[100];
        while (fgets(ligne, sizeof(ligne), f_entraineurs)) {
            if (sscanf(ligne, "%d %49s %49s %d %d %c", 
                      &e.id, e.nometprenom, e.specialite, 
                      &e.experience, &e.age, &e.sexe) == 6) {
                char affichage[100];
                sprintf(affichage, "%s - %s", e.nometprenom, e.specialite);
                gtk_combo_box_append_text(GTK_COMBO_BOX(combo_entraineurs), affichage);
            }
        }
        fclose(f_entraineurs);
    }
    gtk_combo_box_set_active(GTK_COMBO_BOX(combo_entraineurs), 0);
    gtk_box_pack_start(GTK_BOX(vbox), combo_entraineurs, FALSE, FALSE, 0);
    
    GtkWidget *label_date = gtk_label_new("Date (JJ/MM/AAAA):");
    gtk_box_pack_start(GTK_BOX(vbox), label_date, FALSE, FALSE, 0);
    
    entry_date = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(entry_date), "ex: 15/12/2024");
    gtk_box_pack_start(GTK_BOX(vbox), entry_date, FALSE, FALSE, 0);
    
    GtkWidget *label_heure = gtk_label_new("Heure (HH:MM):");
    gtk_box_pack_start(GTK_BOX(vbox), label_heure, FALSE, FALSE, 0);
    
    entry_heure = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(entry_heure), "ex: 14:30");
    gtk_box_pack_start(GTK_BOX(vbox), entry_heure, FALSE, FALSE, 0);
    
    GtkWidget *label_duree = gtk_label_new("Durée (heures):");
    gtk_box_pack_start(GTK_BOX(vbox), label_duree, FALSE, FALSE, 0);
    
    spin_duree = gtk_spin_button_new_with_range(1, 4, 0.5);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(spin_duree), 1);
    gtk_box_pack_start(GTK_BOX(vbox), spin_duree, FALSE, FALSE, 0);
    
    GtkWidget *label_note = gtk_label_new("Note supplémentaire:");
    gtk_box_pack_start(GTK_BOX(vbox), label_note, FALSE, FALSE, 0);
    
    entry_note = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(entry_note), "Spécificités, objectifs...");
    gtk_box_pack_start(GTK_BOX(vbox), entry_note, FALSE, FALSE, 0);
    
    gtk_container_add(GTK_CONTAINER(content_area), vbox);
    gtk_widget_show_all(dialog);
    
    response = gtk_dialog_run(GTK_DIALOG(dialog));
    
    if (response == GTK_RESPONSE_ACCEPT) {
        const char *date = gtk_entry_get_text(GTK_ENTRY(entry_date));
        const char *heure = gtk_entry_get_text(GTK_ENTRY(entry_heure));
        double duree = gtk_spin_button_get_value(GTK_SPIN_BUTTON(spin_duree));
        const char *note = gtk_entry_get_text(GTK_ENTRY(entry_note));
        
        gchar *entraineur_text = NULL;
        gint active = gtk_combo_box_get_active(GTK_COMBO_BOX(combo_entraineurs));
        if (active >= 0) {
            entraineur_text = (gchar *)gtk_combo_box_get_active_text(GTK_COMBO_BOX(combo_entraineurs));
        }
        
        if (strlen(date) == 0 || strlen(heure) == 0 || entraineur_text == NULL) {
            afficher_message(GTK_WIDGET(button), "erreur", "Veuillez remplir tous les champs obligatoires!");
        } else {
            FILE *f_demandes = fopen("demandes.txt", "a");
            if (f_demandes != NULL) {
                char date_heure[50];
                sprintf(date_heure, "%s %s", date, heure);
                
                fprintf(f_demandes, "COACH;%s;%s;%.1f;%s\n", 
                       entraineur_text, date_heure, duree, note);
                fclose(f_demandes);
                
                afficher_message(GTK_WIDGET(button), "info", "Demande de coach privé envoyée!");
            } else {
                afficher_message(GTK_WIDGET(button), "erreur", "Erreur d'enregistrement!");
            }
        }
        
        if (entraineur_text) {
            g_free(entraineur_text);
        }
    }
    
    gtk_widget_destroy(dialog);
}

void on_equip__pouruncour_clicked(GtkButton *button, gpointer user_data)
{
    GtkWidget *widget = GTK_WIDGET(button);
    GtkWidget *dialog;
    GtkWidget *content_area;
    GtkWidget *vbox;
    GtkWidget *label;
    GtkWidget *combo_cours;
    GtkWidget *combo_equipement;
    FILE *f;
    Cours c;
    Equipement e;
    

    dialog = gtk_dialog_new_with_buttons(
        "Réserver équipement pour un cours",
        GTK_WINDOW(gtk_widget_get_toplevel(widget)),
        GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
        "Confirmer", GTK_RESPONSE_OK,
        "Annuler", GTK_RESPONSE_CANCEL,
        NULL);
    
    content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    vbox = gtk_vbox_new(FALSE, 5);
    gtk_container_add(GTK_CONTAINER(content_area), vbox);
    

    label = gtk_label_new("Sélectionner un cours:");
    gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, FALSE, 5);
    
    combo_cours = gtk_combo_box_new_text();
    f = fopen("cours.txt", "r");
    if (f != NULL) {
        while (fscanf(f, "%d %49s %49s %49s %49s %49s", 
                     &c.id, c.cours, c.coach, c.salle, 
                     c.date_heure, c.niveau) == 6) {
            char cours_info[150];
            sprintf(cours_info, "%s - %s (%s)", c.cours, c.salle, c.date_heure);
            gtk_combo_box_append_text(GTK_COMBO_BOX(combo_cours), cours_info);
        }
        fclose(f);
    }
    gtk_combo_box_set_active(GTK_COMBO_BOX(combo_cours), 0);
    gtk_box_pack_start(GTK_BOX(vbox), combo_cours, FALSE, FALSE, 5);
    label = gtk_label_new("Sélectionner un équipement:");
    gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, FALSE, 5);
    
    combo_equipement = gtk_combo_box_new_text();
    f = fopen("equipements.txt", "r");
    if (f != NULL) {
        while (fscanf(f, "%19s %49s %49s %d %c %c", 
                     e.reference, e.type, e.centre, 
                     &e.capacite, &e.disponibilite, &e.etat) == 6) {
            if (e.disponibilite == 'D') {
                char equip_info[150];
                sprintf(equip_info, "%s - %s (Réf: %s)", e.type, e.centre, e.reference);
                gtk_combo_box_append_text(GTK_COMBO_BOX(combo_equipement), equip_info);
            }
        }
        fclose(f);
    }
    gtk_combo_box_set_active(GTK_COMBO_BOX(combo_equipement), 0);
    gtk_box_pack_start(GTK_BOX(vbox), combo_equipement, FALSE, FALSE, 5);
    
    gtk_widget_show_all(dialog);
    
    gint response = gtk_dialog_run(GTK_DIALOG(dialog));
    
    if (response == GTK_RESPONSE_OK)
    {
        const char *cours = gtk_combo_box_get_active_text(GTK_COMBO_BOX(combo_cours));
        const char *equipement = gtk_combo_box_get_active_text(GTK_COMBO_BOX(combo_equipement));
        
        if (!cours || !equipement) {
            afficher_message(widget, "erreur", "Veuillez sélectionner un cours et un équipement!");
            gtk_widget_destroy(dialog);
            return;
        }f = fopen("reservations_equipement.txt", "a");
        if (f != NULL)
        {
            time_t now = time(NULL);
            struct tm *t = localtime(&now);
            fprintf(f, "%s;%s;%02d/%02d/%04d\n", 
                    cours, equipement, 
                    t->tm_mday, t->tm_mon + 1, t->tm_year + 1900);
            fclose(f);
            
            afficher_message(widget, "info", "Équipement réservé avec succès!");
        }
        else
        {
            afficher_message(widget, "erreur", "Erreur lors de la réservation!");
        }
    }
    
    gtk_widget_destroy(dialog);
}

void on_inscrit__cour_clicked(GtkButton *button, gpointer user_data)
{
    GtkWidget *widget = GTK_WIDGET(button);
    GtkWidget *dialog;
    GtkWidget *content_area;
    GtkWidget *vbox;
    GtkWidget *label;
    GtkWidget *combo_cours;
    GtkWidget *spin_participants;
    FILE *f;
    Cours c;
    

    dialog = gtk_dialog_new_with_buttons(
        "S'inscrire à un cours",
        GTK_WINDOW(gtk_widget_get_toplevel(widget)),
        GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
        "S'inscrire", GTK_RESPONSE_OK,
        "Annuler", GTK_RESPONSE_CANCEL,
        NULL);
    
    content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    vbox = gtk_vbox_new(FALSE, 5);
    gtk_container_add(GTK_CONTAINER(content_area), vbox);
    

    label = gtk_label_new("Choisir un cours:");
    gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, FALSE, 5);
    
    combo_cours = gtk_combo_box_new_text();
    f = fopen("cours.txt", "r");
    if (f != NULL) {
        while (fscanf(f, "%d %49s %49s %49s %49s %49s", 
                     &c.id, c.cours, c.coach, c.salle, 
                     c.date_heure, c.niveau) == 6) {
            char cours_info[200];
            sprintf(cours_info, "%s - %s - %s (Niveau: %s)", 
                    c.cours, c.date_heure, c.coach, c.niveau);
            gtk_combo_box_append_text(GTK_COMBO_BOX(combo_cours), cours_info);
        }
        fclose(f);
    }
    gtk_combo_box_set_active(GTK_COMBO_BOX(combo_cours), 0);
    gtk_box_pack_start(GTK_BOX(vbox), combo_cours, FALSE, FALSE, 5);
    label = gtk_label_new("Nombre de participants:");
    gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, FALSE, 5);
    
    spin_participants = gtk_spin_button_new_with_range(1, 20, 1);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(spin_participants), 1);
    gtk_box_pack_start(GTK_BOX(vbox), spin_participants, FALSE, FALSE, 5);
    
    gtk_widget_show_all(dialog);
    
    gint response = gtk_dialog_run(GTK_DIALOG(dialog));
    
    if (response == GTK_RESPONSE_OK)
    {
        const char *cours = gtk_combo_box_get_active_text(GTK_COMBO_BOX(combo_cours));
        int participants = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(spin_participants));
        
        if (!cours) {
            afficher_message(widget, "erreur", "Veuillez sélectionner un cours!");
            gtk_widget_destroy(dialog);
            return;
        }f = fopen("inscriptions_ent_cours.txt", "a");
        if (f != NULL)
        {
            time_t now = time(NULL);
            struct tm *t = localtime(&now);
            fprintf(f, "%s;%d;%02d/%02d/%04d\n", 
                    cours, participants,
                    t->tm_mday, t->tm_mon + 1, t->tm_year + 1900);
            fclose(f);
            
            char msg[200];
            sprintf(msg, "Inscription confirmée pour %d participant(s)!", participants);
            afficher_message(widget, "info", msg);
        }
        else
        {
            afficher_message(widget, "erreur", "Erreur lors de l'inscription!");
        }
    }
    
    gtk_widget_destroy(dialog);
}


void on_inscrit__centre__sportif_clicked(GtkButton *button, gpointer user_data)
{
    GtkWidget *widget = GTK_WIDGET(button);
    GtkWidget *dialog;
    GtkWidget *content_area;
    GtkWidget *vbox;
    GtkWidget *label;
    GtkWidget *combo_centre;
    GtkWidget *entry_date_debut;
    GtkWidget *combo_duree;
    FILE *f;
    Centre centre;
    

    dialog = gtk_dialog_new_with_buttons(
        "S'inscrire à un centre sportif",
        GTK_WINDOW(gtk_widget_get_toplevel(widget)),
        GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
        "S'inscrire", GTK_RESPONSE_OK,
        "Annuler", GTK_RESPONSE_CANCEL,
        NULL);
    
    content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    vbox = gtk_vbox_new(FALSE, 5);
    gtk_container_add(GTK_CONTAINER(content_area), vbox);
    

    label = gtk_label_new("Sélectionner un centre:");
    gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, FALSE, 5);
    
    combo_centre = gtk_combo_box_new_text();
    f = fopen("centres.txt", "r");
    if (f != NULL) {
        char buffer[256];
        while (fgets(buffer, sizeof(buffer), f)) {
            buffer[strcspn(buffer, "\n")] = 0;
            
            char *p = buffer;
            int fields = 0;
            char *field_ptrs[10];
            
            field_ptrs[fields++] = p;
            while (*p && fields < 10) {
                if (*p == ';') {
                    *p = '\0';
                    p++;
                    if (*p && *p != ';') {
                        field_ptrs[fields++] = p;
                    }
                } else {
                    p++;
                }
            }
            
            if (fields >= 5) {
                centre.id = atoi(field_ptrs[0]);
                strcpy(centre.nom, field_ptrs[1]);
                strcpy(centre.adresse, field_ptrs[2]);
                strcpy(centre.ville, field_ptrs[3]);
                centre.capacite = atoi(field_ptrs[4]);
                
                char centre_info[200];
                sprintf(centre_info, "%s - %s (Capacité: %d)", 
                        centre.nom, centre.ville, centre.capacite);
                gtk_combo_box_append_text(GTK_COMBO_BOX(combo_centre), centre_info);
            }
        }
        fclose(f);
    }
    gtk_combo_box_set_active(GTK_COMBO_BOX(combo_centre), 0);
    gtk_box_pack_start(GTK_BOX(vbox), combo_centre, FALSE, FALSE, 5);
    

    label = gtk_label_new("Date de début (JJ/MM/AAAA):");
    gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, FALSE, 5);
    
    entry_date_debut = gtk_entry_new();
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    char date_str[20];
    sprintf(date_str, "%02d/%02d/%04d", t->tm_mday, t->tm_mon + 1, t->tm_year + 1900);
    gtk_entry_set_text(GTK_ENTRY(entry_date_debut), date_str);
    gtk_box_pack_start(GTK_BOX(vbox), entry_date_debut, FALSE, FALSE, 5);
    

    label = gtk_label_new("Durée d'abonnement:");
    gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, FALSE, 5);
    
    combo_duree = gtk_combo_box_new_text();
    gtk_combo_box_append_text(GTK_COMBO_BOX(combo_duree), "1 mois - 50 DT");
    gtk_combo_box_append_text(GTK_COMBO_BOX(combo_duree), "3 mois - 140 DT");
    gtk_combo_box_append_text(GTK_COMBO_BOX(combo_duree), "6 mois - 270 DT");
    gtk_combo_box_append_text(GTK_COMBO_BOX(combo_duree), "1 an - 500 DT");
    gtk_combo_box_set_active(GTK_COMBO_BOX(combo_duree), 0);
    gtk_box_pack_start(GTK_BOX(vbox), combo_duree, FALSE, FALSE, 5);
    
    gtk_widget_show_all(dialog);
    
    gint response = gtk_dialog_run(GTK_DIALOG(dialog));
    
    if (response == GTK_RESPONSE_OK)
    {
        const char *centre_sel = gtk_combo_box_get_active_text(GTK_COMBO_BOX(combo_centre));
        const char *date_debut = gtk_entry_get_text(GTK_ENTRY(entry_date_debut));
        const char *duree = gtk_combo_box_get_active_text(GTK_COMBO_BOX(combo_duree));
        
        if (strlen(date_debut) == 0) {
            afficher_message(widget, "erreur", "Veuillez entrer une date de début!");
            gtk_widget_destroy(dialog);
            return;
        }
        
        if (!centre_sel || !duree) {
            afficher_message(widget, "erreur", "Veuillez sélectionner un centre et une durée!");
            gtk_widget_destroy(dialog);
            return;
        }
        

        f = fopen("inscriptions_centres.txt", "a");
        if (f != NULL)
        {
            fprintf(f, "%s;%s;%s\n", centre_sel, date_debut, duree);
            fclose(f);
            
            afficher_message(widget, "info", "Inscription au centre réussie!");
        }
        else
        {
            afficher_message(widget, "erreur", "Erreur lors de l'inscription!");
        }
    }
    
    gtk_widget_destroy(dialog);
}
//stats
DiagrammeData lire_toutes_donnees(void)
{
    DiagrammeData donnees = {0};
    

    FILE *fichier = fopen("membres.txt", "r");
    if (fichier) {
        char ligne[512];
        while (fgets(ligne, sizeof(ligne), fichier)) {
            ligne[strcspn(ligne, "\n")] = 0;
            if (strlen(ligne) == 0) continue;
            
            int id, age;
            char temp1[100], temp2[100], temp3[100];
            float prix;
            

            if (sscanf(ligne, "%d %s %s %d %s %s %s %f", 
                      &id, temp1, temp2, &age, temp3, temp3, temp3, &prix) >= 4) {
                if (age > 0 && age < 120) {
                    if (age < 18) donnees.jeunes++;
                    else if (age < 60) donnees.adultes++;
                    else donnees.seniors++;
                    donnees.total_membres++;
                }
            }
        }
        fclose(fichier);
    }
    

    fichier = fopen("cours.txt", "r");
    if (fichier) {
        char ligne[512];
        while (fgets(ligne, sizeof(ligne), fichier)) {
            ligne[strcspn(ligne, "\n")] = 0;
            if (strlen(ligne) == 0) continue;
            

            char niveau_str[50];
            char *niveau_ptr;
            

            niveau_ptr = strstr(ligne, "Niveau_");
            if (niveau_ptr != NULL) {

                int niveau = atoi(niveau_ptr + 7); 
                
                if (niveau == 1) donnees.niveau1++;
                else if (niveau == 2) donnees.niveau2++;
                else if (niveau == 3) donnees.niveau3++;
                donnees.total_cours++;
            }

            else {

                int id;
                char temp1[100], temp2[100], temp3[100], temp4[100], temp5[100];
                
                if (sscanf(ligne, "%d %s %s %s %s %s", 
                          &id, temp1, temp2, temp3, temp4, temp5) >= 6) {

                    if (strstr(temp5, "Niveau_1") || strstr(temp5, "niveau_1") || strstr(temp5, "1"))
                        donnees.niveau1++;
                    else if (strstr(temp5, "Niveau_2") || strstr(temp5, "niveau_2") || strstr(temp5, "2"))
                        donnees.niveau2++;
                    else if (strstr(temp5, "Niveau_3") || strstr(temp5, "niveau_3") || strstr(temp5, "3"))
                        donnees.niveau3++;
                    donnees.total_cours++;
                }
            }
        }
        fclose(fichier);
    }
    

    fichier = fopen("entraineurs.txt", "r");
    if (fichier) {
        char ligne[512];
        while (fgets(ligne, sizeof(ligne), fichier)) {
            ligne[strcspn(ligne, "\n")] = 0;
            if (strlen(ligne) == 0) continue;
            donnees.total_entraineurs++;
            

            char *ptr;
            
            if ((ptr = strstr(ligne, "Musculation")) != NULL || 
                (ptr = strstr(ligne, "musculation")) != NULL) {
                donnees.musculation++;
            }
            else if ((ptr = strstr(ligne, "Gymnastique")) != NULL || 
                     (ptr = strstr(ligne, "gymnastique")) != NULL) {
                donnees.gymnastique++;
            }
            else if ((ptr = strstr(ligne, "Yoga")) != NULL || 
                     (ptr = strstr(ligne, "yoga")) != NULL) {
                donnees.yoga++;
            }
            else {

                donnees.musculation++;
            }
        }
        fclose(fichier);
    }
    

    fichier = fopen("equipements.txt", "r");
    if (fichier) {
        char ligne[512];
        while (fgets(ligne, sizeof(ligne), fichier)) {
            ligne[strcspn(ligne, "\n")] = 0;
            if (strlen(ligne) == 0) continue;
            

            donnees.total_equipements++;
            

            char mot1[100], mot2[100], mot3[100];
            int id, quantite;
            
            if (sscanf(ligne, "%d %s %s %d", &id, mot1, mot2, &quantite) >= 4) {

                if (strstr(mot1, "Cardio") || strstr(mot1, "cardio"))
                    donnees.cardio++;
                else if (strstr(mot1, "Mobilites") || strstr(mot1, "mobilites"))
                    donnees.mobilites++;
                else if (strstr(mot1, "Musculation") || strstr(mot1, "musculation"))
                    donnees.musculation_eq++;
                else {

                    donnees.cardio++;
                }
            }
            else {
                donnees.cardio++;
            }
        }
        fclose(fichier);
    }
    

    fichier = fopen("centres.txt", "r");
    if (fichier) {
        char ligne[512];
        while (fgets(ligne, sizeof(ligne), fichier)) {
            ligne[strcspn(ligne, "\n")] = 0;
            if (strlen(ligne) == 0) continue;
            
            donnees.total_centres++;
            char *tokens[10];
            int token_count = 0;
            char *token = strtok(ligne, ";");
            
            while (token != NULL && token_count < 10) {
                tokens[token_count++] = token;
                token = strtok(NULL, ";");
            }
            

            if (token_count >= 4) {
                char *ville = tokens[3];
                
                if (strstr(ville, "Tunis") || strstr(ville, "tunis"))
                    donnees.tunis++;
                else if (strstr(ville, "Ariana") || strstr(ville, "ariana"))
                    donnees.ariana++;
                else if (strstr(ville, "Aouina") || strstr(ville, "aouina"))
                    donnees.aouina++;
                else {

                    donnees.tunis++;
                }
            }
            else {

                char *ptr;
                
                if ((ptr = strstr(ligne, "Tunis")) != NULL || 
                    (ptr = strstr(ligne, "tunis")) != NULL)
                    donnees.tunis++;
                else if ((ptr = strstr(ligne, "Ariana")) != NULL || 
                         (ptr = strstr(ligne, "ariana")) != NULL)
                    donnees.ariana++;
                else if ((ptr = strstr(ligne, "Aouina")) != NULL || 
                         (ptr = strstr(ligne, "aouina")) != NULL)
                    donnees.aouina++;
                else {

                    donnees.tunis++;
                }
            }
        }
        fclose(fichier);
    }
    
    return donnees;
}
void initialiser_diagramme(GtkWidget *fenetre_admin)
{

    g_print("Diagramme initialisé\n");
    

    if (rafraichir_diagramme) {
        rafraichir_diagramme(fenetre_admin);
    }
}
void rafraichir_diagramme(GtkWidget *fenetre_admin)
{

    GtkWidget *drawing_area = lookup_widget(fenetre_admin, "drawingarea_diagramme");
    if (drawing_area) {
        gtk_widget_queue_draw(drawing_area);
    }
}

gboolean
on_drawingarea_stat_cours_expose_event (GtkWidget       *widget,
                                        GdkEventExpose  *event,
                                        gpointer         user_data)
{
    cairo_t *cr = gdk_cairo_create(widget->window);
    
    GtkAllocation allocation;
    gtk_widget_get_allocation(widget, &allocation);
    int width = allocation.width;
    int height = allocation.height;
    
    cairo_set_source_rgb(cr, 1, 1, 1);
    cairo_paint(cr);
    

    DiagrammeData donnees = {0};
    
    FILE *f = fopen("cours.txt", "r");
    if (f != NULL) {
        char ligne[512];
        while (fgets(ligne, sizeof(ligne), f)) {
            ligne[strcspn(ligne, "\n")] = 0;
            if (strlen(ligne) == 0) continue;
            
            char niveau_str[50];
            int id;
            char temp1[100], temp2[100], temp3[100], temp4[100];
            
            if (sscanf(ligne, "%d %49s %49s %49s %49s %49s", 
                      &id, temp1, temp2, temp3, temp4, niveau_str) == 6) {
                
                donnees.total_cours++;
                
                if (strstr(niveau_str, "1") != NULL || 
                    strstr(niveau_str, "Débutant") != NULL ||
                    strstr(niveau_str, "debutant") != NULL ||
                    strstr(niveau_str, "Debutant") != NULL ||
                    strstr(niveau_str, "Niveau_1") != NULL) {
                    donnees.niveau1++;
                }
                else if (strstr(niveau_str, "2") != NULL || 
                         strstr(niveau_str, "Intermédiaire") != NULL ||
                         strstr(niveau_str, "intermediaire") != NULL ||
                         strstr(niveau_str, "Intermediaire") != NULL ||
                         strstr(niveau_str, "Niveau_2") != NULL) {
                    donnees.niveau2++;
                }
                else if (strstr(niveau_str, "3") != NULL || 
                         strstr(niveau_str, "Avancé") != NULL ||
                         strstr(niveau_str, "avance") != NULL ||
                         strstr(niveau_str, "Avance") != NULL ||
                         strstr(niveau_str, "avancé") != NULL ||
                         strstr(niveau_str, "Niveau_3") != NULL) {
                    donnees.niveau3++;
                }
            }
        }
        fclose(f);
    }
    
    if (donnees.total_cours == 0) {
        cairo_set_source_rgb(cr, 0.5, 0.5, 0.5);
        cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
        cairo_set_font_size(cr, 10);
        cairo_move_to(cr, 10, height/2);
        cairo_show_text(cr, "Aucun cours");
        cairo_destroy(cr);
        return FALSE;
    }
    

    float pct_niv1 = (donnees.niveau1 * 100.0) / donnees.total_cours;
    float pct_niv2 = (donnees.niveau2 * 100.0) / donnees.total_cours;
    float pct_niv3 = (donnees.niveau3 * 100.0) / donnees.total_cours;
    

    int legende_width = 100;
    int diagram_width = width - legende_width - 20;
    int centre_x = legende_width + 10 + diagram_width / 2;
    int centre_y = height / 2;
    int rayon = MIN(diagram_width, height) / 3;
    
    double angle_niv1 = (pct_niv1 * 2 * M_PI) / 100.0;
    double angle_niv2 = (pct_niv2 * 2 * M_PI) / 100.0;
    double angle_niv3 = (pct_niv3 * 2 * M_PI) / 100.0;
    
    double angle_actuel = 0;
    

    if (donnees.niveau1 > 0) {
        cairo_set_source_rgb(cr, 0.9, 0.6, 0.1);
        cairo_move_to(cr, centre_x, centre_y);
        cairo_arc(cr, centre_x, centre_y, rayon, angle_actuel, angle_actuel + angle_niv1);
        cairo_close_path(cr);
        cairo_fill(cr);
        angle_actuel += angle_niv1;
    }
    

    if (donnees.niveau2 > 0) {
        cairo_set_source_rgb(cr, 0.6, 0.3, 0.8);
        cairo_move_to(cr, centre_x, centre_y);
        cairo_arc(cr, centre_x, centre_y, rayon, angle_actuel, angle_actuel + angle_niv2);
        cairo_close_path(cr);
        cairo_fill(cr);
        angle_actuel += angle_niv2;
    }
    

    if (donnees.niveau3 > 0) {
        cairo_set_source_rgb(cr, 0.1, 0.7, 0.9);
        cairo_move_to(cr, centre_x, centre_y);
        cairo_arc(cr, centre_x, centre_y, rayon, angle_actuel, angle_actuel + angle_niv3);
        cairo_close_path(cr);
        cairo_fill(cr);
    }
    

    int legende_x = 10;
    int legende_y = 20;
    int espace_ligne = 25;
    
    cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_font_size(cr, 11);
    cairo_set_source_rgb(cr, 0, 0, 0);
    cairo_move_to(cr, legende_x, legende_y);
    cairo_show_text(cr, "Cours par niveau:");
    
    legende_y += 20;
    

    cairo_set_source_rgb(cr, 0.9, 0.6, 0.1);
    cairo_rectangle(cr, legende_x, legende_y, 15, 15);
    cairo_fill(cr);
    cairo_set_source_rgb(cr, 0, 0, 0);
    cairo_set_line_width(cr, 0.5);
    cairo_rectangle(cr, legende_x, legende_y, 15, 15);
    cairo_stroke(cr);
    
    cairo_set_source_rgb(cr, 0, 0, 0);
    cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
    cairo_set_font_size(cr, 9);
    
    char texte_niv1[100];
    sprintf(texte_niv1, "Niveau 1: %d (%.1f%%)", donnees.niveau1, pct_niv1);
    cairo_move_to(cr, legende_x + 20, legende_y + 11);
    cairo_show_text(cr, texte_niv1);
    
    legende_y += espace_ligne;
    

    cairo_set_source_rgb(cr, 0.6, 0.3, 0.8);
    cairo_rectangle(cr, legende_x, legende_y, 15, 15);
    cairo_fill(cr);
    cairo_set_source_rgb(cr, 0, 0, 0);
    cairo_rectangle(cr, legende_x, legende_y, 15, 15);
    cairo_stroke(cr);
    
    char texte_niv2[100];
    sprintf(texte_niv2, "Niveau 2: %d (%.1f%%)", donnees.niveau2, pct_niv2);
    cairo_move_to(cr, legende_x + 20, legende_y + 11);
    cairo_show_text(cr, texte_niv2);
    
    legende_y += espace_ligne;
    

    cairo_set_source_rgb(cr, 0.1, 0.7, 0.9);
    cairo_rectangle(cr, legende_x, legende_y, 15, 15);
    cairo_fill(cr);
    cairo_set_source_rgb(cr, 0, 0, 0);
    cairo_rectangle(cr, legende_x, legende_y, 15, 15);
    cairo_stroke(cr);
    
    char texte_niv3[100];
    sprintf(texte_niv3, "Niveau 3: %d (%.1f%%)", donnees.niveau3, pct_niv3);
    cairo_move_to(cr, legende_x + 20, legende_y + 11);
    cairo_show_text(cr, texte_niv3);
    
    legende_y += espace_ligne + 10;
    

    cairo_set_source_rgb(cr, 0, 0, 0);
    cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_font_size(cr, 10);
    
    char texte_total[100];
    sprintf(texte_total, "Total: %d cours", donnees.total_cours);
    cairo_move_to(cr, legende_x, legende_y);
    cairo_show_text(cr, texte_total);
    
    cairo_destroy(cr);
    return FALSE;
}


gboolean
on_drawingarea_stat_equipements_expose_event (GtkWidget *widget, GdkEventExpose *event, gpointer user_data)
{
    cairo_t *cr = gdk_cairo_create(widget->window);
    
    GtkAllocation allocation;
    gtk_widget_get_allocation(widget, &allocation);
    int width = allocation.width;
    int height = allocation.height;
    
    cairo_set_source_rgb(cr, 1, 1, 1);
    cairo_paint(cr);
    
    DiagrammeData donnees = {0};
    
    FILE *f = fopen("equipements.txt", "r");
    if (f != NULL) {
        char ligne[512];
        while (fgets(ligne, sizeof(ligne), f)) {
            ligne[strcspn(ligne, "\n")] = 0;
            if (strlen(ligne) == 0) continue;
            
            char ref[50], type[50], centre[50];
            int capacite;
            char dispo, etat;
            
            if (sscanf(ligne, "%49s %49s %49s %d %c %c", 
                      ref, type, centre, &capacite, &dispo, &etat) == 6) {
                
                donnees.total_equipements++;
                
                if (strcmp(type, "Cardio") == 0) donnees.cardio++;
                else if (strcmp(type, "Mobilités") == 0) donnees.mobilites++;
                else if (strcmp(type, "Musculation") == 0) donnees.musculation_eq++;
            }
        }
        fclose(f);
    }
    
    if (donnees.total_equipements == 0) {
        cairo_set_source_rgb(cr, 0.5, 0.5, 0.5);
        cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
        cairo_set_font_size(cr, 10);
        cairo_move_to(cr, 10, height/2);
        cairo_show_text(cr, "Aucun équipement");
        cairo_destroy(cr);
        return FALSE;
    }
    
    float pct_mob = (donnees.mobilites * 100.0) / donnees.total_equipements;
    float pct_musc_eq = (donnees.musculation_eq * 100.0) / donnees.total_equipements;
    float pct_cardio = (donnees.cardio * 100.0) / donnees.total_equipements;
    
    int legende_width = 100;
    int diagram_width = width - legende_width - 20;
    int centre_x = legende_width + 10 + diagram_width / 2;
    int centre_y = height / 2;
    int rayon = MIN(diagram_width, height) / 3;
    
    double angle_mob = (pct_mob * 2 * M_PI) / 100.0;
    double angle_musc_eq = (pct_musc_eq * 2 * M_PI) / 100.0;
    double angle_cardio = (pct_cardio * 2 * M_PI) / 100.0;
    
    double angle_actuel = 0;
    

    if (donnees.mobilites > 0) {
        cairo_set_source_rgb(cr, 0.3, 0.7, 0.9);
        cairo_move_to(cr, centre_x, centre_y);
        cairo_arc(cr, centre_x, centre_y, rayon, angle_actuel, angle_actuel + angle_mob);
        cairo_close_path(cr);
        cairo_fill(cr);
        angle_actuel += angle_mob;
    }
    

    if (donnees.musculation_eq > 0) {
        cairo_set_source_rgb(cr, 0.9, 0.5, 0.1);
        cairo_move_to(cr, centre_x, centre_y);
        cairo_arc(cr, centre_x, centre_y, rayon, angle_actuel, angle_actuel + angle_musc_eq);
        cairo_close_path(cr);
        cairo_fill(cr);
        angle_actuel += angle_musc_eq;
    }
    

    if (donnees.cardio > 0) {
        cairo_set_source_rgb(cr, 0.7, 0.1, 0.5);
        cairo_move_to(cr, centre_x, centre_y);
        cairo_arc(cr, centre_x, centre_y, rayon, angle_actuel, angle_actuel + angle_cardio);
        cairo_close_path(cr);
        cairo_fill(cr);
    }
    

    int legende_x = 10;
    int legende_y = 20;
    int espace_ligne = 25;
    
    cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_font_size(cr, 11);
    cairo_set_source_rgb(cr, 0, 0, 0);
    cairo_move_to(cr, legende_x, legende_y);
    cairo_show_text(cr, "Équipements par type:");
    
    legende_y += 20;
    

    cairo_set_source_rgb(cr, 0.7, 0.1, 0.5);
    cairo_rectangle(cr, legende_x, legende_y, 15, 15);
    cairo_fill(cr);
    cairo_set_source_rgb(cr, 0, 0, 0);
    cairo_set_line_width(cr, 0.5);
    cairo_rectangle(cr, legende_x, legende_y, 15, 15);
    cairo_stroke(cr);
    
    cairo_set_source_rgb(cr, 0, 0, 0);
    cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
    cairo_set_font_size(cr, 9);
    
    char texte_cardio[100];
    sprintf(texte_cardio, "Cardio: %d (%.1f%%)", donnees.cardio, pct_cardio);
    cairo_move_to(cr, legende_x + 20, legende_y + 11);
    cairo_show_text(cr, texte_cardio);
    
    legende_y += espace_ligne;
    

    cairo_set_source_rgb(cr, 0.3, 0.7, 0.9);
    cairo_rectangle(cr, legende_x, legende_y, 15, 15);
    cairo_fill(cr);
    cairo_set_source_rgb(cr, 0, 0, 0);
    cairo_rectangle(cr, legende_x, legende_y, 15, 15);
    cairo_stroke(cr);
    
    char texte_mob[100];
    sprintf(texte_mob, "Mobilités: %d (%.1f%%)", donnees.mobilites, pct_mob);
    cairo_move_to(cr, legende_x + 20, legende_y + 11);
    cairo_show_text(cr, texte_mob);
    
    legende_y += espace_ligne;
    

    cairo_set_source_rgb(cr, 0.9, 0.5, 0.1);
    cairo_rectangle(cr, legende_x, legende_y, 15, 15);
    cairo_fill(cr);
    cairo_set_source_rgb(cr, 0, 0, 0);
    cairo_rectangle(cr, legende_x, legende_y, 15, 15);
    cairo_stroke(cr);
    
    char texte_musc_eq[100];
    sprintf(texte_musc_eq, "Musculation: %d (%.1f%%)", donnees.musculation_eq, pct_musc_eq);
    cairo_move_to(cr, legende_x + 20, legende_y + 11);
    cairo_show_text(cr, texte_musc_eq);
    
    legende_y += espace_ligne + 10;
    

    cairo_set_source_rgb(cr, 0, 0, 0);
    cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_font_size(cr, 10);
    
    char texte_total[100];
    sprintf(texte_total, "Total: %d équipements", donnees.total_equipements);
    cairo_move_to(cr, legende_x, legende_y);
    cairo_show_text(cr, texte_total);
    
    cairo_destroy(cr);
    return FALSE;
}


gboolean
on_drawingarea_stat_entraineurs_expose_event (GtkWidget *widget, GdkEventExpose *event, gpointer user_data)
{
    cairo_t *cr = gdk_cairo_create(widget->window);
    
    GtkAllocation allocation;
    gtk_widget_get_allocation(widget, &allocation);
    int width = allocation.width;
    int height = allocation.height;
    
    cairo_set_source_rgb(cr, 1, 1, 1);
    cairo_paint(cr);
    
    DiagrammeData donnees = {0};
    
    FILE *f = fopen("entraineurs.txt", "r");
    if (f != NULL) {
        char ligne[512];
        while (fgets(ligne, sizeof(ligne), f)) {
            ligne[strcspn(ligne, "\n")] = 0;
            if (strlen(ligne) == 0) continue;
            
            int id, experience, age;
            char nom[100], specialite[100], sexe;
            
            if (sscanf(ligne, "%d %99s %99s %d %d %c", 
                      &id, nom, specialite, &experience, &age, &sexe) == 6) {
                
                donnees.total_entraineurs++;
                
                if (strstr(specialite, "Musculation") != NULL) donnees.musculation++;
                else if (strstr(specialite, "Gymnastique") != NULL) donnees.gymnastique++;
                else if (strstr(specialite, "Yoga") != NULL) donnees.yoga++;
                else donnees.musculation++;
            }
        }
        fclose(f);
    }
    
    if (donnees.total_entraineurs == 0) {
        cairo_set_source_rgb(cr, 0.5, 0.5, 0.5);
        cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
        cairo_set_font_size(cr, 10);
        cairo_move_to(cr, 10, height/2);
        cairo_show_text(cr, "Aucun entraîneur");
        cairo_destroy(cr);
        return FALSE;
    }
    
    float pct_musc = (donnees.musculation * 100.0) / donnees.total_entraineurs;
    float pct_gym = (donnees.gymnastique * 100.0) / donnees.total_entraineurs;
    float pct_yoga = (donnees.yoga * 100.0) / donnees.total_entraineurs;
    
    int legende_width = 100;
    int diagram_width = width - legende_width - 20;
    int centre_x = legende_width + 10 + diagram_width / 2;
    int centre_y = height / 2;
    int rayon = MIN(diagram_width, height) / 3;
    
    double angle_musc = (pct_musc * 2 * M_PI) / 100.0;
    double angle_gym = (pct_gym * 2 * M_PI) / 100.0;
    double angle_yoga = (pct_yoga * 2 * M_PI) / 100.0;
    
    double angle_actuel = 0;
    

    if (donnees.musculation > 0) {
        cairo_set_source_rgb(cr, 0.8, 0.2, 0.2);
        cairo_move_to(cr, centre_x, centre_y);
        cairo_arc(cr, centre_x, centre_y, rayon, angle_actuel, angle_actuel + angle_musc);
        cairo_close_path(cr);
        cairo_fill(cr);
        angle_actuel += angle_musc;
    }
    

    if (donnees.gymnastique > 0) {
        cairo_set_source_rgb(cr, 0.2, 0.6, 0.2);
        cairo_move_to(cr, centre_x, centre_y);
        cairo_arc(cr, centre_x, centre_y, rayon, angle_actuel, angle_actuel + angle_gym);
        cairo_close_path(cr);
        cairo_fill(cr);
        angle_actuel += angle_gym;
    }
    

    if (donnees.yoga > 0) {
        cairo_set_source_rgb(cr, 0.4, 0.2, 0.6);
        cairo_move_to(cr, centre_x, centre_y);
        cairo_arc(cr, centre_x, centre_y, rayon, angle_actuel, angle_actuel + angle_yoga);
        cairo_close_path(cr);
        cairo_fill(cr);
    }
    

    int legende_x = 10;
    int legende_y = 20;
    int espace_ligne = 25;
    
    cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_font_size(cr, 11);
    cairo_set_source_rgb(cr, 0, 0, 0);
    cairo_move_to(cr, legende_x, legende_y);
    cairo_show_text(cr, "Entraîneurs par spécialité:");
    
    legende_y += 20;
    

    cairo_set_source_rgb(cr, 0.8, 0.2, 0.2);
    cairo_rectangle(cr, legende_x, legende_y, 15, 15);
    cairo_fill(cr);
    cairo_set_source_rgb(cr, 0, 0, 0);
    cairo_set_line_width(cr, 0.5);
    cairo_rectangle(cr, legende_x, legende_y, 15, 15);
    cairo_stroke(cr);
    
    cairo_set_source_rgb(cr, 0, 0, 0);
    cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
    cairo_set_font_size(cr, 9);
    
    char texte_musc[100];
    sprintf(texte_musc, "Musculation: %d (%.1f%%)", donnees.musculation, pct_musc);
    cairo_move_to(cr, legende_x + 20, legende_y + 11);
    cairo_show_text(cr, texte_musc);
    
    legende_y += espace_ligne;
    

    cairo_set_source_rgb(cr, 0.2, 0.6, 0.2);
    cairo_rectangle(cr, legende_x, legende_y, 15, 15);
    cairo_fill(cr);
    cairo_set_source_rgb(cr, 0, 0, 0);
    cairo_rectangle(cr, legende_x, legende_y, 15, 15);
    cairo_stroke(cr);
    
    char texte_gym[100];
    sprintf(texte_gym, "Gymnastique: %d (%.1f%%)", donnees.gymnastique, pct_gym);
    cairo_move_to(cr, legende_x + 20, legende_y + 11);
    cairo_show_text(cr, texte_gym);
    
    legende_y += espace_ligne;
    

    cairo_set_source_rgb(cr, 0.4, 0.2, 0.6);
    cairo_rectangle(cr, legende_x, legende_y, 15, 15);
    cairo_fill(cr);
    cairo_set_source_rgb(cr, 0, 0, 0);
    cairo_rectangle(cr, legende_x, legende_y, 15, 15);
    cairo_stroke(cr);
    
    char texte_yoga[100];
    sprintf(texte_yoga, "Yoga: %d (%.1f%%)", donnees.yoga, pct_yoga);
    cairo_move_to(cr, legende_x + 20, legende_y + 11);
    cairo_show_text(cr, texte_yoga);
    
    legende_y += espace_ligne + 10;
    

    cairo_set_source_rgb(cr, 0, 0, 0);
    cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_font_size(cr, 10);
    
    char texte_total[100];
    sprintf(texte_total, "Total: %d entraîneurs", donnees.total_entraineurs);
    cairo_move_to(cr, legende_x, legende_y);
    cairo_show_text(cr, texte_total);
    
    cairo_destroy(cr);
    return FALSE;
}

gboolean on_drawingarea_diagramme_expose_event(GtkWidget *widget, GdkEventExpose *event, gpointer user_data)
{
    cairo_t *cr = gdk_cairo_create(widget->window);
    

    GtkAllocation allocation;
    gtk_widget_get_allocation(widget, &allocation);
    int width = allocation.width;
    int height = allocation.height;
    

    cairo_set_source_rgb(cr, 1, 1, 1);
    cairo_paint(cr);
    

    DiagrammeData donnees = lire_toutes_donnees();
    

    if (donnees.total_membres == 0) {
        cairo_set_source_rgb(cr, 0.5, 0.5, 0.5);
        cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
        cairo_set_font_size(cr, 10);
        cairo_move_to(cr, 10, height/2);
        cairo_show_text(cr, "Aucune donnée disponible");
        cairo_stroke(cr);
        cairo_destroy(cr);
        return FALSE;
    }
    

    float pourcentage_jeunes = (donnees.jeunes * 100.0) / donnees.total_membres;
    float pourcentage_adultes = (donnees.adultes * 100.0) / donnees.total_membres;
    float pourcentage_seniors = (donnees.seniors * 100.0) / donnees.total_membres;
    

    int legende_width = 100;
    int diagram_width = width - legende_width - 20;
    

    int centre_x = legende_width + 10 + diagram_width / 2;
    int centre_y = height / 2;
    

    int rayon = MIN(diagram_width, height) / 3;
    

    double angle_jeunes = (pourcentage_jeunes * 2 * M_PI) / 100.0;
    double angle_adultes = (pourcentage_adultes * 2 * M_PI) / 100.0;
    double angle_seniors = (pourcentage_seniors * 2 * M_PI) / 100.0;
    
    double angle_actuel = 0;
    

    

    if (donnees.jeunes > 0) {
        cairo_set_source_rgb(cr, 0.3, 0.5, 0.9);  
        
        cairo_move_to(cr, centre_x, centre_y);
        cairo_arc(cr, centre_x, centre_y, rayon, angle_actuel, angle_actuel + angle_jeunes);
        cairo_close_path(cr);
        cairo_fill(cr);
        
        angle_actuel += angle_jeunes;
    }
    

    if (donnees.adultes > 0) {
        cairo_set_source_rgb(cr, 0.5, 0.9, 0.3);  
        
        cairo_move_to(cr, centre_x, centre_y);
        cairo_arc(cr, centre_x, centre_y, rayon, angle_actuel, angle_actuel + angle_adultes);
        cairo_close_path(cr);
        cairo_fill(cr);
        
        angle_actuel += angle_adultes;
    }
    

    if (donnees.seniors > 0) {
        cairo_set_source_rgb(cr, 0.9, 0.3, 0.3); 
        cairo_move_to(cr, centre_x, centre_y);
        cairo_arc(cr, centre_x, centre_y, rayon, angle_actuel, angle_actuel + angle_seniors);
        cairo_close_path(cr);
        cairo_fill(cr);
    }
    

    angle_actuel = 0;
    cairo_set_source_rgb(cr, 0, 0, 0);
    cairo_set_line_width(cr, 1.0);
    
    if (donnees.jeunes > 0) {
        cairo_move_to(cr, centre_x, centre_y);
        cairo_line_to(cr, centre_x + rayon * cos(angle_actuel), centre_y + rayon * sin(angle_actuel));
        cairo_stroke(cr);
        angle_actuel += angle_jeunes;
    }
    
    if (donnees.adultes > 0) {
        cairo_move_to(cr, centre_x, centre_y);
        cairo_line_to(cr, centre_x + rayon * cos(angle_actuel), centre_y + rayon * sin(angle_actuel));
        cairo_stroke(cr);
        angle_actuel += angle_adultes;
    }
    
    if (donnees.seniors > 0) {
        cairo_move_to(cr, centre_x, centre_y);
        cairo_line_to(cr, centre_x + rayon * cos(angle_actuel), centre_y + rayon * sin(angle_actuel));
        cairo_stroke(cr);
    }
    

    int legende_x = 10;
    int legende_y = 20;
    int espace_ligne = 25;
    
    cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_font_size(cr, 11);
    cairo_set_source_rgb(cr, 0, 0, 0);
    

    cairo_move_to(cr, legende_x, legende_y);
    cairo_show_text(cr, "Répartition membres:");
    
    legende_y += 20;
    

    cairo_set_source_rgb(cr, 0.3, 0.5, 0.9);
    cairo_rectangle(cr, legende_x, legende_y, 15, 15);
    cairo_fill(cr);
    

    cairo_set_source_rgb(cr, 0, 0, 0);
    cairo_set_line_width(cr, 0.5);
    cairo_rectangle(cr, legende_x, legende_y, 15, 15);
    cairo_stroke(cr);

    cairo_set_source_rgb(cr, 0, 0, 0);
    cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
    cairo_set_font_size(cr, 9);
    
    char texte_jeunes[100];
    sprintf(texte_jeunes, "Jeunes: %d (%.1f%%)", donnees.jeunes, pourcentage_jeunes);
    cairo_move_to(cr, legende_x + 20, legende_y + 11);
    cairo_show_text(cr, texte_jeunes);
    
    legende_y += espace_ligne;
    

    cairo_set_source_rgb(cr, 0.5, 0.9, 0.3);
    cairo_rectangle(cr, legende_x, legende_y, 15, 15);
    cairo_fill(cr);
    
    cairo_set_source_rgb(cr, 0, 0, 0);
    cairo_rectangle(cr, legende_x, legende_y, 15, 15);
    cairo_stroke(cr);
    
    char texte_adultes[100];
    sprintf(texte_adultes, "Adultes: %d (%.1f%%)", donnees.adultes, pourcentage_adultes);
    cairo_move_to(cr, legende_x + 20, legende_y + 11);
    cairo_show_text(cr, texte_adultes);
    
    legende_y += espace_ligne;
    

    cairo_set_source_rgb(cr, 0.9, 0.3, 0.3);
    cairo_rectangle(cr, legende_x, legende_y, 15, 15);
    cairo_fill(cr);
    
    cairo_set_source_rgb(cr, 0, 0, 0);
    cairo_rectangle(cr, legende_x, legende_y, 15, 15);
    cairo_stroke(cr);
    
    char texte_seniors[100];
    sprintf(texte_seniors, "Seniors: %d (%.1f%%)", donnees.seniors, pourcentage_seniors);
    cairo_move_to(cr, legende_x + 20, legende_y + 11);
    cairo_show_text(cr, texte_seniors);
    
    legende_y += espace_ligne + 10;
    

    cairo_set_source_rgb(cr, 0, 0, 0);
    cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_font_size(cr, 10);
    
    char texte_total[100];
    sprintf(texte_total, "Total: %d membres", donnees.total_membres);
    cairo_move_to(cr, legende_x, legende_y);
    cairo_show_text(cr, texte_total);
    
    cairo_destroy(cr);
    return FALSE;
}

gboolean
on_drawingarea_stat_centres_expose_event (GtkWidget *widget, GdkEventExpose *event, gpointer user_data)
{
    cairo_t *cr = gdk_cairo_create(widget->window);
    
    GtkAllocation allocation;
    gtk_widget_get_allocation(widget, &allocation);
    int width = allocation.width;
    int height = allocation.height;
    
    cairo_set_source_rgb(cr, 1, 1, 1);
    cairo_paint(cr);
    
    DiagrammeData donnees = {0};
    
    FILE *f = fopen("centres.txt", "r");
    if (f != NULL) {
        char ligne[512];
        while (fgets(ligne, sizeof(ligne), f)) {
            ligne[strcspn(ligne, "\n")] = 0;
            if (strlen(ligne) == 0) continue;
            
            char *tokens[10];
            int token_count = 0;
            char *token = strtok(ligne, ";");
            
            while (token != NULL && token_count < 10) {
                tokens[token_count++] = token;
                token = strtok(NULL, ";");
            }
            
            if (token_count >= 5) {
                donnees.total_centres++;
                char *ville = tokens[3];
                
                if (strstr(ville, "Tunis") != NULL) donnees.tunis++;
                else if (strstr(ville, "Ariana") != NULL) donnees.ariana++;
                else if (strstr(ville, "Aouina") != NULL) donnees.aouina++;
            }
        }
        fclose(f);
    }
    
    if (donnees.total_centres == 0) {
        cairo_set_source_rgb(cr, 0.5, 0.5, 0.5);
        cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
        cairo_set_font_size(cr, 10);
        cairo_move_to(cr, 10, height/2);
        cairo_show_text(cr, "Aucun centre");
        cairo_destroy(cr);
        return FALSE;
    }
    
    float pct_tunis = (donnees.tunis * 100.0) / donnees.total_centres;
    float pct_ariana = (donnees.ariana * 100.0) / donnees.total_centres;
    float pct_aouina = (donnees.aouina * 100.0) / donnees.total_centres;
    
    int legende_width = 100;
    int diagram_width = width - legende_width - 20;
    int centre_x = legende_width + 10 + diagram_width / 2;
    int centre_y = height / 2;
    int rayon = MIN(diagram_width, height) / 3;
    
    double angle_tunis = (pct_tunis * 2 * M_PI) / 100.0;
    double angle_ariana = (pct_ariana * 2 * M_PI) / 100.0;
    double angle_aouina = (pct_aouina * 2 * M_PI) / 100.0;
    
    double angle_actuel = 0;
    

    if (donnees.tunis > 0) {
        cairo_set_source_rgb(cr, 0.9, 0.8, 0.1);
        cairo_move_to(cr, centre_x, centre_y);
        cairo_arc(cr, centre_x, centre_y, rayon, angle_actuel, angle_actuel + angle_tunis);
        cairo_close_path(cr);
        cairo_fill(cr);
        angle_actuel += angle_tunis;
    }
    

    if (donnees.ariana > 0) {
        cairo_set_source_rgb(cr, 0.1, 0.8, 0.9);
        cairo_move_to(cr, centre_x, centre_y);
        cairo_arc(cr, centre_x, centre_y, rayon, angle_actuel, angle_actuel + angle_ariana);
        cairo_close_path(cr);
        cairo_fill(cr);
        angle_actuel += angle_ariana;
    }
    

    if (donnees.aouina > 0) {
        cairo_set_source_rgb(cr, 0.8, 0.1, 0.9);
        cairo_move_to(cr, centre_x, centre_y);
        cairo_arc(cr, centre_x, centre_y, rayon, angle_actuel, angle_actuel + angle_aouina);
        cairo_close_path(cr);
        cairo_fill(cr);
    }
    

    int legende_x = 10;
    int legende_y = 20;
    int espace_ligne = 25;
    
    cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_font_size(cr, 11);
    cairo_set_source_rgb(cr, 0, 0, 0);
    cairo_move_to(cr, legende_x, legende_y);
    cairo_show_text(cr, "Centres par ville:");
    
    legende_y += 20;
    

    cairo_set_source_rgb(cr, 0.9, 0.8, 0.1);
    cairo_rectangle(cr, legende_x, legende_y, 15, 15);
    cairo_fill(cr);
    cairo_set_source_rgb(cr, 0, 0, 0);
    cairo_set_line_width(cr, 0.5);
    cairo_rectangle(cr, legende_x, legende_y, 15, 15);
    cairo_stroke(cr);
    
    cairo_set_source_rgb(cr, 0, 0, 0);
    cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
    cairo_set_font_size(cr, 9);
    
    char texte_tunis[100];
    sprintf(texte_tunis, "Tunis: %d (%.1f%%)", donnees.tunis, pct_tunis);
    cairo_move_to(cr, legende_x + 20, legende_y + 11);
    cairo_show_text(cr, texte_tunis);
    
    legende_y += espace_ligne;
    

    cairo_set_source_rgb(cr, 0.1, 0.8, 0.9);
    cairo_rectangle(cr, legende_x, legende_y, 15, 15);
    cairo_fill(cr);
    cairo_set_source_rgb(cr, 0, 0, 0);
    cairo_rectangle(cr, legende_x, legende_y, 15, 15);
    cairo_stroke(cr);
    
    char texte_ariana[100];
    sprintf(texte_ariana, "Ariana: %d (%.1f%%)", donnees.ariana, pct_ariana);
    cairo_move_to(cr, legende_x + 20, legende_y + 11);
    cairo_show_text(cr, texte_ariana);
    
    legende_y += espace_ligne;
    

    cairo_set_source_rgb(cr, 0.8, 0.1, 0.9);
    cairo_rectangle(cr, legende_x, legende_y, 15, 15);
    cairo_fill(cr);
    cairo_set_source_rgb(cr, 0, 0, 0);
    cairo_rectangle(cr, legende_x, legende_y, 15, 15);
    cairo_stroke(cr);
    
    char texte_aouina[100];
    sprintf(texte_aouina, "Aouina: %d (%.1f%%)", donnees.aouina, pct_aouina);
    cairo_move_to(cr, legende_x + 20, legende_y + 11);
    cairo_show_text(cr, texte_aouina);
    
    legende_y += espace_ligne + 10;
    

    cairo_set_source_rgb(cr, 0, 0, 0);
    cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_font_size(cr, 10);
    
    char texte_total[100];
    sprintf(texte_total, "Total: %d centres", donnees.total_centres);
    cairo_move_to(cr, legende_x, legende_y);
    cairo_show_text(cr, texte_total);
    
    cairo_destroy(cr);
    return FALSE;
}
// Fonction rafraîchit 
static gboolean rafraichir_tout(gpointer data) {

    GList *windows = gtk_window_list_toplevels();
    GList *iter;
    
    for (iter = windows; iter != NULL; iter = iter->next) {
        GtkWidget *window = GTK_WIDGET(iter->data);
        
        if (!GTK_IS_WINDOW(window)) continue;
        

        const char* diagrams[] = {
            "drawingarea_diagramme",
            "drawingarea_stat_cours", 
            "drawingarea_stat_equipements",
            "drawingarea_stat_entraineurs",
            "drawingarea_stat_centres"
        };
        
        for (int i = 0; i < 5; i++) {
            GtkWidget *d = lookup_widget(window, diagrams[i]);
            if (d && GTK_IS_DRAWING_AREA(d)) {
                gtk_widget_queue_draw(d);
            }
        }
        

        GtkWidget *tv;
        
        tv = lookup_widget(window, "treeview_membres");
        if (tv && GTK_IS_TREE_VIEW(tv)) charger_membres_dans_treeview(tv);
        
        tv = lookup_widget(window, "treeview_cours");
        if (tv && GTK_IS_TREE_VIEW(tv)) charger_cours_dans_treeview(tv);
        
        tv = lookup_widget(window, "treeview_entraineurs");
        if (tv && GTK_IS_TREE_VIEW(tv)) charger_entraineurs_dans_treeview(tv);
        
        tv = lookup_widget(window, "treeview_equipements");
        if (tv && GTK_IS_TREE_VIEW(tv)) charger_equipements_dans_treeview(tv);
        
        tv = lookup_widget(window, "treeview_centres");
        if (tv && GTK_IS_TREE_VIEW(tv)) charger_centres_dans_treeview(tv);
        
        tv = lookup_widget(window, "treeviewcoach");
        if (tv && GTK_IS_TREE_VIEW(tv)) charger_demandes_entraineur_dans_treeviewcoach(tv);
    }
    
    g_list_free(windows);
    return TRUE;
}


void start_refresh() {
    g_timeout_add(1000, rafraichir_tout, NULL);
}

void on_button_website_clicked(GtkButton *button, gpointer user_data) {
    // Obtenir le chemin du répertoire courant
    char chemin_courant[1024];
    
    if (getcwd(chemin_courant, sizeof(chemin_courant)) == NULL) {
        // Si on ne peut pas obtenir le chemin courant, essayer avec le chemin relatif
        int result = system("xdg-open midou.html");
        (void)result;
        return;
    }
    
    // Construire le chemin complet vers midou.html
    char chemin_html[2048];
    snprintf(chemin_html, sizeof(chemin_html), "%s/midou.html", chemin_courant);
    
    // Vérifier si le fichier existe
    if (access(chemin_html, F_OK) == -1) {
        // Le fichier n'existe pas dans le répertoire courant
        // Essayer avec xdg-open directement (au cas où le fichier serait dans un autre emplacement connu)
        int result = system("xdg-open midou.html");
        (void)result;
        return;
    }
    
    // Ouvrir le fichier avec le navigateur par défaut
    char commande[4096];
    snprintf(commande, sizeof(commande), "xdg-open \"file://%s\"", chemin_html);
    
    int result = system(commande);
    (void)result;
}
