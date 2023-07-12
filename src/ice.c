#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <inttypes.h>

#include "ports.h"
#include "udp.h"
#include "utils.h"
#include "ice.h"
#include "log.h"

static void ice_candidate_calculate_priority(IceCandidate *candidate) {

  // priority = (2^24)*(type preference) + (2^8)*(local preference) + (256 - component ID)

  switch (candidate->type) {

    case ICE_CANDIDATE_TYPE_HOST:
      candidate->priority = 2130706432 + 16777215 + (256 - candidate->component);
      break;

    case ICE_CANDIDATE_TYPE_SRFLX:
      candidate->priority = 1694498815 + 16777215 + (256 - candidate->component);
      break;

    default:
      break;
  }

}

void ice_candidate_create(IceCandidate *candidate, IceCandidateType type, Address *addr) {

  memcpy(&candidate->addr, addr, sizeof(Address));

  candidate->type = type;

  candidate->foundation = 1;
  // 1: RTP, 2: RTCP
  candidate->component = 1;

  ice_candidate_calculate_priority(candidate);

  snprintf(candidate->transport, sizeof(candidate->transport), "%s", "UDP");
}

void ice_candidate_to_description(IceCandidate *candidate, char *description, int length) {

  char typ_raddr[64];

  memset(typ_raddr, 0, sizeof(typ_raddr));

  switch (candidate->type) {

    case ICE_CANDIDATE_TYPE_HOST:
      snprintf(typ_raddr, sizeof(typ_raddr), "host");
      break;

    case ICE_CANDIDATE_TYPE_SRFLX:
      snprintf(typ_raddr, sizeof(typ_raddr), "srflx raddr %d.%d.%d.%d rport %d",
       candidate->raddr.ipv4[0],
       candidate->raddr.ipv4[1],
       candidate->raddr.ipv4[2],
       candidate->raddr.ipv4[3],
       candidate->raddr.port);
      break;

    default:
      break;
  }

  snprintf(description, length, "a=candidate:%d %d %s %" PRIu32 " %d.%d.%d.%d %d typ %s\n",
   candidate->foundation,
   candidate->component,
   candidate->transport,
   candidate->priority,
   candidate->addr.ipv4[0],
   candidate->addr.ipv4[1],
   candidate->addr.ipv4[2],
   candidate->addr.ipv4[3],
   candidate->addr.port,
   typ_raddr);
}

void ice_candidate_print(IceCandidate *candidate) {
  LOGW("candidate addr->%d,%d,%d,%d:%d", candidate->addr.ipv4[0], candidate->addr.ipv4[1], 
      candidate->addr.ipv4[2], candidate->addr.ipv4[3], candidate->addr.port);
  LOGW("candidate raddr->%d,%d,%d,%d:%d", candidate->raddr.ipv4[0], candidate->raddr.ipv4[1], 
      candidate->raddr.ipv4[2], candidate->raddr.ipv4[3], candidate->raddr.port);
}

void ice_candidate_pair_print(IceCandidatePair *candidate_pair) {
  LOGW("candidate_pair->priority=%d, state=%d, nominated_time=%d", candidate_pair->priority, candidate_pair->state, candidate_pair->nominated_time);
  LOGW("---------------local----------------");
  ice_candidate_print(candidate_pair->local);
  LOGW("---------------remote---------------");
  ice_candidate_print(candidate_pair->remote);
}

int ice_candidate_from_description(IceCandidate *candidate, char *description) {

  char type[16];

  if (description[0] == 'a' && description[1] == '=' ) {
    description += 2;
  }

  if (strstr(description, "local") != 0) {

    // test for mDNS
    char mdns[64];
    sscanf(description, "candidate:%d %d %s %" SCNu32 " %s %hd typ %s",
     &candidate->foundation,
     &candidate->component,
     candidate->transport,
     &candidate->priority,
     mdns,
     &candidate->addr.port, type);

    ports_resolve_mdns_host(mdns, &candidate->addr);
    LOGD("mDNS host: %s, ip: %d.%d.%d.%d", mdns, candidate->addr.ipv4[0], candidate->addr.ipv4[1], candidate->addr.ipv4[2], candidate->addr.ipv4[3]);

  } else if (strstr(description, "UDP") == 0 && strstr(description, "udp") == 0) {
    // Only accept UDP candidates
    return -1;
  } else if (sscanf(description, "candidate:%d %d %s %" SCNu32 " %hhu.%hhu.%hhu.%hhu %hd typ %s",
   &candidate->foundation,
   &candidate->component,
   candidate->transport,
   &candidate->priority,
   &candidate->addr.ipv4[0],
   &candidate->addr.ipv4[1],
   &candidate->addr.ipv4[2],
   &candidate->addr.ipv4[3],
   &candidate->addr.port,
   type) != 10) {

    LOGE("Failed to parse candidate description: %s", description);
    return -1;
  }

  if (strcmp(type, "host") == 0) {

    candidate->type = ICE_CANDIDATE_TYPE_HOST;

  } else if (strcmp(type, "srflx") == 0) {

    candidate->type = ICE_CANDIDATE_TYPE_SRFLX;

  } else {

    LOGE("Unknown candidate type: %s", type);
    return -1;
  }

  return 0;
}

