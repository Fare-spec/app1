#include "client-cryptseq.h"
#include "client.h"
#include <ctype.h>
#include <stdbool.h>
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

typedef float f32;

typedef size_t usize;

void extract_password(char *text, char *extracted) {
  // luckily we can use the fact that each ' is closed ( very lucky ) and that
  // the last one contains the password
  char marker = '\'';
  u8 into = 0;
  usize j = 0;

  for (usize i = 0; text[i] != '\0'; i++) {

    if (text[i] == marker) {
      if (into) {
        into = 0;
        extracted[j] = '\0';
      } else {
        into = 1;
        j = 0;
      }

      continue;
    }

    if (into) {
      extracted[j++] = text[i];
    }
  }
}
int main() {

  // Affiche les échanges avec le serveur (false pour désactiver)
  show_messages(true);

  // Connexion au serveur AppoLab
  connexion("im2ag-appolab.u-ga.fr");

  // Need to put those two following files at the / of the project (APP1/./) and
  // to execute from there as well ...
  FILE *file = fopen("./commands_northwoods.txt", "r");
  FILE *credentials = fopen("./.env", "r");

  if (file == NULL || credentials == NULL) {
    printf("Couldn't open the file");
    return 1;
  }

  char buffer[256];
  char ans[MAXREP];

  char ans2[MAXREP];

  while (fgets(buffer, sizeof(buffer), credentials)) {
    envoyer(buffer);
  }
  while (fgets(buffer, sizeof(buffer), file)) {
    envoyer_recevoir(buffer, ans);
  }
  cryptseq_de(ans, ans2);
  printf("%s\n", ans2);
  char wanted[MAXREP] = "";

  extract_password(ans2, wanted);
  printf("%s\n", wanted);
  envoyer_recevoir(wanted, ans2);
  cryptseq_de(ans2, ans);
  printf("%s\n", ans);
  char password[] = "There will be no Nineteen Eighty-Four";
  cryptseq_en(password, ans);
  envoyer_recevoir(ans, ans2);

  char *message = strstr(ans2, ">>> Access granted <<<");

  if (message != NULL) {
    message += strlen(">>> Access granted <<<");
    message += strspn(message, "\r\n");

    cryptseq_de(message, ans);
    printf("%s\n", ans);
  } else {
    printf("%s\n", ans2);
  }

  printf("Fin d'envoi des messages.\n");
  printf("Pour envoyer d'autres lignes, ajouter des appels à la fonction "
         "`envoyer`\n");

  deconnexion();
  printf("Fin de la connection au serveur\n");
  return 0;
}
