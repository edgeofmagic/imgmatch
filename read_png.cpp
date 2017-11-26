/*
 * Copyright 2017 David Curtis
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy 
 * of this software and associated documentation files (the "Software"), to 
 * deal in the Software without restriction, including without limitation the 
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or 
 * sell copies of the Software, and to permit persons to whom the Software is 
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in 
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, 
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN 
 * THE SOFTWARE.
 */

#include "read_png.h"
#include "lodepng.h"
#include "bitmap_image.hpp"

bool read_png_file (std::string const& filename, bitmap_image& image)
{
	constexpr unsigned pixel_size = 4;
	
	std::vector<unsigned char> pixels; //the raw pixels
	unsigned width, height;

	//decode
	unsigned error = lodepng::decode(pixels, width, height, filename.c_str());

	//if there's an error, display it
	if(error) 
	{
		std::cout << "PNG decoder error " << error << ": " << lodepng_error_text(error) << std::endl;
		return false;
	}

	//the pixels are now in the vector "image", 4 bytes per pixel, ordered RGBARGBA..., use it as texture, draw it, ...

	image.setwidth_height(width, height);

	unsigned char *p = pixels.data();
	
	for (auto iy = 0u; iy < height; ++iy)
	{
		for (auto ix = 0u; ix < width; ++ix)
		{
			unsigned char red = *p++;
			unsigned char green = *p++;
			unsigned char blue = *p++;
			image.set_pixel(ix, iy, red, green, blue);
			++p; // skip alpha channel byte
		}
	}
	return true;
}
