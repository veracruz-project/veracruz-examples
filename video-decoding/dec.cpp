/*
H.264 video decoding example

AUTHORS

The Veracruz Development Team.

COPYRIGHT AND LICENSING

See the `LICENSE_MIT.markdown` file in the video decoding example's root
directory for copyright and licensing information.
*/

#include <stdlib.h>

#include "codec_def.h"
#include "h264dec.h"

char run = 0;

// Callback called by the H.264 decoder whenever a frame is decoded and ready
void onFrameReady(SBufferInfo *bufInfo) {
	if (!run) {
		printf("width:%d height:%d stride[0]:%d stride[1]:%d\n", bufInfo->UsrData.sSystemBuffer.iWidth, bufInfo->UsrData.sSystemBuffer.iHeight, bufInfo->UsrData.sSystemBuffer.iStride[0], bufInfo->UsrData.sSystemBuffer.iStride[1]);
		run = 1;
	}
}

int main() {
	int32_t x = h264_decode("in.h264", NULL, false, &onFrameReady);
	return x;
}
