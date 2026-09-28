#include "client-crypteMove.h"
#include "client.h"
#include <stdio.h>
#include <string.h>

int main(void) {
  show_messages(true);
  connexion("im2ag-appolab.u-ga.fr");

  FILE *credentials = fopen("./.env", "r");
  FILE *file = fopen("./commands_cryptemove.txt", "r");

  if (file == NULL || credentials == NULL) {
    printf("Couldn't open the file");
    return 1;
  }

  char buffer[256];
  char encrypted_help[MAXREP];
  char ciphertext[MAXREP];
  char response[MAXREP];
  bool has_help = false;

  while (fgets(buffer, sizeof(buffer), credentials)) {
    envoyer(buffer);
  }

  while (fgets(buffer, sizeof(buffer), file)) {
    buffer[strcspn(buffer, "\r\n")] = '\0';

    if (strcmp(buffer, "help") == 0) {
      envoyer_recevoir(buffer, encrypted_help);
      has_help = true;
    } else {
      envoyer(buffer);
    }
  }

  if (has_help) {
    encrypt(encrypted_help, ciphertext);
    printf("\nMessage d'aide chiffre :\n%s\n", ciphertext);
    envoyer_recevoir(ciphertext, response);
    printf("\nReponse finale du serveur :\n%s\n", response);
  }

  fclose(file);
  fclose(credentials);
  deconnexion();
  return 0;
}
