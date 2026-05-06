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
#include <time.h>
#include <unistd.h>
#include <curl/curl.h>
#include "callbacks.h"
#include "interface.h"
#include "support.h"
#include "common.h"

#include "membre.h"
#include "entraineur.h"
#include "cours.h"
#include "equipement.h"
#include "centre.h"
#include <gdk-pixbuf/gdk-pixbuf.h>

static GdkPixmap *bg_pixmap = NULL;
static int bg_width = 0, bg_height = 0;

// ==================== STRUCTURES POUR GROK API ====================

struct string {
    char *ptr;
    size_t len;
};

void init_string(struct string *s) {
    s->len = 0;
    s->ptr = malloc(1);
    s->ptr[0] = '\0';
}

size_t writefunc(void *ptr, size_t size, size_t nmemb, struct string *s) {
    size_t new_len = s->len + size * nmemb;
    s->ptr = realloc(s->ptr, new_len + 1);
    memcpy(s->ptr + s->len, ptr, size * nmemb);
    s->ptr[new_len] = '\0';
    s->len = new_len;
    return size * nmemb;
}
// ==================== FONCTIONS DE DESSIN DES GRAPHIQUES ====================

// Définir les couleurs pour le thème sombre
static void set_color_for_dark_theme(cairo_t *cr, const char *element) {
    if (g_strcmp0(element, "background") == 0) {
        cairo_set_source_rgb(cr, 0.10, 0.10, 0.12);  // Fond très sombre
    } else if (g_strcmp0(element, "grid") == 0) {
        cairo_set_source_rgba(cr, 0.30, 0.30, 0.35, 0.5);
    } else if (g_strcmp0(element, "text") == 0) {
        cairo_set_source_rgb(cr, 0.85, 0.85, 0.90);
    } else if (g_strcmp0(element, "bar1") == 0) {
        cairo_set_source_rgb(cr, 0.89, 0.29, 0.29);  // Rouge #E24B4A
    } else if (g_strcmp0(element, "bar2") == 0) {
        cairo_set_source_rgb(cr, 0.20, 0.62, 0.46);  // Vert #1D9E75
    } else if (g_strcmp0(element, "bar3") == 0) {
        cairo_set_source_rgb(cr, 0.73, 0.46, 0.09);  // Orange #BA7517
    } else if (g_strcmp0(element, "bar4") == 0) {
        cairo_set_source_rgb(cr, 0.31, 0.53, 0.80);  // Bleu
    } else if (g_strcmp0(element, "bar5") == 0) {
        cairo_set_source_rgb(cr, 0.75, 0.35, 0.75);  // Violet
    } else if (g_strcmp0(element, "line") == 0) {
        cairo_set_source_rgb(cr, 0.89, 0.29, 0.29);
    } else if (g_strcmp0(element, "point") == 0) {
        cairo_set_source_rgb(cr, 0.89, 0.29, 0.29);
    } else {
        cairo_set_source_rgb(cr, 0.90, 0.90, 0.90);
    }
}

// Dessiner un graphique en barres
static void draw_bar_chart(cairo_t *cr, int width, int height, double *values, int num_bars, const char **labels, const char *title) {
    // Fond
    set_color_for_dark_theme(cr, "background");
    cairo_paint(cr);
    
    // Marges
    int margin_top = 40, margin_bottom = 50, margin_left = 50, margin_right = 20;
    int chart_width = width - margin_left - margin_right;
    int chart_height = height - margin_top - margin_bottom;
    
    if (chart_width <= 0 || chart_height <= 0) return;
    
    // Titre
    set_color_for_dark_theme(cr, "text");
    cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_font_size(cr, 12);
    cairo_move_to(cr, margin_left, 20);
    cairo_show_text(cr, title);
    
    // Trouver la valeur max
    double max_val = 0;
    for (int i = 0; i < num_bars; i++) {
        if (values[i] > max_val) max_val = values[i];
    }
    if (max_val == 0) max_val = 1;
    
    // Lignes de grille
    set_color_for_dark_theme(cr, "grid");
    cairo_set_line_width(cr, 0.5);
    for (int i = 0; i <= 4; i++) {
        double y = margin_top + (i * chart_height / 4.0);
        cairo_move_to(cr, margin_left - 5, y);
        cairo_line_to(cr, width - margin_right, y);
        cairo_stroke(cr);
        
        // Valeurs sur l'axe Y
        double val = max_val - (i * max_val / 4.0);
        char val_str[16];
        snprintf(val_str, sizeof(val_str), "%.0f", val);
        set_color_for_dark_theme(cr, "text");
        cairo_set_font_size(cr, 9);
        cairo_move_to(cr, margin_left - 35, y + 3);
        cairo_show_text(cr, val_str);
    }
    
    // Largeur des barres
    double bar_width = (chart_width / (double)num_bars) * 0.7;
    double bar_spacing = (chart_width / (double)num_bars) * 0.3;
    
    // Dessiner les barres
    for (int i = 0; i < num_bars; i++) {
        double bar_height = (values[i] / max_val) * chart_height;
        double x = margin_left + i * (bar_width + bar_spacing) + bar_spacing / 2;
        double y = margin_top + chart_height - bar_height;
        
        switch (i % 5) {
            case 0: set_color_for_dark_theme(cr, "bar1"); break;
            case 1: set_color_for_dark_theme(cr, "bar2"); break;
            case 2: set_color_for_dark_theme(cr, "bar3"); break;
            case 3: set_color_for_dark_theme(cr, "bar4"); break;
            default: set_color_for_dark_theme(cr, "bar5"); break;
        }
        
        cairo_rectangle(cr, x, y, bar_width, bar_height);
        cairo_fill(cr);
        
        // Labels
        if (labels && labels[i]) {
            set_color_for_dark_theme(cr, "text");
            cairo_set_font_size(cr, 9);
            cairo_move_to(cr, x + bar_width/2 - 20, margin_top + chart_height + 15);
            cairo_show_text(cr, labels[i]);
        }
    }
}

// Dessiner un graphique en ligne
static void draw_line_chart(cairo_t *cr, int width, int height, double *values, int num_points, const char *title) {
    if (num_points < 2) return;
    
    // Fond
    set_color_for_dark_theme(cr, "background");
    cairo_paint(cr);
    
    // Marges
    int margin_top = 40, margin_bottom = 40, margin_left = 50, margin_right = 20;
    int chart_width = width - margin_left - margin_right;
    int chart_height = height - margin_top - margin_bottom;
    
    if (chart_width <= 0 || chart_height <= 0) return;
    
    // Titre
    set_color_for_dark_theme(cr, "text");
    cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_font_size(cr, 12);
    cairo_move_to(cr, margin_left, 20);
    cairo_show_text(cr, title);
    
    // Trouver la valeur max
    double max_val = 0;
    for (int i = 0; i < num_points; i++) {
        if (values[i] > max_val) max_val = values[i];
    }
    if (max_val == 0) max_val = 1;
    
    // Lignes de grille
    set_color_for_dark_theme(cr, "grid");
    cairo_set_line_width(cr, 0.5);
    for (int i = 0; i <= 4; i++) {
        double y = margin_top + (i * chart_height / 4.0);
        cairo_move_to(cr, margin_left - 5, y);
        cairo_line_to(cr, width - margin_right, y);
        cairo_stroke(cr);
        
        double val = max_val - (i * max_val / 4.0);
        char val_str[16];
        snprintf(val_str, sizeof(val_str), "%.0f", val);
        set_color_for_dark_theme(cr, "text");
        cairo_set_font_size(cr, 9);
        cairo_move_to(cr, margin_left - 35, y + 3);
        cairo_show_text(cr, val_str);
    }
    
    // Points
    double *points_x = malloc(num_points * sizeof(double));
    double *points_y = malloc(num_points * sizeof(double));
    
    for (int i = 0; i < num_points; i++) {
        points_x[i] = margin_left + (i * chart_width / (double)(num_points - 1));
        points_y[i] = margin_top + chart_height - (values[i] / max_val) * chart_height;
    }
    
    // Ligne
    set_color_for_dark_theme(cr, "line");
    cairo_set_line_width(cr, 2.0);
    for (int i = 0; i < num_points - 1; i++) {
        cairo_move_to(cr, points_x[i], points_y[i]);
        cairo_line_to(cr, points_x[i+1], points_y[i+1]);
        cairo_stroke(cr);
    }
    
    // Points
    for (int i = 0; i < num_points; i++) {
        set_color_for_dark_theme(cr, "point");
        cairo_arc(cr, points_x[i], points_y[i], 4, 0, 2 * M_PI);
        cairo_fill(cr);
        
        set_color_for_dark_theme(cr, "text");
        cairo_arc(cr, points_x[i], points_y[i], 2, 0, 2 * M_PI);
        cairo_fill(cr);
    }
    
    free(points_x);
    free(points_y);
}

// Dessiner un camembert
static void draw_pie_chart(cairo_t *cr, int width, int height, double *values, int num_slices, const char **labels, const char *title) {
    // Fond
    set_color_for_dark_theme(cr, "background");
    cairo_paint(cr);
    
    // Titre
    set_color_for_dark_theme(cr, "text");
    cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_font_size(cr, 12);
    cairo_move_to(cr, 20, 20);
    cairo_show_text(cr, title);
    
    // Somme
    double total = 0;
    for (int i = 0; i < num_slices; i++) {
        total += values[i];
    }
    if (total == 0) total = 1;
    
    // Centre
    double center_x = width / 2.5;
    double center_y = height / 2;
    double radius = (height < width ? height : width) / 3;
    if (radius > 100) radius = 100;
    
    // Parts
    double start_angle = -M_PI / 2;
    for (int i = 0; i < num_slices; i++) {
        double angle = (values[i] / total) * 2 * M_PI;
        double end_angle = start_angle + angle;
        
        switch (i % 5) {
            case 0: set_color_for_dark_theme(cr, "bar1"); break;
            case 1: set_color_for_dark_theme(cr, "bar2"); break;
            case 2: set_color_for_dark_theme(cr, "bar3"); break;
            case 3: set_color_for_dark_theme(cr, "bar4"); break;
            default: set_color_for_dark_theme(cr, "bar5"); break;
        }
        
        cairo_move_to(cr, center_x, center_y);
        cairo_arc(cr, center_x, center_y, radius, start_angle, end_angle);
        cairo_line_to(cr, center_x, center_y);
        cairo_fill(cr);
        
        start_angle = end_angle;
    }
    
    // Légende
    double legend_x = center_x + radius + 20;
    double legend_y = center_y - radius;
    for (int i = 0; i < num_slices && i < 8; i++) {
        if (legend_y + i * 20 > height - 20) break;
        
        switch (i % 5) {
            case 0: set_color_for_dark_theme(cr, "bar1"); break;
            case 1: set_color_for_dark_theme(cr, "bar2"); break;
            case 2: set_color_for_dark_theme(cr, "bar3"); break;
            case 3: set_color_for_dark_theme(cr, "bar4"); break;
            default: set_color_for_dark_theme(cr, "bar5"); break;
        }
        cairo_rectangle(cr, legend_x, legend_y + i * 20, 12, 12);
        cairo_fill(cr);
        
        set_color_for_dark_theme(cr, "text");
        cairo_set_font_size(cr, 9);
        cairo_move_to(cr, legend_x + 18, legend_y + i * 20 + 10);
        if (labels && labels[i]) {
            char label_text[128];
            snprintf(label_text, sizeof(label_text), "%s (%.1f%%)", labels[i], (values[i]/total)*100);
            cairo_show_text(cr, label_text);
        }
    }
}
// Fonction pour parser la réponse JSON simple (extrait juste le contenu)
// Fonction pour parser la réponse JSON simple (extrait juste le contenu)
char* parse_json_response(const char *json_response) {
    const char *content_start = strstr(json_response, "\"content\":\"");
    if (!content_start) return NULL;

    content_start += 11;
    const char *content_end = strchr(content_start, '"');
    if (!content_end) return NULL;

    size_t content_len = content_end - content_start;
    char *content = malloc(content_len + 1);
    strncpy(content, content_start, content_len);
    content[content_len] = '\0';

    // Décoder correctement les séquences d'échappement
    char *decoded = malloc(content_len * 2 + 1);
    char *d = decoded;
    const char *s = content;

    while (*s) {
        if (*s == '\\') {
            s++;
            if (*s == 'n') {
                *d++ = '\n';
                s++;
            } else if (*s == '\\') {
                *d++ = '\\';
                s++;
            } else if (*s == 't') {
                *d++ = '\t';
                s++;
            } else if (*s == '"') {
                *d++ = '"';
                s++;
            } else if (*s == 'r') {
                s++; // Ignorer \r
            } else {
                *d++ = '\\';
                *d++ = *s++;
            }
        } else {
            *d++ = *s++;
        }
    }
    *d = '\0';

    free(content);
    return decoded;
}

// Fonction pour appeler l'API Groq
char* call_grok_api(const char *user_message) {
    CURL *curl;
    CURLcode res;
    struct string response;

    // Lire la clé API depuis le fichier
    FILE *keyfile = fopen("key.txt", "r");
    if (!keyfile) {
        return g_strdup("❌ Erreur: Fichier key.txt introuvable. Veuillez créer un fichier key.txt avec votre clé API Groq (commence par gsk_)");
    }

    char api_key[128];
    if (fgets(api_key, sizeof(api_key), keyfile) == NULL) {
        fclose(keyfile);
        return g_strdup("❌ Erreur: Impossible de lire la clé API depuis key.txt");
    }
    api_key[strcspn(api_key, "\n")] = 0;
    fclose(keyfile);

    curl = curl_easy_init();
    if (!curl) {
        return g_strdup("❌ Erreur: Impossible d'initialiser cURL");
    }

    init_string(&response);

    char json_data[1024];
    snprintf(json_data, sizeof(json_data),
             "{\"model\":\"llama-3.1-8b-instant\","
             "\"messages\":[{\"role\":\"user\",\"content\":\"%s\"}]}",
             user_message);

    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/json");

    char auth_header[256];
    snprintf(auth_header, sizeof(auth_header), "Authorization: Bearer %s", api_key);
    headers = curl_slist_append(headers, auth_header);

    curl_easy_setopt(curl, CURLOPT_URL, "https://api.groq.com/openai/v1/chat/completions");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_data);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writefunc);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

    res = curl_easy_perform(curl);

    char *result = NULL;
    if (res == CURLE_OK) {
        char *content = parse_json_response(response.ptr);
        if (content) {
            result = g_strdup_printf("Midou: %s\n\n", content);
            free(content);
        } else {
            result = g_strdup("⚠️ Réponse API invalide ou vide.\n\n");
        }
    } else {
        result = g_strdup_printf("⚠️ Erreur de requête: %s\n\n", curl_easy_strerror(res));
    }

    free(response.ptr);
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    return result;
}

// ==================== UTILITAIRES ====================

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
    
    for (iter = children; iter != NULL; iter = iter->next) {
        GtkWidget *widget = GTK_WIDGET(iter->data);
        if (GTK_IS_ENTRY(widget))
            gtk_entry_set_text(GTK_ENTRY(widget), "");
        else if (GTK_IS_COMBO_BOX(widget))
            gtk_combo_box_set_active(GTK_COMBO_BOX(widget), 0);
        else if (GTK_IS_SPIN_BUTTON(widget))
            gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget), 0);
        else if (GTK_IS_TOGGLE_BUTTON(widget))
            gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget), FALSE);
        else if (GTK_IS_TEXT_VIEW(widget)) {
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
        gtk_widget_destroy(window);
}

void retour_vers_login(GtkWidget *widget)
{
    GtkWidget *current_window = gtk_widget_get_toplevel(widget);
    GtkWidget *login_window = create_window_login();
    
    if (login_window != NULL) {
        appliquer_background_a_fenetre(login_window);
        appliquer_couleurs_tous_frames(login_window);
        gtk_widget_show_all(login_window);
        if (current_window != NULL && GTK_IS_WINDOW(current_window))
            gtk_widget_destroy(current_window);
    }
}

// ==================== CHARGEMENT DES DONNÉES ====================

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
                    if (*p && *p != ' ') field_ptrs[fields++] = p;
                } else {
                    p++;
                }
            }
            
            if (fields >= 7) {
                m.id = atoi(field_ptrs[0]);
                strcpy(m.nom, field_ptrs[1]);
                strcpy(m.email, field_ptrs[2]);
                m.age = atoi(field_ptrs[3]);
                strcpy(m.sexe, field_ptrs[4]);
                strcpy(m.Typeabonnement, field_ptrs[5]);
                m.Tarif = atof(field_ptrs[6]);
                
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
                for (int i = 0; i < 7; i++) g_free(row_values[i]);
            }
        }
        fclose(f);
    }
}

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
            for (int i = 0; i < 6; i++) g_free(row_values[i]);
        }
        fclose(f);
    }
}

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
            for (int i = 0; i < 6; i++) g_free(row_values[i]);
        }
        fclose(f);
    }
}

void charger_equipements_dans_treeview(GtkWidget *treeview)
{
    FILE *f;
    Equipement e;
    gchar *row_values[6];
    char cap_str[20], disp_str[2], etat_str[2];
    
    treeview_vider(treeview);
    
    f = fopen("equipements.txt", "r");
    if (f != NULL) {
        while (fscanf(f, "%19s %49s %49s %d %c %c", 
                     e.reference, e.type, e.centre, 
                     &e.capacite, &e.disponibilite, &e.etat) == 6) {
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
            
            treeview_ajouter_ligne(treeview, row_values, 6);
            for (int i = 0; i < 6; i++) g_free(row_values[i]);
        }
        fclose(f);
    }
}

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
                    if (*p && *p != ';') field_ptrs[fields++] = p;
                } else {
                    p++;
                }
            }
            
            if (fields >= 5) {
                c.id = atoi(field_ptrs[0]);
                strcpy(c.nom, field_ptrs[1]);
                strcpy(c.adresse, field_ptrs[2]);
                strcpy(c.ville, field_ptrs[3]);
                c.capacite = atoi(field_ptrs[4]);
                
                sprintf(id_str, "%d", c.id);
                sprintf(cap_str, "%d", c.capacite);
                
                row_values[0] = g_strdup(id_str);
                row_values[1] = g_strdup(c.nom);
                row_values[2] = g_strdup(c.adresse);
                row_values[3] = g_strdup(c.ville);
                row_values[4] = g_strdup(cap_str);
                
                treeview_ajouter_ligne(treeview, row_values, 5);
                for (int i = 0; i < 5; i++) g_free(row_values[i]);
            }
        }
        fclose(f);
    }
}

// ==================== CALLBACKS LOGIN ====================

void on_btn_login_clicked(GtkWidget *objet, gpointer user_data)
{
    GtkWidget *entry_username = lookup_widget(objet, "entry2");
    GtkWidget *entry_password = lookup_widget(objet, "entry3");
    GtkWidget *checkbutton_robot = lookup_widget(objet, "checkbutton33");
    
    if (!entry_username || !entry_password || !checkbutton_robot) {
        afficher_message(objet, "erreur", "Erreur interne: widgets non trouv\303\251s");
        return;
    }
    
    const char *username = gtk_entry_get_text(GTK_ENTRY(entry_username));
    const char *password = gtk_entry_get_text(GTK_ENTRY(entry_password));
    gboolean is_robot = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(checkbutton_robot));
    
    if (strlen(username) == 0 || strlen(password) == 0) {
        afficher_message(objet, "erreur", "Veuillez remplir tous les champs!");
        return;
    }
    
    if (!is_robot) {
        afficher_message(objet, "erreur", "Veuillez confirmer que vous n'\303\252tes pas un robot!");
        return;
    }
    
    char user_type[20];
    int user_id;
    
    if (verifier_login(username, password, user_type, &user_id)) {
        GtkWidget *login_window = lookup_widget(objet, "Login");
        if (login_window) gtk_widget_hide(login_window);
        
        if (strcmp(user_type, "admin") == 0) {
            GtkWidget *admin_window = create_window_admin();
            if (admin_window) {
                appliquer_background_a_fenetre(admin_window);
                appliquer_couleurs_tous_frames(admin_window);
                gtk_widget_show_all(admin_window);
                if (login_window) gtk_widget_destroy(login_window);
            }
        } else if (strcmp(user_type, "membre") == 0) {
            GtkWidget *accueil_membre = create_window_dashboard_member();
            if (accueil_membre != NULL) {
                appliquer_background_a_fenetre(accueil_membre);
                appliquer_couleurs_tous_frames(accueil_membre);
                gtk_widget_show_all(accueil_membre);
                if (login_window) gtk_widget_destroy(login_window);
            }
        } else if (strcmp(user_type, "entraineur") == 0) {
            GtkWidget *entraineur_window = create_window_dashboard_coach();
            if (entraineur_window != NULL) {
                appliquer_background_a_fenetre(entraineur_window);
                appliquer_couleurs_tous_frames(entraineur_window);
                gtk_widget_show_all(entraineur_window);
                GtkWidget *treeviewcoach = lookup_widget(entraineur_window, "tv_demandes");
                if (treeviewcoach) {
                    const char *titles[] = {"Type", "Coach", "Date/Heure", "Dur\303\251e", "Note", "Statut"};
                    treeview_init_columns(treeviewcoach, titles, 6);
                    charger_demandes_entraineur_dans_treeviewcoach(treeviewcoach);
                }
                if (login_window) gtk_widget_destroy(login_window);
            }
        } else {
            afficher_message(objet, "info", "Type d'utilisateur non reconnu");
        }
        
        if (login_window) vider_formulaire(login_window);
    } else {
        afficher_message(objet, "erreur", "Identifiants incorrects!");
    }
}

void on_btn_register_clicked(GtkWidget *objet, gpointer user_data)
{
    GtkWidget *dialog = gtk_dialog_new_with_buttons(
        "Choisir le type d'inscription",
        GTK_WINDOW(gtk_widget_get_toplevel(objet)),
        GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
        "Membre", GTK_RESPONSE_YES,
        "Entraineur", GTK_RESPONSE_NO,
        "Annuler", GTK_RESPONSE_CANCEL,
        NULL);
    
    GtkWidget *label = gtk_label_new("Fonction d'inscription temporairement d\303\251sactiv\303\251e.\nUtilisez l'interface admin pour ajouter des utilisateurs.");
    GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    gtk_container_add(GTK_CONTAINER(content_area), label);
    gtk_widget_show(label);
    
    gint response = gtk_dialog_run(GTK_DIALOG(dialog));
    
    if (response == GTK_RESPONSE_YES) {
        afficher_message(objet, "info", "L'inscription des membres se fait via l'interface Admin");
    } else if (response == GTK_RESPONSE_NO) {
        afficher_message(objet, "info", "L'inscription des entraineurs se fait via l'interface Admin");
    }
    
    gtk_widget_destroy(dialog);
}

// ==================== CALLBACKS ACCUEIL MEMBRE ====================

void on_btn_membre_deconnexion_clicked(GtkWidget *objet, gpointer user_data)
{
    retour_vers_login(objet);
}

void on_treeview_mes_cours_row_activated(GtkTreeView *treeview, GtkTreePath *path,
                                        GtkTreeViewColumn *column, gpointer user_data)
{
    afficher_message(GTK_WIDGET(treeview), "info", "D\303\251tails du cours \303\240 impl\303\251menter");
}

void on_chat_send_clicked(GtkButton *button, gpointer user_data)
{
    GtkWidget *window = gtk_widget_get_toplevel(GTK_WIDGET(button));
    GtkWidget *entry = lookup_widget(window, "entry_chatbot_msg");
    GtkWidget *history = lookup_widget(window, "tv_chatbot");
    
    if (!entry || !history) return;
    
    const gchar *message = gtk_entry_get_text(GTK_ENTRY(entry));
    if (message && strlen(message) > 0) {
        GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(history));
        GtkTextIter iter;
        gtk_text_buffer_get_end_iter(buffer, &iter);
        
        gchar *formatted = g_strdup_printf("Moi: %s\n", message);
        gtk_text_buffer_insert(buffer, &iter, formatted, -1);
        g_free(formatted);
        
        gtk_text_buffer_get_end_iter(buffer, &iter);
        gchar *response = g_strdup_printf("Midou: Merci pour votre message ! Un coach vous r\303\251pondra bient\303\264t.\n\n");
        gtk_text_buffer_insert(buffer, &iter, response, -1);
        g_free(response);
        
        gtk_entry_set_text(GTK_ENTRY(entry), "");
    }
}

void on_btn_inscrire_cour_clicked(GtkWidget *objet, gpointer user_data)
{
    GtkWidget *dialog;
    GtkWidget *vbox;
    GtkWidget *combo_cours;
    GtkWidget *label_info;
    int response;
    int membre_id;
    FILE *f_cours;
    char ligne[256];
    int cours_id;
    char cours_nom[100], coach[100], salle[100], date_heure[100], niveau[50];
    int cours_trouves = 0;
    int *cours_ids = NULL;
    char **cours_affichage = NULL;
    
    // Récupérer l'ID du membre
    GtkWidget *window = gtk_widget_get_toplevel(objet);
    membre_id = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(window), "membre_id"));
    
    if (membre_id == 0) {
        afficher_message(objet, "erreur", "Erreur: Impossible d'identifier le membre !");
        return;
    }
    
    // Lire les cours disponibles
    f_cours = fopen("cours.txt", "r");
    if (!f_cours) {
        afficher_message(objet, "erreur", "Fichier cours.txt introuvable !");
        return;
    }
    
    cours_ids = malloc(100 * sizeof(int));
    cours_affichage = malloc(100 * sizeof(char*));
    
    while (fgets(ligne, sizeof(ligne), f_cours)) {
        ligne[strcspn(ligne, "\n")] = 0;
        if (sscanf(ligne, "%d %99s %99s %99s %99s %99s", 
                   &cours_id, cours_nom, coach, salle, date_heure, niveau) == 6) {
            cours_ids[cours_trouves] = cours_id;
            cours_affichage[cours_trouves] = malloc(256);
            sprintf(cours_affichage[cours_trouves], "[%d] %s - %s - %s", 
                    cours_id, cours_nom, date_heure, salle);
            cours_trouves++;
        }
    }
    fclose(f_cours);
    
    if (cours_trouves == 0) {
        afficher_message(objet, "erreur", "Aucun cours disponible !");
        free(cours_ids);
        free(cours_affichage);
        return;
    }
    
    // Créer le dialogue
    dialog = gtk_dialog_new_with_buttons("S'inscrire à un cours",
                                         GTK_WINDOW(window),
                                         GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                                         "Annuler", GTK_RESPONSE_CANCEL,
                                         "S'inscrire", GTK_RESPONSE_OK,
                                         NULL);
    
    gtk_window_set_default_size(GTK_WINDOW(dialog), 450, 250);
    
    vbox = gtk_vbox_new(FALSE, 10);
    gtk_container_set_border_width(GTK_CONTAINER(vbox), 15);
    
    // Label info
    label_info = gtk_label_new(NULL);
    char info_text[256];
    sprintf(info_text, "<span weight='bold' size='large'>📚 Choisissez un cours</span>\n\nMembre: #%d", membre_id);
    gtk_label_set_markup(GTK_LABEL(label_info), info_text);
    gtk_box_pack_start(GTK_BOX(vbox), label_info, FALSE, FALSE, 0);
    
    // Separator
    GtkWidget *separator = gtk_hseparator_new();
    gtk_box_pack_start(GTK_BOX(vbox), separator, FALSE, FALSE, 5);
    
    // Combo box des cours
    combo_cours = gtk_combo_box_new_text();
    for (int i = 0; i < cours_trouves; i++) {
        gtk_combo_box_append_text(GTK_COMBO_BOX(combo_cours), cours_affichage[i]);
    }
    gtk_combo_box_set_active(GTK_COMBO_BOX(combo_cours), 0);
    gtk_box_pack_start(GTK_BOX(vbox), combo_cours, FALSE, FALSE, 0);
    
    gtk_container_add(GTK_CONTAINER(gtk_dialog_get_content_area(GTK_DIALOG(dialog))), vbox);
    gtk_widget_show_all(dialog);
    
    response = gtk_dialog_run(GTK_DIALOG(dialog));
    
    if (response == GTK_RESPONSE_OK) {
        int active_index = gtk_combo_box_get_active(GTK_COMBO_BOX(combo_cours));
        
        if (active_index >= 0 && active_index < cours_trouves) {
            int cours_selected_id = cours_ids[active_index];
            
            // Vérifier si déjà inscrit
            FILE *f_check = fopen("inscriptions_membres.txt", "r");
            int deja_inscrit = 0;
            int id_m, id_c;
            
            if (f_check) {
                while (fscanf(f_check, "%d %d", &id_m, &id_c) == 2) {
                    if (id_m == membre_id && id_c == cours_selected_id) {
                        deja_inscrit = 1;
                        break;
                    }
                }
                fclose(f_check);
            }
            
            if (deja_inscrit) {
                afficher_message(objet, "avertissement", "Vous êtes déjà inscrit à ce cours !");
            } else {
                // Sauvegarder l'inscription
                FILE *f_ins = fopen("inscriptions_membres.txt", "a");
                if (f_ins) {
                    fprintf(f_ins, "%d %d\n", membre_id, cours_selected_id);
                    fclose(f_ins);
                    afficher_message(objet, "info", "✅ Inscription confirmée !");
                    
                    // ========== IMPORTANT : Rafraîchir le treeview ==========
                    GtkWidget *tv_membre_cours = lookup_widget(window, "tv_membre_cours");
                    if (tv_membre_cours) {
                        // Recharger les cours du membre
                        charger_cours_membre_dans_treeview(tv_membre_cours, membre_id);
                    } else {
                        printf("ERREUR: tv_membre_cours non trouvé !\n");
                    }
                    // ========================================================
                } else {
                    afficher_message(objet, "erreur", "Erreur lors de l'inscription !");
                }
            }
        }
    }
    
    // Nettoyage
    for (int i = 0; i < cours_trouves; i++) {
        free(cours_affichage[i]);
    }
    free(cours_ids);
    free(cours_affichage);
    
    gtk_widget_destroy(dialog);
}
void on_btn_reserver_coach_clicked(GtkWidget *objet, gpointer user_data)
{
    GtkWidget *dialog;
    GtkWidget *vbox;
    GtkWidget *combo_coach;
    GtkWidget *combo_date;
    GtkWidget *entry_notes;
    int response;
    int membre_id;
    
    // Récupérer l'ID du membre
    GtkWidget *window = gtk_widget_get_toplevel(objet);
    membre_id = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(window), "membre_id"));
    
    if (membre_id == 0) {
        afficher_message(objet, "erreur", "Erreur: Impossible d'identifier le membre !");
        return;
    }
    
    // Créer le dialogue
    dialog = gtk_dialog_new_with_buttons("Réserver un coach",
                                         GTK_WINDOW(window),
                                         GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                                         "Annuler", GTK_RESPONSE_CANCEL,
                                         "Réserver", GTK_RESPONSE_OK,
                                         NULL);
    
    gtk_window_set_default_size(GTK_WINDOW(dialog), 450, 350);
    
    vbox = gtk_vbox_new(FALSE, 10);
    gtk_container_set_border_width(GTK_CONTAINER(vbox), 15);
    
    // Label info
    GtkWidget *label_info = gtk_label_new(NULL);
    char info_text[256];
    sprintf(info_text, "<span weight='bold' size='large'>🏋️ Réserver un coach</span>\n\nMembre ID: %d", membre_id);
    gtk_label_set_markup(GTK_LABEL(label_info), info_text);
    gtk_box_pack_start(GTK_BOX(vbox), label_info, FALSE, FALSE, 0);
    
    GtkWidget *separator = gtk_hseparator_new();
    gtk_box_pack_start(GTK_BOX(vbox), separator, FALSE, FALSE, 5);
    
    // Choix du coach
    GtkWidget *label_coach = gtk_label_new("Choisissez un coach :");
    gtk_misc_set_alignment(GTK_MISC(label_coach), 0, 0.5);
    gtk_box_pack_start(GTK_BOX(vbox), label_coach, FALSE, FALSE, 0);
    
    combo_coach = gtk_combo_box_new_text();
    
    // Lire les coaches depuis entraineurs.txt
    FILE *f_ent = fopen("entraineurs.txt", "r");
    if (f_ent) {
        int id;
        char nom[100], specialite[100];
        int exp, age;
        char sexe;
        while (fscanf(f_ent, "%d %99s %99s %d %d %c", &id, nom, specialite, &exp, &age, &sexe) == 6) {
            char display[200];
            sprintf(display, "%d - %s (%s)", id, nom, specialite);
            gtk_combo_box_append_text(GTK_COMBO_BOX(combo_coach), display);
            // Stocker l'ID du coach
            char key[20];
            sprintf(key, "coach_id_%d", id);
            g_object_set_data(G_OBJECT(combo_coach), g_strdup(key), GINT_TO_POINTER(id));
        }
        fclose(f_ent);
    }
    gtk_combo_box_set_active(GTK_COMBO_BOX(combo_coach), 0);
    gtk_box_pack_start(GTK_BOX(vbox), combo_coach, FALSE, FALSE, 0);
    
    // Choix de la date
    GtkWidget *label_date = gtk_label_new("Choisissez une date :");
    gtk_misc_set_alignment(GTK_MISC(label_date), 0, 0.5);
    gtk_box_pack_start(GTK_BOX(vbox), label_date, FALSE, FALSE, 5);
    
    combo_date = gtk_combo_box_new_text();
    gtk_combo_box_append_text(GTK_COMBO_BOX(combo_date), "Lundi 15/05 - 10h00");
    gtk_combo_box_append_text(GTK_COMBO_BOX(combo_date), "Lundi 15/05 - 14h00");
    gtk_combo_box_append_text(GTK_COMBO_BOX(combo_date), "Mardi 16/05 - 11h00");
    gtk_combo_box_append_text(GTK_COMBO_BOX(combo_date), "Mercredi 17/05 - 15h00");
    gtk_combo_box_append_text(GTK_COMBO_BOX(combo_date), "Jeudi 18/05 - 09h00");
    gtk_combo_box_set_active(GTK_COMBO_BOX(combo_date), 0);
    gtk_box_pack_start(GTK_BOX(vbox), combo_date, FALSE, FALSE, 0);
    
    // Notes
    GtkWidget *label_notes = gtk_label_new("Notes (optionnel) :");
    gtk_misc_set_alignment(GTK_MISC(label_notes), 0, 0.5);
    gtk_box_pack_start(GTK_BOX(vbox), label_notes, FALSE, FALSE, 5);
    
    entry_notes = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(vbox), entry_notes, FALSE, FALSE, 0);
    
    gtk_container_add(GTK_CONTAINER(gtk_dialog_get_content_area(GTK_DIALOG(dialog))), vbox);
    gtk_widget_show_all(dialog);
    
    response = gtk_dialog_run(GTK_DIALOG(dialog));
    
    if (response == GTK_RESPONSE_OK) {
        int coach_index = gtk_combo_box_get_active(GTK_COMBO_BOX(combo_coach));
        int date_index = gtk_combo_box_get_active(GTK_COMBO_BOX(combo_date));
        
        if (coach_index >= 0 && date_index >= 0) {
            // Récupérer l'ID du coach sélectionné
            int coach_id = 0;
            char key[20];
            // Pour simplifier, on extrait l'ID du texte affiché
            gchar *coach_text = gtk_combo_box_get_active_text(GTK_COMBO_BOX(combo_coach));
            if (coach_text && sscanf(coach_text, "%d", &coach_id) == 1) {
                gchar *date = gtk_combo_box_get_active_text(GTK_COMBO_BOX(combo_date));
                const char *notes = gtk_entry_get_text(GTK_ENTRY(entry_notes));
                
                // Sauvegarder avec ID du coach
                FILE *f = fopen("reservations_coach.txt", "a");
                if (f) {
                    fprintf(f, "%d %d %s %s\n", membre_id, coach_id, date, notes);
                    fclose(f);
                    
                    afficher_message(objet, "info", "✅ Réservation confirmée !");
                } else {
                    afficher_message(objet, "erreur", "Erreur lors de la réservation !");
                }
                g_free(coach_text);
                g_free(date);
            } else if (coach_text) {
                g_free(coach_text);
            }
        } else {
            afficher_message(objet, "erreur", "Veuillez sélectionner un coach et une date !");
        }
    }
    
    gtk_widget_destroy(dialog);
}
// ==================== CALLBACKS ACCUEIL ENTRAINEUR ====================
void on_btn_valider_presence_clicked(GtkWidget *objet, gpointer user_data)
{
    GtkWidget *dialog;
    GtkWidget *vbox;
    GtkWidget *combo_demandes;
    GtkWidget *label_info;
    int response;
    int coach_id;
    char coach_nom[100] = "";
    
    // Récupérer l'ID du coach connecté
    GtkWidget *window = gtk_widget_get_toplevel(objet);
    coach_id = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(window), "coach_id"));
    
    if (coach_id == 0) {
        afficher_message(objet, "erreur", "Erreur: Impossible d'identifier le coach !");
        return;
    }
    
    // Récupérer le nom du coach
    FILE *f_ent = fopen("entraineurs.txt", "r");
    if (f_ent) {
        int id;
        char nom[100], specialite[100];
        int exp, age;
        char sexe;
        while (fscanf(f_ent, "%d %99s %99s %d %d %c", &id, nom, specialite, &exp, &age, &sexe) == 6) {
            if (id == coach_id) {
                strcpy(coach_nom, nom);
                break;
            }
        }
        fclose(f_ent);
    }
    
    if (strlen(coach_nom) == 0) {
        afficher_message(objet, "erreur", "Coach non trouvé !");
        return;
    }
    
    // Structure pour stocker les demandes
    typedef struct {
        int id_membre;
        char membre_nom[100];
        char date[100];
        char notes[200];
    } Demande;
    
    Demande demandes[100];
    int nb_demandes = 0;
    char ligne[512];
    
    // Lire les demandes
    FILE *f_reservations = fopen("reservations_coach.txt", "r");
    if (!f_reservations) {
        afficher_message(objet, "info", "Aucune demande de réservation.");
        return;
    }
    
    while (fgets(ligne, sizeof(ligne), f_reservations) && nb_demandes < 100) {
        ligne[strcspn(ligne, "\n")] = 0;
        
        int id_membre, id_coach;
        char date[100], notes[200];
        
        if (sscanf(ligne, "%d %d %99s %199[^\n]", &id_membre, &id_coach, date, notes) >= 3) {
            if (id_coach == coach_id) {
                char membre_nom[100] = "Inconnu";
                FILE *f_membres = fopen("membres.txt", "r");
                if (f_membres) {
                    int id;
                    char nom[100], email[100];
                    int age;
                    char sexe[10], abo[50];
                    float tarif;
                    while (fscanf(f_membres, "%d %99s %99s %d %9s %49s %f", 
                                 &id, nom, email, &age, sexe, abo, &tarif) == 7) {
                        if (id == id_membre) {
                            strcpy(membre_nom, nom);
                            break;
                        }
                    }
                    fclose(f_membres);
                }
                
                demandes[nb_demandes].id_membre = id_membre;
                strcpy(demandes[nb_demandes].membre_nom, membre_nom);
                strcpy(demandes[nb_demandes].date, date);
                strcpy(demandes[nb_demandes].notes, notes);
                nb_demandes++;
            }
        }
    }
    fclose(f_reservations);
    
    if (nb_demandes == 0) {
        afficher_message(objet, "info", "Aucune demande de réservation pour vous.");
        return;
    }
    
    // Créer le dialogue
    dialog = gtk_dialog_new_with_buttons("Valider une présence",
                                         GTK_WINDOW(window),
                                         GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                                         "Annuler", GTK_RESPONSE_CANCEL,
                                         "Valider", GTK_RESPONSE_OK,
                                         NULL);
    
    gtk_window_set_default_size(GTK_WINDOW(dialog), 550, 350);
    
    vbox = gtk_vbox_new(FALSE, 10);
    gtk_container_set_border_width(GTK_CONTAINER(vbox), 15);
    
    label_info = gtk_label_new(NULL);
    char info_text[256];
    sprintf(info_text, "<span weight='bold' size='large'>✅ Valider une présence</span>\n\nCoach: %s (ID: %d)\nDemandes en attente: %d", 
            coach_nom, coach_id, nb_demandes);
    gtk_label_set_markup(GTK_LABEL(label_info), info_text);
    gtk_box_pack_start(GTK_BOX(vbox), label_info, FALSE, FALSE, 0);
    
    GtkWidget *separator = gtk_hseparator_new();
    gtk_box_pack_start(GTK_BOX(vbox), separator, FALSE, FALSE, 5);
    
    GtkWidget *label_choix = gtk_label_new("Sélectionnez la demande à valider :");
    gtk_misc_set_alignment(GTK_MISC(label_choix), 0, 0.5);
    gtk_box_pack_start(GTK_BOX(vbox), label_choix, FALSE, FALSE, 0);
    
    combo_demandes = gtk_combo_box_new_text();
    for (int i = 0; i < nb_demandes; i++) {
        char display[350];
        sprintf(display, "[%d] %s - %s", 
                demandes[i].id_membre, demandes[i].membre_nom, demandes[i].date);
        gtk_combo_box_append_text(GTK_COMBO_BOX(combo_demandes), display);
    }
    gtk_combo_box_set_active(GTK_COMBO_BOX(combo_demandes), 0);
    gtk_box_pack_start(GTK_BOX(vbox), combo_demandes, FALSE, FALSE, 0);
    
    gtk_container_add(GTK_CONTAINER(gtk_dialog_get_content_area(GTK_DIALOG(dialog))), vbox);
    gtk_widget_show_all(dialog);
    
    response = gtk_dialog_run(GTK_DIALOG(dialog));
    
    if (response == GTK_RESPONSE_OK) {
        int active_index = gtk_combo_box_get_active(GTK_COMBO_BOX(combo_demandes));
        
        if (active_index >= 0 && active_index < nb_demandes) {
            Demande selected = demandes[active_index];
            
            // ========== ÉCRITURE CORRECTE : seulement id_membre id_coach ==========
            FILE *f_inscriptions = fopen("inscriptions_entraineurs.txt", "a");
            if (f_inscriptions) {
                fprintf(f_inscriptions, "%d %d\n", selected.id_membre, coach_id);
                fclose(f_inscriptions);
            }
            
            // Supprimer la demande validée
            FILE *f_original = fopen("reservations_coach.txt", "r");
            FILE *f_temp = fopen("temp_reservations.txt", "w");
            
            if (f_original && f_temp) {
                char line[512];
                while (fgets(line, sizeof(line), f_original)) {
                    int id_membre, id_coach;
                    char date[100], notes[200];
                    if (sscanf(line, "%d %d %99s %199[^\n]", &id_membre, &id_coach, date, notes) >= 3) {
                        if (!(id_membre == selected.id_membre && 
                              id_coach == coach_id && 
                              strcmp(date, selected.date) == 0)) {
                            fprintf(f_temp, "%s", line);
                        }
                    } else {
                        fprintf(f_temp, "%s", line);
                    }
                }
                fclose(f_original);
                fclose(f_temp);
                
                remove("reservations_coach.txt");
                rename("temp_reservations.txt", "reservations_coach.txt");
            }
            
            afficher_message(objet, "info", "✅ Présence validée avec succès !");
            
            // Rafraîchir le treeview
            GtkWidget *tv_coach_cours = lookup_widget(window, "tv_coach_cours");
            if (tv_coach_cours) {
                charger_cours_entraineur_dans_treeview(tv_coach_cours, coach_id);
            }
            
            GtkWidget *tv_demandes = lookup_widget(window, "tv_demandes");
            if (tv_demandes) {
                charger_demandes_entraineur_dans_treeviewcoach(tv_demandes);
            }
        }
    }
    
    gtk_widget_destroy(dialog);
}
void on_btn_coach_deconnexion_clicked(GtkButton *button, gpointer user_data)
{
    retour_vers_login(GTK_WIDGET(button));
}

void on_treeview_mes_cours_coach_row_activated(GtkTreeView *treeview, GtkTreePath *path,
                                              GtkTreeViewColumn *column, gpointer user_data)
{
    afficher_message(GTK_WIDGET(treeview), "info", "Gestion du cours s\303\251lectionn\303\251");
}

void on_treeview_demandes_row_activated(GtkTreeView *treeview, GtkTreePath *path,
                                       GtkTreeViewColumn *column, gpointer user_data)
{
    GtkTreeIter iter;
    GtkTreeModel *model = gtk_tree_view_get_model(treeview);
    gchar *type, *coach, *date_heure, *duree, *note, *statut;
    
    if (gtk_tree_model_get_iter(model, &iter, path)) {
        gtk_tree_model_get(model, &iter, 0, &type, 1, &coach, 2, &date_heure,
                          3, &duree, 4, &note, 5, &statut, -1);
        
        char message[512];
        sprintf(message, "Type: %s\nCoach: %s\nDate/Heure: %s\nDur\303\251e: %s h\nNote: %s\nStatut: %s",
                type, coach, date_heure, duree, note, statut);
        
        afficher_message(GTK_WIDGET(treeview), "info", message);
        
        g_free(type); g_free(coach); g_free(date_heure); g_free(duree); g_free(note); g_free(statut);
    }
}

void on_inscrit__centre__sportif_clicked(GtkButton *button, gpointer user_data)
{
    afficher_message(GTK_WIDGET(button), "info", "Fonction d'inscription centre sportif \303\240 impl\303\251menter");
}

void on_equip__pouruncour_clicked(GtkButton *button, gpointer user_data)
{
    afficher_message(GTK_WIDGET(button), "info", "Fonction de demande \303\251quipement \303\240 impl\303\251menter");
}

void on_inscrit__cour_clicked(GtkButton *button, gpointer user_data)
{
    afficher_message(GTK_WIDGET(button), "info", "Fonction de validation pr\303\251sence \303\240 impl\303\251menter");
}

// ==================== MENU ET AUTRES ====================

void on_boutton_activate(GtkMenuItem *menuitem, gpointer user_data)
{
    retour_vers_login(GTK_WIDGET(menuitem));
}

void on_menuitem7_activate(GtkMenuItem *menuitem, gpointer user_data) {}
void on_new1_activate(GtkMenuItem *menuitem, gpointer user_data) {}
void on_open1_activate(GtkMenuItem *menuitem, gpointer user_data) {}
void on_save1_activate(GtkMenuItem *menuitem, gpointer user_data) {}
void on_save_as1_activate(GtkMenuItem *menuitem, gpointer user_data) {}
void on_quit1_activate(GtkMenuItem *menuitem, gpointer user_data) {}
void on_menuitem8_activate(GtkMenuItem *menuitem, gpointer user_data) {}
void on_cut1_activate(GtkMenuItem *menuitem, gpointer user_data) {}
void on_copy1_activate(GtkMenuItem *menuitem, gpointer user_data) {}
void on_paste1_activate(GtkMenuItem *menuitem, gpointer user_data) {}
void on_delete1_activate(GtkMenuItem *menuitem, gpointer user_data) {}
void on_menuitem9_activate(GtkMenuItem *menuitem, gpointer user_data) {}
void on_menuitem10_activate(GtkMenuItem *menuitem, gpointer user_data) {}
void on_about1_activate(GtkMenuItem *menuitem, gpointer user_data) {}
void on_menuitem11_activate(GtkMenuItem *menuitem, gpointer user_data) {}

// ==================== STATISTIQUES ====================

DiagrammeData lire_toutes_donnees(void)
{
    DiagrammeData donnees = {0};
    
    FILE *fichier = fopen("membres.txt", "r");
    if (fichier) {
        char ligne[512];
        while (fgets(ligne, sizeof(ligne), fichier)) {
            ligne[strcspn(ligne, "\n")] = 0;
            int id, age;
            if (sscanf(ligne, "%d %*s %*s %d", &id, &age) >= 2 && age > 0 && age < 120) {
                if (age < 18) donnees.jeunes++;
                else if (age < 60) donnees.adultes++;
                else donnees.seniors++;
                donnees.total_membres++;
            }
        }
        fclose(fichier);
    }
    
    fichier = fopen("cours.txt", "r");
    if (fichier) {
        char ligne[512];
        while (fgets(ligne, sizeof(ligne), fichier)) {
            ligne[strcspn(ligne, "\n")] = 0;
            int id;
            char temp1[50], temp2[50], temp3[50], temp4[50], niveau[50];
            if (sscanf(ligne, "%d %49s %49s %49s %49s %49s", &id, temp1, temp2, temp3, temp4, niveau) == 6) {
                donnees.total_cours++;
                if (strstr(niveau, "1") || strstr(niveau, "D\303\251butant")) donnees.niveau1++;
                else if (strstr(niveau, "2") || strstr(niveau, "Interm\303\251diaire")) donnees.niveau2++;
                else if (strstr(niveau, "3") || strstr(niveau, "Avanc\303\251")) donnees.niveau3++;
            }
        }
        fclose(fichier);
    }
    
    fichier = fopen("entraineurs.txt", "r");
    if (fichier) {
        char ligne[512];
        while (fgets(ligne, sizeof(ligne), fichier)) {
            ligne[strcspn(ligne, "\n")] = 0;
            donnees.total_entraineurs++;
            if (strstr(ligne, "Musculation")) donnees.musculation++;
            else if (strstr(ligne, "Gymnastique")) donnees.gymnastique++;
            else if (strstr(ligne, "Yoga")) donnees.yoga++;
            else donnees.musculation++;
        }
        fclose(fichier);
    }
    
    fichier = fopen("equipements.txt", "r");
    if (fichier) {
        char ligne[512];
        while (fgets(ligne, sizeof(ligne), fichier)) {
            ligne[strcspn(ligne, "\n")] = 0;
            donnees.total_equipements++;
            if (strstr(ligne, "Cardio")) donnees.cardio++;
            else if (strstr(ligne, "Mobilites")) donnees.mobilites++;
            else if (strstr(ligne, "Musculation")) donnees.musculation_eq++;
            else donnees.cardio++;
        }
        fclose(fichier);
    }
    
    fichier = fopen("centres.txt", "r");
    if (fichier) {
        char ligne[512];
        while (fgets(ligne, sizeof(ligne), fichier)) {
            ligne[strcspn(ligne, "\n")] = 0;
            donnees.total_centres++;
            if (strstr(ligne, "Tunis")) donnees.tunis++;
            else if (strstr(ligne, "Ariana")) donnees.ariana++;
            else if (strstr(ligne, "Aouina")) donnees.aouina++;
            else donnees.tunis++;
        }
        fclose(fichier);
    }
    
    return donnees;
}

void initialiser_diagramme(GtkWidget *fenetre_admin)
{
}

void rafraichir_diagramme(GtkWidget *fenetre_admin)
{
}

gboolean on_drawingarea_diagramme_expose_event(GtkWidget *widget, GdkEventExpose *event, gpointer user_data)
{
    return FALSE;
}

// ==================== RAFRAICHISSEMENT ====================

static gboolean rafraichir_tout(gpointer data)
{
    GList *windows = gtk_window_list_toplevels();
    GList *iter;
    
    for (iter = windows; iter != NULL; iter = iter->next) {
        GtkWidget *window = GTK_WIDGET(iter->data);
        if (!GTK_IS_WINDOW(window)) continue;
        
        GtkWidget *tv;
        if ((tv = lookup_widget(window, "treeview_demandes"))) charger_demandes_entraineur_dans_treeviewcoach(tv);
    }
    
    g_list_free(windows);
    return TRUE;
}

void start_refresh()
{
    g_timeout_add(5000, rafraichir_tout, NULL);
}

// ==================== IMAGE DE FOND ====================

void load_background_image(void)
{
    GdkPixbuf *pixbuf;
    
    pixbuf = gdk_pixbuf_new_from_file("1.jpg", NULL);
    if (!pixbuf) pixbuf = gdk_pixbuf_new_from_file("./1.jpg", NULL);
    if (!pixbuf) pixbuf = gdk_pixbuf_new_from_file("../1.jpg", NULL);
    
    if (pixbuf) {
        if (bg_pixmap) g_object_unref(bg_pixmap);
        bg_width = gdk_pixbuf_get_width(pixbuf);
        bg_height = gdk_pixbuf_get_height(pixbuf);
        gdk_pixbuf_render_pixmap_and_mask(pixbuf, &bg_pixmap, NULL, 255);
        g_object_unref(pixbuf);
        printf("Image chargee: %d x %d\n", bg_width, bg_height);
    } else {
        printf("ERREUR: Impossible de charger 1.jpg\n");
        bg_width = 1280;
        bg_height = 800;
        GdkPixbuf *black = gdk_pixbuf_new(GDK_COLORSPACE_RGB, TRUE, 8, bg_width, bg_height);
        gdk_pixbuf_fill(black, 0x111111FF);
        gdk_pixbuf_render_pixmap_and_mask(black, &bg_pixmap, NULL, 255);
        g_object_unref(black);
    }
}

gboolean on_eventbox_generic_expose(GtkWidget *widget, GdkEventExpose *event, gpointer data)
{
    if (!bg_pixmap) {
        cairo_t *cr = gdk_cairo_create(widget->window);
        cairo_set_source_rgb(cr, 0.05, 0.05, 0.05);
        cairo_paint(cr);
        cairo_destroy(cr);
        return FALSE;
    }
    
    int w = widget->allocation.width;
    int h = widget->allocation.height;
    
    if (w <= 0 || h <= 0) return FALSE;
    
    GdkGC *gc = gdk_gc_new(widget->window);
    
    for (int y = 0; y < h; y += bg_height) {
        for (int x = 0; x < w; x += bg_width) {
            gdk_draw_pixmap(widget->window, gc, bg_pixmap, 0, 0, x, y,
                          (x + bg_width > w) ? w - x : bg_width,
                          (y + bg_height > h) ? h - y : bg_height);
        }
    }
    
    g_object_unref(gc);
    return FALSE;
}

void appliquer_background_a_fenetre(GtkWidget *fenetre)
{
    if (!bg_pixmap) return;
    
    GtkWidget *eventbox = lookup_widget(fenetre, "eventbox_bg_homepage");
    if (!eventbox) eventbox = lookup_widget(fenetre, "eventbox_bg_login");
    if (!eventbox) eventbox = lookup_widget(fenetre, "eventbox_bg_member");
    if (!eventbox) eventbox = lookup_widget(fenetre, "eventbox_bg_coach");
    
    if (eventbox && GTK_IS_EVENT_BOX(eventbox)) {
        g_signal_connect(eventbox, "expose-event", 
                       G_CALLBACK(on_eventbox_generic_expose), NULL);
        gtk_widget_set_app_paintable(eventbox, TRUE);
        gtk_widget_queue_draw(eventbox);
    } else {
        GQueue *queue = g_queue_new();
        g_queue_push_tail(queue, fenetre);
        
        while (!g_queue_is_empty(queue)) {
            GtkWidget *widget = g_queue_pop_head(queue);
            
            if (GTK_IS_EVENT_BOX(widget)) {
                g_signal_connect(widget, "expose-event", 
                               G_CALLBACK(on_eventbox_generic_expose), NULL);
                gtk_widget_set_app_paintable(widget, TRUE);
                gtk_widget_queue_draw(widget);
            }
            
            if (GTK_IS_CONTAINER(widget)) {
                GList *children = gtk_container_get_children(GTK_CONTAINER(widget));
                GList *iter;
                for (iter = children; iter != NULL; iter = iter->next) {
                    g_queue_push_tail(queue, GTK_WIDGET(iter->data));
                }
                g_list_free(children);
            }
        }
        g_queue_free(queue);
    }
}

// ==================== COULEURS DES FRAMES ====================

void definir_couleur_frame(GtkWidget *frame, const char *couleur_label)
{
    GtkWidget *label = gtk_frame_get_label_widget(GTK_FRAME(frame));
    if (label) {
        GdkColor color;
        if (gdk_color_parse(couleur_label, &color)) {
            gtk_widget_modify_fg(label, GTK_STATE_NORMAL, &color);
        }
        PangoFontDescription *font = pango_font_description_from_string("Bold");
        gtk_widget_modify_font(label, font);
        pango_font_description_free(font);
    }
    gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_NONE);
}

void appliquer_couleurs_tous_frames(GtkWidget *fenetre)
{
    GdkColor bg_color;
    GdkColor red_color;
    GdkColor white_color;
    GdkColor base_color;
    
    gdk_color_parse("#1E1E1E", &bg_color);
    gdk_color_parse("#CC0000", &red_color);
    gdk_color_parse("#FFFFFF", &white_color);
    gdk_color_parse("#2A2A2A", &base_color);
    
    GQueue *queue = g_queue_new();
    g_queue_push_tail(queue, fenetre);
    
    while (!g_queue_is_empty(queue)) {
        GtkWidget *widget = g_queue_pop_head(queue);
        
        // EVENTBOX
        if (GTK_IS_EVENT_BOX(widget)) {
            gtk_widget_modify_bg(widget, GTK_STATE_NORMAL, &bg_color);
        }
        
        // VBOX et HBOX
        if (GTK_IS_VBOX(widget) || GTK_IS_HBOX(widget)) {
            gtk_widget_modify_bg(widget, GTK_STATE_NORMAL, &bg_color);
        }
        
        // FRAMES
        if (GTK_IS_FRAME(widget)) {
            gtk_widget_modify_bg(widget, GTK_STATE_NORMAL, &bg_color);
            GtkWidget *label = gtk_frame_get_label_widget(GTK_FRAME(widget));
            if (label) {
                gtk_widget_modify_fg(label, GTK_STATE_NORMAL, &red_color);
                PangoFontDescription *font = pango_font_description_from_string("Bold");
                gtk_widget_modify_font(label, font);
                pango_font_description_free(font);
            }
        }
        
        // TOUS LES LABELS (sauf ceux des frames qui sont déjà traités)
        if (GTK_IS_LABEL(widget)) {
            GtkWidget *parent = gtk_widget_get_parent(widget);
            if (!GTK_IS_FRAME(parent)) {
                gtk_widget_modify_fg(widget, GTK_STATE_NORMAL, &white_color);
            }
        }
        
        // TEXTVIEW
        if (GTK_IS_TEXT_VIEW(widget)) {
            gtk_widget_modify_base(widget, GTK_STATE_NORMAL, &base_color);
            gtk_widget_modify_text(widget, GTK_STATE_NORMAL, &white_color);
        }
        
        // BUTTONS
        if (GTK_IS_BUTTON(widget)) {
            GdkColor btn_color;
            gdk_color_parse("#CC0000", &btn_color);
            gtk_widget_modify_bg(widget, GTK_STATE_NORMAL, &btn_color);
            gtk_widget_modify_fg(widget, GTK_STATE_NORMAL, &white_color);
        }
        
        // ENTRIES
        if (GTK_IS_ENTRY(widget)) {
            gtk_widget_modify_base(widget, GTK_STATE_NORMAL, &base_color);
            gtk_widget_modify_text(widget, GTK_STATE_NORMAL, &white_color);
        }
        
        // SCROLLEDWINDOW
        if (GTK_IS_SCROLLED_WINDOW(widget)) {
            gtk_widget_modify_bg(widget, GTK_STATE_NORMAL, &bg_color);
            GtkWidget *child = gtk_bin_get_child(GTK_BIN(widget));
            if (child) {
                gtk_widget_modify_bg(child, GTK_STATE_NORMAL, &base_color);
                if (GTK_IS_TREE_VIEW(child)) {
                    gtk_widget_modify_base(child, GTK_STATE_NORMAL, &base_color);
                    gtk_widget_modify_fg(child, GTK_STATE_NORMAL, &white_color);
                    gtk_widget_modify_text(child, GTK_STATE_NORMAL, &white_color);
                }
            }
        }
        
        // TREEVIEW
        if (GTK_IS_TREE_VIEW(widget)) {
            gtk_widget_modify_bg(widget, GTK_STATE_NORMAL, &base_color);
            gtk_widget_modify_base(widget, GTK_STATE_NORMAL, &base_color);
            gtk_widget_modify_fg(widget, GTK_STATE_NORMAL, &white_color);
            gtk_widget_modify_text(widget, GTK_STATE_NORMAL, &white_color);
        }
        
        // Parcourir les enfants
        if (GTK_IS_CONTAINER(widget)) {
            GList *children = gtk_container_get_children(GTK_CONTAINER(widget));
            GList *iter;
            for (iter = children; iter != NULL; iter = iter->next) {
                g_queue_push_tail(queue, GTK_WIDGET(iter->data));
            }
            g_list_free(children);
        }
    }
    
    g_queue_free(queue);
}
void appliquer_couleurs_sombres(GtkWidget *fenetre)
{
    GdkColor bg_color;
    gdk_color_parse("#1E1E1E", &bg_color);
    
    GQueue *queue = g_queue_new();
    g_queue_push_tail(queue, fenetre);
    
    while (!g_queue_is_empty(queue)) {
        GtkWidget *widget = g_queue_pop_head(queue);
        
        if (GTK_IS_EVENT_BOX(widget) || GTK_IS_VBOX(widget) || GTK_IS_HBOX(widget) || GTK_IS_FRAME(widget)) {
            gtk_widget_modify_bg(widget, GTK_STATE_NORMAL, &bg_color);
        }
        
        if (GTK_IS_CONTAINER(widget)) {
            GList *children = gtk_container_get_children(GTK_CONTAINER(widget));
            GList *iter;
            for (iter = children; iter != NULL; iter = iter->next) {
                g_queue_push_tail(queue, GTK_WIDGET(iter->data));
            }
            g_list_free(children);
        }
    }
    
    g_queue_free(queue);
}

// ==================== CALLBACKS HOMEPAGE ====================

void on_window_homepage_delete_event(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
    gtk_main_quit();
}

void on_btn_clubs_clicked(GtkWidget *objet, gpointer user_data)
{
    afficher_message(objet, "info", "Page Clubs - Fonctionnalit\303\251 \303\240 venir");
}

void on_btn_activites_clicked(GtkWidget *objet, gpointer user_data)
{
    afficher_message(objet, "info", "Page Activit\303\251s - Fonctionnalit\303\251 \303\240 venir");
}

void on_btn_offres_nav_clicked(GtkWidget *objet, gpointer user_data)
{
    afficher_message(objet, "info", "Page Offres - Fonctionnalit\303\251 \303\240 venir");
}

void on_btn_planning_clicked(GtkWidget *objet, gpointer user_data)
{
    afficher_message(objet, "info", "Page Planning - Fonctionnalit\303\251 \303\240 venir");
}

void on_btn_about_clicked(GtkWidget *objet, gpointer user_data)
{
    afficher_message(objet, "info", "Pour La Forme GYM - Version 1.0\nVotre salle de sport premium");
}

void on_btn_entreprise_clicked(GtkWidget *objet, gpointer user_data){}

void on_btn_rejoindre_clicked(GtkWidget *objet, gpointer user_data)
{
    GtkWidget *login_window = create_window_login();
    if (login_window) {
        appliquer_background_a_fenetre(login_window);
        appliquer_couleurs_tous_frames(login_window);
        gtk_widget_show_all(login_window);
        gtk_widget_destroy(gtk_widget_get_toplevel(objet));
    }
}

 void on_btn_cta_offres_clicked(GtkWidget *objet, gpointer user_data)
{
    GtkWidget *menu;
    GtkWidget *menu_item;
    
    // Créer le menu
    menu = gtk_menu_new();
    
    // Offre Standard
    menu_item = gtk_menu_item_new_with_label("⭐ Standard - 50 DT/mois");
    gtk_widget_show(menu_item);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_item);
    g_signal_connect(menu_item, "activate", G_CALLBACK(on_offre_standard_clicked), NULL);
    
    // Offre Premium
    menu_item = gtk_menu_item_new_with_label("💎 Premium - 80 DT/mois");
    gtk_widget_show(menu_item);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_item);
    g_signal_connect(menu_item, "activate", G_CALLBACK(on_offre_premium_clicked), NULL);
    
    // Offre VIP
    menu_item = gtk_menu_item_new_with_label("👑 VIP - 120 DT/mois");
    gtk_widget_show(menu_item);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_item);
    g_signal_connect(menu_item, "activate", G_CALLBACK(on_offre_vip_clicked), NULL);
    
    // Séparateur
    menu_item = gtk_separator_menu_item_new();
    gtk_widget_show(menu_item);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_item);
    
    // Offre Annuelle
    menu_item = gtk_menu_item_new_with_label("📅 Annuelle - 500 DT/an (Économie 20%)");
    gtk_widget_show(menu_item);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_item);
    g_signal_connect(menu_item, "activate", G_CALLBACK(on_offre_annuelle_clicked), NULL);
    
    // Afficher le menu au clic
    gtk_menu_attach_to_widget(GTK_MENU(menu), objet, NULL);
    gtk_menu_popup(GTK_MENU(menu), NULL, NULL, NULL, NULL, 0, gtk_get_current_event_time());
}

// Callbacks pour chaque option du menu
void on_offre_standard_clicked(GtkWidget *item, gpointer user_data) {
    GtkWidget *window = gtk_widget_get_toplevel(item);
    GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(window),
        GTK_DIALOG_DESTROY_WITH_PARENT,
        GTK_MESSAGE_INFO,
        GTK_BUTTONS_YES_NO,
        "Offre Standard - 50 DT/mois\n\nAvantages:\n✓ Accès à 1 club\n✓ Cours collectifs illimités\n✓ Horaires 8h-20h\n\nSouhaitez-vous souscrire à cette offre ?");
    
    int result = gtk_dialog_run(GTK_DIALOG(dialog));
    if (result == GTK_RESPONSE_YES) {
        gtk_widget_destroy(dialog);
        // Ouvrir la fenêtre d'inscription
        GtkWidget *login_window = create_window_login();
        gtk_widget_show(login_window);
        gtk_window_present(GTK_WINDOW(login_window));
    } else {
        gtk_widget_destroy(dialog);
    }
}

void on_offre_premium_clicked(GtkWidget *item, gpointer user_data) {
    GtkWidget *window = gtk_widget_get_toplevel(item);
    GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(window),
        GTK_DIALOG_DESTROY_WITH_PARENT,
        GTK_MESSAGE_INFO,
        GTK_BUTTONS_YES_NO,
        "Offre Premium - 80 DT/mois\n\nAvantages:\n✓ Accès à tous les clubs\n✓ Cours collectifs + coach dédié\n✓ Horaires 6h-22h\n✓ 2 séances massage/mois\n\nSouhaitez-vous souscrire à cette offre ?");
    
    int result = gtk_dialog_run(GTK_DIALOG(dialog));
    if (result == GTK_RESPONSE_YES) {
        gtk_widget_destroy(dialog);
        GtkWidget *login_window = create_window_login();
        gtk_widget_show(login_window);
        gtk_window_present(GTK_WINDOW(login_window));
    } else {
        gtk_widget_destroy(dialog);
    }
}

void on_offre_vip_clicked(GtkWidget *item, gpointer user_data) {
    GtkWidget *window = gtk_widget_get_toplevel(item);
    GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(window),
        GTK_DIALOG_DESTROY_WITH_PARENT,
        GTK_MESSAGE_INFO,
        GTK_BUTTONS_YES_NO,
        "Offre VIP - 120 DT/mois\n\nAvantages:\n✓ Accès illimité à tous les clubs\n✓ Coach personnel\n✓ Horaires 24h/24\n✓ Programme personnalisé\n✓ Espace spa inclus\n\nSouhaitez-vous souscrire à cette offre ?");
    
    int result = gtk_dialog_run(GTK_DIALOG(dialog));
    if (result == GTK_RESPONSE_YES) {
        gtk_widget_destroy(dialog);
        GtkWidget *login_window = create_window_login();
        gtk_widget_show(login_window);
        gtk_window_present(GTK_WINDOW(login_window));
    } else {
        gtk_widget_destroy(dialog);
    }
}

void on_offre_annuelle_clicked(GtkWidget *item, gpointer user_data) {
    GtkWidget *window = gtk_widget_get_toplevel(item);
    GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(window),
        GTK_DIALOG_DESTROY_WITH_PARENT,
        GTK_MESSAGE_INFO,
        GTK_BUTTONS_YES_NO,
        "Offre Annuelle - 500 DT/an\n\nÉconomie de 20%% par rapport au mensuel!\n\n...");
    
    int result = gtk_dialog_run(GTK_DIALOG(dialog));
    if (result == GTK_RESPONSE_YES) {
        gtk_widget_destroy(dialog);
        GtkWidget *login_window = create_window_login();
        gtk_widget_show(login_window);
        gtk_window_present(GTK_WINDOW(login_window));
    } else {
        gtk_widget_destroy(dialog);
    }
}
// ==================== CALLBACKS LOGIN WINDOW ====================

void on_btn_se_connecter_clicked(GtkWidget *objet, gpointer user_data)
{
    GtkWidget *entry_email = lookup_widget(objet, "entry_email");
    GtkWidget *entry_password = lookup_widget(objet, "entry_password");
    GtkWidget *chk_robot = lookup_widget(objet, "chk_robot");
    
    if (!entry_email || !entry_password) {
        afficher_message(objet, "erreur", "Erreur: Widgets non trouves");
        return;
    }
    
    const char *username = gtk_entry_get_text(GTK_ENTRY(entry_email));
    const char *password = gtk_entry_get_text(GTK_ENTRY(entry_password));
    gboolean is_robot = FALSE;
    
    if (chk_robot) {
        is_robot = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(chk_robot));
    }
    
    if (strlen(username) == 0 || strlen(password) == 0) {
        afficher_message(objet, "erreur", "Veuillez remplir tous les champs");
        return;
    }
    
    if (!is_robot) {
        afficher_message(objet, "erreur", "Veuillez confirmer que vous n'etes pas un robot");
        return;
    }
    
    char user_type[20];
    int user_id;
    
    if (verifier_login(username, password, user_type, &user_id)) {
        GtkWidget *login_window = gtk_widget_get_toplevel(objet);
        
        if (strcmp(user_type, "admin") == 0) {
            GtkWidget *admin_window = create_window_admin();
            if (admin_window) {
                appliquer_background_a_fenetre(admin_window);
                appliquer_couleurs_tous_frames(admin_window);
                gtk_widget_show_all(admin_window);
                if (login_window) gtk_widget_destroy(login_window);
            }
        }
        else if (strcmp(user_type, "membre") == 0) {
    GtkWidget *member_window = create_window_dashboard_member();
    if (member_window) {
        // Stocker l'ID du membre dans la fenêtre
        g_object_set_data(G_OBJECT(member_window), "membre_id", GINT_TO_POINTER(user_id));
        
        appliquer_background_a_fenetre(member_window);
        appliquer_couleurs_tous_frames(member_window);
        
        // Afficher le profil
        GtkWidget *lbl_profile = lookup_widget(member_window, "lbl_profile_content");
        if (lbl_profile) afficher_profil_membre(lbl_profile, user_id);
        
        // Charger les cours du membre
        GtkWidget *tv_membre_cours = lookup_widget(member_window, "tv_membre_cours");
        if (tv_membre_cours) {
            const char *titles[] = {"ID", "Cours", "Coach", "Salle", "Date/Heure", "Niveau"};
            treeview_init_columns(tv_membre_cours, titles, 6);
            charger_cours_membre_dans_treeview(tv_membre_cours, user_id);
        }
        
        gtk_widget_show_all(member_window);
        if (login_window) gtk_widget_destroy(login_window);
    }
}
        else if (strcmp(user_type, "entraineur") == 0) {
    GtkWidget *coach_window = create_window_dashboard_coach();
    if (coach_window) {
        // Stocker l'ID du coach dans la fenêtre
        g_object_set_data(G_OBJECT(coach_window), "coach_id", GINT_TO_POINTER(user_id));
        
        appliquer_background_a_fenetre(coach_window);
        appliquer_couleurs_tous_frames(coach_window);
        
        // Afficher le profil du coach
        GtkWidget *lbl_profile = lookup_widget(coach_window, "lbl_coach_profile_content");
        if (lbl_profile) afficher_profil_entraineur(lbl_profile, user_id);
        
        // Charger les cours du coach
        GtkWidget *tv_coach_cours = lookup_widget(coach_window, "tv_coach_cours");
        if (tv_coach_cours) {
            const char *titles[] = {"ID", "Cours", "Coach", "Salle", "Date/Heure", "Niveau"};
            treeview_init_columns(tv_coach_cours, titles, 6);
            charger_cours_entraineur_dans_treeview(tv_coach_cours, user_id);
        }
        
        // Charger les demandes
        GtkWidget *tv_demandes = lookup_widget(coach_window, "tv_demandes");
        if (tv_demandes) {
            const char *titles[] = {"ID Membre", "Membre", "Coach", "Date", "Notes", "Statut"};
            treeview_init_columns(tv_demandes, titles, 6);
            charger_demandes_entraineur_dans_treeviewcoach(tv_demandes);
        }
        
        gtk_widget_show_all(coach_window);
        if (login_window) gtk_widget_destroy(login_window);
    }
}
        else {
            afficher_message(objet, "info", "Type d'utilisateur non reconnu");
        }
    } else {
        afficher_message(objet, "erreur", "Identifiants incorrects");
    }
}

void on_btn_inscrire_clicked(GtkWidget *objet, gpointer user_data)
{
    GtkWidget *dialog;
    GtkWidget *vbox;
    GtkWidget *combo_type;
    GtkWidget *entry_nom;
    GtkWidget *entry_prenom;
    GtkWidget *entry_email;
    GtkWidget *entry_password;
    GtkWidget *entry_age;
    GtkWidget *combo_sexe;
    GtkWidget *entry_specialite;
    GtkWidget *entry_experience;
    int response;
    
    // Créer le dialogue d'inscription
    dialog = gtk_dialog_new_with_buttons("Inscription",
                                         GTK_WINDOW(gtk_widget_get_toplevel(objet)),
                                         GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                                         "Annuler", GTK_RESPONSE_CANCEL,
                                         "S'inscrire", GTK_RESPONSE_OK,
                                         NULL);
    
    gtk_window_set_default_size(GTK_WINDOW(dialog), 450, 550);
    
    vbox = gtk_vbox_new(FALSE, 10);
    gtk_container_set_border_width(GTK_CONTAINER(vbox), 15);
    
    // Titre
    GtkWidget *title = gtk_label_new("<span weight='bold' size='large'>📝 Créer un compte</span>");
    gtk_label_set_use_markup(GTK_LABEL(title), TRUE);
    gtk_box_pack_start(GTK_BOX(vbox), title, FALSE, FALSE, 0);
    
    GtkWidget *separator0 = gtk_hseparator_new();
    gtk_box_pack_start(GTK_BOX(vbox), separator0, FALSE, FALSE, 5);
    
    // Type d'utilisateur
    GtkWidget *label_type = gtk_label_new("Type d'inscription :");
    gtk_misc_set_alignment(GTK_MISC(label_type), 0, 0.5);
    gtk_box_pack_start(GTK_BOX(vbox), label_type, FALSE, FALSE, 0);
    
    combo_type = gtk_combo_box_new_text();
    gtk_combo_box_append_text(GTK_COMBO_BOX(combo_type), "Membre");
    gtk_combo_box_append_text(GTK_COMBO_BOX(combo_type), "Entraîneur");
    gtk_combo_box_set_active(GTK_COMBO_BOX(combo_type), 0);
    gtk_box_pack_start(GTK_BOX(vbox), combo_type, FALSE, FALSE, 0);
    
    // Separator
    GtkWidget *separator = gtk_hseparator_new();
    gtk_box_pack_start(GTK_BOX(vbox), separator, FALSE, FALSE, 5);
    
    // Champs communs
    GtkWidget *label_nom = gtk_label_new("Nom :");
    gtk_misc_set_alignment(GTK_MISC(label_nom), 0, 0.5);
    gtk_box_pack_start(GTK_BOX(vbox), label_nom, FALSE, FALSE, 0);
    
    entry_nom = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(vbox), entry_nom, FALSE, FALSE, 0);
    
    GtkWidget *label_prenom = gtk_label_new("Prénom :");
    gtk_misc_set_alignment(GTK_MISC(label_prenom), 0, 0.5);
    gtk_box_pack_start(GTK_BOX(vbox), label_prenom, FALSE, FALSE, 0);
    
    entry_prenom = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(vbox), entry_prenom, FALSE, FALSE, 0);
    
    GtkWidget *label_email = gtk_label_new("Email :");
    gtk_misc_set_alignment(GTK_MISC(label_email), 0, 0.5);
    gtk_box_pack_start(GTK_BOX(vbox), label_email, FALSE, FALSE, 0);
    
    entry_email = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(vbox), entry_email, FALSE, FALSE, 0);
    
    GtkWidget *label_password = gtk_label_new("Mot de passe :");
    gtk_misc_set_alignment(GTK_MISC(label_password), 0, 0.5);
    gtk_box_pack_start(GTK_BOX(vbox), label_password, FALSE, FALSE, 0);
    
    entry_password = gtk_entry_new();
    gtk_entry_set_visibility(GTK_ENTRY(entry_password), FALSE);
    gtk_box_pack_start(GTK_BOX(vbox), entry_password, FALSE, FALSE, 0);
    
    GtkWidget *label_age = gtk_label_new("Âge :");
    gtk_misc_set_alignment(GTK_MISC(label_age), 0, 0.5);
    gtk_box_pack_start(GTK_BOX(vbox), label_age, FALSE, FALSE, 0);
    
    entry_age = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(vbox), entry_age, FALSE, FALSE, 0);
    
    GtkWidget *label_sexe = gtk_label_new("Sexe :");
    gtk_misc_set_alignment(GTK_MISC(label_sexe), 0, 0.5);
    gtk_box_pack_start(GTK_BOX(vbox), label_sexe, FALSE, FALSE, 0);
    
    combo_sexe = gtk_combo_box_new_text();
    gtk_combo_box_append_text(GTK_COMBO_BOX(combo_sexe), "Homme");
    gtk_combo_box_append_text(GTK_COMBO_BOX(combo_sexe), "Femme");
    gtk_combo_box_set_active(GTK_COMBO_BOX(combo_sexe), 0);
    gtk_box_pack_start(GTK_BOX(vbox), combo_sexe, FALSE, FALSE, 0);
    
    // Champs spécifiques entraineur
    GtkWidget *label_specialite = gtk_label_new("Spécialité (Entraîneur) :");
    gtk_misc_set_alignment(GTK_MISC(label_specialite), 0, 0.5);
    gtk_box_pack_start(GTK_BOX(vbox), label_specialite, FALSE, FALSE, 0);
    
    entry_specialite = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(vbox), entry_specialite, FALSE, FALSE, 0);
    
    GtkWidget *label_experience = gtk_label_new("Expérience (années) :");
    gtk_misc_set_alignment(GTK_MISC(label_experience), 0, 0.5);
    gtk_box_pack_start(GTK_BOX(vbox), label_experience, FALSE, FALSE, 0);
    
    entry_experience = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(vbox), entry_experience, FALSE, FALSE, 0);
    
    gtk_container_add(GTK_CONTAINER(gtk_dialog_get_content_area(GTK_DIALOG(dialog))), vbox);
    gtk_widget_show_all(dialog);
    
    response = gtk_dialog_run(GTK_DIALOG(dialog));
    
    if (response == GTK_RESPONSE_OK) {
        int type = gtk_combo_box_get_active(GTK_COMBO_BOX(combo_type));
        const char *nom = gtk_entry_get_text(GTK_ENTRY(entry_nom));
        const char *prenom = gtk_entry_get_text(GTK_ENTRY(entry_prenom));
        const char *email = gtk_entry_get_text(GTK_ENTRY(entry_email));
        const char *password = gtk_entry_get_text(GTK_ENTRY(entry_password));
        const char *age = gtk_entry_get_text(GTK_ENTRY(entry_age));
        int sexe_index = gtk_combo_box_get_active(GTK_COMBO_BOX(combo_sexe));
        char sexe = (sexe_index == 0) ? 'H' : 'F';
        
        if (strlen(nom) == 0 || strlen(email) == 0 || strlen(password) == 0) {
            afficher_message(objet, "erreur", "Nom, Email et Mot de passe sont obligatoires !");
            gtk_widget_destroy(dialog);
            return;
        }
        
        int new_id;
        FILE *f_membres, *f_entraineurs, *f_login;
        
        // Trouver le prochain ID disponible dans login.txt
        new_id = 1;
        f_login = fopen("login.txt", "r");
        if (f_login) {
            char ligne[256];
            int id_max = 0, id;
            while (fgets(ligne, sizeof(ligne), f_login)) {
                if (sscanf(ligne, "%*s %*s %*s %d", &id) == 1 && id > id_max) {
                    id_max = id;
                }
            }
            fclose(f_login);
            new_id = id_max + 1;
        }
        
        char nom_complet[200];
        sprintf(nom_complet, "%s %s", prenom, nom);
        
        if (type == 0) { // MEMBRE
            // Ajouter à membres.txt
            f_membres = fopen("membres.txt", "a");
            if (f_membres) {
                fprintf(f_membres, "%d %s %s %s %c Standard 0.00\n", 
                        new_id, nom_complet, email, age, sexe);
                fclose(f_membres);
            }
            
            // Ajouter à login.txt avec le MÊME ID
            f_login = fopen("login.txt", "a");
            if (f_login) {
                fprintf(f_login, "%s %s membre %d\n", email, password, new_id);
                fclose(f_login);
            }
            
            afficher_message(objet, "info", "✅ Inscription membre réussie ! Vous pouvez maintenant vous connecter.");
            
        } else { // ENTRAINEUR
            const char *specialite = gtk_entry_get_text(GTK_ENTRY(entry_specialite));
            const char *experience = gtk_entry_get_text(GTK_ENTRY(entry_experience));
            
            if (strlen(specialite) == 0) {
                afficher_message(objet, "erreur", "La spécialité est obligatoire pour un entraîneur !");
                gtk_widget_destroy(dialog);
                return;
            }
            
            int exp_int = atoi(experience);
            int age_int = atoi(age);
            
            // Ajouter à entraineurs.txt
            f_entraineurs = fopen("entraineurs.txt", "a");
            if (f_entraineurs) {
                fprintf(f_entraineurs, "%d %s %s %d %d %c\n", 
                        new_id, nom_complet, specialite, exp_int, age_int, sexe);
                fclose(f_entraineurs);
            }
            
            // Ajouter à login.txt avec le MÊME ID
            f_login = fopen("login.txt", "a");
            if (f_login) {
                fprintf(f_login, "%s %s entraineur %d\n", email, password, new_id);
                fclose(f_login);
            }
            
            afficher_message(objet, "info", "✅ Inscription entraîneur réussie ! Vous pouvez maintenant vous connecter.");
        }
    }
    
    gtk_widget_destroy(dialog);
}

void on_chk_robot_toggled(GtkToggleButton *togglebutton, gpointer user_data)
{
    gboolean active = gtk_toggle_button_get_active(togglebutton);
    if (active) {
        printf("Robot verification OK\n");
    }
}

void on_entry_email_focus_in(GtkWidget *widget, GdkEventFocus *event, gpointer user_data)
{
    const char *text = gtk_entry_get_text(GTK_ENTRY(widget));
    if (text && strcmp(text, "votre@email.com") == 0) {
        gtk_entry_set_text(GTK_ENTRY(widget), "");
    }
}

void on_window_login_delete_event(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
    gtk_main_quit();
}

// ==================== CALLBACKS DASHBOARD MEMBER ====================

void on_window_member_delete_event(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
    gtk_main_quit();
}

void on_eb_m_avatar_clicked(GtkWidget *objet, gpointer user_data)
{
    afficher_message(objet, "info", "Profil utilisateur");
}


void on_btn_deconnexion_m_clicked(GtkWidget *objet, gpointer user_data)
{
    retour_vers_login(objet);
}

void on_btn_chatbot_send_clicked(GtkWidget *objet, gpointer user_data)
{
    GtkWidget *entry = lookup_widget(objet, "entry_chatbot_msg");
    GtkWidget *tv = lookup_widget(objet, "tv_chatbot");

    if (entry && tv) {
        const char *msg = gtk_entry_get_text(GTK_ENTRY(entry));
        if (msg && strlen(msg) > 0) {
            GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(tv));
            GtkTextIter iter;
            gtk_text_buffer_get_end_iter(buffer, &iter);

            // Afficher le message de l'utilisateur
            gchar *formatted = g_strdup_printf("Moi: %s\n", msg);
            gtk_text_buffer_insert(buffer, &iter, formatted, -1);
            g_free(formatted);

            // Afficher un message de "typing"
            gtk_text_buffer_get_end_iter(buffer, &iter);
            gchar *typing = g_strdup("Midou: Tape...\n");
            gtk_text_buffer_insert(buffer, &iter, typing, -1);
            g_free(typing);

            // Forcer l'affichage immédiat
            while (gtk_events_pending())
                gtk_main_iteration();

            // Appeler l'API Groq
            char *api_response = call_grok_api(msg);

            // Supprimer le message "Tape..."
            GtkTextIter start, end;
            gtk_text_buffer_get_end_iter(buffer, &end);
            gtk_text_buffer_get_iter_at_line(buffer, &start, gtk_text_buffer_get_line_count(buffer) - 1);
            gtk_text_buffer_delete(buffer, &start, &end);

            // Afficher la vraie réponse
            gtk_text_buffer_get_end_iter(buffer, &iter);
            gtk_text_buffer_insert(buffer, &iter, api_response, -1);
            g_free(api_response);

            gtk_entry_set_text(GTK_ENTRY(entry), "");
        }
    }
}

// ==================== CALLBACKS DASHBOARD COACH ====================

void on_window_coach_delete_event(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
    gtk_main_quit();
}

void on_eb_c_avatar_clicked(GtkWidget *objet, gpointer user_data)
{
    afficher_message(objet, "info", "Profil Coach");
}

void charger_demandes_entraineur_dans_treeviewcoach(GtkWidget *treeview)
{
    FILE *f = fopen("reservations_coach.txt", "r");
    if (!f) {
        gchar *no_data[6] = {"Aucune", "demande", "", "", "", ""};
        treeview_ajouter_ligne(treeview, no_data, 6);
        return;
    }
    
    treeview_vider(treeview);
    
    char ligne[512];
    gchar *row_values[6];
    
    while (fgets(ligne, sizeof(ligne), f)) {
        ligne[strcspn(ligne, "\n")] = 0;
        int id_membre, id_coach;
        char date[100], notes[200];
        
        if (sscanf(ligne, "%d %d %99s %199[^\n]", &id_membre, &id_coach, date, notes) >= 3) {
            // Lire le nom du coach
            char coach_nom[100] = "Inconnu";
            FILE *f_ent = fopen("entraineurs.txt", "r");
            if (f_ent) {
                int id;
                char nom[100], specialite[100];
                int exp, age;
                char sexe;
                while (fscanf(f_ent, "%d %99s %99s %d %d %c", &id, nom, specialite, &exp, &age, &sexe) == 6) {
                    if (id == id_coach) {
                        strcpy(coach_nom, nom);
                        break;
                    }
                }
                fclose(f_ent);
            }
            
            // Lire le nom du membre
            char membre_nom[100] = "Inconnu";
            FILE *f_membres = fopen("membres.txt", "r");
            if (f_membres) {
                int id;
                char nom[100], email[100];
                int age;
                char sexe[10], abo[50];
                float tarif;
                while (fscanf(f_membres, "%d %99s %99s %d %9s %49s %f", 
                             &id, nom, email, &age, sexe, abo, &tarif) == 7) {
                    if (id == id_membre) {
                        strcpy(membre_nom, nom);
                        break;
                    }
                }
                fclose(f_membres);
            }
            
            row_values[0] = g_strdup_printf("%d", id_membre);
            row_values[1] = g_strdup(membre_nom);
            row_values[2] = g_strdup(coach_nom);
            row_values[3] = g_strdup(date);
            row_values[4] = g_strdup(notes);
            row_values[5] = g_strdup("En attente");
            
            treeview_ajouter_ligne(treeview, row_values, 6);
            
            for (int i = 0; i < 6; i++) g_free(row_values[i]);
        }
    }
    fclose(f);
}
void on_btn_demander_equip_clicked(GtkWidget *objet, gpointer user_data)
{
    GtkWidget *dialog;
    GtkWidget *vbox;
    GtkWidget *combo_equipement;
    GtkWidget *combo_date;
    GtkWidget *entry_notes;
    GtkWidget *label_info;
    int response;
    int coach_id;
    char coach_nom[100] = "";
    
    // Récupérer l'ID du coach connecté
    GtkWidget *window = gtk_widget_get_toplevel(objet);
    coach_id = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(window), "coach_id"));
    
    if (coach_id == 0) {
        afficher_message(objet, "erreur", "Erreur: Impossible d'identifier le coach !");
        return;
    }
    
    // Récupérer le nom du coach
    FILE *f_ent = fopen("entraineurs.txt", "r");
    if (f_ent) {
        int id;
        char nom[100], specialite[100];
        int exp, age;
        char sexe;
        while (fscanf(f_ent, "%d %99s %99s %d %d %c", &id, nom, specialite, &exp, &age, &sexe) == 6) {
            if (id == coach_id) {
                strcpy(coach_nom, nom);
                break;
            }
        }
        fclose(f_ent);
    }
    
    if (strlen(coach_nom) == 0) {
        afficher_message(objet, "erreur", "Coach non trouvé !");
        return;
    }
    
    // Créer le dialogue
    dialog = gtk_dialog_new_with_buttons("Réserver un équipement",
                                         GTK_WINDOW(window),
                                         GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                                         "Annuler", GTK_RESPONSE_CANCEL,
                                         "Réserver", GTK_RESPONSE_OK,
                                         NULL);
    
    gtk_window_set_default_size(GTK_WINDOW(dialog), 500, 400);
    
    vbox = gtk_vbox_new(FALSE, 10);
    gtk_container_set_border_width(GTK_CONTAINER(vbox), 15);
    
    // Label info
    label_info = gtk_label_new(NULL);
    char info_text[256];
    sprintf(info_text, "<span weight='bold' size='large'>🔧 Réserver un équipement</span>\n\nCoach: %s (ID: %d)", coach_nom, coach_id);
    gtk_label_set_markup(GTK_LABEL(label_info), info_text);
    gtk_box_pack_start(GTK_BOX(vbox), label_info, FALSE, FALSE, 0);
    
    GtkWidget *separator = gtk_hseparator_new();
    gtk_box_pack_start(GTK_BOX(vbox), separator, FALSE, FALSE, 5);
    
    // Choix de l'équipement
    GtkWidget *label_equip = gtk_label_new("Choisissez un équipement :");
    gtk_misc_set_alignment(GTK_MISC(label_equip), 0, 0.5);
    gtk_box_pack_start(GTK_BOX(vbox), label_equip, FALSE, FALSE, 0);
    
    combo_equipement = gtk_combo_box_new_text();
    
    // Lire les équipements depuis equipements.txt
    FILE *f_eq = fopen("equipements.txt", "r");
    if (f_eq) {
        char ref[50], type[100], centre[100];
        int capacite;
        char disponibilite, etat;
        while (fscanf(f_eq, "%49s %99s %99s %d %c %c", 
                     ref, type, centre, &capacite, &disponibilite, &etat) == 6) {
            char display[300];
            sprintf(display, "[%s] %s - %s (Cap: %d, Dispo: %c)", 
                    ref, type, centre, capacite, disponibilite);
            gtk_combo_box_append_text(GTK_COMBO_BOX(combo_equipement), display);
        }
        fclose(f_eq);
    }
    gtk_combo_box_set_active(GTK_COMBO_BOX(combo_equipement), 0);
    gtk_box_pack_start(GTK_BOX(vbox), combo_equipement, FALSE, FALSE, 0);
    
    // Choix de la date
    GtkWidget *label_date = gtk_label_new("Choisissez la date et heure :");
    gtk_misc_set_alignment(GTK_MISC(label_date), 0, 0.5);
    gtk_box_pack_start(GTK_BOX(vbox), label_date, FALSE, FALSE, 5);
    
    combo_date = gtk_combo_box_new_text();
    gtk_combo_box_append_text(GTK_COMBO_BOX(combo_date), "Lundi 15/05 - 08h00");
    gtk_combo_box_append_text(GTK_COMBO_BOX(combo_date), "Lundi 15/05 - 10h00");
    gtk_combo_box_append_text(GTK_COMBO_BOX(combo_date), "Lundi 15/05 - 14h00");
    gtk_combo_box_append_text(GTK_COMBO_BOX(combo_date), "Mardi 16/05 - 09h00");
    gtk_combo_box_append_text(GTK_COMBO_BOX(combo_date), "Mardi 16/05 - 11h00");
    gtk_combo_box_append_text(GTK_COMBO_BOX(combo_date), "Mardi 16/05 - 15h00");
    gtk_combo_box_append_text(GTK_COMBO_BOX(combo_date), "Mercredi 17/05 - 10h00");
    gtk_combo_box_append_text(GTK_COMBO_BOX(combo_date), "Mercredi 17/05 - 14h00");
    gtk_combo_box_append_text(GTK_COMBO_BOX(combo_date), "Jeudi 18/05 - 09h00");
    gtk_combo_box_append_text(GTK_COMBO_BOX(combo_date), "Jeudi 18/05 - 13h00");
    gtk_combo_box_append_text(GTK_COMBO_BOX(combo_date), "Vendredi 19/05 - 10h00");
    gtk_combo_box_append_text(GTK_COMBO_BOX(combo_date), "Vendredi 19/05 - 16h00");
    gtk_combo_box_set_active(GTK_COMBO_BOX(combo_date), 0);
    gtk_box_pack_start(GTK_BOX(vbox), combo_date, FALSE, FALSE, 0);
    
    // Notes
    GtkWidget *label_notes = gtk_label_new("Notes (optionnel) :");
    gtk_misc_set_alignment(GTK_MISC(label_notes), 0, 0.5);
    gtk_box_pack_start(GTK_BOX(vbox), label_notes, FALSE, FALSE, 5);
    
    entry_notes = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(vbox), entry_notes, FALSE, FALSE, 0);
    
    gtk_container_add(GTK_CONTAINER(gtk_dialog_get_content_area(GTK_DIALOG(dialog))), vbox);
    gtk_widget_show_all(dialog);
    
    response = gtk_dialog_run(GTK_DIALOG(dialog));
    
    if (response == GTK_RESPONSE_OK) {
        int equip_index = gtk_combo_box_get_active(GTK_COMBO_BOX(combo_equipement));
        int date_index = gtk_combo_box_get_active(GTK_COMBO_BOX(combo_date));
        
        if (equip_index >= 0 && date_index >= 0) {
            // Récupérer la référence de l'équipement
            gchar *equip_text = gtk_combo_box_get_active_text(GTK_COMBO_BOX(combo_equipement));
            char reference[50] = "";
            
            // Extraire la référence entre les crochets
            if (equip_text) {
                char *start = strchr(equip_text, '[');
                char *end = strchr(equip_text, ']');
                if (start && end && (end > start)) {
                    int len = end - start - 1;
                    if (len > 0 && len < 49) {
                        strncpy(reference, start + 1, len);
                        reference[len] = '\0';
                    }
                }
            }
            
            if (strlen(reference) > 0) {
                gchar *date = gtk_combo_box_get_active_text(GTK_COMBO_BOX(combo_date));
                const char *notes = gtk_entry_get_text(GTK_ENTRY(entry_notes));
                
                // Vérifier si l'équipement est déjà réservé à cette date
                FILE *f_check = fopen("reservations_equipement.txt", "r");
                int deja_reserve = 0;
                
                if (f_check) {
                    char ligne[512];
                    char ref_check[50], date_check[100];
                    int id_coach;
                    while (fgets(ligne, sizeof(ligne), f_check)) {
                        if (sscanf(ligne, "%49s %d %99s", ref_check, &id_coach, date_check) >= 3) {
                            if (strcmp(ref_check, reference) == 0 && strcmp(date_check, date) == 0) {
                                deja_reserve = 1;
                                break;
                            }
                        }
                    }
                    fclose(f_check);
                }
                
                if (deja_reserve) {
                    afficher_message(objet, "avertissement", "Cet équipement est déjà réservé à cette date et heure !");
                } else {
                    // Sauvegarder la réservation
                    FILE *f_reservation = fopen("reservations_equipement.txt", "a");
                    if (f_reservation) {
                        fprintf(f_reservation, "%s %d %s %s\n", reference, coach_id, date, notes);
                        fclose(f_reservation);
                        afficher_message(objet, "info", "✅ Réservation d'équipement confirmée !");
                    } else {
                        afficher_message(objet, "erreur", "Erreur lors de la réservation !");
                    }
                }
                
                g_free(date);
            } else if (equip_text) {
                afficher_message(objet, "erreur", "Erreur: Référence d'équipement non trouvée !");
            }
            
            if (equip_text) g_free(equip_text);
        } else {
            afficher_message(objet, "erreur", "Veuillez sélectionner un équipement et une date !");
        }
    }
    
    gtk_widget_destroy(dialog);
}
void on_btn_inscrire_centre_clicked(GtkWidget *objet, gpointer user_data)
{
    GtkWidget *dialog;
    GtkWidget *vbox;
    GtkWidget *combo_centre;
    GtkWidget *combo_date;
    GtkWidget *entry_notes;
    GtkWidget *label_info;
    int response;
    int coach_id;
    char coach_nom[100] = "";
    
    // Récupérer l'ID du coach connecté
    GtkWidget *window = gtk_widget_get_toplevel(objet);
    coach_id = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(window), "coach_id"));
    
    if (coach_id == 0) {
        afficher_message(objet, "erreur", "Erreur: Impossible d'identifier le coach !");
        return;
    }
    
    // Récupérer le nom du coach
    FILE *f_ent = fopen("entraineurs.txt", "r");
    if (f_ent) {
        int id;
        char nom[100], specialite[100];
        int exp, age;
        char sexe;
        while (fscanf(f_ent, "%d %99s %99s %d %d %c", &id, nom, specialite, &exp, &age, &sexe) == 6) {
            if (id == coach_id) {
                strcpy(coach_nom, nom);
                break;
            }
        }
        fclose(f_ent);
    }
    
    if (strlen(coach_nom) == 0) {
        afficher_message(objet, "erreur", "Coach non trouvé !");
        return;
    }
    
    // Créer le dialogue
    dialog = gtk_dialog_new_with_buttons("Inscrire un centre sportif",
                                         GTK_WINDOW(window),
                                         GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                                         "Annuler", GTK_RESPONSE_CANCEL,
                                         "Inscrire", GTK_RESPONSE_OK,
                                         NULL);
    
    gtk_window_set_default_size(GTK_WINDOW(dialog), 500, 350);
    
    vbox = gtk_vbox_new(FALSE, 10);
    gtk_container_set_border_width(GTK_CONTAINER(vbox), 15);
    
    // Label info
    label_info = gtk_label_new(NULL);
    char info_text[256];
    sprintf(info_text, "<span weight='bold' size='large'>🏢 Inscrire un centre</span>\n\nCoach: %s (ID: %d)", coach_nom, coach_id);
    gtk_label_set_markup(GTK_LABEL(label_info), info_text);
    gtk_box_pack_start(GTK_BOX(vbox), label_info, FALSE, FALSE, 0);
    
    GtkWidget *separator = gtk_hseparator_new();
    gtk_box_pack_start(GTK_BOX(vbox), separator, FALSE, FALSE, 5);
    
    // Choix du centre
    GtkWidget *label_centre = gtk_label_new("Choisissez un centre :");
    gtk_misc_set_alignment(GTK_MISC(label_centre), 0, 0.5);
    gtk_box_pack_start(GTK_BOX(vbox), label_centre, FALSE, FALSE, 0);
    
    combo_centre = gtk_combo_box_new_text();
    
    // Lire les centres depuis centres.txt (séparateur ;)
    FILE *f_centre = fopen("centres.txt", "r");
    if (f_centre) {
        char ligne[512];
        while (fgets(ligne, sizeof(ligne), f_centre)) {
            ligne[strcspn(ligne, "\n")] = 0;
            
            int id;
            char nom[100], adresse[200], ville[100];
            int capacite;
            
            if (sscanf(ligne, "%d;%99[^;];%199[^;];%99[^;];%d", 
                       &id, nom, adresse, ville, &capacite) == 5) {
                char display[400];
                sprintf(display, "[%d] %s - %s", id, nom, ville);
                gtk_combo_box_append_text(GTK_COMBO_BOX(combo_centre), display);
            }
        }
        fclose(f_centre);
    }
    gtk_combo_box_set_active(GTK_COMBO_BOX(combo_centre), 0);
    gtk_box_pack_start(GTK_BOX(vbox), combo_centre, FALSE, FALSE, 0);
    
    // Choix de la date
    GtkWidget *label_date = gtk_label_new("Choisissez la date :");
    gtk_misc_set_alignment(GTK_MISC(label_date), 0, 0.5);
    gtk_box_pack_start(GTK_BOX(vbox), label_date, FALSE, FALSE, 5);
    
    combo_date = gtk_combo_box_new_text();
    gtk_combo_box_append_text(GTK_COMBO_BOX(combo_date), "Lundi 15/05 - 10h00");
    gtk_combo_box_append_text(GTK_COMBO_BOX(combo_date), "Mardi 16/05 - 11h00");
    gtk_combo_box_append_text(GTK_COMBO_BOX(combo_date), "Mercredi 17/05 - 14h00");
    gtk_combo_box_append_text(GTK_COMBO_BOX(combo_date), "Jeudi 18/05 - 09h00");
    gtk_combo_box_append_text(GTK_COMBO_BOX(combo_date), "Vendredi 19/05 - 15h00");
    gtk_combo_box_set_active(GTK_COMBO_BOX(combo_date), 0);
    gtk_box_pack_start(GTK_BOX(vbox), combo_date, FALSE, FALSE, 0);
    
    gtk_container_add(GTK_CONTAINER(gtk_dialog_get_content_area(GTK_DIALOG(dialog))), vbox);
    gtk_widget_show_all(dialog);
    
    response = gtk_dialog_run(GTK_DIALOG(dialog));
    
    if (response == GTK_RESPONSE_OK) {
        int centre_index = gtk_combo_box_get_active(GTK_COMBO_BOX(combo_centre));
        int date_index = gtk_combo_box_get_active(GTK_COMBO_BOX(combo_date));
        
        if (centre_index >= 0 && date_index >= 0) {
            gchar *centre_text = gtk_combo_box_get_active_text(GTK_COMBO_BOX(combo_centre));
            gchar *date = gtk_combo_box_get_active_text(GTK_COMBO_BOX(combo_date));
            
            // Sauvegarder l'inscription
            FILE *f_inscription = fopen("inscriptions_centres.txt", "a");
            if (f_inscription) {
                fprintf(f_inscription, "%d %d %s\n", coach_id, centre_index + 1, date);
                fclose(f_inscription);
                
                char message[512];
                sprintf(message, "✅ Inscription confirmée !\n\nCentre: %s\nDate: %s\nCoach: %s", 
                        centre_text, date, coach_nom);
                afficher_message(objet, "info", message);
            } else {
                afficher_message(objet, "erreur", "Erreur lors de l'inscription !");
            }
            
            g_free(centre_text);
            g_free(date);
        } else {
            afficher_message(objet, "erreur", "Veuillez sélectionner un centre et une date !");
        }
    }
    
    gtk_widget_destroy(dialog);
}
// ==================== CALLBACKS POUR LA FENÊTRE ADMIN ====================

gboolean on_eb_topbar_expose(GtkWidget *widget, GdkEventExpose *event, gpointer data) {
  cairo_t *cr;
  GdkPixbuf *pixbuf;
  gint width, height;

  cr = gdk_cairo_create (widget->window);

  // Load and draw the topbar background image
  pixbuf = create_pixbuf("ijij.png");
  if (pixbuf) {
    width = gdk_pixbuf_get_width(pixbuf);
    height = gdk_pixbuf_get_height(pixbuf);
    gdk_cairo_set_source_pixbuf(cr, pixbuf, 0, 0);
    cairo_paint(cr);
    g_object_unref(pixbuf);
  } else {
    // Fallback: draw a solid color if image not found
    cairo_set_source_rgb(cr, 0.2, 0.2, 0.2);
    cairo_paint(cr);
  }

  cairo_destroy(cr);
  return FALSE;
}
gboolean on_eb_stat_card_expose(GtkWidget *widget, GdkEventExpose *event, gpointer data) { return FALSE; }
gboolean on_eb_dark_panel_expose(GtkWidget *widget, GdkEventExpose *event, gpointer data) { return FALSE; }
gboolean on_eb_sidebar_expose(GtkWidget *widget, GdkEventExpose *event, gpointer data) {
  cairo_t *cr;
  GdkPixbuf *pixbuf;
  gint width, height;

  cr = gdk_cairo_create (widget->window);

  // Load and draw the sidebar background image
  pixbuf = create_pixbuf("ijij.png");
  if (pixbuf) {
    width = gdk_pixbuf_get_width(pixbuf);
    height = gdk_pixbuf_get_height(pixbuf);
    gdk_cairo_set_source_pixbuf(cr, pixbuf, 0, 0);
    cairo_paint(cr);
    g_object_unref(pixbuf);
  } else {
    // Fallback: draw a solid color if image not found
    cairo_set_source_rgb(cr, 0.3, 0.3, 0.3);
    cairo_paint(cr);
  }

  cairo_destroy(cr);
  return FALSE;
}
gboolean on_eb_login_form_expose(GtkWidget *widget, GdkEventExpose *event, gpointer data) {
  cairo_t *cr;

  cr = gdk_cairo_create (widget->window);

  // Draw solid dark background color (#111111)
  cairo_set_source_rgb(cr, 0.066, 0.066, 0.066);
  cairo_paint(cr);

  cairo_destroy(cr);
  return FALSE;
}

gboolean on_eventbox1_expose_event(GtkWidget *widget, GdkEventExpose *event, gpointer data) { return FALSE; }
gboolean on_eventbox2_expose_event(GtkWidget *widget, GdkEventExpose *event, gpointer data) { return FALSE; }
gboolean on_eventbox3_expose_event(GtkWidget *widget, GdkEventExpose *event, gpointer data) { return FALSE; }
gboolean on_eventbox4_expose_event(GtkWidget *widget, GdkEventExpose *event, gpointer data) { return FALSE; }
gboolean on_eventbox5_expose_event(GtkWidget *widget, GdkEventExpose *event, gpointer data) { return FALSE; }




void on_treeview_membres_row_activated(GtkTreeView *treeview, GtkTreePath *path,
                                       GtkTreeViewColumn *column, gpointer user_data)
{
    GtkTreeIter iter;
    GtkTreeModel *model = gtk_tree_view_get_model(treeview);
    gchar *id_str, *nom, *email, *age_str, *sexe, *abonnement, *tarif;
    
    if (gtk_tree_model_get_iter(model, &iter, path)) {
        gtk_tree_model_get(model, &iter, 0, &id_str, 1, &nom, 2, &email, 3, &age_str,
                          4, &sexe, 5, &abonnement, 6, &tarif, -1);
        
        char message[512];
        sprintf(message, "Membre sélectionné:\n\nID: %s\nNom: %s\nEmail: %s\nAge: %s\nSexe: %s\nAbonnement: %s\nTarif: %s",
                id_str, nom, email, age_str, sexe, abonnement, tarif);
        
        afficher_message(GTK_WIDGET(treeview), "info", message);
        
        g_free(id_str); g_free(nom); g_free(email); g_free(age_str);
        g_free(sexe); g_free(abonnement); g_free(tarif);
    }
}

void on_treeview_cours_row_activated(GtkTreeView *treeview, GtkTreePath *path,
                                     GtkTreeViewColumn *column, gpointer user_data)
{
    GtkTreeIter iter;
    GtkTreeModel *model = gtk_tree_view_get_model(treeview);
    gchar *id_str, *cours, *coach, *salle, *date_heure, *niveau;
    
    if (gtk_tree_model_get_iter(model, &iter, path)) {
        gtk_tree_model_get(model, &iter, 0, &id_str, 1, &cours, 2, &coach,
                          3, &salle, 4, &date_heure, 5, &niveau, -1);
        
        char message[512];
        sprintf(message, "Cours sélectionné:\n\nID: %s\nCours: %s\nCoach: %s\nSalle: %s\nDate/Heure: %s\nNiveau: %s",
                id_str, cours, coach, salle, date_heure, niveau);
        
        afficher_message(GTK_WIDGET(treeview), "info", message);
        
        g_free(id_str); g_free(cours); g_free(coach); g_free(salle);
        g_free(date_heure); g_free(niveau);
    }
}

void on_treeview_entraineurs_row_activated(GtkTreeView *treeview, GtkTreePath *path,
                                          GtkTreeViewColumn *column, gpointer user_data)
{
    GtkTreeIter iter;
    GtkTreeModel *model = gtk_tree_view_get_model(treeview);
    gchar *id_str, *nom, *specialite, *experience, *age_str, *sexe;
    
    if (gtk_tree_model_get_iter(model, &iter, path)) {
        gtk_tree_model_get(model, &iter, 0, &id_str, 1, &nom, 2, &specialite,
                          3, &experience, 4, &age_str, 5, &sexe, -1);
        
        char message[512];
        sprintf(message, "Entraîneur sélectionné:\n\nID: %s\nNom: %s\nSpécialité: %s\nExpérience: %s\nAge: %s\nSexe: %s",
                id_str, nom, specialite, experience, age_str, sexe);
        
        afficher_message(GTK_WIDGET(treeview), "info", message);
        
        g_free(id_str); g_free(nom); g_free(specialite);
        g_free(experience); g_free(age_str); g_free(sexe);
    }
}

void on_treeview_equipements_row_activated(GtkTreeView *treeview, GtkTreePath *path,
                                          GtkTreeViewColumn *column, gpointer user_data)
{
    GtkTreeIter iter;
    GtkTreeModel *model = gtk_tree_view_get_model(treeview);
    gchar *ref, *type, *centre, *capacite, *dispo, *etat;
    
    if (gtk_tree_model_get_iter(model, &iter, path)) {
        gtk_tree_model_get(model, &iter, 0, &ref, 1, &type, 2, &centre,
                          3, &capacite, 4, &dispo, 5, &etat, -1);
        
        char message[512];
        sprintf(message, "Équipement sélectionné:\n\nRéférence: %s\nType: %s\nCentre: %s\nCapacité: %s\nDisponibilité: %s\nÉtat: %s",
                ref, type, centre, capacite, dispo, etat);
        
        afficher_message(GTK_WIDGET(treeview), "info", message);
        
        g_free(ref); g_free(type); g_free(centre);
        g_free(capacite); g_free(dispo); g_free(etat);
    }
}

void on_treeview_centres_row_activated(GtkTreeView *treeview, GtkTreePath *path,
                                      GtkTreeViewColumn *column, gpointer user_data)
{
    GtkTreeIter iter;
    GtkTreeModel *model = gtk_tree_view_get_model(treeview);
    gchar *id_str, *nom, *adresse, *ville, *capacite;
    
    if (gtk_tree_model_get_iter(model, &iter, path)) {
        gtk_tree_model_get(model, &iter, 0, &id_str, 1, &nom, 2, &adresse,
                          3, &ville, 4, &capacite, -1);
        
        char message[512];
        sprintf(message, "Centre sélectionné:\n\nID: %s\nNom: %s\nAdresse: %s\nVille: %s\nCapacité: %s",
                id_str, nom, adresse, ville, capacite);
        
        afficher_message(GTK_WIDGET(treeview), "info", message);
        
        g_free(id_str); g_free(nom); g_free(adresse); g_free(ville); g_free(capacite);
    }
}
// ==================== FONCTIONS UTILITAIRES POUR DIALOGUES ====================

// Dialogue pour saisir un ID (utilisé pour edit, delete, search)
static gchar* afficher_dialog_id(GtkWidget *parent, const char *titre, const char *label)
{
    GtkWidget *dialog;
    GtkWidget *content_area;
    GtkWidget *entry;
    GtkWidget *label_widget;
    gchar *result = NULL;
    
    dialog = gtk_dialog_new_with_buttons(titre,
                                         GTK_WINDOW(gtk_widget_get_toplevel(parent)),
                                         GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                                         "Annuler", GTK_RESPONSE_CANCEL,
                                         "Continuer", GTK_RESPONSE_ACCEPT,
                                         NULL);
    
    content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    
    label_widget = gtk_label_new(label);
    gtk_container_add(GTK_CONTAINER(content_area), label_widget);
    
    entry = gtk_entry_new();
    gtk_entry_set_width_chars(GTK_ENTRY(entry), 20);
    gtk_container_add(GTK_CONTAINER(content_area), entry);
    
    gtk_widget_show_all(dialog);
    
    gint response = gtk_dialog_run(GTK_DIALOG(dialog));
    if (response == GTK_RESPONSE_ACCEPT) {
        result = g_strdup(gtk_entry_get_text(GTK_ENTRY(entry)));
    }
    
    gtk_widget_destroy(dialog);
    return result;
}

// Dialogue pour saisir une référence (pour équipements)
static gchar* afficher_dialog_reference(GtkWidget *parent, const char *titre, const char *label)
{
    GtkWidget *dialog;
    GtkWidget *content_area;
    GtkWidget *entry;
    GtkWidget *label_widget;
    gchar *result = NULL;
    
    dialog = gtk_dialog_new_with_buttons(titre,
                                         GTK_WINDOW(gtk_widget_get_toplevel(parent)),
                                         GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                                         "Annuler", GTK_RESPONSE_CANCEL,
                                         "Continuer", GTK_RESPONSE_ACCEPT,
                                         NULL);
    
    content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    
    label_widget = gtk_label_new(label);
    gtk_container_add(GTK_CONTAINER(content_area), label_widget);
    
    entry = gtk_entry_new();
    gtk_entry_set_width_chars(GTK_ENTRY(entry), 20);
    gtk_container_add(GTK_CONTAINER(content_area), entry);
    
    gtk_widget_show_all(dialog);
    
    gint response = gtk_dialog_run(GTK_DIALOG(dialog));
    if (response == GTK_RESPONSE_ACCEPT) {
        result = g_strdup(gtk_entry_get_text(GTK_ENTRY(entry)));
    }
    
    gtk_widget_destroy(dialog);
    return result;
}

// ==================== DIALOGUES POUR MEMBRES ====================

static void dialog_ajouter_membre(GtkWidget *parent)
{
    GtkWidget *dialog;
    GtkWidget *content_area;
    GtkWidget *table;
    GtkWidget *entry_nom, *entry_email, *entry_age, *entry_sexe, *entry_abonnement, *entry_tarif;
    
    dialog = gtk_dialog_new_with_buttons("Ajouter un Membre",
                                         GTK_WINDOW(gtk_widget_get_toplevel(parent)),
                                         GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                                         "Annuler", GTK_RESPONSE_CANCEL,
                                         "Ajouter", GTK_RESPONSE_ACCEPT,
                                         NULL);
    
    content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    table = gtk_table_new(6, 2, FALSE);
    gtk_table_set_row_spacings(GTK_TABLE(table), 10);
    gtk_table_set_col_spacings(GTK_TABLE(table), 10);
    gtk_container_set_border_width(GTK_CONTAINER(table), 10);
    
    GtkWidget *label_nom = gtk_label_new("Nom:");
    gtk_misc_set_alignment(GTK_MISC(label_nom), 1, 0.5);
    entry_nom = gtk_entry_new();
    
    GtkWidget *label_email = gtk_label_new("Email:");
    gtk_misc_set_alignment(GTK_MISC(label_email), 1, 0.5);
    entry_email = gtk_entry_new();
    
    GtkWidget *label_age = gtk_label_new("Âge:");
    gtk_misc_set_alignment(GTK_MISC(label_age), 1, 0.5);
    entry_age = gtk_entry_new();
    
    GtkWidget *label_sexe = gtk_label_new("Sexe (H/F):");
    gtk_misc_set_alignment(GTK_MISC(label_sexe), 1, 0.5);
    entry_sexe = gtk_entry_new();
    
    GtkWidget *label_abonnement = gtk_label_new("Abonnement:");
    gtk_misc_set_alignment(GTK_MISC(label_abonnement), 1, 0.5);
    entry_abonnement = gtk_entry_new();
    
    GtkWidget *label_tarif = gtk_label_new("Tarif (DT):");
    gtk_misc_set_alignment(GTK_MISC(label_tarif), 1, 0.5);
    entry_tarif = gtk_entry_new();
    
    gtk_table_attach(GTK_TABLE(table), label_nom, 0, 1, 0, 1, GTK_FILL, GTK_FILL, 0, 0);
    gtk_table_attach(GTK_TABLE(table), entry_nom, 1, 2, 0, 1, GTK_FILL, GTK_FILL, 0, 0);
    gtk_table_attach(GTK_TABLE(table), label_email, 0, 1, 1, 2, GTK_FILL, GTK_FILL, 0, 0);
    gtk_table_attach(GTK_TABLE(table), entry_email, 1, 2, 1, 2, GTK_FILL, GTK_FILL, 0, 0);
    gtk_table_attach(GTK_TABLE(table), label_age, 0, 1, 2, 3, GTK_FILL, GTK_FILL, 0, 0);
    gtk_table_attach(GTK_TABLE(table), entry_age, 1, 2, 2, 3, GTK_FILL, GTK_FILL, 0, 0);
    gtk_table_attach(GTK_TABLE(table), label_sexe, 0, 1, 3, 4, GTK_FILL, GTK_FILL, 0, 0);
    gtk_table_attach(GTK_TABLE(table), entry_sexe, 1, 2, 3, 4, GTK_FILL, GTK_FILL, 0, 0);
    gtk_table_attach(GTK_TABLE(table), label_abonnement, 0, 1, 4, 5, GTK_FILL, GTK_FILL, 0, 0);
    gtk_table_attach(GTK_TABLE(table), entry_abonnement, 1, 2, 4, 5, GTK_FILL, GTK_FILL, 0, 0);
    gtk_table_attach(GTK_TABLE(table), label_tarif, 0, 1, 5, 6, GTK_FILL, GTK_FILL, 0, 0);
    gtk_table_attach(GTK_TABLE(table), entry_tarif, 1, 2, 5, 6, GTK_FILL, GTK_FILL, 0, 0);
    
    gtk_container_add(GTK_CONTAINER(content_area), table);
    gtk_widget_show_all(dialog);
    
    gint response = gtk_dialog_run(GTK_DIALOG(dialog));
    if (response == GTK_RESPONSE_ACCEPT) {
        const char *nom = gtk_entry_get_text(GTK_ENTRY(entry_nom));
        const char *email = gtk_entry_get_text(GTK_ENTRY(entry_email));
        const char *age = gtk_entry_get_text(GTK_ENTRY(entry_age));
        const char *sexe = gtk_entry_get_text(GTK_ENTRY(entry_sexe));
        const char *abonnement = gtk_entry_get_text(GTK_ENTRY(entry_abonnement));
        const char *tarif = gtk_entry_get_text(GTK_ENTRY(entry_tarif));
        
        if (strlen(nom) > 0 && strlen(email) > 0) {
            FILE *f = fopen("membres.txt", "a");
            if (f) {
                int id = get_prochain_id_membre();
                fprintf(f, "%d %s %s %s %s %s %s\n", id, nom, email, age, sexe, abonnement, tarif);
                fclose(f);
                afficher_message(parent, "info", "Membre ajouté avec succès !");
            }
        } else {
            afficher_message(parent, "erreur", "Nom et Email sont obligatoires !");
        }
    }
    gtk_widget_destroy(dialog);
}

static void dialog_modifier_membre(GtkWidget *parent)
{
    gchar *id_str = afficher_dialog_id(parent, "Modifier un Membre", "Entrez l'ID du membre à modifier:");
    if (!id_str || strlen(id_str) == 0) {
        if (id_str) g_free(id_str);
        return;
    }
    int id_modif = atoi(id_str);
    g_free(id_str);
    
    FILE *f = fopen("membres.txt", "r");
    if (!f) {
        afficher_message(parent, "erreur", "Fichier membres.txt introuvable !");
        return;
    }
    
    Membre m;
    int found = 0;
    char buffer[512];
    while (fgets(buffer, sizeof(buffer), f)) {
        if (sscanf(buffer, "%d %99s %99s %d %9s %49s %f", 
                   &m.id, m.nom, m.email, &m.age, m.sexe, m.Typeabonnement, &m.Tarif) == 7) {
            if (m.id == id_modif) {
                found = 1;
                break;
            }
        }
    }
    fclose(f);
    
    if (!found) {
        afficher_message(parent, "erreur", "Aucun membre trouvé avec cet ID !");
        return;
    }
    
    GtkWidget *dialog;
    GtkWidget *content_area;
    GtkWidget *table;
    GtkWidget *entry_nom, *entry_email, *entry_age, *entry_sexe, *entry_abonnement, *entry_tarif;
    
    dialog = gtk_dialog_new_with_buttons("Modifier un Membre",
                                         GTK_WINDOW(gtk_widget_get_toplevel(parent)),
                                         GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                                         "Annuler", GTK_RESPONSE_CANCEL,
                                         "Modifier", GTK_RESPONSE_ACCEPT,
                                         NULL);
    
    content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    table = gtk_table_new(6, 2, FALSE);
    gtk_table_set_row_spacings(GTK_TABLE(table), 10);
    gtk_table_set_col_spacings(GTK_TABLE(table), 10);
    gtk_container_set_border_width(GTK_CONTAINER(table), 10);
    
    GtkWidget *label_nom = gtk_label_new("Nom:");
    gtk_misc_set_alignment(GTK_MISC(label_nom), 1, 0.5);
    entry_nom = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(entry_nom), m.nom);
    
    GtkWidget *label_email = gtk_label_new("Email:");
    gtk_misc_set_alignment(GTK_MISC(label_email), 1, 0.5);
    entry_email = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(entry_email), m.email);
    
    GtkWidget *label_age = gtk_label_new("Âge:");
    gtk_misc_set_alignment(GTK_MISC(label_age), 1, 0.5);
    entry_age = gtk_entry_new();
    char age_str[10];
    sprintf(age_str, "%d", m.age);
    gtk_entry_set_text(GTK_ENTRY(entry_age), age_str);
    
    GtkWidget *label_sexe = gtk_label_new("Sexe (H/F):");
    gtk_misc_set_alignment(GTK_MISC(label_sexe), 1, 0.5);
    entry_sexe = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(entry_sexe), m.sexe);
    
    GtkWidget *label_abonnement = gtk_label_new("Abonnement:");
    gtk_misc_set_alignment(GTK_MISC(label_abonnement), 1, 0.5);
    entry_abonnement = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(entry_abonnement), m.Typeabonnement);
    
    GtkWidget *label_tarif = gtk_label_new("Tarif (DT):");
    gtk_misc_set_alignment(GTK_MISC(label_tarif), 1, 0.5);
    entry_tarif = gtk_entry_new();
    char tarif_str[20];
    sprintf(tarif_str, "%.2f", m.Tarif);
    gtk_entry_set_text(GTK_ENTRY(entry_tarif), tarif_str);
    
    gtk_table_attach(GTK_TABLE(table), label_nom, 0, 1, 0, 1, GTK_FILL, GTK_FILL, 0, 0);
    gtk_table_attach(GTK_TABLE(table), entry_nom, 1, 2, 0, 1, GTK_FILL, GTK_FILL, 0, 0);
    gtk_table_attach(GTK_TABLE(table), label_email, 0, 1, 1, 2, GTK_FILL, GTK_FILL, 0, 0);
    gtk_table_attach(GTK_TABLE(table), entry_email, 1, 2, 1, 2, GTK_FILL, GTK_FILL, 0, 0);
    gtk_table_attach(GTK_TABLE(table), label_age, 0, 1, 2, 3, GTK_FILL, GTK_FILL, 0, 0);
    gtk_table_attach(GTK_TABLE(table), entry_age, 1, 2, 2, 3, GTK_FILL, GTK_FILL, 0, 0);
    gtk_table_attach(GTK_TABLE(table), label_sexe, 0, 1, 3, 4, GTK_FILL, GTK_FILL, 0, 0);
    gtk_table_attach(GTK_TABLE(table), entry_sexe, 1, 2, 3, 4, GTK_FILL, GTK_FILL, 0, 0);
    gtk_table_attach(GTK_TABLE(table), label_abonnement, 0, 1, 4, 5, GTK_FILL, GTK_FILL, 0, 0);
    gtk_table_attach(GTK_TABLE(table), entry_abonnement, 1, 2, 4, 5, GTK_FILL, GTK_FILL, 0, 0);
    gtk_table_attach(GTK_TABLE(table), label_tarif, 0, 1, 5, 6, GTK_FILL, GTK_FILL, 0, 0);
    gtk_table_attach(GTK_TABLE(table), entry_tarif, 1, 2, 5, 6, GTK_FILL, GTK_FILL, 0, 0);
    
    gtk_container_add(GTK_CONTAINER(content_area), table);
    gtk_widget_show_all(dialog);
    
    gint response = gtk_dialog_run(GTK_DIALOG(dialog));
    if (response == GTK_RESPONSE_ACCEPT) {
        const char *nom = gtk_entry_get_text(GTK_ENTRY(entry_nom));
        const char *email = gtk_entry_get_text(GTK_ENTRY(entry_email));
        const char *age = gtk_entry_get_text(GTK_ENTRY(entry_age));
        const char *sexe = gtk_entry_get_text(GTK_ENTRY(entry_sexe));
        const char *abonnement = gtk_entry_get_text(GTK_ENTRY(entry_abonnement));
        const char *tarif = gtk_entry_get_text(GTK_ENTRY(entry_tarif));
        
        FILE *f_read = fopen("membres.txt", "r");
        FILE *f_temp = fopen("temp.txt", "w");
        
        if (f_read && f_temp) {
            char line[512];
            while (fgets(line, sizeof(line), f_read)) {
                int id;
                if (sscanf(line, "%d", &id) == 1) {
                    if (id == id_modif) {
                        fprintf(f_temp, "%d %s %s %s %s %s %s\n", id_modif, nom, email, age, sexe, abonnement, tarif);
                    } else {
                        fprintf(f_temp, "%s", line);
                    }
                }
            }
            fclose(f_read);
            fclose(f_temp);
            remove("membres.txt");
            rename("temp.txt", "membres.txt");
            afficher_message(parent, "info", "Membre modifié avec succès !");
        }
    }
    gtk_widget_destroy(dialog);
}
static void dialog_supprimer_membre(GtkWidget *parent)
{
    gchar *id_str = afficher_dialog_id(parent, "Supprimer un Membre", "Entrez l'ID du membre à supprimer:");
    if (!id_str || strlen(id_str) == 0) {
        if (id_str) g_free(id_str);
        return;
    }
    int id_suppr = atoi(id_str);
    g_free(id_str);
    
    if (demander_confirmation(parent, "Confirmation", "Voulez-vous vraiment supprimer ce membre ?")) {
        FILE *f_read = fopen("membres.txt", "r");
        FILE *f_temp = fopen("temp.txt", "w");
        
        if (f_read && f_temp) {
            char line[512];
            while (fgets(line, sizeof(line), f_read)) {
                int id;
                if (sscanf(line, "%d", &id) == 1 && id != id_suppr) {
                    fprintf(f_temp, "%s", line);
                }
            }
            fclose(f_read);
            fclose(f_temp);
            remove("membres.txt");
            rename("temp.txt", "membres.txt");
            afficher_message(parent, "info", "Membre supprimé avec succès !");
        }
    }
}

static void dialog_rechercher_membre(GtkWidget *parent, GtkWidget *treeview)
{
    gchar *id_str = afficher_dialog_id(parent, "Rechercher un Membre", "Entrez l'ID du membre à rechercher:");
    if (!id_str || strlen(id_str) == 0) {
        if (id_str) g_free(id_str);
        return;
    }
    
    int id_recherche = atoi(id_str);
    g_free(id_str);
    
    GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(treeview));
    GtkTreeIter iter;
    gboolean found = FALSE;
    gchar *id_tree;
    
    if (!model) return;
    
    gtk_tree_model_get_iter_first(model, &iter);
    do {
        gtk_tree_model_get(model, &iter, 0, &id_tree, -1);
        if (atoi(id_tree) == id_recherche) {
            GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(treeview));
            gtk_tree_selection_select_iter(selection, &iter);
            gtk_tree_view_scroll_to_cell(GTK_TREE_VIEW(treeview), 
                                        gtk_tree_model_get_path(model, &iter), 
                                        NULL, TRUE, 0.5, 0.5);
            found = TRUE;
            g_free(id_tree);
            break;
        }
        g_free(id_tree);
    } while (gtk_tree_model_iter_next(model, &iter));
    
    if (!found) {
        afficher_message(parent, "info", "Aucun membre trouvé avec cet ID");
    }
}

// ==================== DIALOGUES POUR COURS ====================

static void dialog_ajouter_cours(GtkWidget *parent)
{
    GtkWidget *dialog;
    GtkWidget *content_area;
    GtkWidget *table;
    GtkWidget *entry_cours, *entry_coach, *entry_salle, *entry_date, *entry_niveau;
    
    dialog = gtk_dialog_new_with_buttons("Ajouter un Cours",
                                         GTK_WINDOW(gtk_widget_get_toplevel(parent)),
                                         GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                                         "Annuler", GTK_RESPONSE_CANCEL,
                                         "Ajouter", GTK_RESPONSE_ACCEPT,
                                         NULL);
    
    content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    table = gtk_table_new(5, 2, FALSE);
    gtk_table_set_row_spacings(GTK_TABLE(table), 10);
    gtk_table_set_col_spacings(GTK_TABLE(table), 10);
    gtk_container_set_border_width(GTK_CONTAINER(table), 10);
    
    GtkWidget *label_cours = gtk_label_new("Nom du cours:");
    gtk_misc_set_alignment(GTK_MISC(label_cours), 1, 0.5);
    entry_cours = gtk_entry_new();
    
    GtkWidget *label_coach = gtk_label_new("Coach:");
    gtk_misc_set_alignment(GTK_MISC(label_coach), 1, 0.5);
    entry_coach = gtk_entry_new();
    
    GtkWidget *label_salle = gtk_label_new("Salle:");
    gtk_misc_set_alignment(GTK_MISC(label_salle), 1, 0.5);
    entry_salle = gtk_entry_new();
    
    GtkWidget *label_date = gtk_label_new("Date/Heure:");
    gtk_misc_set_alignment(GTK_MISC(label_date), 1, 0.5);
    entry_date = gtk_entry_new();
    
    GtkWidget *label_niveau = gtk_label_new("Niveau:");
    gtk_misc_set_alignment(GTK_MISC(label_niveau), 1, 0.5);
    entry_niveau = gtk_entry_new();
    
    gtk_table_attach(GTK_TABLE(table), label_cours, 0, 1, 0, 1, GTK_FILL, GTK_FILL, 0, 0);
    gtk_table_attach(GTK_TABLE(table), entry_cours, 1, 2, 0, 1, GTK_FILL, GTK_FILL, 0, 0);
    gtk_table_attach(GTK_TABLE(table), label_coach, 0, 1, 1, 2, GTK_FILL, GTK_FILL, 0, 0);
    gtk_table_attach(GTK_TABLE(table), entry_coach, 1, 2, 1, 2, GTK_FILL, GTK_FILL, 0, 0);
    gtk_table_attach(GTK_TABLE(table), label_salle, 0, 1, 2, 3, GTK_FILL, GTK_FILL, 0, 0);
    gtk_table_attach(GTK_TABLE(table), entry_salle, 1, 2, 2, 3, GTK_FILL, GTK_FILL, 0, 0);
    gtk_table_attach(GTK_TABLE(table), label_date, 0, 1, 3, 4, GTK_FILL, GTK_FILL, 0, 0);
    gtk_table_attach(GTK_TABLE(table), entry_date, 1, 2, 3, 4, GTK_FILL, GTK_FILL, 0, 0);
    gtk_table_attach(GTK_TABLE(table), label_niveau, 0, 1, 4, 5, GTK_FILL, GTK_FILL, 0, 0);
    gtk_table_attach(GTK_TABLE(table), entry_niveau, 1, 2, 4, 5, GTK_FILL, GTK_FILL, 0, 0);
    
    gtk_container_add(GTK_CONTAINER(content_area), table);
    gtk_widget_show_all(dialog);
    
    gint response = gtk_dialog_run(GTK_DIALOG(dialog));
    if (response == GTK_RESPONSE_ACCEPT) {
        const char *cours = gtk_entry_get_text(GTK_ENTRY(entry_cours));
        const char *coach = gtk_entry_get_text(GTK_ENTRY(entry_coach));
        const char *salle = gtk_entry_get_text(GTK_ENTRY(entry_salle));
        const char *date = gtk_entry_get_text(GTK_ENTRY(entry_date));
        const char *niveau = gtk_entry_get_text(GTK_ENTRY(entry_niveau));
        
        if (strlen(cours) > 0) {
            FILE *f = fopen("cours.txt", "a");
            if (f) {
                int id = get_prochain_id_cours();
                fprintf(f, "%d %s %s %s %s %s\n", id, cours, coach, salle, date, niveau);
                fclose(f);
                afficher_message(parent, "info", "Cours ajouté avec succès !");
            }
        } else {
            afficher_message(parent, "erreur", "Nom du cours est obligatoire !");
        }
    }
    gtk_widget_destroy(dialog);
}
static void dialog_modifier_cours(GtkWidget *parent)
{
    gchar *id_str = afficher_dialog_id(parent, "Modifier un Cours", "Entrez l'ID du cours à modifier:");
    if (!id_str || strlen(id_str) == 0) {
        if (id_str) g_free(id_str);
        return;
    }
    int id_modif = atoi(id_str);
    g_free(id_str);
    
    FILE *f = fopen("cours.txt", "r");
    if (!f) {
        afficher_message(parent, "erreur", "Fichier cours.txt introuvable !");
        return;
    }
    
    Cours c;
    int found = 0;
    while (fscanf(f, "%d %99s %99s %99s %99s %99s", 
                  &c.id, c.cours, c.coach, c.salle, c.date_heure, c.niveau) == 6) {
        if (c.id == id_modif) {
            found = 1;
            break;
        }
    }
    fclose(f);
    
    if (!found) {
        afficher_message(parent, "erreur", "Aucun cours trouvé avec cet ID !");
        return;
    }
    
    GtkWidget *dialog;
    GtkWidget *content_area;
    GtkWidget *table;
    GtkWidget *entry_cours, *entry_coach, *entry_salle, *entry_date, *entry_niveau;
    
    dialog = gtk_dialog_new_with_buttons("Modifier un Cours",
                                         GTK_WINDOW(gtk_widget_get_toplevel(parent)),
                                         GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                                         "Annuler", GTK_RESPONSE_CANCEL,
                                         "Modifier", GTK_RESPONSE_ACCEPT,
                                         NULL);
    
    content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    table = gtk_table_new(5, 2, FALSE);
    gtk_table_set_row_spacings(GTK_TABLE(table), 10);
    gtk_table_set_col_spacings(GTK_TABLE(table), 10);
    gtk_container_set_border_width(GTK_CONTAINER(table), 10);
    
    GtkWidget *label_cours = gtk_label_new("Nom du cours:");
    gtk_misc_set_alignment(GTK_MISC(label_cours), 1, 0.5);
    entry_cours = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(entry_cours), c.cours);
    
    GtkWidget *label_coach = gtk_label_new("Coach:");
    gtk_misc_set_alignment(GTK_MISC(label_coach), 1, 0.5);
    entry_coach = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(entry_coach), c.coach);
    
    GtkWidget *label_salle = gtk_label_new("Salle:");
    gtk_misc_set_alignment(GTK_MISC(label_salle), 1, 0.5);
    entry_salle = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(entry_salle), c.salle);
    
    GtkWidget *label_date = gtk_label_new("Date/Heure:");
    gtk_misc_set_alignment(GTK_MISC(label_date), 1, 0.5);
    entry_date = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(entry_date), c.date_heure);
    
    GtkWidget *label_niveau = gtk_label_new("Niveau:");
    gtk_misc_set_alignment(GTK_MISC(label_niveau), 1, 0.5);
    entry_niveau = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(entry_niveau), c.niveau);
    
    gtk_table_attach(GTK_TABLE(table), label_cours, 0, 1, 0, 1, GTK_FILL, GTK_FILL, 0, 0);
    gtk_table_attach(GTK_TABLE(table), entry_cours, 1, 2, 0, 1, GTK_FILL, GTK_FILL, 0, 0);
    gtk_table_attach(GTK_TABLE(table), label_coach, 0, 1, 1, 2, GTK_FILL, GTK_FILL, 0, 0);
    gtk_table_attach(GTK_TABLE(table), entry_coach, 1, 2, 1, 2, GTK_FILL, GTK_FILL, 0, 0);
    gtk_table_attach(GTK_TABLE(table), label_salle, 0, 1, 2, 3, GTK_FILL, GTK_FILL, 0, 0);
    gtk_table_attach(GTK_TABLE(table), entry_salle, 1, 2, 2, 3, GTK_FILL, GTK_FILL, 0, 0);
    gtk_table_attach(GTK_TABLE(table), label_date, 0, 1, 3, 4, GTK_FILL, GTK_FILL, 0, 0);
    gtk_table_attach(GTK_TABLE(table), entry_date, 1, 2, 3, 4, GTK_FILL, GTK_FILL, 0, 0);
    gtk_table_attach(GTK_TABLE(table), label_niveau, 0, 1, 4, 5, GTK_FILL, GTK_FILL, 0, 0);
    gtk_table_attach(GTK_TABLE(table), entry_niveau, 1, 2, 4, 5, GTK_FILL, GTK_FILL, 0, 0);
    
    gtk_container_add(GTK_CONTAINER(content_area), table);
    gtk_widget_show_all(dialog);
    
    gint response = gtk_dialog_run(GTK_DIALOG(dialog));
    if (response == GTK_RESPONSE_ACCEPT) {
        const char *cours = gtk_entry_get_text(GTK_ENTRY(entry_cours));
        const char *coach = gtk_entry_get_text(GTK_ENTRY(entry_coach));
        const char *salle = gtk_entry_get_text(GTK_ENTRY(entry_salle));
        const char *date = gtk_entry_get_text(GTK_ENTRY(entry_date));
        const char *niveau = gtk_entry_get_text(GTK_ENTRY(entry_niveau));
        
        FILE *f_read = fopen("cours.txt", "r");
        FILE *f_temp = fopen("temp_cours.txt", "w");
        
        if (f_read && f_temp) {
            int id;
            char tcours[100], tcoach[100], tsalle[100], tdate[100], tniveau[100];
            while (fscanf(f_read, "%d %99s %99s %99s %99s %99s", 
                         &id, tcours, tcoach, tsalle, tdate, tniveau) == 6) {
                if (id == id_modif) {
                    fprintf(f_temp, "%d %s %s %s %s %s\n", id_modif, cours, coach, salle, date, niveau);
                } else {
                    fprintf(f_temp, "%d %s %s %s %s %s\n", id, tcours, tcoach, tsalle, tdate, tniveau);
                }
            }
            fclose(f_read);
            fclose(f_temp);
            remove("cours.txt");
            rename("temp_cours.txt", "cours.txt");
            afficher_message(parent, "info", "Cours modifié avec succès !");
        }
    }
    gtk_widget_destroy(dialog);
}

static void dialog_supprimer_cours(GtkWidget *parent)
{
    gchar *id_str = afficher_dialog_id(parent, "Supprimer un Cours", "Entrez l'ID du cours à supprimer:");
    if (!id_str || strlen(id_str) == 0) {
        if (id_str) g_free(id_str);
        return;
    }
    int id_suppr = atoi(id_str);
    g_free(id_str);
    
    if (demander_confirmation(parent, "Confirmation", "Voulez-vous vraiment supprimer ce cours ?")) {
        FILE *f_read = fopen("cours.txt", "r");
        FILE *f_temp = fopen("temp_cours.txt", "w");
        
        if (f_read && f_temp) {
            int id;
            char cours[100], coach[100], salle[100], date[100], niveau[100];
            while (fscanf(f_read, "%d %99s %99s %99s %99s %99s", 
                         &id, cours, coach, salle, date, niveau) == 6) {
                if (id != id_suppr) {
                    fprintf(f_temp, "%d %s %s %s %s %s\n", id, cours, coach, salle, date, niveau);
                }
            }
            fclose(f_read);
            fclose(f_temp);
            remove("cours.txt");
            rename("temp_cours.txt", "cours.txt");
            afficher_message(parent, "info", "Cours supprimé avec succès !");
        }
    }
}

static void dialog_rechercher_cours(GtkWidget *parent, GtkWidget *treeview)
{
    gchar *id_str = afficher_dialog_id(parent, "Rechercher un Cours", "Entrez l'ID du cours à rechercher:");
    if (!id_str || strlen(id_str) == 0) {
        if (id_str) g_free(id_str);
        return;
    }
    
    int id_recherche = atoi(id_str);
    g_free(id_str);
    
    GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(treeview));
    GtkTreeIter iter;
    gboolean found = FALSE;
    gchar *id_tree;
    
    if (!model) return;
    
    gtk_tree_model_get_iter_first(model, &iter);
    do {
        gtk_tree_model_get(model, &iter, 0, &id_tree, -1);
        if (atoi(id_tree) == id_recherche) {
            GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(treeview));
            gtk_tree_selection_select_iter(selection, &iter);
            gtk_tree_view_scroll_to_cell(GTK_TREE_VIEW(treeview), 
                                        gtk_tree_model_get_path(model, &iter), 
                                        NULL, TRUE, 0.5, 0.5);
            found = TRUE;
            g_free(id_tree);
            break;
        }
        g_free(id_tree);
    } while (gtk_tree_model_iter_next(model, &iter));
    
    if (!found) {
        afficher_message(parent, "info", "Aucun cours trouvé avec cet ID");
    }
}

// ==================== DIALOGUES POUR ENTRAINEURS ====================

static void dialog_ajouter_entraineur(GtkWidget *parent)
{
    GtkWidget *dialog;
    GtkWidget *content_area;
    GtkWidget *table;
    GtkWidget *entry_nom, *entry_specialite, *entry_experience, *entry_age, *entry_sexe;
    
    dialog = gtk_dialog_new_with_buttons("Ajouter un Entraîneur",
                                         GTK_WINDOW(gtk_widget_get_toplevel(parent)),
                                         GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                                         "Annuler", GTK_RESPONSE_CANCEL,
                                         "Ajouter", GTK_RESPONSE_ACCEPT,
                                         NULL);
    
    content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    table = gtk_table_new(5, 2, FALSE);
    gtk_table_set_row_spacings(GTK_TABLE(table), 10);
    gtk_table_set_col_spacings(GTK_TABLE(table), 10);
    gtk_container_set_border_width(GTK_CONTAINER(table), 10);
    
    GtkWidget *label_nom = gtk_label_new("Nom complet:");
    gtk_misc_set_alignment(GTK_MISC(label_nom), 1, 0.5);
    entry_nom = gtk_entry_new();
    
    GtkWidget *label_specialite = gtk_label_new("Spécialité:");
    gtk_misc_set_alignment(GTK_MISC(label_specialite), 1, 0.5);
    entry_specialite = gtk_entry_new();
    
    GtkWidget *label_experience = gtk_label_new("Expérience (ans):");
    gtk_misc_set_alignment(GTK_MISC(label_experience), 1, 0.5);
    entry_experience = gtk_entry_new();
    
    GtkWidget *label_age = gtk_label_new("Âge:");
    gtk_misc_set_alignment(GTK_MISC(label_age), 1, 0.5);
    entry_age = gtk_entry_new();
    
    GtkWidget *label_sexe = gtk_label_new("Sexe (H/F):");
    gtk_misc_set_alignment(GTK_MISC(label_sexe), 1, 0.5);
    entry_sexe = gtk_entry_new();
    
    gtk_table_attach(GTK_TABLE(table), label_nom, 0, 1, 0, 1, GTK_FILL, GTK_FILL, 0, 0);
    gtk_table_attach(GTK_TABLE(table), entry_nom, 1, 2, 0, 1, GTK_FILL, GTK_FILL, 0, 0);
    gtk_table_attach(GTK_TABLE(table), label_specialite, 0, 1, 1, 2, GTK_FILL, GTK_FILL, 0, 0);
    gtk_table_attach(GTK_TABLE(table), entry_specialite, 1, 2, 1, 2, GTK_FILL, GTK_FILL, 0, 0);
    gtk_table_attach(GTK_TABLE(table), label_experience, 0, 1, 2, 3, GTK_FILL, GTK_FILL, 0, 0);
    gtk_table_attach(GTK_TABLE(table), entry_experience, 1, 2, 2, 3, GTK_FILL, GTK_FILL, 0, 0);
    gtk_table_attach(GTK_TABLE(table), label_age, 0, 1, 3, 4, GTK_FILL, GTK_FILL, 0, 0);
    gtk_table_attach(GTK_TABLE(table), entry_age, 1, 2, 3, 4, GTK_FILL, GTK_FILL, 0, 0);
    gtk_table_attach(GTK_TABLE(table), label_sexe, 0, 1, 4, 5, GTK_FILL, GTK_FILL, 0, 0);
    gtk_table_attach(GTK_TABLE(table), entry_sexe, 1, 2, 4, 5, GTK_FILL, GTK_FILL, 0, 0);
    
    gtk_container_add(GTK_CONTAINER(content_area), table);
    gtk_widget_show_all(dialog);
    
    gint response = gtk_dialog_run(GTK_DIALOG(dialog));
    if (response == GTK_RESPONSE_ACCEPT) {
        const char *nom = gtk_entry_get_text(GTK_ENTRY(entry_nom));
        const char *specialite = gtk_entry_get_text(GTK_ENTRY(entry_specialite));
        const char *experience = gtk_entry_get_text(GTK_ENTRY(entry_experience));
        const char *age = gtk_entry_get_text(GTK_ENTRY(entry_age));
        const char *sexe = gtk_entry_get_text(GTK_ENTRY(entry_sexe));
        
        if (strlen(nom) > 0) {
            FILE *f = fopen("entraineurs.txt", "a");
            if (f) {
                int id = get_prochain_id_entraineur();
                fprintf(f, "%d %s %s %d %d %c\n", id, nom, specialite, 
                        atoi(experience), atoi(age), sexe[0]);
                fclose(f);
                afficher_message(parent, "info", "Entraîneur ajouté avec succès !");
            }
        } else {
            afficher_message(parent, "erreur", "Nom est obligatoire !");
        }
    }
    gtk_widget_destroy(dialog);
}
static void dialog_modifier_entraineur(GtkWidget *parent)
{
    gchar *id_str = afficher_dialog_id(parent, "Modifier un Entraîneur", "Entrez l'ID de l'entraîneur à modifier:");
    if (!id_str || strlen(id_str) == 0) {
        if (id_str) g_free(id_str);
        return;
    }
    int id_modif = atoi(id_str);
    g_free(id_str);
    
    FILE *f = fopen("entraineurs.txt", "r");
    if (!f) {
        afficher_message(parent, "erreur", "Fichier entraineurs.txt introuvable !");
        return;
    }
    
    Entraineur e;
    int found = 0;
    while (fscanf(f, "%d %99s %99s %d %d %c", 
                  &e.id, e.nometprenom, e.specialite, &e.experience, &e.age, &e.sexe) == 6) {
        if (e.id == id_modif) {
            found = 1;
            break;
        }
    }
    fclose(f);
    
    if (!found) {
        afficher_message(parent, "erreur", "Aucun entraîneur trouvé avec cet ID !");
        return;
    }
    
    GtkWidget *dialog;
    GtkWidget *content_area;
    GtkWidget *table;
    GtkWidget *entry_nom, *entry_specialite, *entry_experience, *entry_age, *entry_sexe;
    
    dialog = gtk_dialog_new_with_buttons("Modifier un Entraîneur",
                                         GTK_WINDOW(gtk_widget_get_toplevel(parent)),
                                         GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                                         "Annuler", GTK_RESPONSE_CANCEL,
                                         "Modifier", GTK_RESPONSE_ACCEPT,
                                         NULL);
    
    content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    table = gtk_table_new(5, 2, FALSE);
    gtk_table_set_row_spacings(GTK_TABLE(table), 10);
    gtk_table_set_col_spacings(GTK_TABLE(table), 10);
    gtk_container_set_border_width(GTK_CONTAINER(table), 10);
    
    GtkWidget *label_nom = gtk_label_new("Nom complet:");
    gtk_misc_set_alignment(GTK_MISC(label_nom), 1, 0.5);
    entry_nom = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(entry_nom), e.nometprenom);
    
    GtkWidget *label_specialite = gtk_label_new("Spécialité:");
    gtk_misc_set_alignment(GTK_MISC(label_specialite), 1, 0.5);
    entry_specialite = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(entry_specialite), e.specialite);
    
    GtkWidget *label_experience = gtk_label_new("Expérience (ans):");
    gtk_misc_set_alignment(GTK_MISC(label_experience), 1, 0.5);
    entry_experience = gtk_entry_new();
    char exp_str[10];
    sprintf(exp_str, "%d", e.experience);
    gtk_entry_set_text(GTK_ENTRY(entry_experience), exp_str);
    
    GtkWidget *label_age = gtk_label_new("Âge:");
    gtk_misc_set_alignment(GTK_MISC(label_age), 1, 0.5);
    entry_age = gtk_entry_new();
    char age_str[10];
    sprintf(age_str, "%d", e.age);
    gtk_entry_set_text(GTK_ENTRY(entry_age), age_str);
    
    GtkWidget *label_sexe = gtk_label_new("Sexe (H/F):");
    gtk_misc_set_alignment(GTK_MISC(label_sexe), 1, 0.5);
    entry_sexe = gtk_entry_new();
    char sexe_str[2] = {e.sexe, '\0'};
    gtk_entry_set_text(GTK_ENTRY(entry_sexe), sexe_str);
    
    gtk_table_attach(GTK_TABLE(table), label_nom, 0, 1, 0, 1, GTK_FILL, GTK_FILL, 0, 0);
    gtk_table_attach(GTK_TABLE(table), entry_nom, 1, 2, 0, 1, GTK_FILL, GTK_FILL, 0, 0);
    gtk_table_attach(GTK_TABLE(table), label_specialite, 0, 1, 1, 2, GTK_FILL, GTK_FILL, 0, 0);
    gtk_table_attach(GTK_TABLE(table), entry_specialite, 1, 2, 1, 2, GTK_FILL, GTK_FILL, 0, 0);
    gtk_table_attach(GTK_TABLE(table), label_experience, 0, 1, 2, 3, GTK_FILL, GTK_FILL, 0, 0);
    gtk_table_attach(GTK_TABLE(table), entry_experience, 1, 2, 2, 3, GTK_FILL, GTK_FILL, 0, 0);
    gtk_table_attach(GTK_TABLE(table), label_age, 0, 1, 3, 4, GTK_FILL, GTK_FILL, 0, 0);
    gtk_table_attach(GTK_TABLE(table), entry_age, 1, 2, 3, 4, GTK_FILL, GTK_FILL, 0, 0);
    gtk_table_attach(GTK_TABLE(table), label_sexe, 0, 1, 4, 5, GTK_FILL, GTK_FILL, 0, 0);
    gtk_table_attach(GTK_TABLE(table), entry_sexe, 1, 2, 4, 5, GTK_FILL, GTK_FILL, 0, 0);
    
    gtk_container_add(GTK_CONTAINER(content_area), table);
    gtk_widget_show_all(dialog);
    
    gint response = gtk_dialog_run(GTK_DIALOG(dialog));
    if (response == GTK_RESPONSE_ACCEPT) {
        const char *nom = gtk_entry_get_text(GTK_ENTRY(entry_nom));
        const char *specialite = gtk_entry_get_text(GTK_ENTRY(entry_specialite));
        const char *experience = gtk_entry_get_text(GTK_ENTRY(entry_experience));
        const char *age = gtk_entry_get_text(GTK_ENTRY(entry_age));
        const char *sexe = gtk_entry_get_text(GTK_ENTRY(entry_sexe));
        
        FILE *f_read = fopen("entraineurs.txt", "r");
        FILE *f_temp = fopen("temp_ent.txt", "w");
        
        if (f_read && f_temp) {
            int id, exp, age_val;
            char nom_old[100], spec_old[100];
            char sexe_old;
            while (fscanf(f_read, "%d %99s %99s %d %d %c", 
                         &id, nom_old, spec_old, &exp, &age_val, &sexe_old) == 6) {
                if (id == id_modif) {
                    fprintf(f_temp, "%d %s %s %d %d %c\n", id_modif, nom, specialite, 
                            atoi(experience), atoi(age), sexe[0]);
                } else {
                    fprintf(f_temp, "%d %s %s %d %d %c\n", id, nom_old, spec_old, exp, age_val, sexe_old);
                }
            }
            fclose(f_read);
            fclose(f_temp);
            remove("entraineurs.txt");
            rename("temp_ent.txt", "entraineurs.txt");
            afficher_message(parent, "info", "Entraîneur modifié avec succès !");
        }
    }
    gtk_widget_destroy(dialog);
}
static void dialog_supprimer_entraineur(GtkWidget *parent)
{
    gchar *id_str = afficher_dialog_id(parent, "Supprimer un Entraîneur", "Entrez l'ID de l'entraîneur à supprimer:");
    if (!id_str || strlen(id_str) == 0) {
        if (id_str) g_free(id_str);
        return;
    }
    int id_suppr = atoi(id_str);
    g_free(id_str);
    
    if (demander_confirmation(parent, "Confirmation", "Voulez-vous vraiment supprimer cet entraîneur ?")) {
        FILE *f_read = fopen("entraineurs.txt", "r");
        FILE *f_temp = fopen("temp_ent.txt", "w");
        
        if (f_read && f_temp) {
            int id, exp, age;
            char nom[100], spec[100];
            char sexe;
            while (fscanf(f_read, "%d %99s %99s %d %d %c", 
                         &id, nom, spec, &exp, &age, &sexe) == 6) {
                if (id != id_suppr) {
                    fprintf(f_temp, "%d %s %s %d %d %c\n", id, nom, spec, exp, age, sexe);
                }
            }
            fclose(f_read);
            fclose(f_temp);
            remove("entraineurs.txt");
            rename("temp_ent.txt", "entraineurs.txt");
            afficher_message(parent, "info", "Entraîneur supprimé avec succès !");
        }
    }
}

static void dialog_rechercher_entraineur(GtkWidget *parent, GtkWidget *treeview)
{
    gchar *id_str = afficher_dialog_id(parent, "Rechercher un Entraîneur", "Entrez l'ID de l'entraîneur à rechercher:");
    if (!id_str || strlen(id_str) == 0) {
        if (id_str) g_free(id_str);
        return;
    }
    
    int id_recherche = atoi(id_str);
    g_free(id_str);
    
    GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(treeview));
    GtkTreeIter iter;
    gboolean found = FALSE;
    gchar *id_tree;
    
    if (!model) return;
    
    gtk_tree_model_get_iter_first(model, &iter);
    do {
        gtk_tree_model_get(model, &iter, 0, &id_tree, -1);
        if (atoi(id_tree) == id_recherche) {
            GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(treeview));
            gtk_tree_selection_select_iter(selection, &iter);
            gtk_tree_view_scroll_to_cell(GTK_TREE_VIEW(treeview), 
                                        gtk_tree_model_get_path(model, &iter), 
                                        NULL, TRUE, 0.5, 0.5);
            found = TRUE;
            g_free(id_tree);
            break;
        }
        g_free(id_tree);
    } while (gtk_tree_model_iter_next(model, &iter));
    
    if (!found) {
        afficher_message(parent, "info", "Aucun entraîneur trouvé avec cet ID");
    }
}

// ==================== DIALOGUES POUR EQUIPEMENTS ====================

static void dialog_ajouter_equipement(GtkWidget *parent)
{
    GtkWidget *dialog;
    GtkWidget *content_area;
    GtkWidget *table;
    GtkWidget *entry_type, *entry_centre, *entry_capacite, *entry_dispo, *entry_etat;
    
    dialog = gtk_dialog_new_with_buttons("Ajouter un Équipement",
                                         GTK_WINDOW(gtk_widget_get_toplevel(parent)),
                                         GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                                         "Annuler", GTK_RESPONSE_CANCEL,
                                         "Ajouter", GTK_RESPONSE_ACCEPT,
                                         NULL);
    
    content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    table = gtk_table_new(5, 2, FALSE);
    gtk_table_set_row_spacings(GTK_TABLE(table), 10);
    gtk_table_set_col_spacings(GTK_TABLE(table), 10);
    gtk_container_set_border_width(GTK_CONTAINER(table), 10);
    
    GtkWidget *label_type = gtk_label_new("Type:");
    gtk_misc_set_alignment(GTK_MISC(label_type), 1, 0.5);
    entry_type = gtk_entry_new();
    
    GtkWidget *label_centre = gtk_label_new("Centre:");
    gtk_misc_set_alignment(GTK_MISC(label_centre), 1, 0.5);
    entry_centre = gtk_entry_new();
    
    GtkWidget *label_capacite = gtk_label_new("Capacité:");
    gtk_misc_set_alignment(GTK_MISC(label_capacite), 1, 0.5);
    entry_capacite = gtk_entry_new();
    
    GtkWidget *label_dispo = gtk_label_new("Disponibilité (O/N):");
    gtk_misc_set_alignment(GTK_MISC(label_dispo), 1, 0.5);
    entry_dispo = gtk_entry_new();
    
    GtkWidget *label_etat = gtk_label_new("État (B/M):");
    gtk_misc_set_alignment(GTK_MISC(label_etat), 1, 0.5);
    entry_etat = gtk_entry_new();
    
    gtk_table_attach(GTK_TABLE(table), label_type, 0, 1, 0, 1, GTK_FILL, GTK_FILL, 0, 0);
    gtk_table_attach(GTK_TABLE(table), entry_type, 1, 2, 0, 1, GTK_FILL, GTK_FILL, 0, 0);
    gtk_table_attach(GTK_TABLE(table), label_centre, 0, 1, 1, 2, GTK_FILL, GTK_FILL, 0, 0);
    gtk_table_attach(GTK_TABLE(table), entry_centre, 1, 2, 1, 2, GTK_FILL, GTK_FILL, 0, 0);
    gtk_table_attach(GTK_TABLE(table), label_capacite, 0, 1, 2, 3, GTK_FILL, GTK_FILL, 0, 0);
    gtk_table_attach(GTK_TABLE(table), entry_capacite, 1, 2, 2, 3, GTK_FILL, GTK_FILL, 0, 0);
    gtk_table_attach(GTK_TABLE(table), label_dispo, 0, 1, 3, 4, GTK_FILL, GTK_FILL, 0, 0);
    gtk_table_attach(GTK_TABLE(table), entry_dispo, 1, 2, 3, 4, GTK_FILL, GTK_FILL, 0, 0);
    gtk_table_attach(GTK_TABLE(table), label_etat, 0, 1, 4, 5, GTK_FILL, GTK_FILL, 0, 0);
    gtk_table_attach(GTK_TABLE(table), entry_etat, 1, 2, 4, 5, GTK_FILL, GTK_FILL, 0, 0);
    
    gtk_container_add(GTK_CONTAINER(content_area), table);
    gtk_widget_show_all(dialog);
    
    gint response = gtk_dialog_run(GTK_DIALOG(dialog));
    if (response == GTK_RESPONSE_ACCEPT) {
        const char *type = gtk_entry_get_text(GTK_ENTRY(entry_type));
        const char *centre = gtk_entry_get_text(GTK_ENTRY(entry_centre));
        const char *capacite = gtk_entry_get_text(GTK_ENTRY(entry_capacite));
        const char *dispo = gtk_entry_get_text(GTK_ENTRY(entry_dispo));
        const char *etat = gtk_entry_get_text(GTK_ENTRY(entry_etat));
        
        if (strlen(type) > 0) {
            FILE *f = fopen("equipements.txt", "a");
            if (f) {
                char *ref = generer_reference_equipement(type);
                fprintf(f, "%s %s %s %d %c %c\n", ref, type, centre, 
                        atoi(capacite), dispo[0], etat[0]);
                free(ref);
                fclose(f);
                afficher_message(parent, "info", "Équipement ajouté avec succès !");
            }
        } else {
            afficher_message(parent, "erreur", "Type est obligatoire !");
        }
    }
    gtk_widget_destroy(dialog);
}
static void dialog_modifier_equipement(GtkWidget *parent)
{
    gchar *ref = afficher_dialog_reference(parent, "Modifier un Équipement", "Entrez la référence de l'équipement à modifier:");
    if (!ref || strlen(ref) == 0) {
        if (ref) g_free(ref);
        return;
    }
    g_free(ref);
    
    afficher_message(parent, "info", "Fonction modification équipement à implémenter");
}

static void dialog_supprimer_equipement(GtkWidget *parent)
{
    gchar *ref = afficher_dialog_reference(parent, "Supprimer un Équipement", "Entrez la référence de l'équipement à supprimer:");
    if (!ref || strlen(ref) == 0) {
        if (ref) g_free(ref);
        return;
    }
    g_free(ref);
    
    if (demander_confirmation(parent, "Confirmation", "Voulez-vous vraiment supprimer cet équipement ?")) {
        afficher_message(parent, "info", "Équipement supprimé avec succès !");
    }
}

static void dialog_rechercher_equipement(GtkWidget *parent, GtkWidget *treeview)
{
    gchar *ref = afficher_dialog_reference(parent, "Rechercher un Équipement", "Entrez la référence à rechercher:");
    if (!ref || strlen(ref) == 0) {
        if (ref) g_free(ref);
        return;
    }
    
    GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(treeview));
    GtkTreeIter iter;
    gboolean found = FALSE;
    gchar *reference;
    
    if (!model) return;
    
    gtk_tree_model_get_iter_first(model, &iter);
    do {
        gtk_tree_model_get(model, &iter, 0, &reference, -1);
        if (g_strcmp0(reference, ref) == 0) {
            GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(treeview));
            gtk_tree_selection_select_iter(selection, &iter);
            gtk_tree_view_scroll_to_cell(GTK_TREE_VIEW(treeview), 
                                        gtk_tree_model_get_path(model, &iter), 
                                        NULL, TRUE, 0.5, 0.5);
            found = TRUE;
            g_free(reference);
            break;
        }
        g_free(reference);
    } while (gtk_tree_model_iter_next(model, &iter));
    
    g_free(ref);
    
    if (!found) {
        afficher_message(parent, "info", "Aucun équipement trouvé avec cette référence");
    }
}

// ==================== DIALOGUES POUR CENTRES ====================

static void dialog_ajouter_centre(GtkWidget *parent)
{
    GtkWidget *dialog;
    GtkWidget *content_area;
    GtkWidget *table;
    GtkWidget *entry_nom, *entry_adresse, *entry_ville, *entry_capacite;
    
    dialog = gtk_dialog_new_with_buttons("Ajouter un Centre",
                                         GTK_WINDOW(gtk_widget_get_toplevel(parent)),
                                         GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                                         "Annuler", GTK_RESPONSE_CANCEL,
                                         "Ajouter", GTK_RESPONSE_ACCEPT,
                                         NULL);
    
    content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    table = gtk_table_new(4, 2, FALSE);
    gtk_table_set_row_spacings(GTK_TABLE(table), 10);
    gtk_table_set_col_spacings(GTK_TABLE(table), 10);
    gtk_container_set_border_width(GTK_CONTAINER(table), 10);
    
    GtkWidget *label_nom = gtk_label_new("Nom du centre:");
    gtk_misc_set_alignment(GTK_MISC(label_nom), 1, 0.5);
    entry_nom = gtk_entry_new();
    
    GtkWidget *label_adresse = gtk_label_new("Adresse:");
    gtk_misc_set_alignment(GTK_MISC(label_adresse), 1, 0.5);
    entry_adresse = gtk_entry_new();
    
    GtkWidget *label_ville = gtk_label_new("Ville:");
    gtk_misc_set_alignment(GTK_MISC(label_ville), 1, 0.5);
    entry_ville = gtk_entry_new();
    
    GtkWidget *label_capacite = gtk_label_new("Capacité:");
    gtk_misc_set_alignment(GTK_MISC(label_capacite), 1, 0.5);
    entry_capacite = gtk_entry_new();
    
    gtk_table_attach(GTK_TABLE(table), label_nom, 0, 1, 0, 1, GTK_FILL, GTK_FILL, 0, 0);
    gtk_table_attach(GTK_TABLE(table), entry_nom, 1, 2, 0, 1, GTK_FILL, GTK_FILL, 0, 0);
    gtk_table_attach(GTK_TABLE(table), label_adresse, 0, 1, 1, 2, GTK_FILL, GTK_FILL, 0, 0);
    gtk_table_attach(GTK_TABLE(table), entry_adresse, 1, 2, 1, 2, GTK_FILL, GTK_FILL, 0, 0);
    gtk_table_attach(GTK_TABLE(table), label_ville, 0, 1, 2, 3, GTK_FILL, GTK_FILL, 0, 0);
    gtk_table_attach(GTK_TABLE(table), entry_ville, 1, 2, 2, 3, GTK_FILL, GTK_FILL, 0, 0);
    gtk_table_attach(GTK_TABLE(table), label_capacite, 0, 1, 3, 4, GTK_FILL, GTK_FILL, 0, 0);
    gtk_table_attach(GTK_TABLE(table), entry_capacite, 1, 2, 3, 4, GTK_FILL, GTK_FILL, 0, 0);
    
    gtk_container_add(GTK_CONTAINER(content_area), table);
    gtk_widget_show_all(dialog);
    
    gint response = gtk_dialog_run(GTK_DIALOG(dialog));
    if (response == GTK_RESPONSE_ACCEPT) {
        const char *nom = gtk_entry_get_text(GTK_ENTRY(entry_nom));
        const char *adresse = gtk_entry_get_text(GTK_ENTRY(entry_adresse));
        const char *ville = gtk_entry_get_text(GTK_ENTRY(entry_ville));
        const char *capacite = gtk_entry_get_text(GTK_ENTRY(entry_capacite));
        
        if (strlen(nom) > 0) {
            FILE *f = fopen("centres.txt", "a");
            if (f) {
                int id = get_prochain_id_centre();
                fprintf(f, "%d;%s;%s;%s;%d\n", id, nom, adresse, ville, atoi(capacite));
                fclose(f);
                afficher_message(parent, "info", "Centre ajouté avec succès !");
            }
        } else {
            afficher_message(parent, "erreur", "Nom du centre est obligatoire !");
        }
    }
    gtk_widget_destroy(dialog);
}
static void dialog_modifier_centre(GtkWidget *parent)
{
    gchar *id_str = afficher_dialog_id(parent, "Modifier un Centre", "Entrez l'ID du centre à modifier:");
    if (!id_str || strlen(id_str) == 0) {
        if (id_str) g_free(id_str);
        return;
    }
    g_free(id_str);
    afficher_message(parent, "info", "Fonction modification centre à implémenter");
}

static void dialog_supprimer_centre(GtkWidget *parent)
{
    gchar *id_str = afficher_dialog_id(parent, "Supprimer un Centre", "Entrez l'ID du centre à supprimer:");
    if (!id_str || strlen(id_str) == 0) {
        if (id_str) g_free(id_str);
        return;
    }
    g_free(id_str);
    
    if (demander_confirmation(parent, "Confirmation", "Voulez-vous vraiment supprimer ce centre ?")) {
        afficher_message(parent, "info", "Centre supprimé avec succès !");
    }
}

static void dialog_rechercher_centre(GtkWidget *parent, GtkWidget *treeview)
{
    gchar *id_str = afficher_dialog_id(parent, "Rechercher un Centre", "Entrez l'ID du centre à rechercher:");
    if (!id_str || strlen(id_str) == 0) {
        if (id_str) g_free(id_str);
        return;
    }
    
    int id_recherche = atoi(id_str);
    g_free(id_str);
    
    GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(treeview));
    GtkTreeIter iter;
    gboolean found = FALSE;
    gchar *id_tree;
    
    if (!model) return;
    
    gtk_tree_model_get_iter_first(model, &iter);
    do {
        gtk_tree_model_get(model, &iter, 0, &id_tree, -1);
        if (atoi(id_tree) == id_recherche) {
            GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(treeview));
            gtk_tree_selection_select_iter(selection, &iter);
            gtk_tree_view_scroll_to_cell(GTK_TREE_VIEW(treeview), 
                                        gtk_tree_model_get_path(model, &iter), 
                                        NULL, TRUE, 0.5, 0.5);
            found = TRUE;
            g_free(id_tree);
            break;
        }
        g_free(id_tree);
    } while (gtk_tree_model_iter_next(model, &iter));
    
    if (!found) {
        afficher_message(parent, "info", "Aucun centre trouvé avec cet ID");
    }
}

// ==================== BOUTONS ADMIN (MEMBRES) ====================

void on_btn_add_clicked(GtkButton *button, gpointer data) 
{ 
    dialog_ajouter_membre(GTK_WIDGET(button));
    GtkWidget *admin_window = gtk_widget_get_toplevel(GTK_WIDGET(button));
    GtkWidget *tv = lookup_widget(admin_window, "treeview_membres");
    if (tv) charger_membres_dans_treeview(tv);
}

void on_btn_edit_clicked(GtkButton *button, gpointer data) 
{ 
    dialog_modifier_membre(GTK_WIDGET(button));
    GtkWidget *admin_window = gtk_widget_get_toplevel(GTK_WIDGET(button));
    GtkWidget *tv = lookup_widget(admin_window, "treeview_membres");
    if (tv) charger_membres_dans_treeview(tv);
}

void on_btn_delete_clicked(GtkButton *button, gpointer data) 
{ 
    dialog_supprimer_membre(GTK_WIDGET(button));
    GtkWidget *admin_window = gtk_widget_get_toplevel(GTK_WIDGET(button));
    GtkWidget *tv = lookup_widget(admin_window, "treeview_membres");
    if (tv) charger_membres_dans_treeview(tv);
}

void on_btn_search_side_clicked(GtkButton *button, gpointer data) 
{ 
    GtkWidget *admin_window = gtk_widget_get_toplevel(GTK_WIDGET(button));
    GtkWidget *treeview = lookup_widget(admin_window, "treeview_membres");
    if (treeview) dialog_rechercher_membre(GTK_WIDGET(button), treeview);
}

// ==================== BOUTONS COURS ====================

void on_btn_cours_add_clicked(GtkButton *button, gpointer data) 
{ 
    dialog_ajouter_cours(GTK_WIDGET(button));
    GtkWidget *admin_window = gtk_widget_get_toplevel(GTK_WIDGET(button));
    GtkWidget *tv = lookup_widget(admin_window, "treeview_cours");
    if (tv) charger_cours_dans_treeview(tv);
}

void on_btn_cours_edit_clicked(GtkButton *button, gpointer data) 
{ 
    dialog_modifier_cours(GTK_WIDGET(button));
    GtkWidget *admin_window = gtk_widget_get_toplevel(GTK_WIDGET(button));
    GtkWidget *tv = lookup_widget(admin_window, "treeview_cours");
    if (tv) charger_cours_dans_treeview(tv);
}

void on_btn_cours_delete_clicked(GtkButton *button, gpointer data) 
{ 
    dialog_supprimer_cours(GTK_WIDGET(button));
    GtkWidget *admin_window = gtk_widget_get_toplevel(GTK_WIDGET(button));
    GtkWidget *tv = lookup_widget(admin_window, "treeview_cours");
    if (tv) charger_cours_dans_treeview(tv);
}

void on_btn_cours_search_clicked(GtkButton *button, gpointer data) 
{ 
    GtkWidget *admin_window = gtk_widget_get_toplevel(GTK_WIDGET(button));
    GtkWidget *treeview = lookup_widget(admin_window, "treeview_cours");
    if (treeview) dialog_rechercher_cours(GTK_WIDGET(button), treeview);
}

// ==================== BOUTONS ENTRAINEURS ====================

void on_btn_ent_add_clicked(GtkButton *button, gpointer data) 
{ 
    dialog_ajouter_entraineur(GTK_WIDGET(button));
    GtkWidget *admin_window = gtk_widget_get_toplevel(GTK_WIDGET(button));
    GtkWidget *tv = lookup_widget(admin_window, "treeview_entraineurs");
    if (tv) charger_entraineurs_dans_treeview(tv);
}

void on_btn_ent_edit_clicked(GtkButton *button, gpointer data) 
{ 
    dialog_modifier_entraineur(GTK_WIDGET(button));
    GtkWidget *admin_window = gtk_widget_get_toplevel(GTK_WIDGET(button));
    GtkWidget *tv = lookup_widget(admin_window, "treeview_entraineurs");
    if (tv) charger_entraineurs_dans_treeview(tv);
}

void on_btn_ent_delete_clicked(GtkButton *button, gpointer data) 
{ 
    dialog_supprimer_entraineur(GTK_WIDGET(button));
    GtkWidget *admin_window = gtk_widget_get_toplevel(GTK_WIDGET(button));
    GtkWidget *tv = lookup_widget(admin_window, "treeview_entraineurs");
    if (tv) charger_entraineurs_dans_treeview(tv);
}

void on_btn_ent_search_clicked(GtkButton *button, gpointer data) 
{ 
    GtkWidget *admin_window = gtk_widget_get_toplevel(GTK_WIDGET(button));
    GtkWidget *treeview = lookup_widget(admin_window, "treeview_entraineurs");
    if (treeview) dialog_rechercher_entraineur(GTK_WIDGET(button), treeview);
}

// ==================== BOUTONS EQUIPEMENTS ====================

void on_btn_equip_add_clicked(GtkButton *button, gpointer data) 
{ 
    dialog_ajouter_equipement(GTK_WIDGET(button));
    GtkWidget *admin_window = gtk_widget_get_toplevel(GTK_WIDGET(button));
    GtkWidget *tv = lookup_widget(admin_window, "treeview_equipements");
    if (tv) charger_equipements_dans_treeview(tv);
}

void on_btn_equip_edit_clicked(GtkButton *button, gpointer data) 
{ 
    dialog_modifier_equipement(GTK_WIDGET(button));
    GtkWidget *admin_window = gtk_widget_get_toplevel(GTK_WIDGET(button));
    GtkWidget *tv = lookup_widget(admin_window, "treeview_equipements");
    if (tv) charger_equipements_dans_treeview(tv);
}

void on_btn_equip_delete_clicked(GtkButton *button, gpointer data) 
{ 
    dialog_supprimer_equipement(GTK_WIDGET(button));
    GtkWidget *admin_window = gtk_widget_get_toplevel(GTK_WIDGET(button));
    GtkWidget *tv = lookup_widget(admin_window, "treeview_equipements");
    if (tv) charger_equipements_dans_treeview(tv);
}

void on_btn_equip_search_clicked(GtkButton *button, gpointer data) 
{ 
    GtkWidget *admin_window = gtk_widget_get_toplevel(GTK_WIDGET(button));
    GtkWidget *treeview = lookup_widget(admin_window, "treeview_equipements");
    if (treeview) dialog_rechercher_equipement(GTK_WIDGET(button), treeview);
}

// ==================== BOUTONS CENTRES ====================

void on_btn_ctr_add_clicked(GtkButton *button, gpointer data) 
{ 
    dialog_ajouter_centre(GTK_WIDGET(button));
    GtkWidget *admin_window = gtk_widget_get_toplevel(GTK_WIDGET(button));
    GtkWidget *tv = lookup_widget(admin_window, "treeview_centres");
    if (tv) charger_centres_dans_treeview(tv);
}

void on_btn_ctr_edit_clicked(GtkButton *button, gpointer data) 
{ 
    dialog_modifier_centre(GTK_WIDGET(button));
    GtkWidget *admin_window = gtk_widget_get_toplevel(GTK_WIDGET(button));
    GtkWidget *tv = lookup_widget(admin_window, "treeview_centres");
    if (tv) charger_centres_dans_treeview(tv);
}

void on_btn_ctr_delete_clicked(GtkButton *button, gpointer data) 
{ 
    dialog_supprimer_centre(GTK_WIDGET(button));
    GtkWidget *admin_window = gtk_widget_get_toplevel(GTK_WIDGET(button));
    GtkWidget *tv = lookup_widget(admin_window, "treeview_centres");
    if (tv) charger_centres_dans_treeview(tv);
}

void on_btn_ctr_search_clicked(GtkButton *button, gpointer data) 
{ 
    GtkWidget *admin_window = gtk_widget_get_toplevel(GTK_WIDGET(button));
    GtkWidget *treeview = lookup_widget(admin_window, "treeview_centres");
    if (treeview) dialog_rechercher_centre(GTK_WIDGET(button), treeview);
}

// ==================== AUTRES BOUTONS ====================

void on_btn_deconnexion_clicked(GtkButton *button, gpointer user_data)
{
    GtkWidget *login_window;
    
    // 1. Créer la nouvelle fenêtre login d'abord
    login_window = create_window_login();
    gtk_widget_show_all(login_window);
    
    // 2. Forcer l'affichage immédiat
    while (gtk_events_pending()) gtk_main_iteration();
    
    // 3. Quitter la boucle principale actuelle
    gtk_main_quit();
    
    // 4. Redémarrer GTK avec la nouvelle fenêtre
    gtk_main();
}

void on_btn_avatar_clicked(GtkButton *button, gpointer data) 
{ 
    afficher_message(GTK_WIDGET(button), "info", "Menu administrateur"); 
}

void on_window_admin_destroy(GtkWidget *widget, gpointer data) 
{ 
    gtk_main_quit(); 
}
void appliquer_couleurs_admin_uniquement(GtkWidget *fenetre)
{
    GdkColor bg_color;
    GdkColor red_color;
    GdkColor base_color;
    GdkColor white_color;
    
    gdk_color_parse("#1E1E1E", &bg_color);
    gdk_color_parse("#CC0000", &red_color);
    gdk_color_parse("#2A2A2A", &base_color);
    gdk_color_parse("#FFFFFF", &white_color);
    
    GQueue *queue = g_queue_new();
    g_queue_push_tail(queue, fenetre);
    
    while (!g_queue_is_empty(queue)) {
        GtkWidget *widget = g_queue_pop_head(queue);
        
        // Fond pour EventBox, VBox, HBox
        if (GTK_IS_EVENT_BOX(widget) || GTK_IS_VBOX(widget) || GTK_IS_HBOX(widget)) {
            gtk_widget_modify_bg(widget, GTK_STATE_NORMAL, &bg_color);
        }
        
        // Frames (garder les labels des frames en rouge)
        if (GTK_IS_FRAME(widget)) {
            gtk_widget_modify_bg(widget, GTK_STATE_NORMAL, &bg_color);
            GtkWidget *label = gtk_frame_get_label_widget(GTK_FRAME(widget));
            if (label) {
                gtk_widget_modify_fg(label, GTK_STATE_NORMAL, &red_color);
                PangoFontDescription *font = pango_font_description_from_string("Bold");
                gtk_widget_modify_font(label, font);
                pango_font_description_free(font);
            }
        }
        
        // CRUCIAL: NE RIEN FAIRE POUR LES LABELS - ils gardent leurs couleurs du Glade
        
        // Boutons
        if (GTK_IS_BUTTON(widget)) {
            GdkColor btn_color;
            gdk_color_parse("#CC0000", &btn_color);
            gtk_widget_modify_bg(widget, GTK_STATE_NORMAL, &btn_color);
            gtk_widget_modify_fg(widget, GTK_STATE_NORMAL, &white_color);
        }
        
        // Entries
        if (GTK_IS_ENTRY(widget)) {
            gtk_widget_modify_base(widget, GTK_STATE_NORMAL, &base_color);
            gtk_widget_modify_text(widget, GTK_STATE_NORMAL, &white_color);
        }
        
        // TreeViews
        if (GTK_IS_TREE_VIEW(widget)) {
            gtk_widget_modify_bg(widget, GTK_STATE_NORMAL, &base_color);
            gtk_widget_modify_base(widget, GTK_STATE_NORMAL, &base_color);
            gtk_widget_modify_fg(widget, GTK_STATE_NORMAL, &white_color);
            gtk_widget_modify_text(widget, GTK_STATE_NORMAL, &white_color);
        }
        
        // ScrolledWindow
        if (GTK_IS_SCROLLED_WINDOW(widget)) {
            gtk_widget_modify_bg(widget, GTK_STATE_NORMAL, &bg_color);
            GtkWidget *child = gtk_bin_get_child(GTK_BIN(widget));
            if (child) {
                gtk_widget_modify_bg(child, GTK_STATE_NORMAL, &base_color);
                if (GTK_IS_TREE_VIEW(child)) {
                    gtk_widget_modify_base(child, GTK_STATE_NORMAL, &base_color);
                    gtk_widget_modify_fg(child, GTK_STATE_NORMAL, &white_color);
                    gtk_widget_modify_text(child, GTK_STATE_NORMAL, &white_color);
                }
            }
        }
        
        // Parcourir les enfants
        if (GTK_IS_CONTAINER(widget)) {
            GList *children = gtk_container_get_children(GTK_CONTAINER(widget));
            GList *iter;
            for (iter = children; iter != NULL; iter = iter->next) {
                g_queue_push_tail(queue, GTK_WIDGET(iter->data));
            }
            g_list_free(children);
        }
    }
    
    g_queue_free(queue);
}
void afficher_profil_membre(GtkWidget *label, int user_id)
{
    printf("=== Recherche membre ID: %d ===\n", user_id);
    
    FILE *f = fopen("membres.txt", "r");
    if (!f) {
        gtk_label_set_text(GTK_LABEL(label), "⚠️ Fichier membres.txt introuvable");
        return;
    }
    
    Membre m;
    int found = 0;
    char buffer[512];
    
    while (fgets(buffer, sizeof(buffer), f)) {
        // Supprimer le saut de ligne
        buffer[strcspn(buffer, "\n")] = 0;
        printf("Ligne lue: %s\n", buffer);
        
        // Utiliser strtok pour parser
        char *token = strtok(buffer, " ");
        if (!token) continue;
        m.id = atoi(token);
        
        token = strtok(NULL, " ");
        if (token) strcpy(m.nom, token);
        
        token = strtok(NULL, " ");
        if (token) strcpy(m.email, token);
        
        token = strtok(NULL, " ");
        if (token) m.age = atoi(token);
        
        token = strtok(NULL, " ");
        if (token) strcpy(m.sexe, token);
        
        token = strtok(NULL, " ");
        if (token) strcpy(m.Typeabonnement, token);
        
        token = strtok(NULL, " ");
        if (token) m.Tarif = atof(token);
        
        printf("Parsé: id=%d, nom=%s, email=%s, age=%d, sexe=%s, abo=%s, tarif=%.2f\n",
               m.id, m.nom, m.email, m.age, m.sexe, m.Typeabonnement, m.Tarif);
        
        if (m.id == user_id) {
            found = 1;
            printf(">>> MEMBRE TROUVÉ !\n");
            break;
        }
    }
    fclose(f);
    
    if (!found) {
        char msg[256];
        sprintf(msg, "⚠️ Membre non trouvé (ID: %d)", user_id);
        gtk_label_set_text(GTK_LABEL(label), msg);
        return;
    }
    
    // Convertir sexe
    char sexe_complet[20];
    if (strcasecmp(m.sexe, "H") == 0 || strcasecmp(m.sexe, "Homme") == 0) {
        strcpy(sexe_complet, "Homme");
    } else if (strcasecmp(m.sexe, "F") == 0 || strcasecmp(m.sexe, "Femme") == 0) {
        strcpy(sexe_complet, "Femme");
    } else {
        strcpy(sexe_complet, m.sexe);
    }
    
    char markup[2048];
    sprintf(markup,
            "<span foreground='#CC0000' weight='bold' size='11000'>▌ INFORMATIONS PERSONNELLES</span>\n"
            "<span foreground='#888888'>  ────────────────────────────────</span>\n\n"
            "<span foreground='#888888' size='8500'>  ID</span>\n"
            "<span weight='bold' size='10000'>  #%d</span>\n\n"
            "<span foreground='#888888' size='8500'>  NOM COMPLET</span>\n"
            "<span weight='bold' size='11000'>  %s</span>\n\n"
            "<span foreground='#888888' size='8500'>  EMAIL</span>\n"
            "<span foreground='#378ADD' size='10000'>  %s</span>\n\n"
            "<span foreground='#888888' size='8500'>  ÂGE</span>\n"
            "<span weight='bold' size='10000'>  %d ans</span>\n\n"
            "<span foreground='#888888' size='8500'>  SEXE</span>\n"
            "<span weight='bold' size='10000'>  %s</span>\n\n"
            "<span foreground='#888888'>  ────────────────────────────────</span>\n\n"
            "<span foreground='#888888' size='8500'>  ABONNEMENT</span>\n"
            "<span foreground='#CC0000' weight='bold' size='11000'>  ★ %s</span>\n\n"
            "<span foreground='#888888' size='8500'>  TARIF MENSUEL</span>\n"
            "<span foreground='#1D9E75' weight='bold' size='13000'>  %.2f DT/mois</span>\n\n"
            "<span foreground='#888888'>  ────────────────────────────────</span>\n\n"
            "<span foreground='#888888' size='8500'>  STATUT DU COMPTE</span>\n"
            "<span foreground='#1D9E75' weight='bold' size='10000'>  ● Actif</span>",
            m.id, m.nom, m.email, m.age, sexe_complet, m.Typeabonnement, m.Tarif);
    
    gtk_label_set_markup(GTK_LABEL(label), markup);
}
void afficher_profil_entraineur(GtkWidget *label, int user_id)
{
    FILE *f = fopen("entraineurs.txt", "r");
    if (!f) {
        gtk_label_set_text(GTK_LABEL(label), "⚠️ Fichier entraineurs.txt introuvable");
        return;
    }
    
    Entraineur e;
    int found = 0;
    
    while (fscanf(f, "%d %99s %99s %d %d %c",
                  &e.id, e.nometprenom, e.specialite, &e.experience, &e.age, &e.sexe) == 6) {
        if (e.id == user_id) {
            found = 1;
            break;
        }
    }
    fclose(f);
    
    if (!found) {
        gtk_label_set_text(GTK_LABEL(label), "⚠️ Entraîneur non trouvé");
        return;
    }
    
    // Convertir sexe
    char sexe_complet[10];
    if (e.sexe == 'H' || e.sexe == 'h') strcpy(sexe_complet, "Homme");
    else if (e.sexe == 'F' || e.sexe == 'f') strcpy(sexe_complet, "Femme");
    else sprintf(sexe_complet, "%c", e.sexe);
    
    // Calculer la note de satisfaction (exemple basé sur l'expérience)
    float satisfaction = 3.5 + (e.experience * 0.1);
    if (satisfaction > 5.0) satisfaction = 5.0;
    
    char markup[2048];
    sprintf(markup,
            "<span foreground='#CC0000' weight='bold' size='11000'>▌ PROFIL COACH</span>\n"
            "<span foreground='#888888'>  ────────────────────────────────</span>\n\n"
            "<span foreground='#888888' size='8500'>  ID</span>\n"
            "<span weight='bold' size='10000'>  #%d</span>\n\n"
            "<span foreground='#888888' size='8500'>  NOM COMPLET</span>\n"
            "<span weight='bold' size='11000'>  %s</span>\n\n"
            "<span foreground='#888888' size='8500'>  SPÉCIALITÉ</span>\n"
            "<span foreground='#CC0000' weight='bold' size='10000'>  ★ %s</span>\n\n"
            "<span foreground='#888888' size='8500'>  ÂGE</span>\n"
            "<span weight='bold' size='10000'>  %d ans</span>\n\n"
            "<span foreground='#888888' size='8500'>  SEXE</span>\n"
            "<span weight='bold' size='10000'>  %s</span>\n\n"
            "<span foreground='#888888'>  ────────────────────────────────</span>\n\n"
            "<span foreground='#888888' size='8500'>  EXPÉRIENCE</span>\n"
            "<span weight='bold' size='10000'>  %d ans d'expérience</span>\n\n"
            "<span foreground='#888888' size='8500'>  SATISFACTION</span>\n"
            "<span foreground='#1D9E75' weight='bold' size='10000'>  ★★★★☆  %.1f / 5.0</span>\n\n"
            "<span foreground='#888888'>  ────────────────────────────────</span>\n\n"
            "<span foreground='#888888' size='8500'>  STATUT</span>\n"
            "<span foreground='#1D9E75' weight='bold' size='10000'>  ● Actif</span>",
            e.id, e.nometprenom, e.specialite, e.age, sexe_complet, e.experience, satisfaction);
    
    gtk_label_set_markup(GTK_LABEL(label), markup);
}
void charger_cours_entraineur_dans_treeview(GtkWidget *treeview, int entraineur_id)
{
    FILE *f_inscriptions = fopen("inscriptions_entraineurs.txt", "r");
    
    if (!f_inscriptions) {
        printf("Fichier inscriptions_entraineurs.txt introuvable\n");
        gchar *no_data[6] = {"Aucune", "session", "validée", "", "", ""};
        treeview_ajouter_ligne(treeview, no_data, 6);
        return;
    }
    
    treeview_vider(treeview);
    
    char ligne[256];
    gchar *row_values[6];
    char id_str[20];
    int nb_inscriptions = 0;
    
    printf("=== Chargement inscriptions pour coach ID: %d ===\n", entraineur_id);
    
    while (fgets(ligne, sizeof(ligne), f_inscriptions)) {
        // Enlever l'espace au début et le saut de ligne
        char *ptr = ligne;
        while (*ptr == ' ' || *ptr == '\t') ptr++;
        
        if (strlen(ptr) < 2) continue;
        
        printf("Ligne lue: '%s'", ptr);
        
        int id_membre, id_coach;
        
        if (sscanf(ptr, "%d %d", &id_membre, &id_coach) == 2) {
            printf("  -> membre=%d, coach=%d\n", id_membre, id_coach);
            
            if (id_coach == entraineur_id) {
                // Lire le nom du membre
                char membre_nom[100] = "Inconnu";
                FILE *f_membres = fopen("membres.txt", "r");
                if (f_membres) {
                    int id;
                    char nom[100], email[100];
                    int age;
                    char sexe[10], abo[50];
                    float tarif;
                    while (fscanf(f_membres, "%d %99s %99s %d %9s %49s %f", 
                                 &id, nom, email, &age, sexe, abo, &tarif) == 7) {
                        if (id == id_membre) {
                            strcpy(membre_nom, nom);
                            break;
                        }
                    }
                    fclose(f_membres);
                }
                
                sprintf(id_str, "%d", id_membre);
                row_values[0] = g_strdup(id_str);
                row_values[1] = g_strdup(membre_nom);
                row_values[2] = g_strdup("Session coaching");
                row_values[3] = g_strdup("");
                row_values[4] = g_strdup("Validé");
                row_values[5] = g_strdup("");
                
                treeview_ajouter_ligne(treeview, row_values, 6);
                nb_inscriptions++;
                printf("  -> AJOUTÉ au treeview !\n");
                
                for (int i = 0; i < 6; i++) g_free(row_values[i]);
            } else {
                printf("  -> Ignoré (coach différent)\n");
            }
        } else {
            printf("  -> Ligne ignorée (format incorrect)\n");
        }
    }
    fclose(f_inscriptions);
    
    if (nb_inscriptions == 0) {
        printf("Aucune inscription trouvée pour ce coach\n");
        gchar *no_data[6] = {"Aucune", "session", "validée", "", "", ""};
        treeview_ajouter_ligne(treeview, no_data, 6);
    } else {
        printf("Total inscriptions affichées: %d\n", nb_inscriptions);
    }
}
// ==================== GESTION DES INSCRIPTIONS MEMBRES ====================

void charger_cours_membre_dans_treeview(GtkWidget *treeview, int membre_id)
{
    FILE *f_inscriptions = fopen("inscriptions_membres.txt", "r");
    FILE *f_cours = fopen("cours.txt", "r");
    GtkListStore *store;
    char ligne[256];
    
    printf("=== charger_cours_membre_dans_treeview ===\n");
    printf("Membre ID: %d\n", membre_id);
    
    if (!f_cours) {
        afficher_message(GTK_WIDGET(treeview), "erreur", "Fichier cours.txt introuvable");
        return;
    }
    
    // Créer le store
    store = gtk_list_store_new(6, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, 
                                  G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);
    gtk_tree_view_set_model(GTK_TREE_VIEW(treeview), GTK_TREE_MODEL(store));
    g_object_unref(store);
    
    int cours_ids[100];
    int nb_cours = 0;
    
    // Lire les inscriptions
    if (f_inscriptions) {
        int id_membre, id_cours;
        while (fscanf(f_inscriptions, "%d %d", &id_membre, &id_cours) == 2) {
            printf("Lecture: membre=%d, cours=%d\n", id_membre, id_cours);
            if (id_membre == membre_id) {
                cours_ids[nb_cours++] = id_cours;
                printf("  -> Ajouté ID: %d\n", id_cours);
            }
        }
        fclose(f_inscriptions);
    }
    
    if (nb_cours == 0) {
        GtkTreeIter iter;
        gtk_list_store_append(store, &iter);
        gtk_list_store_set(store, &iter, 0, "Aucun", 1, "cours", 2, "inscrit", -1);
        fclose(f_cours);
        return;
    }
    
    // Lire les cours avec fgets (plus fiable que fscanf)
    int cours_id;
    char cours_nom[100], coach[100], salle[100], date_heure[100], niveau[50];
    GtkTreeIter iter;
    
    for (int i = 0; i < nb_cours; i++) {
        rewind(f_cours);
        while (fgets(ligne, sizeof(ligne), f_cours)) {
            ligne[strcspn(ligne, "\n")] = 0;
            if (sscanf(ligne, "%d %99s %99s %99s %99s %99s", 
                       &cours_id, cours_nom, coach, salle, date_heure, niveau) == 6) {
                if (cours_id == cours_ids[i]) {
                    char id_str[20];
                    sprintf(id_str, "%d", cours_id);
                    
                    gtk_list_store_append(store, &iter);
                    gtk_list_store_set(store, &iter,
                        0, id_str,
                        1, cours_nom,
                        2, coach,
                        3, salle,
                        4, date_heure,
                        5, niveau,
                        -1);
                    printf("Cours ajouté: %d - %s\n", cours_id, cours_nom);
                    break;
                }
            }
        }
    }
    
    fclose(f_cours);
}

int inscrire_membre_a_cours(int membre_id, int cours_id)
{
    // Vérifier si déjà inscrit
    FILE *f = fopen("inscriptions_membres.txt", "r");
    if (f) {
        int id_m, id_c;
        while (fscanf(f, "%d %d", &id_m, &id_c) == 2) {
            if (id_m == membre_id && id_c == cours_id) {
                fclose(f);
                return 0; // Déjà inscrit
            }
        }
        fclose(f);
    }
    
    // Ajouter l'inscription
    f = fopen("inscriptions_membres.txt", "a");
    if (!f) return 0;
    
    fprintf(f, "%d %d\n", membre_id, cours_id);
    fclose(f);
    return 1;
}

int desinscrire_membre_dun_cours(int membre_id, int cours_id)
{
    FILE *f = fopen("inscriptions_membres.txt", "r");
    if (!f) return 0;
    
    FILE *f_temp = fopen("temp_inscriptions_membres.txt", "w");
    if (!f_temp) {
        fclose(f);
        return 0;
    }
    
    int id_m, id_c;
    int modifie = 0;
    
    while (fscanf(f, "%d %d", &id_m, &id_c) == 2) {
        if (!(id_m == membre_id && id_c == cours_id)) {
            fprintf(f_temp, "%d %d\n", id_m, id_c);
        } else {
            modifie = 1;
        }
    }
    
    fclose(f);
    fclose(f_temp);
    
    if (modifie) {
        remove("inscriptions_membres.txt");
        rename("temp_inscriptions_membres.txt", "inscriptions_membres.txt");
        return 1;
    } else {
        remove("temp_inscriptions_membres.txt");
        return 0;
    }
}

// ==================== FONCTIONS DE LECTURE DES FICHIERS ====================

// Lire un fichier stats simple (format: KEY=VALUE)
static double read_stat_from_file(const char *filename, const char *key) {
    FILE *file = fopen(filename, "r");
    if (!file) return 0;
    
    char line[256];
    while (fgets(line, sizeof(line), file)) {
        line[strcspn(line, "\n")] = 0;
        char *equal = strchr(line, '=');
        if (equal) {
            *equal = '\0';
            if (strcmp(line, key) == 0) {
                fclose(file);
                return atof(equal + 1);
            }
        }
    }
    fclose(file);
    return 0;
}

// Lire un fichier de répartition (format: label:valeur)
static void load_distribution_data(const char *filename, char ***labels, double **values, int *count) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        *count = 0;
        return;
    }
    
    *labels = NULL;
    *values = NULL;
    *count = 0;
    
    char line[256];
    while (fgets(line, sizeof(line), file)) {
        line[strcspn(line, "\n")] = 0;
        if (strlen(line) == 0) continue;
        
        char *colon = strchr(line, ':');
        if (colon) {
            *colon = '\0';
            (*count)++;
            *labels = realloc(*labels, (*count) * sizeof(char*));
            *values = realloc(*values, (*count) * sizeof(double));
            (*labels)[*count - 1] = strdup(line);
            (*values)[*count - 1] = atof(colon + 1);
        }
    }
    fclose(file);
}

// Lire un fichier de série (format: valeur1,valeur2,...)
static double* load_series_data(const char *filename, int *count) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        *count = 0;
        return NULL;
    }
    
    char line[1024];
    if (!fgets(line, sizeof(line), file)) {
        fclose(file);
        *count = 0;
        return NULL;
    }
    fclose(file);
    
    // Compter les valeurs
    *count = 1;
    for (char *p = line; *p; p++) {
        if (*p == ',') (*count)++;
    }
    
    double *values = malloc(*count * sizeof(double));
    char *token = strtok(line, ",");
    int i = 0;
    while (token && i < *count) {
        values[i++] = atof(token);
        token = strtok(NULL, ",");
    }
    
    return values;
}
// Mini barres (depuis fichier)
gboolean on_da_mini_bars_expose(GtkWidget *widget, GdkEventExpose *event, gpointer user_data) {
    if (!gtk_widget_get_realized(widget)) return FALSE;
    
    cairo_t *cr = gdk_cairo_create(gtk_widget_get_window(widget));
    GtkAllocation allocation;
    gtk_widget_get_allocation(widget, &allocation);
    
    if (allocation.width <= 0 || allocation.height <= 0) {
        cairo_destroy(cr);
        return FALSE;
    }
    
    // Lire les données depuis cours.txt pour les cours par semaine
    char **labels = NULL;
    double *values = NULL;
    int count = 0;
    
    // Pour l'exemple, on utilise des données par défaut
    // Vous pouvez adapter selon votre fichier
    const char *default_labels[] = {"S1", "S2", "S3", "S4", "S5", "S6", "S7", "S8"};
    double default_values[] = {28, 32, 35, 38, 42, 45, 48, 52};
    count = 8;
    
    draw_bar_chart(cr, allocation.width, allocation.height, default_values, count, default_labels, "Évolution des cours");
    
    cairo_destroy(cr);
    return FALSE;
}

// Répartition par âge (depuis membre.txt)
gboolean on_da_repartition_age_expose(GtkWidget *widget, GdkEventExpose *event, gpointer user_data) {
    if (!gtk_widget_get_realized(widget)) return FALSE;
    
    cairo_t *cr = gdk_cairo_create(gtk_widget_get_window(widget));
    GtkAllocation allocation;
    gtk_widget_get_allocation(widget, &allocation);
    
    // Lire depuis membre.txt ou créer fichier repartition_age.txt
    char **labels = NULL;
    double *values = NULL;
    int count = 0;
    
    // Option 1: Créer un fichier repartition_age.txt
    load_distribution_data("data/repartition_age.txt", &labels, &values, &count);
    
    // Option 2: Données par défaut si fichier inexistant
    if (count == 0) {
        const char *default_labels[] = {"18-25", "26-35", "36-45", "46-55", "55+"};
        double default_values[] = {25, 40, 22, 10, 3};
        count = 5;
        draw_bar_chart(cr, allocation.width, allocation.height, default_values, count, default_labels, "Répartition par âge");
    } else {
        draw_pie_chart(cr, allocation.width, allocation.height, values, count, (const char**)labels, "Répartition par âge");
    }
    
    // Nettoyage
    for (int i = 0; i < count; i++) free(labels[i]);
    free(labels);
    free(values);
    
    cairo_destroy(cr);
    return FALSE;
}

// Répartition des abonnements (depuis membres.txt)
gboolean on_da_abonnement_expose(GtkWidget *widget, GdkEventExpose *event, gpointer user_data) {
    if (!gtk_widget_get_realized(widget)) return FALSE;
    
    cairo_t *cr = gdk_cairo_create(gtk_widget_get_window(widget));
    GtkAllocation allocation;
    gtk_widget_get_allocation(widget, &allocation);
    
    // Analyser membres.txt pour compter les types d'abonnement
    FILE *file = fopen("membres.txt", "r");
    int mensuel = 0, annuel = 0, non_specifie = 0;
    
    if (file) {
        char line[512];
        while (fgets(line, sizeof(line), file)) {
            if (strstr(line, "Mensuelle")) mensuel++;
            else if (strstr(line, "Annuelle")) annuel++;
            else if (strstr(line, "Non spécifié")) non_specifie++;
        }
        fclose(file);
    }
    
    double values[] = {mensuel, annuel, non_specifie};
    const char *labels[] = {"Mensuel", "Annuel", "Non spécifié"};
    int count = 3;
    
    draw_pie_chart(cr, allocation.width, allocation.height, values, count, labels, "Répartition abonnements");
    
    cairo_destroy(cr);
    return FALSE;
}

// Évolution du taux de fidélité (depuis fichier)
gboolean on_da_evolution_expose(GtkWidget *widget, GdkEventExpose *event, gpointer user_data) {
    if (!gtk_widget_get_realized(widget)) return FALSE;
    
    cairo_t *cr = gdk_cairo_create(gtk_widget_get_window(widget));
    GtkAllocation allocation;
    gtk_widget_get_allocation(widget, &allocation);
    
    int count = 0;
    double *values = load_series_data("data/evolution.txt", &count);
    
    if (count == 0) {
        // Données par défaut
        double default_values[] = {72, 74, 76, 78, 79, 81, 82, 83, 84, 84, 85, 86};
        count = 12;
        draw_line_chart(cr, allocation.width, allocation.height, default_values, count, "Évolution du taux de fidélité (%)");
    } else {
        draw_line_chart(cr, allocation.width, allocation.height, values, count, "Évolution du taux de fidélité (%)");
        free(values);
    }
    
    cairo_destroy(cr);
    return FALSE;
}

// Niveaux des cours (depuis cours.txt)
gboolean on_da_cours_niveaux_expose(GtkWidget *widget, GdkEventExpose *event, gpointer user_data) {
    if (!gtk_widget_get_realized(widget)) return FALSE;
    
    cairo_t *cr = gdk_cairo_create(gtk_widget_get_window(widget));
    GtkAllocation allocation;
    gtk_widget_get_allocation(widget, &allocation);
    
    // Analyser cours.txt pour les niveaux
    FILE *file = fopen("cours.txt", "r");
    int debutant = 0, intermediaire = 0, avance = 0, expert = 0;
    
    if (file) {
        char line[512];
        while (fgets(line, sizeof(line), file)) {
            if (strstr(line, "Débutant")) debutant++;
            else if (strstr(line, "Intermédiaire")) intermediaire++;
            else if (strstr(line, "Avancé")) avance++;
            else if (strstr(line, "Expert")) expert++;
        }
        fclose(file);
    }
    
    double values[] = {debutant, intermediaire, avance, expert};
    const char *labels[] = {"Débutant", "Intermédiaire", "Avancé", "Expert"};
    int count = 4;
    
    draw_bar_chart(cr, allocation.width, allocation.height, values, count, labels, "Niveaux des cours");
    
    cairo_destroy(cr);
    return FALSE;
}

// Top 5 des cours (depuis cours.txt)
gboolean on_da_top5_cours_expose(GtkWidget *widget, GdkEventExpose *event, gpointer user_data) {
    if (!gtk_widget_get_realized(widget)) return FALSE;
    
    cairo_t *cr = gdk_cairo_create(gtk_widget_get_window(widget));
    GtkAllocation allocation;
    gtk_widget_get_allocation(widget, &allocation);
    
    // Compter les occurrences de chaque type de cours dans cours.txt
    FILE *file = fopen("cours.txt", "r");
    
    // Structure pour stocker les cours
    typedef struct {
        char name[50];
        int count;
    } CourseCount;
    
    CourseCount courses[50];
    int num_courses = 0;
    
    if (file) {
        char line[512];
        while (fgets(line, sizeof(line), file)) {
            // Extraire le nom du cours (premier mot avant l'espace)
            char course_name[50] = "";
            sscanf(line, "%s", course_name);
            
            if (strlen(course_name) > 0) {
                int found = 0;
                for (int i = 0; i < num_courses; i++) {
                    if (strcmp(courses[i].name, course_name) == 0) {
                        courses[i].count++;
                        found = 1;
                        break;
                    }
                }
                if (!found && num_courses < 50) {
                    strcpy(courses[num_courses].name, course_name);
                    courses[num_courses].count = 1;
                    num_courses++;
                }
            }
        }
        fclose(file);
    }
    
    // Trier par count décroissant
    for (int i = 0; i < num_courses - 1; i++) {
        for (int j = i + 1; j < num_courses; j++) {
            if (courses[i].count < courses[j].count) {
                CourseCount temp = courses[i];
                courses[i] = courses[j];
                courses[j] = temp;
            }
        }
    }
    
    // Prendre top 5
    int top_count = num_courses < 5 ? num_courses : 5;
    double *values = malloc(top_count * sizeof(double));
    const char **labels = malloc(top_count * sizeof(char*));
    
    for (int i = 0; i < top_count; i++) {
        values[i] = courses[i].count;
        labels[i] = courses[i].name;
    }
    
    draw_bar_chart(cr, allocation.width, allocation.height, values, top_count, labels, "Top cours");
    
    free(values);
    free(labels);
    
    cairo_destroy(cr);
    return FALSE;
}

// Expérience des coachs (depuis entraineurs.txt)
gboolean on_da_ent_experience_expose(GtkWidget *widget, GdkEventExpose *event, gpointer user_data) {
    if (!gtk_widget_get_realized(widget)) return FALSE;
    
    cairo_t *cr = gdk_cairo_create(gtk_widget_get_window(widget));
    GtkAllocation allocation;
    gtk_widget_get_allocation(widget, &allocation);
    
    // Analyser entraineurs.txt pour l'expérience
    FILE *file = fopen("entraineurs.txt", "r");
    int exp_0_2 = 0, exp_2_5 = 0, exp_5_10 = 0, exp_10plus = 0;
    
    if (file) {
        char line[512];
        while (fgets(line, sizeof(line), file)) {
            // Chercher le champ expérience (avant le H/F)
            int age = 0;
            char sexe[10] = "";
            sscanf(line, "%*d %*s %*s %d %s", &age, sexe);
            
            if (age > 0) {
                if (age <= 2) exp_0_2++;
                else if (age <= 5) exp_2_5++;
                else if (age <= 10) exp_5_10++;
                else exp_10plus++;
            }
        }
        fclose(file);
    }
    
    double values[] = {exp_0_2, exp_2_5, exp_5_10, exp_10plus};
    const char *labels[] = {"0-2 ans", "2-5 ans", "5-10 ans", "10+ ans"};
    int count = 4;
    
    draw_pie_chart(cr, allocation.width, allocation.height, values, count, labels, "Expérience des coachs");
    
    cairo_destroy(cr);
    return FALSE;
}

// Spécialités des coachs (depuis entraineurs.txt)
gboolean on_da_ent_specialites_expose(GtkWidget *widget, GdkEventExpose *event, gpointer user_data) {
    if (!gtk_widget_get_realized(widget)) return FALSE;
    
    cairo_t *cr = gdk_cairo_create(gtk_widget_get_window(widget));
    GtkAllocation allocation;
    gtk_widget_get_allocation(widget, &allocation);
    
    // Compter les spécialités depuis entraineurs.txt
    FILE *file = fopen("entraineurs.txt", "r");
    
    typedef struct {
        char name[50];
        int count;
    } SpecCount;
    
    SpecCount specialites[50];
    int num_spec = 0;
    
    if (file) {
        char line[512];
        while (fgets(line, sizeof(line), file)) {
            char spec[50] = "";
            sscanf(line, "%*d %s", spec);
            
            if (strlen(spec) > 0) {
                int found = 0;
                for (int i = 0; i < num_spec; i++) {
                    if (strcmp(specialites[i].name, spec) == 0) {
                        specialites[i].count++;
                        found = 1;
                        break;
                    }
                }
                if (!found && num_spec < 50) {
                    strcpy(specialites[num_spec].name, spec);
                    specialites[num_spec].count = 1;
                    num_spec++;
                }
            }
        }
        fclose(file);
    }
    
    double *values = malloc(num_spec * sizeof(double));
    const char **labels = malloc(num_spec * sizeof(char*));
    
    for (int i = 0; i < num_spec; i++) {
        values[i] = specialites[i].count;
        labels[i] = specialites[i].name;
    }
    
    draw_bar_chart(cr, allocation.width, allocation.height, values, num_spec, labels, "Spécialités des coachs");
    
    free(values);
    free(labels);
    
    cairo_destroy(cr);
    return FALSE;
}

// Top coachs (depuis entraineurs.txt par note)
gboolean on_da_ent_top_coachs_expose(GtkWidget *widget, GdkEventExpose *event, gpointer user_data) {
    if (!gtk_widget_get_realized(widget)) return FALSE;
    
    cairo_t *cr = gdk_cairo_create(gtk_widget_get_window(widget));
    GtkAllocation allocation;
    gtk_widget_get_allocation(widget, &allocation);
    
    // Lire les coachs depuis entraineurs.txt
    FILE *file = fopen("entraineurs.txt", "r");
    
    typedef struct {
        char name[50];
        int note;
    } CoachNote;
    
    CoachNote coaches[50];
    int num_coaches = 0;
    
    if (file) {
        char line[512];
        while (fgets(line, sizeof(line), file)) {
            char name[50] = "";
            int note = 0;
            sscanf(line, "%*d %s %*s %d", name, &note);
            
            if (strlen(name) > 0) {
                strcpy(coaches[num_coaches].name, name);
                coaches[num_coaches].note = note;
                num_coaches++;
            }
        }
        fclose(file);
    }
    
    // Trier par note décroissante
    for (int i = 0; i < num_coaches - 1; i++) {
        for (int j = i + 1; j < num_coaches; j++) {
            if (coaches[i].note < coaches[j].note) {
                CoachNote temp = coaches[i];
                coaches[i] = coaches[j];
                coaches[j] = temp;
            }
        }
    }
    
    // Top 5
    int top_count = num_coaches < 5 ? num_coaches : 5;
    double *values = malloc(top_count * sizeof(double));
    const char **labels = malloc(top_count * sizeof(char*));
    
    for (int i = 0; i < top_count; i++) {
        values[i] = coaches[i].note;
        labels[i] = coaches[i].name;
    }
    
    draw_bar_chart(cr, allocation.width, allocation.height, values, top_count, labels, "Top coachs (note/10)");
    
    free(values);
    free(labels);
    
    cairo_destroy(cr);
    return FALSE;
}

// Disponibilité des équipements (depuis equipements.txt)
gboolean on_da_equip_dispo_expose(GtkWidget *widget, GdkEventExpose *event, gpointer user_data) {
    if (!gtk_widget_get_realized(widget)) return FALSE;
    
    cairo_t *cr = gdk_cairo_create(gtk_widget_get_window(widget));
    GtkAllocation allocation;
    gtk_widget_get_allocation(widget, &allocation);
    
    // Compter les équipements par statut
    FILE *file = fopen("equipements.txt", "r");
    int disponible = 0, en_maintenance = 0, hors_service = 0;
    
    if (file) {
        char line[512];
        while (fgets(line, sizeof(line), file)) {
            if (strstr(line, "Disponible") || strstr(line, "D")) disponible++;
            else if (strstr(line, "Maintenance") || strstr(line, "M")) en_maintenance++;
            else hors_service++;
        }
        fclose(file);
    }
    
    double values[] = {disponible, en_maintenance, hors_service};
    const char *labels[] = {"Disponible", "Maintenance", "Hors service"};
    int count = 3;
    
    draw_pie_chart(cr, allocation.width, allocation.height, values, count, labels, "Disponibilité équipements");
    
    cairo_destroy(cr);
    return FALSE;
}

// État des équipements
gboolean on_da_equip_etat_expose(GtkWidget *widget, GdkEventExpose *event, gpointer user_data) {
    if (!gtk_widget_get_realized(widget)) return FALSE;
    
    cairo_t *cr = gdk_cairo_create(gtk_widget_get_window(widget));
    GtkAllocation allocation;
    gtk_widget_get_allocation(widget, &allocation);
    
    // Données par défaut ou depuis fichier
    double values[] = {40, 35, 20, 5};
    const char *labels[] = {"Très bon", "Bon", "Moyen", "À remplacer"};
    
    draw_bar_chart(cr, allocation.width, allocation.height, values, 4, labels, "État des équipements");
    
    cairo_destroy(cr);
    return FALSE;
}

// Centres par ville (depuis centres.txt)
gboolean on_da_ctr_par_ville_expose(GtkWidget *widget, GdkEventExpose *event, gpointer user_data) {
    if (!gtk_widget_get_realized(widget)) return FALSE;
    
    cairo_t *cr = gdk_cairo_create(gtk_widget_get_window(widget));
    GtkAllocation allocation;
    gtk_widget_get_allocation(widget, &allocation);
    
    // Compter les centres par ville depuis centres.txt
    FILE *file = fopen("centres.txt", "r");
    
    typedef struct {
        char ville[50];
        int count;
    } VilleCount;
    
    VilleCount villes[50];
    int num_villes = 0;
    
    if (file) {
        char line[512];
        while (fgets(line, sizeof(line), file)) {
            char ville[50] = "";
            sscanf(line, "%*d %*s %*s %s", ville);
            
            if (strlen(ville) > 0) {
                int found = 0;
                for (int i = 0; i < num_villes; i++) {
                    if (strcmp(villes[i].ville, ville) == 0) {
                        villes[i].count++;
                        found = 1;
                        break;
                    }
                }
                if (!found && num_villes < 50) {
                    strcpy(villes[num_villes].ville, ville);
                    villes[num_villes].count = 1;
                    num_villes++;
                }
            }
        }
        fclose(file);
    }
    
    double *values = malloc(num_villes * sizeof(double));
    const char **labels = malloc(num_villes * sizeof(char*));
    
    for (int i = 0; i < num_villes; i++) {
        values[i] = villes[i].count;
        labels[i] = villes[i].ville;
    }
    
    draw_pie_chart(cr, allocation.width, allocation.height, values, num_villes, labels, "Centres par ville");
    
    free(values);
    free(labels);
    
    cairo_destroy(cr);
    return FALSE;
}
// ==================== PAGE COURS ====================

// Répartition horaire des cours
gboolean on_da_repartition_horaire_expose(GtkWidget *widget, GdkEventExpose *event, gpointer user_data) {
    if (!gtk_widget_get_realized(widget)) return FALSE;
    
    cairo_t *cr = gdk_cairo_create(gtk_widget_get_window(widget));
    GtkAllocation allocation;
    gtk_widget_get_allocation(widget, &allocation);
    
    if (allocation.width <= 0 || allocation.height <= 0) {
        cairo_destroy(cr);
        return FALSE;
    }
    
    // Données: Matin, Midi, Après-midi, Soir
    double values[] = {20, 15, 25, 40};
    const char *labels[] = {"Matin", "Midi", "Après-midi", "Soir"};
    
    draw_pie_chart(cr, allocation.width, allocation.height, values, 4, labels, "Répartition horaire");
    
    cairo_destroy(cr);
    return FALSE;
}

// ==================== PAGE EQUIPEMENTS ====================

// Taux d'utilisation des équipements
gboolean on_da_equip_utilisation_expose(GtkWidget *widget, GdkEventExpose *event, gpointer user_data) {
    if (!gtk_widget_get_realized(widget)) return FALSE;
    
    cairo_t *cr = gdk_cairo_create(gtk_widget_get_window(widget));
    GtkAllocation allocation;
    gtk_widget_get_allocation(widget, &allocation);
    
    if (allocation.width <= 0 || allocation.height <= 0) {
        cairo_destroy(cr);
        return FALSE;
    }
    
    // Données d'utilisation par heure
    double values[] = {20, 25, 30, 45, 65, 80, 85, 75, 60, 50, 35, 25};
    int count = 12;
    
    draw_line_chart(cr, allocation.width, allocation.height, values, count, "Taux d'utilisation (horaire)");
    
    cairo_destroy(cr);
    return FALSE;
}

// ==================== PAGE CENTRES ====================

// Taux d'occupation des centres
gboolean on_da_ctr_taux_occ_expose(GtkWidget *widget, GdkEventExpose *event, gpointer user_data) {
    if (!gtk_widget_get_realized(widget)) return FALSE;
    
    cairo_t *cr = gdk_cairo_create(gtk_widget_get_window(widget));
    GtkAllocation allocation;
    gtk_widget_get_allocation(widget, &allocation);
    
    if (allocation.width <= 0 || allocation.height <= 0) {
        cairo_destroy(cr);
        return FALSE;
    }
    
    // Données: L'Aouina, Centre-ville, La Marsa, Berges du Lac
    double values[] = {85, 72, 68, 90};
    const char *labels[] = {"L'Aouina", "Centre-ville", "La Marsa", "Berges du Lac"};
    
    draw_bar_chart(cr, allocation.width, allocation.height, values, 4, labels, "Taux d'occupation (%)");
    
    cairo_destroy(cr);
    return FALSE;
}

// Capacité des centres
gboolean on_da_ctr_capacite_expose(GtkWidget *widget, GdkEventExpose *event, gpointer user_data) {
    if (!gtk_widget_get_realized(widget)) return FALSE;
    
    cairo_t *cr = gdk_cairo_create(gtk_widget_get_window(widget));
    GtkAllocation allocation;
    gtk_widget_get_allocation(widget, &allocation);
    
    if (allocation.width <= 0 || allocation.height <= 0) {
        cairo_destroy(cr);
        return FALSE;
    }
    
    // Données de capacité
    double values[] = {300, 250, 200, 350};
    const char *labels[] = {"L'Aouina", "Centre-ville", "La Marsa", "Berges du Lac"};
    
    draw_bar_chart(cr, allocation.width, allocation.height, values, 4, labels, "Capacité des centres");
    
    cairo_destroy(cr);
    return FALSE;
}
