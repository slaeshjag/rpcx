#ifndef RPCX_H_
#define	RPCX_H_

#include <stdint.h>

struct RPCXInfo {
	uint8_t		palette[256*3];
	int		colors;

	int		w;
	int		h;

	unsigned char	*data;
};

int rpcx_init(const char *fname, struct RPCXInfo *ri);
int rpcx_read(struct RPCXInfo *ri);
void rpcx_close();

#endif
