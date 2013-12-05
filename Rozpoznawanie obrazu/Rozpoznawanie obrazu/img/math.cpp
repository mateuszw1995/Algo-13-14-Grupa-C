#pragma once
#include "math.h"
#include <algorithm>

point::point(int x, int y) : x(x), y(y) {}

point point::operator+(const point& p) {
	return point(x + p.x, y + p.y);
}

rect_wh::rect_wh(const rect_ltrb& rr) : w(rr.w()), h(rr.h()) {} 
rect_wh::rect_wh(const rect_xywh& rr) : w(rr.w), h(rr.h) {} 
rect_wh::rect_wh(int w, int h) : w(w), h(h) {}

int rect_wh::fits(const rect_wh& r) const {
	if(w == r.w && h == r.h) return 3;
	if(h == r.w && w == r.h) return 4;
	if(w <= r.w && h <= r.h) return 1;
	if(h <= r.w && w <= r.h) return 2;
	return 0;
}

rect_ltrb::rect_ltrb() : l(0), t(0), r(0), b(0) {}
rect_ltrb::rect_ltrb(const rect_wh& rr) : l(0), t(0), r(rr.w), b(rr.h) {}
rect_ltrb::rect_ltrb(const rect_xywh& rr) : l(rr.x), t(rr.y), r(rr.x+rr.w), b(rr.y+rr.h) {}
rect_ltrb::rect_ltrb(int l, int t, int r, int b) : l(l), t(t), r(r), b(b) {}

int rect_ltrb::w() const {
	return r-l;
}

int rect_ltrb::h() const {
	return b-t;
}

int rect_ltrb::area() const {
	return w()*h();
}

int rect_ltrb::perimeter() const {
	return 2*w() + 2*h();
}

int rect_ltrb::max_side() const {
	return std::max(w(), h());
}

bool rect_ltrb::clip(const rect_ltrb& rc) {
	if(l >= rc.r || t >= rc.b || r <= rc.l || b <= rc.t) {
		*this = rect_ltrb();
		return false;
	}
	*this = rect_ltrb(std::max(l, rc.l),
		std::max(t, rc.t),
		std::min(r, rc.r),
		std::min(b, rc.b));
	return true;
}

bool rect_ltrb::hover(const point& m) {
	return m.x >= l && m.y >= t && m.x <= r && m.y <= b;
}

void rect_ltrb::w(int ww) {
	r = l+ww;
}

void rect_ltrb::h(int hh) {
	b = t+hh;
}

rect_xywh::rect_xywh() : x(0), y(0) {}
rect_xywh::rect_xywh(const rect_wh& rr) : x(0), y(0), rect_wh(rr) {}
rect_xywh::rect_xywh(const rect_ltrb& rc) : x(rc.l), y(rc.t) { b(rc.b); r(rc.r); }
rect_xywh::rect_xywh(int x, int y, int w, int h) : x(x), y(y), rect_wh(w, h) {}

bool rect_xywh::clip(const rect_xywh& rc) {
	if(x >= rc.r() || y >= rc.b() || r() <= rc.x || b() <= rc.y) {
		*this = rect_ltrb(std::max(x, rc.x),
			std::max(y, rc.y),
			std::min(r(), rc.r()),
			std::min(b(), rc.b()));
		return true;
	}
	*this = rect_xywh();
	return false;
}

bool rect_xywh::hover(const point& m) {
	return m.x >= x && m.y >= y && m.x <= r() && m.y <= b();
}

int rect_xywh::r() const {
	return x+w;
};

int rect_xywh::b() const {
	return y+h;
}

void rect_xywh::r(int right) {
	w = right-x;
}

void rect_xywh::b(int bottom) {
	h = bottom-y;
}

int rect_wh::area() const {
	return w*h;
}

int rect_wh::perimeter() const {
	return 2*w + 2*h; 
}

int rect_wh::max_side() const {
	return std::max(w, h);
}


rect_xywhf::rect_xywhf(const rect_ltrb& rr) : rect_xywh(rr), flipped(false) {}
rect_xywhf::rect_xywhf(int x, int y, int width, int height, bool flipped) : rect_xywh(x, y, width, height), flipped(flipped) {}
rect_xywhf::rect_xywhf() : flipped(false) {}

void rect_xywhf::flip() { 
	flipped = !flipped;
	std::swap(w, h);
}
