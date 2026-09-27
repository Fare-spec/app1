// Goal was to make the submittion only when ping seemed to be low, it worked
// relatively well.

#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <sys/types.h>

static int low_latency;
static ssize_t receive_quickack(int sock, void *buffer, size_t size,
                                int flags) {
  ssize_t result = recv(sock, buffer, size, flags);
#ifdef TCP_QUICKACK
  if (low_latency && result > 0) {
    int enabled = 1;
    /* Re-arm after reads, including the header: the body may arrive separately.
     */
    (void)setsockopt(sock, IPPROTO_TCP, TCP_QUICKACK, &enabled,
                     sizeof(enabled));
  }
#endif
  return result;
}

#define recv receive_quickack
#define connexion original_connexion
#include "client.c"
#undef connexion
#undef recv

int connexion(char *hostname) {
  low_latency = getenv("ONEMILLION_TCP_BASELINE") == NULL;
  int connected = original_connexion(hostname);
  if (connected && low_latency) {
    int enabled = 1;
    if (setsockopt(a.sock, IPPROTO_TCP, TCP_NODELAY, &enabled, sizeof(enabled)))
      perror("TCP_NODELAY");
  }
  return connected;
}

void onemillion_network_stats(void) {
#ifdef __linux__
  struct tcp_info info;
  socklen_t size = sizeof(info);
  if (!getsockopt(a.sock, IPPROTO_TCP, TCP_INFO, &info, &size))
    printf("TCP RTT estimate: %.3f ms (%s)\n", info.tcpi_rtt / 1000.0,
           low_latency ? "low latency enabled" : "baseline");
#endif
}
