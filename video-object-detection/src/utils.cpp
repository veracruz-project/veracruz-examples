/*
This file provides util functions for the main program.

AUTHORS

The Veracruz Development Team.

COPYRIGHT AND LICENSING

See the `LICENSE_MIT.markdown` file in the example's root directory for
copyright and licensing information.
Based on darknet, YOLO LICENSE https://github.com/pjreddie/darknet/blob/master/LICENSE
*/

#include <assert.h>

extern "C" {
    #include "image.h"
    #include "stb_image.h"
}
#include "codec_def.h"
#include "utils.h"

// Print detection probability for each object detected
void print_detection_probabilities(image im, detection *dets, int num,
                                   float thresh, char **names, int classes)
{
    int i, j;
    bool found = false;

    for (i = 0; i < num; i++) {
        for (j = 0; j < classes; j++) {
            if (dets[i].prob[j] > thresh) {
                printf("%s: %.0f%%\n", names[j], dets[i].prob[j]*100);
                found = true;
            }
        }
    }

    if (!found)
        printf("No objects detected\n");
}

// Linearize OpenH264 frame buffer and revert chroma subsampling by doubling Cb
// and Cr pixels.
// OpenH264 outputs frames whose rows are not contiguous (separated by a
// variable stride)
// TODO: test with images with odd width or height 
void linearize_openh264_frame_buffer(SBufferInfo *bufInfo,
                                     unsigned char *buffer_linearized)
{
    int i, j;
    int w = bufInfo->UsrData.sSystemBuffer.iWidth;
    int h = bufInfo->UsrData.sSystemBuffer.iHeight;
    int stride0 = bufInfo->UsrData.sSystemBuffer.iStride[0];
    int stride1 = bufInfo->UsrData.sSystemBuffer.iStride[1];
    unsigned char *ptr;

    // Luminance (Y) channel
    ptr = bufInfo->pDst[0];
    for (i = 0; i < h; i++) {
        memcpy(buffer_linearized, ptr, w);
        ptr += stride0;
        buffer_linearized += w;
    }

    // Cb channel
    ptr = bufInfo->pDst[1];
    for (i = 0; i < h; i++) {
        for (j = 0; j < w; j++)
            buffer_linearized[j + i*w] = ptr[j/2];
        // Use each Cb row twice
        if(i > 0 && i % 2 == 0)
            ptr += stride1;
    }

    // Cr channel
    ptr = bufInfo->pDst[2];
    buffer_linearized += w*h;
    for (i = 0; i < h; i++) {
        for (j = 0; j < w; j++)
            buffer_linearized[j + i*w] = ptr[j/2];
        // Use each Cr row twice
        if (i > 0 && i % 2 == 0)
            ptr += stride1;
    }
}

// Convert frame from JFIF YUV to RGB color space (cf. ITU-T T.871).
// Copied and adapted from Darknet's codebase
#define stbi__float2fixed(x)  (((int) ((x) * 4096.0f + 0.5f)) << 8)
static void stbi__YCbCr_to_RGB_row(stbi_uc *out, const stbi_uc *y,
                                   const stbi_uc *pcb, const stbi_uc *pcr,
                                   int width, int height)
{
   int i;
   for (i = 0; i < width*height; i++) {
      int y_fixed = (y[i] << 20) + (1<<19); // rounding
      int r, g, b;
      int cr = pcr[i] - 128;
      int cb = pcb[i] - 128;
      r = y_fixed + cr * stbi__float2fixed(1.40200f);
      g = y_fixed + (cr * -stbi__float2fixed(0.71414f))
          + ((cb * -stbi__float2fixed(0.34414f)) & 0xffff0000);
      b = y_fixed + cb * stbi__float2fixed(1.77200f);
      r >>= 20;
      g >>= 20;
      b >>= 20;
      if ((unsigned) r > 255) { if (r < 0) r = 0; else r = 255; }
      if ((unsigned) g > 255) { if (g < 0) g = 0; else g = 255; }
      if ((unsigned) b > 255) { if (b < 0) b = 0; else b = 255; }
      out[i] = (stbi_uc)r;
      out[i + width*height] = (stbi_uc)g;
      out[i + width*height*2] = (stbi_uc)b;
   }
}

// Convert OpenH264 I420 frame into a Darknet image structure.
// Process:
//   1 - Linearize OpenH264 frame buffer (I420 YUV pixel format)
//   2 - Revert chroma subsampling: duplicate Cb and Cr pixels
//   3 - Transform to RGB color space
//   4 - Convert integer array to Darknet image (float array)
// Input: OpenH264 I420 frame buffer
// Output: Darknet-compatible RGB image
image load_image_from_raw_yuv(SBufferInfo *bufInfo)
{
    int i;
    int w = bufInfo->UsrData.sSystemBuffer.iWidth;
    int h = bufInfo->UsrData.sSystemBuffer.iHeight;
    unsigned char *yuv_frame;

    yuv_frame = (unsigned char *) malloc(w*h*CHANNELS);

    // Linearize OpenH264 frame buffer and revert chroma subsampling
    linearize_openh264_frame_buffer(bufInfo, yuv_frame);

    // Transform frame to RGB
    stbi__YCbCr_to_RGB_row(yuv_frame, yuv_frame, yuv_frame + w*h,
                           yuv_frame + w*h*2, w, h);

    // Convert RGB frame to Darknet image (float array)
    image im = make_image(w, h, CHANNELS);
    for (i = 0; i < w*h*CHANNELS; i++)
        im.data[i] = (float)yuv_frame[i]/255.;

    free(yuv_frame);

    if ((h && w) && (h != im.h || w != im.w)) {
        image resized = resize_image(im, w, h);
        free_image(im);
        im = resized;
    }

    return im;
}
