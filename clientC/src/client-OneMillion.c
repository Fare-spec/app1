#include "client-crypteMove.h"
#include "client.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define BLOCK 100u
#define LENGTH 1000000u
#define CAPACITY (1u << 20)
#define MASK (CAPACITY - 1u)

void onemillion_network_stats(void);

/* Precompute the known base before start. Tags 256..355 refer to decoded key
 * positions. This allow us to get really low times < microsecond,
 * althought Rust alternative is sligthly better due to the fact that I usually
 * code in Rust and understand the language much better, as well as I
 * reimplemented the whole client so that the rust version is standalone and
 * spawns a thread for the network and then join during the submission */
static void prepare_prefix(const char *base, unsigned short *ring,
                           unsigned short plan[BLOCK]) {
  /* We already know the base before "start", so we can do almost all the work
   * here. Values 256..355 are placeholders for the 100 characters of the key
   * that we only receive after "start".
   *
   * At the end, plan[] tells us where each of the first 100 characters of the
   * final result comes from, so once we get the key there is very little left
   * to calculate. */
  size_t head = 0, length = BLOCK;
  for (size_t i = 0; i < BLOCK; ++i)
    ring[i] = 256 + i;
  for (size_t i = LENGTH - BLOCK; i > 0; --i) {
    unsigned char c = (unsigned char)base[(i - 1) % BLOCK];
    size_t shifts = c % 8u;
    if (shifts < length) {
      for (size_t j = 0; j < shifts; ++j) {
        unsigned short last = ring[(head + length - 1) & MASK];
        head = (head - 1) & MASK;
        ring[head] = last;
      }
    } /* Rotating by the full current length is an identity. */
    head = (head - 1) & MASK;
    ring[head] = c;
    ++length;
  }
  for (size_t i = 0; i < BLOCK; ++i)
    plan[i] = ring[(head + i) & MASK];
}

static void apply_prefix(const unsigned short plan[BLOCK], const char *key,
                         char result[BLOCK + 1]) {
  /* Finish the work that couldn't be done before "start".
   * First build the ring for the 100-character key, then use plan[] to
   * replace the placeholders and get the final result. */
  char ring[128];
  size_t head = 0, length = 0;
  for (size_t i = BLOCK; i > 0; --i) {
    unsigned char c = (unsigned char)key[i - 1];
    size_t shifts = c % 8u;
    if (shifts < length) {
      for (size_t j = 0; j < shifts; ++j) {
        char last = ring[(head + length - 1) & 127u];
        head = (head - 1) & 127u;
        ring[head] = last;
      }
    } /* Rotating by the full current length is an identity. */
    head = (head - 1) & 127u;
    ring[head] = (char)c;
    ++length;
  }
  for (size_t i = 0; i < BLOCK; ++i)
    result[i] =
        plan[i] < 256 ? (char)plan[i] : ring[(head + plan[i] - 256) & 127u];
  result[BLOCK] = '\0';
}

int main(int argc, char **argv) {
  /* Allow multiple attempts since network latency can change quite a bit
   * between two runs. */
  long attempts = 1;
  if (argc == 2) {
    char *end;
    attempts = strtol(argv[1], &end, 10);
    if (!*argv[1] || *end || attempts < 1 || attempts > 20)
      attempts = 0;
  }
  if (argc > 2 || !attempts) {
    fprintf(stderr, "Usage: %s [attempts: 1..20]\n", argv[0]);
    return 1;
  }
  FILE *credentials = fopen(".env", "r");
  unsigned short *ring = malloc(CAPACITY * sizeof(*ring));
  unsigned short plan[BLOCK];
  if (!credentials || !ring) {
    perror("OneMillion setup");
    if (credentials)
      fclose(credentials);
    free(ring);
    return 1;
  }
  show_messages(false);
  char line[256], answer[MAXREP + 1], base[BLOCK + 1], result[BLOCK + 1];
  int status = 1;
  bool connected = false;
  for (long attempt = 1; attempt <= attempts; ++attempt) {
    if (!connexion("im2ag-appolab.u-ga.fr"))
      goto done; // I know we're not really supposed to use goto in this
                 // course, but it makes the error cleanup much simpler here.
    connected = true;
    rewind(credentials);
    while (fgets(line, sizeof(line), credentials)) {
      if (envoyer_recevoir(line, answer) < 0)
        goto done;
    }
    if (envoyer_recevoir("load OneMillion", answer) < 0)
      goto done;
    if (envoyer_recevoir("aide", answer) < 0)
      goto done;
    answer[strcspn(answer, "\r\n")] = '\0';
    if (strlen(answer) != BLOCK) {
      fprintf(stderr, "Unexpected base string: %s\n", answer);
      goto done;
    }
    memcpy(base, answer, sizeof(base));
    /* Everything before "start" isn't timed by AppoLab, so do as much work
     * as possible here while the base is already known. */
    prepare_prefix(base, ring, plan);
    struct timespec start, finish;
    clock_gettime(CLOCK_MONOTONIC, &start);
    if (envoyer_recevoir("start", answer) < 0)
      goto done;
    answer[strcspn(answer, "\r\n")] = '\0';
    if (strlen(answer) != BLOCK) {
      fprintf(stderr, "Unexpected key: %s\n", answer);
      goto done;
    }
    apply_prefix(plan, answer, result);
    if (envoyer_recevoir(result, answer) < 0)
      goto done;
    clock_gettime(CLOCK_MONOTONIC, &finish);
    double milliseconds = (finish.tv_sec - start.tv_sec) * 1000.0 +
                          (finish.tv_nsec - start.tv_nsec) / 1000000.0;
    char decoded[MAXREP + 1];
    decrypt(answer, decoded);
    printf("Attempt %ld: start + submission exchanges %.3f ms\n%s\n", attempt,
           milliseconds, decoded);
    onemillion_network_stats();
    fflush(stdout);
    if (!strstr(decoded, "Je savais que je pouvais compter sur toi"))
      goto done;
    deconnexion();
    connected = false;
  }
  status = 0;
done:
  fclose(credentials);
  free(ring);
  if (connected)
    deconnexion();
  return status;
}
