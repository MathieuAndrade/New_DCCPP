# Comprendre le timer

Le timer renseigné au moment de la connexion à la centrale est exprimé en millisecondes (1 seconde équivaut à 1000 millisecondes). Ce timer représente le "temps" entre chaque commande envoyées à la centrale Arduino.

Les centrales Arduino sont de petites cartes limitées en puissance et ne sont pas toutes équivalentes, certaines cartes dites "clones" (cartes Arduino non officielles) peuvent parfois manquer de puissance au démarrage.

Ce timer permet donc d'empêcher CDM-Rail de surcharger la centrale au démarrage du mode run sur les plus gros réseaux. Nous préconisons en moyenne un timer de 50 millisecondes (qui suffit à la plupart des centrales Arduino).

:warning:
Le timer ne possède pas de valeur minimum, il peut donc être égale à 0 afin de n'avoir aucun temps de latence entre les commandes. A contrario il impossible d'avoir un timer plus grand que 200 millisecondes (ce qui correspond à 0.2 s) afin d'éviter des décalages trop importants en mode run sur CDM-Rail.