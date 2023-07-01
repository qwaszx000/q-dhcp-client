#include<netinet/in.h>
#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>

#include"./structures.h"
#include"./defines.h"

extern ARGS_OPTIONS options;

static DHCP_MESSAGE msg;
static char xid[4];
static int opt_i = 0;
static int tmp = 0;

char ip[IP_ADDR_LEN];
char net_mask[IP_ADDR_LEN];
char sip[IP_ADDR_LEN];
char rip[IP_ADDR_LEN];
int lease_secs;
uint8_t dhcp_result = 0;

#include"./dhcp_macros.h"

static int send_dhcp_message(const int s, const uint32_t dest_ip){	
	struct sockaddr_in dest;
	dest.sin_family = AF_INET;
	dest.sin_port = htons(SERVER_UDP_PORT);
	dest.sin_addr.s_addr = htonl(dest_ip);

	tmp = sendto(s, &msg, MSG_SIZE, 0,
		(struct sockaddr*)&dest, sizeof(dest));

	if(-1 == tmp){
		perror("send_dhcp_message sendto error");
		exit(-1);
	}
	return tmp;
}

static int recv_dhcp_message(const int s){
	tmp = recv(s, &msg, sizeof(msg), 0);
	if(-1 == tmp){
		perror("recv error in recv_dhcp_message");
		exit(-1);
	}
	return tmp;
}

static void generate_xid(){
	FILE *f = fopen("/dev/urandom", "rb");
	if(NULL == f){
		*(int*)xid = rand();
	} else {
		fread(xid, 4, 1, f);
		fclose(f);
	}
}

static void init_dhcp_msg(char *hwaddr){
	opt_i = 0;
	memset(&msg, 0, sizeof(msg));
	msg.op = BOOTREQUEST;
	msg.htype = HTYPE_ETH;
	msg.hlen = HWADDR_LEN;

	memcpy(msg.xid, xid, 4);
	memcpy(msg.chaddr, hwaddr, HWADDR_LEN);

	*(uint32_t*)msg.magic = htonl(DHCP_MAGIC_COOKIE);
	SET_OPT(OPTION_CLIENTID);
	SET_OPT(HWADDR_LEN+1);
	SET_OPT(HTYPE_ETH);
	memcpy(msg.opt+3, hwaddr, HWADDR_LEN);
	opt_i+=6;
}

static void prep_dhcp_discover(){
	SET_OPT(OPTION_MSG_TYPE);
	SET_OPT(1);
	SET_OPT(MSGTYPE_DISCOVER);
	SET_OPT(OPTION_MSG_SIZEMAX);
	SET_OPT(2);
	SET_OPT16( htons(MAX_DHCP_RECV_SIZE) );
	SET_OPT(OPTION_END);
}

static void process_dhcp_offer(){
	memcpy(ip, msg.yiaddr, IP_ADDR_LEN);

	opt_i = 0;
	unsigned char c;
	while( (c = GET_OPT()) != OPTION_END){
		switch(c){
		case OPTION_SUBNETMASK:
			opt_i++;
			GET_OPTL(net_mask, IP_ADDR_LEN);
			break;
		case OPTION_ROUTERS:
			tmp = GET_OPT();
			GET_OPTL(rip, IP_ADDR_LEN);
			opt_i += tmp - IP_ADDR_LEN;
			break;
		case OPTION_SERVER_IP:
			tmp = GET_OPT();
			GET_OPTL(sip, IP_ADDR_LEN);
			break;
		case OPTION_LEASE_TIME:
			opt_i++;
			lease_secs = GET_OPT32();
			lease_secs = ntohl(lease_secs);
			break;
		case OPTION_PAD:
			break;
		/*Skip other options*/
		default:
			tmp = GET_OPT();
			opt_i+=tmp;
			break;
		}
	}
}

static void prep_dhcp_request(){
	memcpy(msg.siaddr, sip, IP_ADDR_LEN);

	SET_OPT(OPTION_MSG_TYPE);
	SET_OPT(1);
	SET_OPT(MSGTYPE_REQUEST);

	SET_OPT(OPTION_MSG_SIZEMAX);
	SET_OPT(2);
	SET_OPT16( htons(MAX_DHCP_RECV_SIZE) );

	SET_OPT(OPTION_REQUESTED_IP);
	SET_OPT(IP_ADDR_LEN);
	SET_OPTL(ip, IP_ADDR_LEN);

	SET_OPT(OPTION_SERVER_IP);
	SET_OPT(IP_ADDR_LEN);
	SET_OPTL(sip, IP_ADDR_LEN);

	SET_OPT(OPTION_END);
}

static void process_resp(){
	opt_i = 0;
	unsigned char c;
	while( (c = GET_OPT()) != OPTION_END){
		switch(c){
		case OPTION_MSG_TYPE:
			opt_i++;
			dhcp_result = GET_OPT();
			goto end;
		case OPTION_PAD:
			break;
		default:
			tmp = GET_OPT();
			opt_i+=tmp;
			break;
		}
	}
end: return;
}

static void prep_dhcp_release(){
	SET_OPT(OPTION_MSG_TYPE);
	SET_OPT(1);
	SET_OPT(MSGTYPE_RELEASE);

	SET_OPT(OPTION_REQUESTED_IP);
	SET_OPT(IP_ADDR_LEN);
	SET_OPTL(ip, IP_ADDR_LEN);

	SET_OPT(OPTION_SERVER_IP);
	SET_OPT(IP_ADDR_LEN);
	SET_OPTL(sip, IP_ADDR_LEN);

	SET_OPT(OPTION_END);
}

static void prep_dhcp_decline(){
	SET_OPT(OPTION_MSG_TYPE);
	SET_OPT(1);
	SET_OPT(MSGTYPE_DECLINE);

	SET_OPT(OPTION_REQUESTED_IP);
	SET_OPT(IP_ADDR_LEN);
	SET_OPTL(ip, IP_ADDR_LEN);

	SET_OPT(OPTION_SERVER_IP);
	SET_OPT(IP_ADDR_LEN);
	SET_OPTL(sip, IP_ADDR_LEN);

	SET_OPT(OPTION_END);
}

void perform_dhcp(const int s, char *hwaddr){
	generate_xid();

	init_dhcp_msg(hwaddr);
	prep_dhcp_discover();

	tmp = send_dhcp_message(s, BROADCAST_ADDR);
	DEBUG_DHCP_SEND("./dhcp_discover.bin");
	tmp = recv_dhcp_message(s);
	DEBUG_DHCP_RECV("./dhcp_offer.bin");
	process_dhcp_offer();

	init_dhcp_msg(hwaddr);
	prep_dhcp_request();

	tmp = send_dhcp_message(s, BROADCAST_ADDR);
	DEBUG_DHCP_SEND("./dhcp_request.bin");
	tmp = recv_dhcp_message(s);
	DEBUG_DHCP_RECV("./dhcp_resp.bin");

	process_resp();
}

void dhcp_release(const int s, char *hwaddr){
	generate_xid();

	init_dhcp_msg(hwaddr);
	prep_dhcp_release();

	tmp = send_dhcp_message(s, BROADCAST_ADDR);
	DEBUG_DHCP_SEND("./dhcp_release.bin");
}

void dhcp_decline(const int s, char *hwaddr){
	init_dhcp_msg(hwaddr);
	prep_dhcp_decline();

	tmp = send_dhcp_message(s, BROADCAST_ADDR);
	DEBUG_DHCP_SEND("./dhcp_decline.bin");
}
