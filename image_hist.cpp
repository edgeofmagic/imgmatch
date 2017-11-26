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

#include "image_hist.h"
#include "bitmap_image.hpp"

rgb_image_hist::rgb_image_hist()
:
pixel_count_{0}, bins_({0u})
{
}

rgb_image_hist::rgb_image_hist(bitmap_image const& image)
:
pixel_count_{image.width() * image.height()}, bins_({0u})
{
//	for (auto& b : bins_)
//	{
//		b = 0u;
//	}

	for (auto ix = 0u; ix < image.width(); ++ix)
	{
		for (auto iy = 0u; iy < image.height(); ++iy)
		{
			rgb_value pix;
			image.get_pixel(ix, iy, pix.red, pix.green, pix.blue);
			++bin(pix);
		}
	}
}

double
rgb_image_hist::chi_sqr_dist(rgb_image_hist const& other) const
{
	// ignore null-contructed histograms
	if (pixel_count_ == 0 || other.pixel_count_ == 0)
	{
		return 0.0;
	}

	double sum = 0.0;

	for (auto i = 0ul; i < bin_count; ++i)
	{
		double norm = static_cast<double> (bin(i)) / pixel_count();
		double other_norm = static_cast<double> (other.bin(i)) / other.pixel_count();
		double diff = norm - other_norm;
		double avg = (norm + other_norm) / 2.0;

		if (avg != 0.0)
		{
			sum += (diff * diff) / avg;
		}
	}

	return sum;
}
