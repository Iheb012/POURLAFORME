# Intégration de l'API Groq dans l'interface membre

## Résumé

L'intégration de l'API Groq pour le chatbot Midou dans l'interface membre a été complétée avec succès. Voici ce qui a été mis en place :

## Modifications effectuées

### 1. Fichier `src/callbacks.c`
- Ajout des includes pour `curl/curl.h`
- Implémentation des fonctions pour l'API Groq :
  - `init_string()` et `writefunc()` pour gérer les réponses HTTP
  - `parse_json_response()` pour parser les réponses JSON simples
  - `call_grok_api()` pour appeler l'API Groq avec la clé d'API
- Modification de `on_btn_chatbot_send_clicked()` pour utiliser l'API réelle au lieu des réponses simulées

### 2. Fichier `src/Makefile.am`
- Ajout de `-lcurl` à `project3_LDADD`

### 3. Fichier `src/Makefile.in`
- Ajout de `-lcurl` à `project3_LDADD`

### 4. Fichier `src/Makefile`
- Ajout de `-lcurl` à `project3_LDADD`

### 5. Fichier `key.txt`
- Création d'un fichier modèle pour la clé API Groq

## Configuration requise

### Dépendances système
- **libcurl** : Pour les requêtes HTTP
- **GTK+ 2.0** : Pour l'interface graphique
- **Compilateur C** : GCC ou équivalent

### Clé API Groq
1. Obtenez une clé API Groq gratuite sur : https://console.groq.com/keys
2. Créez un fichier `key.txt` à la racine du projet
3. Collez votre clé API (commençant par `gsk_`) dans le fichier

Exemple de `key.txt` :
```
gsk_abc123def456ghi789jkl012mno345pqr678
```

## Compilation

### Sur Linux/Unix :
```bash
cd src
make clean
make
```

### Sur Windows (avec MinGW) :
```bash
cd src
mingw32-make clean
mingw32-make
```

## Fonctionnalités implémentées

### Interface chatbot existante
L'interface chatbot était déjà présente dans le fichier GLADE (`project3.glade`) avec :
- Zone de texte pour l'historique des messages (`tv_chatbot`)
- Champ de saisie pour les messages (`entry_chatbot_msg`)
- Bouton d'envoi (`btn_envoyer`)

### Nouvelle fonctionnalité AI
- **Messages en temps réel** : Affichage d'un indicateur "Midou: Tape..." pendant le traitement
- **Appel API Groq** : Utilisation du modèle `llama-3.1-8b-instant`
- **Gestion d'erreurs** : Messages d'erreur explicites pour les problèmes de clé API ou de réseau
- **Décodage JSON** : Analyse simple des réponses JSON sans dépendances externes

## Messages d'erreur gérés

1. **Fichier key.txt introuvable**
2. **Clé API invalide ou vide**
3. **Erreurs de réseau ou de connexion**
4. **Réponses API invalides**

## Tests

Pour tester l'intégration :

1. Compilez le projet
2. Lancez l'application
3. Connectez-vous en tant que membre
4. Allez dans le tableau de bord membre
5. Utilisez le chatbot dans le cadre "Midou (Chatbot)"
6. Tapez un message et cliquez sur "Envoyer"

## Notes techniques

- **Performance** : Les appels API sont asynchrones et n'bloquent pas l'interface
- **Sécurité** : La clé API est lue depuis un fichier local, pas codée en dur
- **Compatibilité** : Utilise une approche simple de parsing JSON sans bibliothèque externe
- **UI/UX** : Feedback visuel pendant le traitement des requêtes

## Prochaines améliorations possibles

1. **Cache des réponses** : Mémoriser les réponses pour des questions similaires
2. **Historique des conversations** : Sauvegarder les conversations par utilisateur
3. **Multi-langues** : Support multilingue pour le chatbot
4. **Intégration de contexte** : Utiliser les informations du membre pour des réponses personnalisées

## Dépannage

### Erreurs courantes :

1. **`undefined reference to curl_easy_init`** :
   - Vérifiez que libcurl est installé : `sudo apt-get install libcurl4-openssl-dev` (Ubuntu)
   - Ou `sudo yum install libcurl-devel` (CentOS/RHEL)

2. **Fichier key.txt non trouvé** :
   - Créez le fichier à la racine du projet
   - Vérifiez les permissions de lecture

3. **Erreurs de compilation GTK** :
   - Installez les développement libraries GTK : `sudo apt-get install libgtk2.0-dev`

## Support

Pour toute question ou problème, consultez :
- Documentation Groq API : https://console.groq.com/docs
- Documentation libcurl : https://curl.se/libcurl/
- Documentation GTK+ : https://www.gtk.org/docs/