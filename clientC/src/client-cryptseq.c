#include "client-crypteMove.h"
#include "client.h"
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
typedef struct {
  char key;
  char value;
} entry;

i8 contains(char *array, char elt) {
  usize len = strlen(array);

  for (usize i = 0; i < len; i++) {
    if (array[i] == elt) {
      return (i8)i;
    }
  }

  return -1;
}

void rotate_f(char *array, usize index) {
  usize len = strlen(array);
  char elt = array[index];

  for (usize i = index; i + 1 < len; i++) {
    array[i] = array[i + 1];
  }

  array[len - 1] = elt;
}

static void cryptassoc(char *text, char *result, bool decode) {
  entry sequence[256];
  usize count = 0;
  usize i;

  for (i = 0; text[i] != '\0'; i++) {
    usize pos = 0;
    while (pos < count &&
           (decode ? sequence[pos].value : sequence[pos].key) != text[i]) {
      pos++;
    }

    if (pos == count) {
      sequence[count++] = (entry){text[i], text[i]};
    } else {
      if (decode) {
        pos = (pos + 1) % count;
      }
      usize previous = (pos + count - 1) % count;
      char association = sequence[pos].value;
      sequence[pos].value = sequence[previous].value;
      sequence[previous].value = association;
    }

    result[i] = decode ? sequence[pos].key : sequence[pos].value;
    entry current = sequence[pos];
    for (usize j = pos; j + 1 < count; j++) {
      sequence[j] = sequence[j + 1];
    }
    sequence[count - 1] = current;
  }
  result[i] = '\0';
}

void cryptassoc_de(char *text, char *deciphered) {
  cryptassoc(text, deciphered, true);
}

void cryptassoc_en(char *text, char *ciphered) {
  cryptassoc(text, ciphered, false);
}

void cryptseq_de(char *text, char *new_text) {
  char encounter[125] = "";
  usize enc_len = 0;
  usize len = strlen(text);

  for (usize i = 0; i < len; i++) {
    i8 contained = contains(encounter, text[i]);

    if (contained == -1) {
      encounter[enc_len] = text[i];
      enc_len++;
      encounter[enc_len] = '\0';

      new_text[i] = text[i];

    } else {
      usize pos = (usize)contained;
      usize next_pos;

      if (pos == enc_len - 1) {
        next_pos = 0;
      } else {
        next_pos = pos + 1;
      }

      char original = encounter[next_pos];

      new_text[i] = original;

      rotate_f(encounter, next_pos);
    }
  }

  new_text[len] = '\0';
}

void cryptseq_en(char *text, char *new_text) {
  char encounter[125] = ""; // to avoid int overflow over i8
  usize enc_len = 0;
  usize len = strlen(text);
  for (usize i = 0; i < len; i++) {
    i8 contained = contains(encounter, text[i]);

    if (contained == -1) {
      encounter[enc_len] = text[i];
      enc_len++;
      encounter[enc_len] = '\0';

      new_text[i] = text[i];
    } else {
      usize pos = (usize)contained;
      if (pos == 0) {

        new_text[i] = encounter[enc_len - 1];

      } else {
        new_text[i] = encounter[pos - 1];

        // Here we need to move the char at the end of the sequence ? sounds
        // like LRU
      }

      rotate_f(encounter, pos);
    }
  }
  new_text[len] = '\0';
}

#ifndef CLIENT_CRYPTSEQ_LIBRARY
int main(void) {

  // Affiche les échanges avec le serveur (false pour désactiver)
  show_messages(true);

  // Connexion au serveur AppoLab
  connexion("im2ag-appolab.u-ga.fr");

  // Need to put those two following files at the / of the project (APP1/./)
  // and to execute from there as well ...
  FILE *credentials = fopen("./.env", "r");
  FILE *file = fopen("./commands_cryptseq.txt", "r");

  if (file == NULL || credentials == NULL) {
    printf("Couldn't open the file");
    return 1;
  }

  char buffer[MAXREP];
  char answer[MAXREP];

  while (fgets(buffer, sizeof(buffer), credentials)) {
    envoyer(buffer);
  }
  while (fgets(buffer, sizeof(buffer), file)) {
    envoyer_recevoir(buffer, answer);
  }
  char ans[MAXREP];
  char ans2[MAXREP];
  decrypt(answer, ans);
  printf("%s\n", ans);
  cryptseq_en(ans, ans2);
  printf("%s\n", ans2);

  envoyer_recevoir(ans2, buffer);
  char encd[] =
      "Alice,\n"
      "Sl tuiaraSvcsuusAruvaArrmcvevgmAcr'gmS,qS'tsqem'uiqtaulati,on rS "
      "staAS,s\n"
      "mvivqhScvdmmmmnoypi'ldmdldoguycumtoy.gCpaodleinod.nCaaas'ro Sojl,iousC "
      "eubajlsSSinguovuicavnglbonbbSqlstfCshscssndn.Jt.gJucpJc'sspfqpeunroq "
      "umqsmmbqfllsnvofrorrimeilr o-nc.citAumtmiqv-rms-afturnt'at\n"
      "ub f "
      "eegiiasod-itmjntuu-lnvmvv?c.e.ddnmbAuibtad.evddtjEulsv-vgSmfubvtbeIru?-"
      "isl-lveJhettttaltpeIeeql Csluht-rthbeteiedlCld,Ieeegeai,I-crIc\n"
      "Iethpcpp\n"
      "'oosIdm\n"
      "'eplqIRd'j'u'Afpbrnhppi'cRx'c,orxslNmxb,wNwmn "
      "ue.AaiNSRNi.attmAlnlAnnnnvetJc,xSEquiabo\n"
      "-edbpjbmn,yCwe,et,Egg,E,eggrd,,eiltggwaddajv\n"
      "p\n"
      "oe:twNvenh:rttiuEsmgvllhrpgdoi:wbBtq''";
  cryptseq_de(encd, ans2);
  printf("%s\n", ans2);
  printf("Fin d'envoi des messages.\n");
  printf("Pour envoyer d'autres lignes, ajouter des appels à la fonction "
         "`envoyer`\n");
  deconnexion();
  printf("Fin de la connection au serveur\n");
  return 0;
}
#endif
