#ifndef NETIF
#define NETIF

extern void get_interface_hwaddr(const int s, char *dst, ARGS_OPTIONS *options);

extern void get_interface_ip(const int s, ARGS_OPTIONS *options, DHCP_RESULT *dhcp_res);

extern void update_interface(const int s, ARGS_OPTIONS *options, DHCP_RESULT *dhcp_res);

#endif
