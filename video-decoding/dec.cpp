/*
H.264 video decoding example

AUTHORS

The Veracruz Development Team.

COPYRIGHT AND LICENSING

See the `LICENSE_MIT.markdown` file in the video decoding example's root
directory for copyright and licensing information.
*/

#include <stdlib.h>

#include "h264dec.h"

int main() {
	int32_t x = h264_decode("in.h264", "output");
	return x;
}
