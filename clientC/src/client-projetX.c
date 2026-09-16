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
  char letter;
  f32 value;
} Entry;

// I find the idea interesting of finding the offset heuristicly even though we
// could have used the fact that the first char is a c and find the whole offset
// using it.
static const Entry table[26] = {
    {'a', 7.11f}, {'b', 1.14f}, {'c', 3.18f}, {'d', 3.67f}, {'e', 12.10f},
    {'f', 1.11f}, {'g', 1.23f}, {'h', 1.11f}, {'i', 6.59f}, {'j', 0.34f},
    {'k', 0.29f}, {'l', 4.96f}, {'m', 2.62f}, {'n', 6.39f}, {'o', 5.02f},
    {'p', 2.49f}, {'q', 0.65f}, {'r', 6.07f}, {'s', 6.51f}, {'t', 5.92f},
    {'u', 4.49f}, {'v', 1.11f}, {'w', 0.17f}, {'x', 0.38f}, {'y', 0.46f},
    {'z', 0.15f},
};

f32 distance_with_offset(const Entry arr1[26], const Entry arr2[26],
                         u8 offset) {
  f32 distance = 0.0;

  for (u8 i = 0; i < 26; i++) {
    u8 shifted = (i + offset) % 26;

    f32 diff = arr1[shifted].value - arr2[i].value;
    distance += diff * diff;
  }

  return distance;
}
void count_chars(const char *text, Entry letters[26]) {
  for (u8 i = 0; i < 26; i++) {
    letters[i].letter = 'a' + i;
    letters[i].value = 0.0f;
  }

  u32 total = 0;

  for (u32 i = 0; text[i] != '\0'; i++) {
    char c = text[i];

    if (c >= 'A' && c <= 'Z')
      c = c - 'A' + 'a';

    if (c >= 'a' && c <= 'z') {
      letters[c - 'a'].value++;
      total++;
    }
  }
  for (u8 i = 0; i < 26; i++) {
    letters[i].value = letters[i].value * 100.0f / (f32)total;
  }
}

void create_offset(const char *text, u8 offset, char *new_text) {
  u32 i;

  for (i = 0; text[i] != '\0'; i++) {
    if (text[i] >= 'a' && text[i] <= 'z') {
      new_text[i] = (text[i] - 'a' + offset) % 26 + 'a';

    } else if (text[i] >= 'A' && text[i] <= 'Z') {
      new_text[i] = (text[i] - 'A' + offset) % 26 + 'A';

    } else {
      new_text[i] = text[i];
    }
  }

  new_text[i] = '\0';
}

u8 find_offset(char *text) {

  u8 best;
  Entry letters_count[26];

  count_chars(text, letters_count);
  f32 best_dist = distance_with_offset(table, letters_count, 0);
  for (u8 i = 0; i < 26; i++) {
    f32 new_dist = distance_with_offset(table, letters_count, i);
    if (new_dist < best_dist) {
      best_dist = new_dist;
      best = i;
    }
  }

  return best;
}

int main() {

  // Affiche les échanges avec le serveur (false pour désactiver)
  show_messages(true);

  // Connexion au serveur AppoLab
  connexion("im2ag-appolab.u-ga.fr");

  // Need to put those two following files at the / of the project (APP1/./) and
  // to execute from there as well ...
  FILE *file = fopen("./commands_projetX.txt", "r");
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
  u8 offset = find_offset(answer);
  printf("\nOffset detected: %d\n", offset);
  char decoded_ans[MAXREP];
  create_offset(answer, offset, decoded_ans);
  printf("%s\n", decoded_ans);
  envoyer("start");
  envoyer("veni vidi vici"); // recovered from the script over here

  printf("Fin d'envoi des messages.\n");
  printf("Pour envoyer d'autres lignes, ajouter des appels à la fonction "
         "`envoyer`\n");
  deconnexion();
  printf("Fin de la connection au serveur\n");
  return 0;
}
