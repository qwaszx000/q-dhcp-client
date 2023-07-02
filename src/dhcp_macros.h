#ifndef DHCP_MACROS
#define DHCP_MACROS

static int opt_i = 0;

#define OPT_COUNTER_RESET() opt_i = 0;
#define OPT_COUNTER_ADD(_num) opt_i += _num;

#define SET_OPT(_val) msg->opt[opt_i++] = _val
#define SET_OPT16(_val) *(uint16_t*)(msg->opt+opt_i) = (uint16_t)_val; opt_i+=2
#define SET_OPTL(_val, _len) for(tmp=0;tmp<_len;tmp++)\
				SET_OPT(_val[tmp]);

#define GET_OPT() msg->opt[opt_i++]
#define GET_OPT32() *(uint32_t*)(msg->opt+opt_i); opt_i += 4
#define GET_OPTL(_var, _len) for(tmp=0;tmp<_len;tmp++)\
				_var[tmp] = GET_OPT();

#define MSG_SIZE (sizeof(*msg)-308+opt_i)

#define DEBUG_DHCP_SEND(_file) if(options->debug && !options->no_dbg_files){\
		printf("Sent %d bytes\nWriting to " _file "\n", tmp);\
		f = fopen(_file, "wb");\
		fwrite(&msg, tmp, 1, f);\
		fclose(f);\
	}

#define DEBUG_DHCP_RECV(_file) if(options->debug && !options->no_dbg_files){\
		printf("Writing recv buffer to " _file "\n");\
		f = fopen(_file, "wb");\
		fwrite(&msg, tmp, 1, f);\
		fclose(f);\
	}

#endif
