#ifndef DHCP_PROTO_HANDLER
#define DHCP_PROTO_HANDLER

extern void perform_dhcp(const int s, char *hwaddr);
extern void dhcp_release(const int s, char *hwaddr);
extern void dhcp_decline(const int s, char *hwaddr);

#endif
