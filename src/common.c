#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <sys/stat.h>

// ============================================================================
// FONCTIONS GESTION FICHIERS
// ============================================================================

int fichier_existe(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (file) {
        fclose(file);
        return 1;
    }
    return 0;
}

void creer_fichier_si_inexistant(const char *filename) {
    if (!fichier_existe(filename)) {
        FILE *file = fopen(filename, "w");
        if (file) {
            fclose(file);
            printf("Fichier %s créé\n", filename);
        } else {
            printf("Erreur création fichier %s\n", filename);
        }
    }
}

void initialiser_fichiers_donnees() {
    printf("Initialisation des fichiers de données...\n");
    
    creer_fichier_si_inexistant("login.txt");
    creer_fichier_si_inexistant("membres.txt");
    creer_fichier_si_inexistant("entraineurs.txt");
    creer_fichier_si_inexistant("cours.txt");
    creer_fichier_si_inexistant("equipements.txt");
    creer_fichier_si_inexistant("centres.txt");
    creer_fichier_si_inexistant("inscriptions.txt");
    creer_fichier_si_inexistant("reservations.txt");
    creer_fichier_si_inexistant("demandes.txt");
    creer_fichier_si_inexistant("inscriptions_membres.txt");  // AJOUTEZ CETTE LIGNE
    
    if (est_fichier_vide("login.txt")) {
        ajouter_utilisateur("admin", "admin123", "admin", 0);
        printf("Compte admin créé (admin/admin123)\n");
    }
}

int compter_lignes_fichier(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) return 0;
    
    int count = 0;
    char buffer[256];
    while (fgets(buffer, sizeof(buffer), file)) {
        count++;
    }
    fclose(file);
    return count;
}

int est_fichier_vide(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) return 1;
    
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fclose(file);
    
    return size == 0;
}

// ============================================================================
// FONCTIONS AUTHENTIFICATION
// ============================================================================

int verifier_login(const char *username, const char *password, char *user_type, int *user_id) {
    FILE *file = fopen("login.txt", "r");
    if (!file) {
        printf("Erreur: Impossible d'ouvrir login.txt\n");
        return 0;
    }
    
    char ligne[256];
    char file_username[100], file_password[100], file_type[50];
    int file_id;
    int trouve = 0;
    
    while (fgets(ligne, sizeof(ligne), file)) {
        // Supprimer le saut de ligne
        ligne[strcspn(ligne, "\n")] = 0;
        
        // Ignorer les lignes vides
        if (strlen(ligne) == 0) continue;
        
        // Format de votre fichier: username password role id
        int n = sscanf(ligne, "%99s %99s %49s %d", 
                       file_username, file_password, file_type, &file_id);
        
        if (n == 4) {
            printf("Lu: user=%s, pass=%s, role=%s, id=%d\n", 
                   file_username, file_password, file_type, file_id);
            
            if (strcmp(file_username, username) == 0 && 
                strcmp(file_password, password) == 0) {
                strcpy(user_type, file_type);
                *user_id = file_id;
                trouve = 1;
                printf("Authentification réussie! Rôle: %s, ID: %d\n", user_type, file_id);
                break;
            }
        } else {
            printf("Format incorrect pour la ligne: %s (n=%d)\n", ligne, n);
        }
    }
    
    fclose(file);
    
    if (!trouve) {
        printf("Authentification échouée pour: %s\n", username);
    }
    
    return trouve;
}

int ajouter_utilisateur(const char *username, const char *password, const char *type, int user_id) {
    if (utilisateur_existe(username)) {
        printf("Erreur: L'utilisateur %s existe déjà\n", username);
        return 0;
    }
    
    FILE *file = fopen("login.txt", "a");
    if (!file) {
        printf("Erreur: Impossible d'ouvrir login.txt\n");
        return 0;
    }
    
    fprintf(file, "%s %s %s %d\n", username, password, type, user_id);
    fclose(file);
    return 1;
}

int utilisateur_existe(const char *username) {
    FILE *file = fopen("login.txt", "r");
    if (!file) return 0;
    
    Utilisateur user;
    int existe = 0;
    
    while (fscanf(file, "%49s %49s %19s %d", 
                  user.username, user.password, user.type, &user.user_id) == 4) {
        if (strcmp(user.username, username) == 0) {
            existe = 1;
            break;
        }
    }
    
    fclose(file);
    return existe;
}

int supprimer_utilisateur(const char *username) {
    FILE *file = fopen("login.txt", "r");
    if (!file) return 0;
    
    FILE *temp = fopen("temp.txt", "w");
    if (!temp) {
        fclose(file);
        return 0;
    }
    
    Utilisateur user;
    int supprime = 0;
    
    while (fscanf(file, "%49s %49s %19s %d", 
                  user.username, user.password, user.type, &user.user_id) == 4) {
        if (strcmp(user.username, username) != 0) {
            fprintf(temp, "%s %s %s %d\n", user.username, user.password, user.type, user.user_id);
        } else {
            supprime = 1;
        }
    }
    
    fclose(file);
    fclose(temp);
    
    remove("login.txt");
    rename("temp.txt", "login.txt");
    
    return supprime;
}

// ============================================================================
// FONCTIONS GÉNÉRATION ID
// ============================================================================

int get_prochain_id_membre() {
    return compter_lignes_fichier("membres.txt") + 1;
}

int get_prochain_id_entraineur() {
    return compter_lignes_fichier("entraineurs.txt") + 1;
}

int get_prochain_id_cours() {
    return compter_lignes_fichier("cours.txt") + 1;
}

int get_prochain_id_centre() {
    return compter_lignes_fichier("centres.txt") + 1;
}

char* generer_reference_equipement(const char *type) {
    static char reference[20];
    int count = compter_lignes_fichier("equipements.txt") + 1;
    
    if (strcmp(type, "cardio") == 0) {
        sprintf(reference, "CARD%03d", count);
    } else if (strcmp(type, "musculation") == 0) {
        sprintf(reference, "MUSC%03d", count);
    } else if (strcmp(type, "mobilite") == 0) {
        sprintf(reference, "MOB%03d", count);
    } else {
        sprintf(reference, "EQP%03d", count);
    }
    
    return reference;
}

// ============================================================================
// FONCTIONS VALIDATION
// ============================================================================

int est_email_valide(const char *email) {
    int at_count = 0;
    int dot_count = 0;
    int length = strlen(email);
    
    for (int i = 0; i < length; i++) {
        if (email[i] == '@') at_count++;
        if (email[i] == '.') dot_count++;
    }
    
    return (at_count == 1 && dot_count >= 1 && length >= 5);
}

int est_telephone_valide(const char *telephone) {
    int length = strlen(telephone);
    if (length < 8 || length > 15) return 0;
    
    for (int i = 0; i < length; i++) {
        if (!isdigit(telephone[i]) && telephone[i] != '+' && telephone[i] != ' ') {
            return 0;
        }
    }
    return 1;
}

int est_date_valide(const char *date) {
    // Format simple JJ/MM/AAAA
    int j, m, a;
    return sscanf(date, "%d/%d/%d", &j, &m, &a) == 3;
}

int est_nom_valide(const char *nom) {
    int length = strlen(nom);
    if (length < 2 || length > 29) return 0;
    
    for (int i = 0; i < length; i++) {
        if (!isalpha(nom[i]) && nom[i] != ' ' && nom[i] != '-') {
            return 0;
        }
    }
    return 1;
}

int est_age_valide(int age) {
    return (age >= 5 && age <= 120);
}

int est_prix_valide(float prix) {
    return (prix >= 0);
}

// ============================================================================
// FONCTIONS DATE ET HEURE
// ============================================================================

void get_date_actuelle(char *date) {
    time_t t = time(NULL);
    struct tm *tm_info = localtime(&t);
    strftime(date, 11, "%d/%m/%Y", tm_info);
}

void get_heure_actuelle(char *heure) {
    time_t t = time(NULL);
    struct tm *tm_info = localtime(&t);
    strftime(heure, 9, "%H:%M:%S", tm_info);
}

void get_datetime_actuelle(char *datetime) {
    time_t t = time(NULL);
    struct tm *tm_info = localtime(&t);
    strftime(datetime, 20, "%d/%m/%Y %H:%M:%S", tm_info);
}

int comparer_dates(const char *date1, const char *date2) {
    // Retourne -1 si date1 < date2, 0 si égales, 1 si date1 > date2
    int j1, m1, a1, j2, m2, a2;
    sscanf(date1, "%d/%d/%d", &j1, &m1, &a1);
    sscanf(date2, "%d/%d/%d", &j2, &m2, &a2);
    
    if (a1 != a2) return (a1 < a2) ? -1 : 1;
    if (m1 != m2) return (m1 < m2) ? -1 : 1;
    if (j1 != j2) return (j1 < j2) ? -1 : 1;
    return 0;
}

int ajouter_jours_date(const char *date_source, int jours, char *date_resultat) {
    // Implémentation simplifiée
    strcpy(date_resultat, date_source);
    return 1;
}

// ============================================================================
// FONCTIONS CHAÎNES DE CARACTÈRES
// ============================================================================

void trim_string(char *str) {
    char *end;
    
    // Trim leading space
    while(isspace((unsigned char)*str)) str++;
    
    if(*str == 0) return;
    
    // Trim trailing space
    end = str + strlen(str) - 1;
    while(end > str && isspace((unsigned char)*end)) end--;
    
    // Write new null terminator
    *(end+1) = 0;
}

void to_uppercase(char *str) {
    for(int i = 0; str[i]; i++) {
        str[i] = toupper(str[i]);
    }
}

void to_lowercase(char *str) {
    for(int i = 0; str[i]; i++) {
        str[i] = tolower(str[i]);
    }
}

int contient_seulement_lettres(const char *str) {
    for(int i = 0; str[i]; i++) {
        if(!isalpha(str[i]) && str[i] != ' ') {
            return 0;
        }
    }
    return 1;
}

int contient_seulement_chiffres(const char *str) {
    for(int i = 0; str[i]; i++) {
        if(!isdigit(str[i])) {
            return 0;
        }
    }
    return 1;
}

void supprimer_caracteres_speciaux(char *str) {
    char *p = str;
    while (*str) {
        if (isalnum((unsigned char)*str) || *str == ' ') {
            *p++ = *str;
        }
        str++;
    }
    *p = 0;
}

// ============================================================================
// FONCTIONS CONVERSION
// ============================================================================

char* statut_equipement_char_to_str(char statut) {
    static char str[20];
    switch(statut) {
        case 'D': strcpy(str, "Disponible"); break;
        case 'O': strcpy(str, "Occupé"); break;
        case 'M': strcpy(str, "Maintenance"); break;
        default: strcpy(str, "Inconnu");
    }
    return str;
}

char statut_equipement_str_to_char(const char *statut) {
    if (strcmp(statut, "Disponible") == 0) return 'D';
    if (strcmp(statut, "Occupé") == 0) return 'O';
    if (strcmp(statut, "Maintenance") == 0) return 'M';
    return 'I';
}

char* type_abonnement_to_str(int type) {
    static char str[20];
    switch(type) {
        case 1: strcpy(str, "Basique"); break;
        case 2: strcpy(str, "Premium"); break;
        case 3: strcpy(str, "VIP"); break;
        default: strcpy(str, "Inconnu");
    }
    return str;
}

char* sexe_to_str(char sexe) {
    static char str[10];
    switch(sexe) {
        case 'M': strcpy(str, "Masculin"); break;
        case 'F': strcpy(str, "Féminin"); break;
        default: strcpy(str, "Inconnu");
    }
    return str;
}

char sexe_str_to_char(const char *sexe) {
    if (strcmp(sexe, "Masculin") == 0) return 'M';
    if (strcmp(sexe, "Féminin") == 0) return 'F';
    return 'I';
}

// ============================================================================
// FONCTIONS STATISTIQUES
// ============================================================================

int get_nombre_membres_actifs() {
    return compter_lignes_fichier("membres.txt");
}

int get_nombre_entraineurs_actifs() {
    return compter_lignes_fichier("entraineurs.txt");
}

int get_nombre_cours_actifs() {
    return compter_lignes_fichier("cours.txt");
}

int get_nombre_equipements_disponibles() {
    // Implémentation simplifiée
    return compter_lignes_fichier("equipements.txt");
}

float get_taux_remplissage_centre(int centre_id) {
    // Implémentation simplifiée
    return 75.0f; // 75% pour l'exemple
}

// ============================================================================
// FONCTIONS RAPPORTS
// ============================================================================

void generer_rapport_membres(const char *filename) {
    FILE *rapport = fopen(filename, "w");
    if (!rapport) return;
    
    fprintf(rapport, "RAPPORT MEMBRES\n");
    fprintf(rapport, "===============\n");
    fprintf(rapport, "Nombre total: %d\n", get_nombre_membres_actifs());
    fprintf(rapport, "Date: ");
    
    char date[20];
    get_date_actuelle(date);
    fprintf(rapport, "%s\n", date);
    
    fclose(rapport);
}

void generer_rapport_cours(const char *filename) {
    FILE *rapport = fopen(filename, "w");
    if (!rapport) return;
    
    fprintf(rapport, "RAPPORT COURS\n");
    fprintf(rapport, "=============\n");
    fprintf(rapport, "Nombre total: %d\n", get_nombre_cours_actifs());
    
    fclose(rapport);
}

void generer_rapport_financier(const char *filename) {
    FILE *rapport = fopen(filename, "w");
    if (!rapport) return;
    
    fprintf(rapport, "RAPPORT FINANCIER\n");
    fprintf(rapport, "=================\n");
    fprintf(rapport, "Membres actifs: %d\n", get_nombre_membres_actifs());
    
    fclose(rapport);
}
