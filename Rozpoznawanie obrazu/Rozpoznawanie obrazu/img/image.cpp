#pragma once
#include "image.h"
#include <Windows.h>
#include <gdiplus.h>
#include <soil\SOIL.h>

using namespace Gdiplus;

GdiplusStartupInput gdi;
ULONG_PTR           gdit;

void image::init() {
	GdiplusStartup(&gdit, &gdi, NULL);
}

void image::deinit() {
	GdiplusShutdown(gdit);
}

void image::_zero() {
	allocated = file = false;
	data = 0;
	channels = 0;
	size = rect_wh(0, 0);
}

image::image() { _zero(); }

image::~image() {
	destroy();
}

image::image(unsigned char* p, int channels, const rect_wh& size) {
	_zero();
	set(p, channels, size);
}

image::image(const image& img) {
	_zero();
	copy(img);
}

image& image::operator=(const image& img) {
	copy(img);
	return *this;
}

void image::set(unsigned char* p, int ch, const rect_wh& s) {
	destroy();
	data = p;
	channels = ch;
	size = s;
}

void image::set(const image& img) {
	set(img.data, img.channels, img.size);
}

void image::create(int w, int h, int ch) {
	destroy();
	size = rect_wh(w, h);
	channels = ch;
	data = new unsigned char[get_bytes()];
	allocated = true;
}

bool image::from_clipboard() {
	using namespace Gdiplus;

	bool ret = false;
	if(OpenClipboard(0)) {
		HBITMAP h = (HBITMAP)GetClipboardData(CF_BITMAP);
		if(h) {
			Bitmap b(h, 0);
			BitmapData bi = {0};
			b.LockBits(&Rect(0,0,b.GetWidth(), b.GetHeight()), ImageLockModeRead, PixelFormat24bppRGB, &bi);
			create(b.GetWidth(), b.GetHeight(), 3);

			for(int i = 0; i < bi.Height; ++i)
				for(int j = 0; j < bi.Width; ++j) {
					*pixel(j, i, 0) = ((unsigned char*)bi.Scan0)[i*bi.Stride + j * 3 + 2];
					*pixel(j, i, 1) = ((unsigned char*)bi.Scan0)[i*bi.Stride + j * 3 + 1];
					*pixel(j, i, 2) = ((unsigned char*)bi.Scan0)[i*bi.Stride + j * 3 + 0];
				}

				b.UnlockBits(&bi);
				DeleteObject(h);
				ret = true;
		}
		CloseClipboard();
	}
	return ret;
}

void image::from_file(const char* filename, unsigned force_channels) {
	destroy();
	data = SOIL_load_image(filename, &size.w, &size.h, &channels, force_channels);
	if(force_channels) channels = force_channels;
	allocated = file = true;
}

void image::fill(unsigned char val) {
	memset(data, val, get_bytes());
}

void image::fill(unsigned char* channel_vals) {
	int i = 0, bytes = get_bytes(), c = 0;

	while(i < bytes)
		for(c = 0; c < channels; ++c)
			data[i++] = channel_vals[c];
}

void image::fill_channel(int ch, unsigned char v) {
	int i = 0, bytes = get_bytes()/channels;

	while(i < bytes) data[ch+channels*i++] = v;
}

void image::copy(const image& img) {
	if(get_bytes() != img.get_bytes())
		create(img.size.w, img.size.h, img.channels);

	memcpy(data, img.data, get_bytes());
	channels = img.channels;
}


#define LOOP  for(int s_y = src.y, d_y = y; s_y < src.b(); ++s_y, ++d_y) \
	for(int s_x = src.x, d_x = x; s_x < src.r(); ++s_x, ++d_x)
#define SLOOP for(int s_y = src_rc.y, d_y = dest_rc.y; (s_y < src_rc.b() && d_y < dest_rc.b()); ++s_y, ++d_y) \
	for(int s_x = src_rc.x, d_x = dest_rc.x; (s_x < src_rc.r() && d_x < dest_rc.r()); ++s_x, ++d_x) 
#define BCH(ch, src_ch) *pixel(d_x, d_y, ch) = src.flipped ? (*img(s_y, s_x, src_ch)) : (*img(s_x, s_y, src_ch))
#define DCH(ch) (pixel(d_x, d_y, ch))
#define SCH(src_ch) (src.flipped ? (img(s_y, s_x, src_ch)) : (img(s_x, s_y, src_ch)))


void image::blit(const image& img, int x, int y, const rect_xywhf& src, bool luminance_to_alpha) {
	int c;
	if(channels == img.channels) {										   
		LOOP {
			for(c = 0; c < channels; ++c) BCH(c, c);												   
		}
	}
	else {
		if(channels <= 2) {
			if(img.channels == 1) {	
				if(luminance_to_alpha) {
					LOOP {
						*DCH(0) = 255;	
						BCH(1, 0);
					}
				}
				else LOOP BCH(0, 0);
			}
			else {						   
				LOOP *DCH(0) = (*SCH(0) + *SCH(1) + *SCH(2)) / 3;															   

				if(channels == 2 && img.channels == 4) LOOP BCH(1, 3);
			}
		}
		if(channels >= 3) {                                                            
			if(img.channels == 2) {													   
				LOOP {																   
					BCH(0, 0);														   
					BCH(1, 0);														   
					BCH(2, 0);														   
				}																	   
				if(channels == 4) LOOP BCH(3, 1);			   			   
			}
			else if(img.channels == 1) {
				if(channels == 4 && luminance_to_alpha) 
					LOOP {
						BCH(3, 0);
						*DCH(0) = *DCH(1) = *DCH(2) = 255;
				}
				else										   
					LOOP {																   
						BCH(0, 0);														   
						BCH(1, 0);														   
						BCH(2, 0);														   
				}						
			}
			else {																	   
				LOOP {																   
					BCH(0, 0);														   
					BCH(1, 1);														   
					BCH(2, 2);														   
				}																	   
			}																		   
		}		
	}
}


void image::blit_channel(const image& img, int x, int y, const rect_xywhf& src, int ch, int src_ch) {
	LOOP BCH(ch, src_ch);
}


unsigned char* image::operator()(int x, int y, int channel) const {
	return pixel(x, y, channel);
}

unsigned char* image::pixel(int x, int y, int channel) const {
	return data + (size.w * y + x) * channels + channel;
}

int image::get_bytes() const {
	return sizeof(unsigned char) * size.w * size.h * channels;
}

int image::get_channels() const {
	return channels;
}

int image::get_num_pixels() const {
	return get_bytes()/get_channels();
}

const rect_wh& image::get_size() const {
	return size;
}

void image::destroy() {
	if(allocated) {
		delete [] data;
	}

	_zero();
}


