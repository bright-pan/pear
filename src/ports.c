#include <string.h>
#include <sys/types.h>
// #include <net/if.h>

#ifdef CONFIG_USE_ALIOS
#include <string.h>
#include <lwip/opt.h>
#include <lwip/sockets.h>
#include <lwip/sys.h>
#include <lwip/api.h>

#include <lwip/inet.h>
#include <lwip/netdb.h>
#else
#include <arpa/inet.h>
#include <ifaddrs.h>
#include <sys/ioctl.h>
#include <netdb.h>
#include <errno.h>
#endif

#include "ports.h"
#include "utils.h"
#include "log.h"

static char g_current_ip[INET_ADDRSTRLEN] = {0};

void ports_set_current_ip(const char *ip) {

  snprintf(g_current_ip, INET_ADDRSTRLEN, "%s", ip);
}

int ports_get_current_ip(UdpSocket *udp_socket, Address *addr) {
  int ret;

#ifdef CONFIG_USE_ALIOS

  if (udp_socket->fd < 0) {

    LOGE("get_host_address before socket init");
    return -1;
  }

  struct sockaddr_in sin;

  socklen_t len = sizeof(sin);

  if (udp_socket->fd < 0) {
    LOGE("Failed to create socket");
    return -1;
  }

  if (getsockname(udp_socket->fd, (struct sockaddr *)&sin, &len) < 0) {
    LOGE("Failed to get local address");
    return -1;
  }

  memcpy(addr->ipv4, &sin.sin_addr.s_addr, 4);

  addr->port = ntohs(sin.sin_port);

  addr->family = AF_INET;

  LOGD("local port: %d", addr->port);
  LOGD("local address: %d.%d.%d.%d", addr->ipv4[0], addr->ipv4[1], addr->ipv4[2], addr->ipv4[3]);

  ret = 1;
#else
  struct ifaddrs *addrs,*tmp;

  struct ifreq ifr;

  if (udp_socket->fd < 0) {

    LOGE("get_host_address before socket init");
    return -1;
  }

  getifaddrs(&addrs);

  tmp = addrs;

  while (tmp) {

    if (tmp->ifa_addr && tmp->ifa_addr->sa_family == AF_PACKET) {

      strncpy(ifr.ifr_name, tmp->ifa_name, IFNAMSIZ);

      if (ioctl(udp_socket->fd, SIOCGIFADDR, &ifr) == 0) {

        LOGD("interface: %s, address: %s", ifr.ifr_name, inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr));

        addr[ret].family = AF_INET;
        memcpy(addr[ret].ipv4, &((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr.s_addr, 4);
        ret++;
      }


    }

    tmp = tmp->ifa_next;
  }

  freeifaddrs(addrs);
#endif
  return ret;
}


int ports_resolve_mdns_host(const char *host, Address *addr) {

  int ret = -1;

  struct addrinfo hints, *res, *p;
  int status;
  char ipstr[INET_ADDRSTRLEN];
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;

  if ((status = getaddrinfo(host, NULL, &hints, &res)) != 0) {
    //LOGE("getaddrinfo error: %s\n", gai_strerror(status));
    LOGE("getaddrinfo error: host:%s, %d\n", host, status);
    return ret;
  }

 for (p = res; p != NULL; p = p->ai_next) {

    if (p->ai_family == AF_INET) {
      struct sockaddr_in *ipv4 = (struct sockaddr_in *)p->ai_addr;
      ret = 0;
      memcpy(addr->ipv4, &ipv4->sin_addr.s_addr, 4);
    }
  }

  freeaddrinfo(res); 

  return ret;
}


