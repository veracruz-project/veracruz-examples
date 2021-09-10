#include <stdlib.h>

#include "h264dec.h"

int main() {
	int32_t x = h264_decode("in.h264", "output");
	return x;
}
