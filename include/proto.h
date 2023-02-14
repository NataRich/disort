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
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#include "net.h"
#include "utils.h"

#define PACKET_PAD 14        // number of padding shorts
#define MAX_PACKET_BUF 65500 // 655 100-byte records

#define LFM_F_FRST 0x01 // First packet of a part (or an entire file)
#define IS_FRST_SET(flags) (LFM_F_FRST & flags)
#define LFM_F_PART 0x02 // Part of a larger file
#define IS_PART_SET(flags) ((LFM_F_PART & flags) >> 1)
#define LFM_F_LAST 0x04 // Last packet of a part (or an entire file)
#define IS_LAST_SET(flags) ((LFM_F_LAST & flags) >> 2)
#define LFM_F_RETRY 0x08 // Request to resend the last part
#define IS_RETRY_SET(flags) ((LFM_F_RETRY & flags) >> 3)
#define LFM_F_REPLY 0x10 // Reply
#define IS_REPLY_SET(flags) ((LFM_F_REPLY & flags) >> 4)

struct packet
{
    u_int16_t seq;
    u_int16_t flags;
    u_int32_t size;
    u_int16_t opts[PACKET_PAD];
    unsigned char buf[MAX_PACKET_BUF];
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
 * Replies to a confirmation.
 *
 * @param sockfd The established socket file descriptor.
 * @param size The size of the entire file (in bytes).
 * @return Positive on success and negative on error.
 */
int reply(int sockfd, u_int32_t *size);

/**
 * Sends packet (with auto serialization & timeout).
 *
 * @param sockfd The socket file descriptor.
 * @param pkt The original data packet to be sent.
 * @param off The offset of data to send.
 * @param len The length of data to send.
 * @return -1 on error and positive long means bytes sent.
 */
ssize_t lfm_send(int sockfd, struct packet *pkt);

/**
 * Receives packet (with auto deserialization & timeout).
 *
 * @param sockfd The socket file descriptor.
 * @param pkt The chunkd of memory to hold received packet.
 * @return -1 on error and positive long means bytes received.
 */
ssize_t lfm_recv(int sockfd, struct packet **pkt);

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