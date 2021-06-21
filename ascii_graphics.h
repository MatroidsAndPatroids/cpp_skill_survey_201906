/*
 * Simple ASCII graphics class.
 *
 * Draws circles and arbitrary text to a printable canvas.
 *
 * Based on the short article:
 * https://hoven.in/cpp-graphics/c-program-draw-a-circle.html
 */

#pragma once
#include <iostream>
#include <ostream>
#include <vector>
#include <string>

class Ascii_graphics {
	std::vector<std::string> canvas;
	
public:
	// Create a canvas of the given size
	Ascii_graphics(int rows, int cols);
	
	// Draw a circle with given radius and origin coordinates
	void draw_circle(int radius, int o_x, int o_y);
	
	// Draw given text starting at the given coordinates
	void draw_text(std::string text, int o_x, int o_y);
	
	// Print the canvas to the given stream
	void print(std::ostream &stream = std::cout) const;
};
