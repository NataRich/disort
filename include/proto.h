#ifndef _PROTO_H_
#define _PROTO_H_

// Large File Multipart Protocol (LFM)

// 1) Host1 sends meta data of files and then waits for Host2 to reply.
//    The meta data should include at least the size of the entire file.
//    The reply should contain at least the received size.
// 2) Once Host1 receives the correct reply from Host2, Host1 can send
//    binary data over until all data have been sent.
// 3) Having sent all the data, Host1 waits for Host2 to reply.
// 4) Having received all the data, Host2 sends Host1 a reply.
//    The reply should contain at least the accumulated size of data.
// 5) Host1 should check if this size is complete. If not, start over.

#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#include "utils.h"

#define MAX_SOCKET_BUF 65536 // 64KB
#define MAX_PACKET_BUF 65500 // 655 100-byte records
#define PAD_SIZE 29          // 36 - 2 - 4 - 1

#define LFM_F_FRST 0x01  // First packet of a part (or an entire file)
#define LFM_F_PART 0x02  // Part of a larger file
#define LFM_F_LAST 0x04  // Last packet of a part (or an entire file)
#define LFM_F_RETRY 0x08 // Request to resend the last part
#define LFM_F_RECVD 0x10 // Received state

struct packet
{
    u_int16_t seq;
    u_int32_t size;
    u_int8_t flags;
    char buf[MAX_PACKET_BUF];
    char pad[PAD_SIZE];
};

/**
 * Confirms the meta data packet before transfering file data.
 *
 * @param sockfd The established socket file descriptor.
 * @param pkt The meta packet to be confirmed.
 * @param retyr The number of retries on error.
 * @return Positive on success and negative on error.
 */
int confirm(int sockfd, struct packet *pkt, short retry);

/**
 * Sets general custom timeout.
 *
 * @param tmv User defined timeout.
 * @return 0 on success and -1 on error.
 */
int settimeout(struct timeval *tmv);

/**
 * Serializes appropriate fields to network endianness.
 *
 * @param pkt The packet to be sent.
 * @return Same serialized data casted to (void *).
 */
void *serialize(struct packet *pkt);

/**
 * Deserializes appropriate fields to host endianness.
 *
 * @param pkt The received packet.
 * @return Same deserialized data casted to packet type.
 */
struct packet *deserialize(void *pkt);

#endif