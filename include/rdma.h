#ifndef _RDMA_H_
#define _RDMA_H_
#include <cstdint>
#include <memory>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <cstdint>
#include <inttypes.h>
#include <endian.h>
#include <byteswap.h>
#include <getopt.h>

#include <sys/time.h>
#include <arpa/inet.h>
#include <infiniband/verbs.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include "../bin/rapidjson/document.h"
#include "../bin/rapidjson/writer.h"
#include "../bin/rapidjson/stringbuffer.h"
#define MAX_POLL_CQ_TIMEOUT 2000
#define WAIT_RESPONSE_POLL_CQ_TIMEOUT int(1e10)
#if __BYTE_ORDER == __LITTLE_ENDIAN
static inline uint64_t htonll(uint64_t x) { return bswap_64(x); }
static inline uint64_t ntohll(uint64_t x) { return bswap_64(x); }
#elif __BYTE_ORDER == __BIG_ENDIAN
static inline uint64_t htonll(uint64_t x) { return x; }
static inline uint64_t ntohll(uint64_t x) { return x; }
#else
#error __BYTE_ORDER is neither __LITTLE_ENDIAN nor __BIG_ENDIAN
#endif
/*----------unchanged line ------------*/
#define MSG "SEND operation 36"
#define RDMAMSGR "RDMA read operation 36 "
#define RDMAMSGW "RDMA write operation 36"
// #define BUFF_SIZE (strlen(MSG) + 1)
#define BUFF_SIZE 1024 * 1024 * 1024
namespace RDMAEngine
{
    /* structure of test parameters */
    struct config_t
    {
        const char *dev_name; /* IB device name */
        char *server_name;    /* server host name */
        u_int32_t tcp_port;   /* server TCP port */
        int ib_port;          /* local IB port to work with */
        int gid_idx;          /* gid index to use */
    } __attribute__((packed));
    /* structure to exchange data which is needed to connect the QPs */
    struct cm_con_data_t
    {
        uint64_t addr;   /* Buffer address */
        uint32_t rkey;   /* Remote key */
        uint32_t qp_num; /* QP number */
        uint16_t lid;    /* LID of the IB port */
        uint8_t gid[16]; /* gid */
    } __attribute__((packed));
    /* structure of system resources */
    struct resources
    {
        struct ibv_device_attr device_attr; /* Device attributes */
        struct ibv_port_attr port_attr;     /* IB port attributes */
        struct cm_con_data_t remote_props;  /* values to connect to remote side */
        struct ibv_context *ib_ctx;         /* device handle */
        struct ibv_pd *pd;                  /* PD handle */
        struct ibv_cq *cq;                  /* CQ handle */
        struct ibv_qp *qp;                  /* QP handle */
        struct ibv_mr *mr;                  /* MR handle for buf */
        char *buf;                          /* memory buffer pointer, used for RDMA and send ops */
        int sock;                           /* TCP socket file descriptor */
    } __attribute__((packed));

    static char config_file_name[100] = "./configuration.conf";
    class RDMA_Manager
    {
    public:
        config_t config;
        resources *res;
        RDMA_Manager(bool config_flag);
        void print_config()
        {
            fprintf(stdout, " ------------------------------------------------\n");
            fprintf(stdout, " Device name : \"%s\"\n", config.dev_name);
            fprintf(stdout, " IB port : %d\n", this->config.ib_port);
            fprintf(stdout, " TCP port : %d\n", config.tcp_port);
            if (config.gid_idx >= 0)
                fprintf(stdout, " GID index : %d\n", config.gid_idx);
            fprintf(stdout, " ------------------------------------------------\n\n");
        }
        void usage(const char *argv0)
        {
            fprintf(stdout, "Usage:\n");
            fprintf(stdout, " %s start a server and wait for connection\n", argv0);
            fprintf(stdout, " %s <host> connect to server at <host>\n", argv0);
            fprintf(stdout, "\n");
            fprintf(stdout, "Options:\n");
            fprintf(stdout, " -p, --port <port> listen on/connect to port <port> (default 18515)\n");
            fprintf(stdout, " -d, --ib-dev <dev> use IB device <dev> (default first device found)\n");
            fprintf(stdout, " -i, --ib-port <port> use port <port> of IB device (default 1)\n");
            fprintf(stdout, " -g, --gid_idx <git index> gid index to be used in GRH (default not used)\n");
        }
        int init();
        int resources_create();
        int connect_qp();
        int connect_qp_2();
        int connect_qp_reset();
        int modify_qp_to_init(struct ibv_qp *qp);
        int modify_qp_to_rtr(struct ibv_qp *qp, uint32_t remote_qpn, uint16_t dlid, uint8_t *dgid);
        int modify_qp_to_rts(struct ibv_qp *qp);

        int resources_destroy();
        int sock_sync_data(int sock, int xfer_size, char *local_data, char *remote_data);
        int poll_completion(long unsigned int max_poll_cq_timeout);
        int post_send(int opcode);
        int post_receive();

        int RDMA_Send(long unsigned int max_poll_cq_timeout);
        int RDMA_Receive(long unsigned int max_poll_cq_timeout);
        int RDMA_Read();
        // int RDMA_Read(char* char_set);
        int RDMA_Write();
        int socket_connect();
        int device_connect();
        int sock_connect();
        int local_memory_register();
        int allocate_pd();
        int create_cq();
        int create_qp();

    private:
        
    };
} // namespace RDMAEngine

#endif