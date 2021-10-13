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

/* Whether the callback executed already */
char run = 0;

/* Callback called by the H.264 decoder whenever a frame is decoded and ready.
 * Prints metadata about the video
 */
void onFrameReady(SBufferInfo *bufInfo)
{
    if (!run) {
        printf("width:%d height:%d stride[0]:%d stride[1]:%d\n",
		       bufInfo->UsrData.sSystemBuffer.iWidth,
			   bufInfo->UsrData.sSystemBuffer.iHeight,
			   bufInfo->UsrData.sSystemBuffer.iStride[0],
			   bufInfo->UsrData.sSystemBuffer.iStride[1]);
        run = 1;
    }
}

/* Decode an H.264 video.
 * Input:
 *   - video filename
 *   - output filename: where the frames are saved
 *   - whether frames should be saved
 *   - callback called whenever a frame is decoded and ready
 * Output: error code
 */
int main()
{
    int32_t x = h264_decode("in.h264", "output", true, &onFrameReady);
    return x;
}
