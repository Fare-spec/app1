#include "client-projetX.h"
#include "client.h"
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

int main() {
  show_messages(true);
  connexion("im2ag-appolab.u-ga.fr");

  FILE *file = fopen("./commands.txt", "r");
  FILE *credentials = fopen("./.env", "r");

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

  uint8_t offset = find_offset(answer);
  char decoded_answer[MAXREP];
  create_offset(answer, offset, decoded_answer);
  printf("%s\n", decoded_answer);

  printf("Fin d'envoi des messages.\n");
  deconnexion();
  printf("Fin de la connection au serveur\n");
  return 0;
}
