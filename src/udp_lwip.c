

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <time.h>
#include <lwip/opt.h>
#include <lwip/sockets.h>
#include <lwip/sys.h>
#include <lwip/api.h>
#include <lwip/netdb.h>
#include <arpa/inet.h>
#include <mbedtls/net_sockets.h>

#include "ports.h"
#include "utils.h"
#include "udp.h"
#include "log.h"


void udp_socket_reset(UdpSocket *udp_socket) {
  mbedtls_net_init((mbedtls_net_context *)udp_socket);
  memset(&udp_socket->bind_addr, 0, sizeof(Address));
  memset(&udp_socket->sendto_addr, 0, sizeof(Address));
}

int udp_socket_open(UdpSocket *udp_socket) {

  udp_socket->fd = socket(AF_INET, SOCK_DGRAM, 0);

  int flags = fcntl(udp_socket->fd, F_GETFL, 0);

  fcntl(udp_socket->fd, F_SETFL, flags | O_NONBLOCK);
  return 0;
}

int udp_socket_connect_ex(UdpSocket *udp_socket, const char *host, const char *port) {
  int ret = MBEDTLS_ERR_NET_INVALID_CONTEXT;
  if (udp_socket != NULL && udp_socket->fd >= 0) {
    if (host && port) {
      ret = mbedtls_net_connect((mbedtls_net_context *)udp_socket, host,
                                port, MBEDTLS_NET_PROTO_UDP);
      if (ret < 0) {
        LOGE("connect->%s:%s failed", host, port);
      } else {
        ports_resolve_mdns_host(host, &udp_socket->sendto_addr);
        udp_socket->sendto_addr.port = atoi(port);
        LOGI("connect->%s:%s ok", host, port);
      }
    } else {
      LOGE("host == NULL || port == NULL");
    }
  } else {
    LOGE("socket == NULL || fd < 0");
  }
  return ret;
}

int udp_socket_bind_ex(UdpSocket *udp_socket, const char *host, const char *port) {
  int ret = MBEDTLS_ERR_NET_INVALID_CONTEXT;
  if (udp_socket != NULL) {
    if (host && port) {
      ret = mbedtls_net_bind((mbedtls_net_context *)udp_socket, host,
                          port, MBEDTLS_NET_PROTO_UDP);
      if (ret < 0) {
        LOGE("bind->%s:%s failed:0x%x", host, port, ret);
      } else {
        ports_resolve_mdns_host(host, &udp_socket->bind_addr);
        udp_socket->bind_addr.port = atoi(port);
        LOGI("bind->%s:%s ok", host, port);
      }
    } else {
      LOGE("host == NULL || port == NULL");
    }
  } else {
    LOGE("socket is NULL");
  }
  return ret;
}

int udp_socket_accept(UdpSocket *udp_socket, UdpSocket *client_socket) {
  int ret = MBEDTLS_ERR_NET_INVALID_CONTEXT;
  struct sockaddr_in sin;

  if (udp_socket != NULL && udp_socket->fd >= 0) {
    if (client_socket) {
      ret = mbedtls_net_accept_ex((mbedtls_net_context *)udp_socket, 
                                  (mbedtls_net_context *)client_socket, 
                                  &sin);
      if (ret < 0) {
        LOGE("udp_socket_accept->failed");
      } else {
        memcpy(client_socket->sendto_addr.ipv4, &sin.sin_addr.s_addr, 4);
        client_socket->sendto_addr.port = ntohs(sin.sin_port);
        client_socket->sendto_addr.family = AF_INET;
        LOGI("udp_socket_accept->ok");
      }
    } else {
      LOGE("client_socket == NULL");
    }
  } else {
    LOGE("socket == NULL || fd < 0");
  }
  return ret;
}

int udp_socket_bind(UdpSocket *udp_socket, Address *addr) {
  int ret = MBEDTLS_ERR_NET_INVALID_CONTEXT;
  char *host = inet_ntoa(addr->ipv4);
  char port[16];

  sprintf(port, "%d", addr->port);
  ret = udp_socket_bind_ex(udp_socket, host, port);
  return ret;
}

int udp_socket_connect(UdpSocket *udp_socket, Address *addr) {
  int ret = MBEDTLS_ERR_NET_INVALID_CONTEXT;
  char *host = inet_ntoa(addr->ipv4);
  char port[16];

  sprintf(port, "%d", addr->port);
  ret = udp_socket_connect_ex(udp_socket, host, port);
  return ret;
}

void udp_socket_close(UdpSocket *udp_socket) {
  if (udp_socket != NULL && udp_socket->fd >= 0) {
    LOGI("close->%d", udp_socket->fd);
    mbedtls_net_close((mbedtls_net_context *)udp_socket);
  } else {
    LOGE("socket is NULL || fd < 0");
  }
}

int udp_get_local_address(UdpSocket *udp_socket, Address *addr) {

  if (udp_socket != NULL && udp_socket->fd >= 0) {
    struct sockaddr_in sin;

    socklen_t len = sizeof(sin);

    if (getsockname(udp_socket->fd, (struct sockaddr *)&sin, &len) < 0) {
      LOGE("Failed to get local address");
      return -1;
    }

    memcpy(addr->ipv4, &sin.sin_addr.s_addr, 4);

    addr->port = ntohs(sin.sin_port);

    addr->family = AF_INET;

    LOGD("local port: %d", addr->port);
    LOGD("local address: %d.%d.%d.%d", addr->ipv4[0], addr->ipv4[1], addr->ipv4[2], addr->ipv4[3]);

  } else {
    LOGE("socket is NULL || fd < 0");
  }

  return 0;
}

int udp_socket_send(UdpSocket *udp_socket, const uint8_t *buf, int len) {
  int ret = MBEDTLS_ERR_NET_INVALID_CONTEXT;

  if (udp_socket != NULL && udp_socket->fd >= 0) {
      ret = mbedtls_net_send((mbedtls_net_context *)udp_socket, buf, len);
  } else {
    LOGE("socket is NULL || fd < 0");
  }
  return ret;
}

int udp_socket_recv(UdpSocket *udp_socket, uint8_t *buf, int len) {
  int ret = MBEDTLS_ERR_NET_INVALID_CONTEXT;

  if (udp_socket != NULL && udp_socket->fd >= 0) {
      ret = mbedtls_net_recv((mbedtls_net_context *)udp_socket, buf, len);
  } else {
    LOGE("socket is NULL || fd < 0");
  }
  return ret;
}

int udp_socket_recv_timeout(UdpSocket *udp_socket, uint8_t *buf, int len, uint32_t timeout) {
  int ret = MBEDTLS_ERR_NET_INVALID_CONTEXT;

  if (udp_socket != NULL && udp_socket->fd >= 0) {
      ret = mbedtls_net_recv_timeout((mbedtls_net_context *)udp_socket, buf, len, timeout);
  } else {
    LOGE("socket is NULL || fd < 0");
  }
  return ret;
}

int udp_socket_sendto(UdpSocket *udp_socket, Address *addr, const uint8_t *buf, int len) {
  int ret = MBEDTLS_ERR_NET_INVALID_CONTEXT;
  struct sockaddr_in sin;

  if (udp_socket != NULL && udp_socket->fd >= 0) {

      sin.sin_family = AF_INET;
      memcpy(&sin.sin_addr.s_addr, addr->ipv4, 4);
      //LOGD("s_addr: %d", sin.sin_addr.s_addr);
      sin.sin_port = htons(addr->port);

      //LOGD("sendto addr %d.%d.%d.%d (%d)", addr->ipv4[0], addr->ipv4[1], addr->ipv4[2], addr->ipv4[3], addr->port);
      ret = mbedtls_net_sendto((mbedtls_net_context *)udp_socket, &sin, buf, len);
  } else {
    LOGE("socket is NULL || fd < 0");
  }
  return ret;
}

int udp_socket_recvfrom(UdpSocket *udp_socket, Address *addr, uint8_t *buf, int len) {
  int ret = MBEDTLS_ERR_NET_INVALID_CONTEXT;
  struct sockaddr_in sin;

  if (udp_socket != NULL && udp_socket->fd >= 0) {
    memset(&sin, 0, sizeof(sin));
    ret = mbedtls_net_recvfrom((mbedtls_net_context *)udp_socket, &sin, buf, len);
    if (ret > 0) {
      memcpy(addr->ipv4, &sin.sin_addr.s_addr, 4);
      addr->port = ntohs(sin.sin_port);
      addr->family = AF_INET;
    }
  } else {
    LOGE("socket is NULL || fd < 0");
  }
  return ret;
}

int udp_socket_recvfrom_timeout(UdpSocket *udp_socket, Address *addr, uint8_t *buf, int len, uint32_t timeout) {
  int ret = MBEDTLS_ERR_NET_INVALID_CONTEXT;
  struct sockaddr_in sin;

  if (udp_socket != NULL && udp_socket->fd >= 0) {
    memset(&sin, 0, sizeof(sin));
    ret = mbedtls_net_recvfrom_timeout((mbedtls_net_context *)udp_socket, &sin, buf, len, timeout);
    if (ret > 0) {
      memcpy(addr->ipv4, &sin.sin_addr.s_addr, 4);
      addr->port = ntohs(sin.sin_port);
      addr->family = AF_INET;
    }
  } else {
    LOGE("socket is NULL || fd < 0");
  }
  return ret;
}

int udp_socket_get_host_address(UdpSocket *udp_socket, Address *addr) {

  int ret = 0;
  return ret;
}


int udp_resolve_mdns_host(const char *host, Address *addr) {

  int ret = -1;
  struct addrinfo hints, *res, *p;
  int status;
  char ipstr[INET_ADDRSTRLEN];
/*
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
*/
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_protocol = IPPROTO_UDP;
	hints.ai_flags = AI_PASSIVE | AI_NUMERICSERV;
  
  if ((status = getaddrinfo(host, NULL, &hints, &res)) != 0) {
    LOGE("getaddrinfo error: %s\n", strerror(status));
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



static void test_udp_resolve_mdns_host(int argc, char **argv) {
    Address addr;
    udp_resolve_mdns_host(argv[1], &addr);
    LOGI("addr %d.%d.%d.%d (%d)", addr.ipv4[0], addr.ipv4[1], addr.ipv4[2], addr.ipv4[3]);
}


ALIOS_CLI_CMD_REGISTER(test_udp_resolve_mdns_host, test_udp_resolve_mdns_host, test_udp_resolve_mdns_host);

