#include "client-crypteMove.h"
#include "client.h"
#include <stdio.h>

int main(void) {

  // Affiche les échanges avec le serveur (false pour désactiver)
  show_messages(true);

  // Connexion au serveur AppoLab
  connexion("im2ag-appolab.u-ga.fr");

  // Need to put those two following files at the / of the project (APP1/./) and
  // to execute from there as well ...
  FILE *credentials = fopen("./.env", "r");
  FILE *file = fopen("./commands_cryptseq.txt", "r");

  if (file == NULL || credentials == NULL) {
    printf("Couldn't open the file");
    return 1;
  }

  char buffer[256];
  char answer[MAXREP];

  while (fgets(buffer, sizeof(buffer), credentials)) {
    envoyer(buffer);
  }
  while (fgets(buffer, sizeof(buffer), file)) {
    envoyer_recevoir(buffer, answer);
  }
  char ans[MAXREP];
  char buffer2[] = "Patria o muerte"; // midnight no idea what I am doing btw
  char ans2[MAXREP];
  decrypt(answer, ans);
  encrypt(buffer2, ans2);
  printf("%s\n", ans);
  printf("%s\n", ans2);
  printf("Fin d'envoi des messages.\n");
  printf("Pour envoyer d'autres lignes, ajouter des appels à la fonction "
         "`envoyer`\n");
  deconnexion();
  printf("Fin de la connection au serveur\n");
  return 0;
}
