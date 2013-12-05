#pragma once
#include <vector>

struct rect_ltrb;
struct rect_xywh;

struct point {
	int x, y;
	point(int x = 0, int y = 0);
	point operator+(const point&);
};

struct rect_wh {
	rect_wh(const rect_ltrb&);
	rect_wh(const rect_xywh&);
	rect_wh(int w = 0, int h = 0);
	int w,h, area() const, perimeter() const, max_side() const, 
		fits(const rect_wh& bigger) const; // 0 - no, 1 - yes, 2 - flipped, 3 - perfectly, 4 perfectly flipped
};

struct rect_ltrb {
	rect_ltrb();
	rect_ltrb(const rect_wh&);
	rect_ltrb(const rect_xywh&);
	rect_ltrb(int left, int top, int right, int bottom);

	bool clip(const rect_ltrb& bigger);
	bool hover(const point& mouse);
	int l, t, r, b, w() const, h() const, area() const, perimeter() const, max_side() const; // false - null rectangle
	void w(int), h(int);
};

struct rect_xywh : public rect_wh {
	rect_xywh();
	rect_xywh(const rect_wh&);
	rect_xywh(const rect_ltrb&);
	rect_xywh(int x, int y, int width, int height);

	bool clip(const rect_xywh& bigger); // false - null rectangle
	bool hover(const point& mouse);

	int x, y, r() const, b() const;
	void r(int), b(int);
};

struct rect_xywhf : public rect_xywh {
	rect_xywhf(const rect_ltrb&);
	rect_xywhf(int x, int y, int width, int height, bool flipped = false);
	rect_xywhf();
	void flip();
	bool flipped;
};