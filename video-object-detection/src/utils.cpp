#include <assert.h>

extern "C" {
	#include "image.h"
}
#include "codec_def.h"

// Print detection probability for each object detected
void print_detection_probabilities(image im, detection *dets, int num, float thresh, char **names, int classes)
{
    int i,j;
    bool found = false;

    for(i = 0; i < num; ++i){
        for(j = 0; j < classes; ++j){
            if (dets[i].prob[j] > thresh){
                printf("%s: %.0f%%\n", names[j], dets[i].prob[j]*100);
                found = true;
            }
        }
    }

    if (!found)
        printf("No objects detected\n");
}

static float get_pixel(image m, int x, int y, int c)
{
    assert(x < m.w && y < m.h && c < m.c);
    return m.data[c*m.h*m.w + y*m.w + x];
}

static void set_pixel(image m, int x, int y, int c, float val)
{
    if (x < 0 || y < 0 || c < 0 || x >= m.w || y >= m.h || c >= m.c) return;
    assert(x < m.w && y < m.h && c < m.c);
    m.data[c*m.h*m.w + y*m.w + x] = val;
}

// Make image contiguous.
// OpenH264 outputs frames whose rows are not contiguous (separated by a variable stride)
void linearize_openh264_frame_buffer(SBufferInfo *bufInfo, unsigned char *buffer_linearized)
{
    int i;
    int w = bufInfo->UsrData.sSystemBuffer.iWidth;
    int h = bufInfo->UsrData.sSystemBuffer.iHeight;
    int stride0 = bufInfo->UsrData.sSystemBuffer.iStride[0];
    int stride1 = bufInfo->UsrData.sSystemBuffer.iStride[1];
    unsigned char *ptr;

    // Luminance (Y) channel
    ptr = bufInfo->pDst[0];
    for(i = 0; i < h; i++) {
        memcpy(buffer_linearized, ptr, w);
        ptr += stride0;
        buffer_linearized += w;
    }

    w = w / 2;
    h = h / 2;

    // Cb channel
    ptr = bufInfo->pDst[1];
    for(i = 0; i < h; i++) {
        memcpy(buffer_linearized, ptr, w);
        ptr += stride1;
        buffer_linearized += w;
    }

    // Cr channel
    ptr = bufInfo->pDst[2];
    for(i = 0; i < h; i++) {
        memcpy(buffer_linearized, ptr, w);
        ptr += stride1;
        buffer_linearized += w;
    }
}

// Convert to JFIF YUV colorspace (cf. ITU-T T.871)
// TODO: make sure this colorspace transformation works for any H264 video
void yuv_jfif_to_rgb(image im)
{
    assert(im.c == 3);
    int i, j;
    float r, g, b;
    float y, u, v;
    for(j = 0; j < im.h; ++j){
        for(i = 0; i < im.w; ++i){
            y = get_pixel(im, i, j, 0);
            u = get_pixel(im, i, j, 1);
            v = get_pixel(im, i, j, 2);

            r = y + 1.402*(v-0.5);
            g = y + -.34414*(u-0.5) + -.71414*(v-0.5);
            b = y + 1.772*(u-0.5);

            set_pixel(im, i, j, 0, r);
            set_pixel(im, i, j, 1, g);
            set_pixel(im, i, j, 2, b);
        }
    }
}

// Load image from YUV data (I420)
// TODO: test with images with odd width or height 
image load_image_from_raw_yuv(unsigned char *yuv_data, int w, int h, int c)
{
    if (!yuv_data) {
        fprintf(stderr, "Cannot load image\n");
        exit(0);
    }
    int i,j,k;
    image im = make_image(w, h, c);

    // Luminance (Y) channel
    k = 0;
    for(j = 0; j < h; ++j){
        for(i = 0; i < w; ++i){
            int dst_index = i + w*j + w*h*k;
            int src_index = dst_index;
            im.data[dst_index] = (float)yuv_data[src_index]/255.;
        }
    }

    // Cb channel
	k = 1;
	for(j = 0; j < h; ++j){
		for(i = 0; i < w; ++i){
			int dst_index = i + w*j + w*h*k;
			int src_index = i/2 + (w/2)*(j/2) + w*h;
			im.data[dst_index] = (float)yuv_data[src_index]/255.;
		}
    }

    // Cr channel
	k = 2;
	for(j = 0; j < h; ++j){
		for(i = 0; i < w; ++i){
			int dst_index = i + w*j + w*h*k;
			int src_index = i/2 + (w/2)*(j/2) + w*h + (w/2)*(h/2);
			im.data[dst_index] = (float)yuv_data[src_index]/255.;
		}
    }

    return im;
}

// Load image from YUV image (I420) and convert it to RGB
image load_image_color_from_raw_yuv(unsigned char *yuv_data, int w, int h)
{
    image out = load_image_from_raw_yuv(yuv_data, w, h, 3);

    if((h && w) && (h != out.h || w != out.w)){
        image resized = resize_image(out, w, h);
        free_image(out);
        out = resized;
    }

    // XXX: would be a bit faster to convert each pixel to RGB in `load_image_from_raw_yuv()` directly
    yuv_jfif_to_rgb(out);

    return out;
}

