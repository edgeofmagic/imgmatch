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

#ifndef IMAGE_HIST_H
#define IMAGE_HIST_H

#include <cstdint>
#include <array>

class bitmap_image;

struct rgb_value
{
	rgb_value(std::uint8_t r, std::uint8_t g, std::uint8_t b)
	: red{r}, green{g}, blue{b}
	{
	}
	rgb_value()
	: red{0}, green{0}, blue{0}
	{
	}
	std::uint8_t red;
	std::uint8_t green;
	std::uint8_t blue;
};

class rgb_image_hist
{
public:
	
	rgb_image_hist(bitmap_image const& image);

	rgb_image_hist();

	double chi_sqr_dist(rgb_image_hist const& other) const;
	
	inline bool
	is_valid() const
	{
		return pixel_count_ != 0ul;
	}

	inline std::size_t
	pixel_count() const
	{
		return pixel_count_;
	}
	
	inline std::uint32_t& operator[](rgb_value const& pixel)
	{
		return bin(bin_index(pixel));
	}

	inline std::uint32_t const& operator[](rgb_value const& pixel) const
	{
		return bin(bin_index(pixel));
	}
	
	inline std::uint32_t const& operator[](std::size_t index) const
	{
		return bin(index);
	}

	inline std::uint32_t& operator[](std::size_t index)
	{
		return bin(index);
	}

protected:
	static constexpr std::size_t channel_depth = 256;
	static constexpr std::size_t bins_on_axis = 16;
	static constexpr std::size_t axis_shift = 4; // log2(bins_on_axis)
	static constexpr std::size_t bin_count =
			bins_on_axis * bins_on_axis * bins_on_axis;
	static constexpr std::size_t bin_width = channel_depth / bins_on_axis;
	static constexpr std::size_t bin_index_mask = bin_width - 1;
	static constexpr std::size_t channel_to_bin_index_shift = 4; // log2(bin_width)
	
	inline std::uint32_t&
	bin(std::size_t index)
	{
		return bins_[index];
	}

	inline std::uint32_t const&
	bin(std::size_t index) const
	{
		return bins_[index];
	}

	inline std::uint32_t&
	bin(std::uint8_t red, std::uint8_t green, std::uint8_t blue)
	{
		return bin(bin_index(red, green, blue));
	}

	inline std::uint32_t&
	bin(rgb_value const& pixel)
	{
		return bin(pixel.red, pixel.green, pixel.blue);
	}
	
	inline std::uint32_t const&
	bin(rgb_value const& pixel) const
	{
		return bin(pixel.red, pixel.green, pixel.blue);
	}
	
	inline std::uint32_t const&
	bin(std::uint8_t red, std::uint8_t green, std::uint8_t blue) const
	{
		return bin(bin_index(red, green, blue));
	}

	inline std::size_t
	bin_index(std::uint8_t red, std::uint8_t green, std::uint8_t blue) const
	{
		std::size_t result = (red >> channel_to_bin_index_shift);
		result <<= axis_shift;
		result |= (green >> channel_to_bin_index_shift);
		result <<= axis_shift;
		result |= (blue >> channel_to_bin_index_shift);
		return result;
	}
	
	inline std::size_t
	bin_index(rgb_value const& pixel) const
	{
		return bin_index(pixel.red, pixel.green, pixel.blue);
	}

	std::array<std::uint32_t, bin_count> bins_;
	std::size_t pixel_count_;
};

#endif /* IMAGE_HIST_H */

