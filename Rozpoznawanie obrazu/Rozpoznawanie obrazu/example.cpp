#include <iostream>
#include "img/image.h"

using namespace std;

int main() {
	image::init();
	
	image img;
	img.from_file("test.png");
	if(img.get_size().w) {
		// true - jest bitmapa w schowku i udalo sie ja odczytac

		for(int y = 0; y < img.get_size().h; ++y) {
			for(int x = 0; x < img.get_size().w; ++x) {
				for(int channel = 0; channel < img.get_channels(); ++channel) {

					unsigned char color = *img(x, y, channel); // odnoszenie sie do koloru pixela o danych koordynatach (0 - 255)
					cout << int(color) << " "; // by pokazywalo w formie liczbowej a nie jako znak
				}
			}
			cout << endl;
		}
	}

	cout << img.get_bytes() << " bytes read from image." << endl;

	system("pause");
	
	image::deinit();
}