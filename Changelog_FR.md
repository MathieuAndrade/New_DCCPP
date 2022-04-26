# Changelog

## 1.0.0 - 26/04/2022
- Réécriture complete de la DLL en C++ pour plus de stabilité et de performance
- Prise en charge des Arduino "clones" en USB
- Possibilité d'établir une connexion Wifi (Websockets) pour les centrales basées sur des Wemos
- Sauvegarde des paramètres de connexion
- Stabilisation de la connexion au démarrage de la DLL (redondance de tentative, très utile pour les Arduino un peu lent)
- Amelioration de la gestion des messages entrant et sortant

### Fonctionnalités prises en charge dans cette version :
- Arrêts d'urgence
- Vitesse et Fonctions des locos
- Activation des accessoires
- Prise en charge du protocol de detection S88
- Arrêt et démarrage de la centrale
- Lecture de l'état de la centrale

### Bugs rencontrés sur cette version :
- Perte de fonctionnement des accessoires après un arrêt d'urgence (extremest aléatoire et tout aussi rare donc difficile à saisir et a corrigé..)

--------------------

### Fonctionnalités non prises en charge (pour le moment):
- Lecture/Écriture des CVs
- Affichage des erreurs
- Affichage des statistiques
- Affichage du buffer

### Fonctionnalités en cours de développement :
- Lecture/Écriture des CVs
- Retour d'état de la vitesse d'une loco (en cas de pilotage parallel par un autre controller, ce qui peut être le cas avec une centrale Wifi)
- Retour d'état des fonctions d'une loco (en cas de pilotage parallel par un autre controller, ce qui peut être le cas avec une centrale Wifi)