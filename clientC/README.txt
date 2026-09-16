Quickstart sur Turing (ou machine perso)
========================================

Pour démarrer la présentation d'AppoLab, commencez par compiler le tout:
> make

Puis lancez le programme interactif d'introduction.
> ./client-interactif

et suivez les consignes :-)


Quickstart sur Caseine
======================

Exécutez le code avec le bouton "fusée".
Pour changer de programme à utiliser, modifiez la variable MAIN au début du Makefile.



Liste des fichiers :
====================

client-interactif.c
  => à lancer initialement pour découvrir AppoLab
  => à utiliser plus tard pour découvrir les exercices d'AppoLab
     de manière interactive (en envoyant des messages au clavier)

client-tutoriel.c
  => sera utilisé pendant le tutoriel, pour programmer à l'avance
     les échanges avec AppoLab
     /!\ pensez à recompiler (avec `make`) après modification /!\

client.h
  => interface de la librairie de communication avec AppoLab.
     À NE PAS MODIFIER !! (sauf si vous voulez changer les couleurs d'affichage)

client.c
  => librairie de communication avec AppoLab. À NE PAS MODIFIER !!
     (mais vous pouvez aller regarder dedans si vous osez... :-)
