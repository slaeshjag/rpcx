#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "rpcx.h"

int main(int argc, char **argv) {
	struct RPCXInfo ri;
	FILE *out;

	rpcx_init(argv[1], &ri);
	ri.data = malloc(ri.w * ri.h);
	memset(ri.data, 0, ri.w * ri.h);
	rpcx_read(&ri);
	
	out = fopen("/tmp/outpic.raw", "w");
	fwrite(ri.data, ri.w, ri.h, out);
	printf("offset: %i\n", (int) ftell(out));
	fwrite(ri.palette, 3, 256, out);
	fclose(out);

	return 0;
}
