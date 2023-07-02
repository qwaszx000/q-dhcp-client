#include<netinet/in.h>
#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>

#include"./structures.h"
#include"./defines.h"
#include"./dhcp_macros.h"

static int send_dhcp_message(const int s, const uint32_t dest_ip, DHCP_MESSAGE *msg)
{
	struct sockaddr_in dst;
	int tmp;

	dst.sin_family = AF_INET;
	dst.sin_port = htons(SERVER_UDP_PORT);
	dst.sin_addr.s_addr = htonl(dest_ip);

	tmp = sendto(s, msg, MSG_SIZE, 0,
		(struct sockaddr*)&dst, sizeof(dst));

	if(-1 == tmp){
		perror("send_dhcp_message sendto error");
		exit(-1);
	}
	return tmp;
}

static int recv_dhcp_message(const int s, DHCP_MESSAGE *buffer)
{
	int tmp;

	tmp = recv(s, buffer, sizeof(*buffer), 0);
	if(-1 == tmp){
		perror("recv error in recv_dhcp_message");
		exit(-1);
	}
	return tmp;
}

static void generate_xid(DHCP_RESULT *dhcp_res)
{
	FILE *f;

	f = fopen("/dev/urandom", "rb");
	if(NULL == f){
		dhcp_res->xid = rand();
	} else {
		fread(&dhcp_res->xid, 4, 1, f);
		fclose(f);
	}
}

static void init_dhcp_msg(DHCP_MESSAGE *msg, char *hwaddr, int xid)
{
	OPT_COUNTER_RESET();
	memset(msg, 0, sizeof(*msg));
	msg->op = BOOTREQUEST;
	msg->htype = HTYPE_ETH;
	msg->hlen = HWADDR_LEN;

	msg->xid = xid;
	memcpy(msg->chaddr, hwaddr, HWADDR_LEN);

	*(uint32_t*)msg->magic = htonl(DHCP_MAGIC_COOKIE);
	SET_OPT(OPTION_CLIENTID);
	SET_OPT(HWADDR_LEN+1);
	SET_OPT(HTYPE_ETH);
	memcpy(msg->opt+3, hwaddr, HWADDR_LEN);
	OPT_COUNTER_ADD(6);
}

static void prep_dhcp_discover(DHCP_MESSAGE *msg)
{
	SET_OPT(OPTION_MSG_TYPE);
	SET_OPT(1);
	SET_OPT(MSGTYPE_DISCOVER);
	SET_OPT(OPTION_MSG_SIZEMAX);
	SET_OPT(2);
	SET_OPT16( htons(MAX_DHCP_RECV_SIZE) );
	SET_OPT(OPTION_END);
}

static void process_dhcp_offer(DHCP_MESSAGE *msg, DHCP_RESULT *dhcp_res)
{
	int tmp;
	unsigned char c;

	memcpy(dhcp_res->ip, msg->yiaddr, IP_ADDR_LEN);

	OPT_COUNTER_RESET();
	while( (c = GET_OPT()) != OPTION_END){
		switch(c){
		case OPTION_SUBNETMASK:
			OPT_COUNTER_ADD(1);
			GET_OPTL(dhcp_res->net_mask, IP_ADDR_LEN);
			break;
		case OPTION_ROUTERS:
			tmp = GET_OPT();
			GET_OPTL(dhcp_res->router_ip, IP_ADDR_LEN);
			OPT_COUNTER_ADD(tmp - IP_ADDR_LEN);
			break;
		case OPTION_SERVER_IP:
			tmp = GET_OPT();
			GET_OPTL(dhcp_res->server_ip, IP_ADDR_LEN);
			break;
		case OPTION_LEASE_TIME:
			OPT_COUNTER_ADD(1);
			tmp = GET_OPT32();
			dhcp_res->lease_secs = ntohl(tmp);
			break;
		case OPTION_PAD:
			break;
		/*Skip other options*/
		default:
			tmp = GET_OPT();
			OPT_COUNTER_ADD(tmp);
			break;
		}
	}
}

static void prep_dhcp_request(DHCP_MESSAGE *msg, DHCP_RESULT *dhcp_res)
{
	int tmp;

	memcpy(msg->siaddr, dhcp_res->server_ip, IP_ADDR_LEN);

	SET_OPT(OPTION_MSG_TYPE);
	SET_OPT(1);
	SET_OPT(MSGTYPE_REQUEST);

	SET_OPT(OPTION_MSG_SIZEMAX);
	SET_OPT(2);
	SET_OPT16( htons(MAX_DHCP_RECV_SIZE) );

	SET_OPT(OPTION_REQUESTED_IP);
	SET_OPT(IP_ADDR_LEN);
	SET_OPTL(dhcp_res->ip, IP_ADDR_LEN);

	SET_OPT(OPTION_SERVER_IP);
	SET_OPT(IP_ADDR_LEN);
	SET_OPTL(dhcp_res->server_ip, IP_ADDR_LEN);

	SET_OPT(OPTION_END);
}

static void process_resp(DHCP_MESSAGE *msg, DHCP_RESULT *dhcp_res)
{
	unsigned char c;
	int tmp;

	OPT_COUNTER_RESET();
	while( (c = GET_OPT()) != OPTION_END){
		switch(c){
		case OPTION_MSG_TYPE:
			OPT_COUNTER_ADD(1);
			dhcp_res->answer = GET_OPT();
			goto end;
		case OPTION_PAD:
			break;
		default:
			tmp = GET_OPT();
			OPT_COUNTER_ADD(tmp);
			break;
		}
	}
end: return;
}

static void prep_dhcp_release(DHCP_MESSAGE *msg, DHCP_RESULT *dhcp_res)
{
	int tmp;

	SET_OPT(OPTION_MSG_TYPE);
	SET_OPT(1);
	SET_OPT(MSGTYPE_RELEASE);

	SET_OPT(OPTION_REQUESTED_IP);
	SET_OPT(IP_ADDR_LEN);
	SET_OPTL(dhcp_res->ip, IP_ADDR_LEN);

	/*SET_OPT(OPTION_SERVER_IP);
	SET_OPT(IP_ADDR_LEN);
	SET_OPTL(dhcp_res->server_ip, IP_ADDR_LEN);*/

	SET_OPT(OPTION_END);
}

static void prep_dhcp_decline(DHCP_MESSAGE *msg, DHCP_RESULT *dhcp_res)
{
	int tmp;

	SET_OPT(OPTION_MSG_TYPE);
	SET_OPT(1);
	SET_OPT(MSGTYPE_DECLINE);

	SET_OPT(OPTION_REQUESTED_IP);
	SET_OPT(IP_ADDR_LEN);
	SET_OPTL(dhcp_res->ip, IP_ADDR_LEN);

	SET_OPT(OPTION_SERVER_IP);
	SET_OPT(IP_ADDR_LEN);
	SET_OPTL(dhcp_res->server_ip, IP_ADDR_LEN);

	SET_OPT(OPTION_END);
}

void perform_dhcp(const int s, char *hwaddr, ARGS_OPTIONS *options, DHCP_RESULT *dhcp_res)
{
	int tmp;
	DHCP_MESSAGE msg;
	FILE *f;

	generate_xid(dhcp_res);

	init_dhcp_msg(&msg, hwaddr, dhcp_res->xid);
	prep_dhcp_discover(&msg);

	tmp = send_dhcp_message(s, BROADCAST_ADDR, &msg);
	DEBUG_DHCP_SEND( DEBUG_DIR_PATH "dhcp_discover.bin" );
	tmp = recv_dhcp_message(s, &msg);
	DEBUG_DHCP_RECV( DEBUG_DIR_PATH "dhcp_offer.bin" );

	process_dhcp_offer(&msg, dhcp_res);

	init_dhcp_msg(&msg, hwaddr, dhcp_res->xid);
	prep_dhcp_request(&msg, dhcp_res);

	tmp = send_dhcp_message(s, BROADCAST_ADDR, &msg);
	DEBUG_DHCP_SEND( DEBUG_DIR_PATH "dhcp_request.bin" );
	tmp = recv_dhcp_message(s, &msg);
	DEBUG_DHCP_RECV( DEBUG_DIR_PATH "dhcp_resp.bin" );

	process_resp(&msg, dhcp_res);
}

void dhcp_release(const int s, char *hwaddr, ARGS_OPTIONS *options, DHCP_RESULT *dhcp_res)
{
	int tmp;
	DHCP_MESSAGE msg;
	FILE *f;

	generate_xid(dhcp_res);

	init_dhcp_msg(&msg, hwaddr, dhcp_res->xid);
	prep_dhcp_release(&msg, dhcp_res);

	tmp = send_dhcp_message(s, BROADCAST_ADDR, &msg);
	DEBUG_DHCP_SEND( DEBUG_DIR_PATH "dhcp_release.bin" );
}

void dhcp_decline(const int s, char *hwaddr, ARGS_OPTIONS *options, DHCP_RESULT *dhcp_res)
{
	int tmp;
	DHCP_MESSAGE msg;
	FILE *f;

	init_dhcp_msg(&msg, hwaddr, dhcp_res->xid);
	prep_dhcp_decline(&msg, dhcp_res);

	tmp = send_dhcp_message(s, BROADCAST_ADDR, &msg);
	DEBUG_DHCP_SEND( DEBUG_DIR_PATH "dhcp_decline.bin" );
}
