
#include "base_type.h"
#include "log.h"
#include "enc_base.h"
#include "enc_raw_rgb.h"

int  rgb888x_encoder_rgb565(uint16_t * pix_msg ,uint32_t * framebuffer ,int x, int y, int right, int bottom, int line_width)
{
int    last_copied_x, last_copied_y;
int pos = 0;

		// locate to the begining...
		framebuffer += (y * line_width + x);

		LOG("%s .\n",__func__);

#if 1
		for (last_copied_y = y; last_copied_y <= bottom; ++last_copied_y) {

			for (last_copied_x = x; last_copied_x <= right; ++last_copied_x) {


				pixel_type_t pix = *framebuffer;
				uint8_t r, g, b;
				//LOG("fb %p\n",framebuffer);
				r = pix & 0xff;
				g = (pix >> 8) & 0xff;
				b = (pix >> 16) & 0xff;
				uint16_t current_pixel_le = rgb565(b, g, r);
				//current_pixel_le = (current_pixel_le >> 8) | (current_pixel_le << 8);
				*pix_msg = current_pixel_le;
				pix_msg++;
				++framebuffer;
			}
			framebuffer += line_width - right - 1 + x;
		}
#endif

		LOG("%s ..\n",__func__);

		return (right - x + 1) * (bottom - y + 1) * 2;

}

int enc_rgb565::enc( uint8_t * enc, uint8_t * src,int x, int y, int right, int bottom, int line_width,int limit)
{
	return rgb888x_encoder_rgb565((uint16_t *)enc,(uint32_t*)src, x, y, right, bottom, line_width);

}

int enc_mono1::enc(uint8_t *enc, uint8_t *src, int x, int y, int right, int bottom, int line_width, int limit)
{
	const int width=right-x+1, height=bottom-y+1;
	static const uint8_t bayer[8][8]={{0,48,12,60,3,51,15,63},{32,16,44,28,35,19,47,31},{8,56,4,52,11,59,7,55},{40,24,36,20,43,27,39,23},{2,50,14,62,1,49,13,61},{34,18,46,30,33,17,45,29},{10,58,6,54,9,57,5,53},{42,26,38,22,41,25,37,21}};
	// This selector is read on the PC: the 1-bit panel only receives packed pixels.
	static ULONGLONG next_config_read = 0;
	static int mode = 0; // 0 Clear, 1 Dark, 2 Light, 3 Photo, 4 Invert, 5 InvertPhoto
	ULONGLONG now = GetTickCount64();
	if (now >= next_config_read) {
		// Do not use the profile API here: UMDF keeps an INI cache for the
		// lifetime of DriverHost, which prevents live selector changes.
		char text[64] = {};
		DWORD read = 0;
		HANDLE file = CreateFileW(L"C:\\ProgramData\\RLCD-USB-Display\\mode.ini",
			GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL, nullptr);
		if (file != INVALID_HANDLE_VALUE) {
			ReadFile(file, text, sizeof(text) - 1, &read, nullptr);
			CloseHandle(file);
			const char* value = strstr(text, "Mode=");
			if (value && value[5] >= '0' && value[5] <= '5') mode = value[5] - '0';
		}
		if (mode < 0 || mode > 5) mode = 0;
		next_config_read = now + 500;
	}
	if(width!=400 || height!=300 || limit<15000) return -1;
	memset(enc,0,15000);
	for(int py=0;py<300;py++){ int iy=299-py; for(int px=0;px<400;px++){
		uint32_t p=((uint32_t*)src)[(py+y)*line_width+px+x];
		int r=p&0xff, g=(p>>8)&0xff, b=(p>>16)&0xff;
		int lum=(r*77+g*150+b*29)>>8;
		int hi=r>g?(r>b?r:b):(g>b?g:b), lo=r<g?(r<b?r:b):(g<b?g:b);
		int chroma=hi-lo;
		int byte=(px>>1)*75+(iy>>2), bit=7-(((iy&3)<<1)|(px&1));
		uint8_t mask=(uint8_t)(1u<<bit);
		bool white;
		switch (mode) {
		case 1: white = lum > 176; break;
		case 2: white = lum > 144; break;
		case 3: {
			int threshold=bayer[py&7][px&7]*4+2;
			white = lum > threshold;
			break;
		}
		case 4: white = lum <= 128; break;
		case 5: {
			int threshold=bayer[py&7][px&7]*4+2;
			white = lum <= threshold;
			break;
		}
		default: white = (lum - (chroma * 112 + 127) / 255) > 128; break;
		}
		if(white) enc[byte]|=mask;
	}}
	return 15000;
}
