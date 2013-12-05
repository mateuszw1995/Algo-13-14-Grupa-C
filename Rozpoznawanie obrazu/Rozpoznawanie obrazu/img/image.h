#pragma once
#include "math.h"

class image {
	friend class image_file;
	unsigned char* data;
	rect_wh size;
	bool allocated, file;
	int channels;
	void _zero();
public:
	static void init();
	static void deinit();
	image();
	virtual ~image();

	// set
	image(unsigned char* ptr, int channels, const rect_wh& size);

	image(const image&);
	image& operator=(const image&);

	void set(unsigned char* ptr, int channels, const rect_wh& size);
	void set(const image&);

	void create	  (int w, int h, int channels);
	void from_file(const char* filename, unsigned channels = 0);
	bool from_clipboard();

	void fill(unsigned char val), 
		fill(unsigned char* channel_vals),
		fill_channel(int channel, unsigned char val);

	void copy(const image&);

	void blit		 (const image&, int x, int y, const rect_xywhf& src, bool luminance_to_alpha = false);
	void blit_channel(const image&, int x, int y, const rect_xywhf& src, int channel, int src_channel);

	unsigned char* operator()(int x, int y, int channel = 0) const; // get pixel
	unsigned char* pixel(int x = 0, int y = 0, int channel = 0) const;

	int get_channels() const, get_bytes() const, get_num_pixels() const;
	const rect_wh& get_size() const;

	void destroy();
};