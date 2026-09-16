#include "client.h"
#include <ctype.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef size_t usize;
const char END[] = "Félicitations";

int main() {

  // Affiche les échanges avec le serveur (false pour désactiver)
  show_messages(true);

  // Connexion au serveur AppoLab
  connexion("im2ag-appolab.u-ga.fr");

  FILE *file = fopen("commandsFORT.txt", "r");
  FILE *credentials = fopen(".env", "r");

  if (file == NULL || credentials == NULL) {
    printf("Couldn't open the file");
    return 1;
  }

  char buffer[256];
  char buffer_answers[MAXREP];

  while (fgets(buffer, sizeof(buffer), credentials)) {
    envoyer(buffer);
  }
  while (fgets(buffer, sizeof(buffer), file)) {
    envoyer_recevoir(buffer, buffer_answers);
  }

  u16 length = -1;
  while (buffer_answers[0] != '\0' && length != 0 &&
         !strstr(buffer_answers, END)) {
    length = strlen(buffer_answers);
    char buffer_to_answer[MAXREP] = {0};
    for (usize u = 0; u <= length; u++) {
      buffer_to_answer[u] = (char)toupper((unsigned char)buffer_answers[u]);
    }

    buffer_to_answer[length] = '\0';
    envoyer_recevoir(buffer_to_answer, buffer_answers);
  }

  printf("Fin d'envoi des messages.\n");
  printf("Pour envoyer d'autres lignes, ajouter des appels à la fonction "
         "`envoyer`\n");
  deconnexion();
  printf("Fin de la connection au serveur\n");
  return 0;
}
