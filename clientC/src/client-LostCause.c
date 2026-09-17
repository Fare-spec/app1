#include "client-cryptseq.h"
#include "client.h"
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

int main() {

  // Affiche les échanges avec le serveur (false pour désactiver)
  show_messages(true);

  // Connexion au serveur AppoLab
  connexion("im2ag-appolab.u-ga.fr");

  // Need to put those two following files at the / of the project (APP1/./) and
  // to execute from there as well ...
  FILE *file = fopen("./commands_LostCause.txt", "r");
  FILE *credentials = fopen("./.env", "r");

  if (file == NULL || credentials == NULL) {
    printf("Couldn't open the file");
    return 1;
  }

  char buffer[256];
  char answer[MAXREP];

  char answer_d[MAXREP];

  while (fgets(buffer, sizeof(buffer), credentials)) {
    envoyer(buffer);
  }
  while (fgets(buffer, sizeof(buffer), file)) {
    envoyer_recevoir(buffer, answer);
  }
  cryptseq_de(answer, answer_d);
  printf("%s\n", answer_d);

  printf("Fin d'envoi des messages.\n");
  printf("Pour envoyer d'autres lignes, ajouter des appels à la fonction "
         "`envoyer`\n");
  deconnexion();
  printf("Fin de la connection au serveur\n");
  return 0;
}
