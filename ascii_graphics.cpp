#include <ostream>
#include <vector>
#include <string>
#include "ascii_graphics.h"

Ascii_graphics::Ascii_graphics(int rows, int cols)
	: canvas(rows, std::string(cols, ' ')) {}

void Ascii_graphics::draw_circle(int radius, int o_x, int o_y) {
	// the origin will be at (12, 12) 
	// so that our drawing is 
	// well positioned 
	// all the points that satisfy the 
	// equation x^2 + y^2 = 10^2 will be 
	// filled as * 
	const int delta = int(0.8 * radius);
	const int minsq = radius * radius - delta;
	const int maxsq = radius * radius + delta;
	
	for(size_t row = 0; row < canvas.size(); row++) {
		for(size_t col = 0; col < canvas[row].size(); col++) {
			// obtain the x and y coordinates of each element with 
			// respect to the origin 
			int x = col - o_x;
			int y = o_y - row;

			// x^2 + y^2 = radius^2 
			// since we are dealing with integers, we can keep 
			// +/-delta tolerance for nicer looks 
			int sumsq = x*x + y*y;

			if((minsq < sumsq) && (sumsq < maxsq)) {
				canvas[row][col] = '*';
			}
		}
	}
}

void Ascii_graphics::draw_text(std::string text, int o_x, int o_y) {
	// just overwrite the canvas with the new text
	canvas[o_y].replace(o_x, text.size(), text);
}
	
void Ascii_graphics::print(std::ostream &stream) const {
	for(size_t row = 0; row < canvas.size(); row++) {
		for(size_t col = 0; col < canvas[row].size(); col++) {
			// THERE IS A SPACE AFTER each character 
			// ... done for nicer looks 
			stream << canvas[row][col] << ' ';
		}
		stream << std::endl;
	}
}

// Test class by drawing larger and larger circles
void test_ascii_graphics(std::ostream &stream) {
	for (size_t dim = 6; dim < 120; dim += 6) {
		Ascii_graphics canvas(dim, dim);
		int radius = (int)(0.4 * dim);
		stream << radius << std::endl;
		canvas.draw_circle(radius, dim / 2, dim / 2);
		canvas.print();
	}
}
