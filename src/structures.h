#ifndef DHCP_STRUCTURES
#define DHCP_STRUCTURES

#include<net/if.h>
#include<stdint.h>

typedef struct DHCP_MESSAGE
{
	unsigned char op, htype, hlen, hops;
	uint32_t xid;
	unsigned char secs[2], flags[2];
	unsigned char ciaddr[4], yiaddr[4], siaddr[4], giaddr[4];
	unsigned char chaddr[16];
	unsigned char sname[64];
	unsigned char file[128];
	unsigned char magic[4];
	unsigned char opt[312+28-4];
} DHCP_MESSAGE;
#endif

#ifndef ARGS_HANDLER_STRUCTURES
#define ARGS_HANDLER_STRUCTURES
typedef struct ARGS_OPTIONS
{
	uint8_t debug;
	uint8_t dry;
	uint8_t arp_check;
	uint8_t release;
	uint8_t no_dbg_files;
	char *interface_name;
} ARGS_OPTIONS;
#endif

#ifndef RAW_ARP_DEF
#define RAW_ARP_DEF
typedef struct RAW_ARP
{
	char dst_hwaddr[6], src_hwaddr[6];
	uint16_t ether_type;
	uint16_t htype, ptype;
	char hlen, plen;
	uint16_t op;
	char sha[6];
	char spa[4];
	char tha[6];
	char tpa[4];
} RAW_ARP;
#endif

#ifndef DHCP_RESULT_DEF
#define DHCP_RESULT_DEF
typedef struct DHCP_RESULT
{	
	char ip[4];
	char net_mask[4];
	char server_ip[4];
	char router_ip[4];
	uint8_t answer;
	int xid;
	int lease_secs;
} DHCP_RESULT;
#endif
