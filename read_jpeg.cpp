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

#include <cstdint>
#include <iostream>
#include <cstdio>
#include <csetjmp>
#include <string>
#include <assert.h>
#include <jpeglib.h>
#include "bitmap_image.hpp"

void put_rgb_scanline_in_image(JSAMPROW row_buffer, std::size_t row, std::size_t width, bitmap_image& image)
{
	for (std::size_t i = 0; i < width; ++i)
	{
		std::uint8_t red = row_buffer[(i * 3)];
		std::uint8_t green = row_buffer[(i * 3) + 1];
		std::uint8_t blue = row_buffer[(i * 3) + 2];
		image.set_pixel(i, row, red, green, blue);
	}
}

void put_gs_scanline_in_image(JSAMPROW row_buffer, std::size_t row, std::size_t width, bitmap_image& image)
{
	for (std::size_t i = 0; i < width; ++i)
	{
		std::uint8_t value = row_buffer[i];
		image.set_pixel(i, row, value, value, value);
	}
}

std::string color_space_name(J_COLOR_SPACE cs)
{
	switch (cs)
	{
	case J_COLOR_SPACE::JCS_GRAYSCALE :
		return std::string("grayscale");
	case J_COLOR_SPACE::JCS_RGB :
		return std::string("RGB/sRGB");
	case J_COLOR_SPACE::JCS_YCbCr :
		return std::string("YUV/sYCC");
	case J_COLOR_SPACE::JCS_CMYK :
		return std::string("CMYK");
	case J_COLOR_SPACE::JCS_YCCK :
		return std::string("YCCK");
	case J_COLOR_SPACE::JCS_BG_RGB :
		return std::string("big gamut RGB/bg-sRGB");
	case J_COLOR_SPACE::JCS_BG_YCC :
		return std::string("big gamut YCC/bg-sYCC");
	default:
		return "unknown color space value";
	};
}

/*
 *	The following was taken nearly verbatim from example.c in libjpeg
 */

struct my_error_mgr
{
	struct jpeg_error_mgr pub;
	jmp_buf setjmp_buffer;
};

typedef struct my_error_mgr * my_error_ptr;

METHODDEF(void)
my_error_exit(j_common_ptr cinfo)
{
	my_error_ptr myerr = (my_error_ptr) cinfo->err;
	(*cinfo->err->output_message) (cinfo);
	longjmp(myerr->setjmp_buffer, 1);
}

bool
read_jpeg_file(std::string const& filename, bitmap_image& image)
{
	struct jpeg_decompress_struct cinfo;

	struct my_error_mgr jerr;

	FILE * infile; /* source file */
	JSAMPARRAY buffer; /* Output row buffer */
	int row_stride; /* physical row width in output buffer */

	if ((infile = fopen(filename.c_str(), "rb")) == NULL)
	{
		fprintf(stderr, "can't open %s\n", filename.c_str());
		return false;
	}

	cinfo.err = jpeg_std_error(&jerr.pub);
	jerr.pub.error_exit = my_error_exit;

	if (setjmp(jerr.setjmp_buffer))
	{
		std::cerr << "in error handler" << std::endl;
		jpeg_destroy_decompress(&cinfo);
		fclose(infile);
		return false;
	}

	jpeg_create_decompress(&cinfo);
	jpeg_stdio_src(&cinfo, infile);
	(void) jpeg_read_header(&cinfo, TRUE);
	(void) jpeg_start_decompress(&cinfo);
	if (cinfo.out_color_space != J_COLOR_SPACE::JCS_RGB && cinfo.out_color_space != J_COLOR_SPACE::JCS_GRAYSCALE)
	{
		std::cout << "unexpected color space: " << color_space_name(cinfo.out_color_space) << std::endl;
		return false;
	}
	row_stride = cinfo.output_width * cinfo.output_components;
	buffer = (*cinfo.mem->alloc_sarray) ((j_common_ptr) & cinfo, JPOOL_IMAGE, row_stride, 1);
	image.setwidth_height(cinfo.output_width, cinfo.output_height);
	std::size_t nrow = 0ul;

	while (cinfo.output_scanline < cinfo.output_height)
	{
		(void) jpeg_read_scanlines(&cinfo, buffer, 1);
		if (cinfo.out_color_space == J_COLOR_SPACE::JCS_RGB)
		{
			put_rgb_scanline_in_image(buffer[0], nrow++, cinfo.output_width, image);
		}
		else
		{
			put_gs_scanline_in_image(buffer[0], nrow++, cinfo.output_width, image);
		}
	}

	(void) jpeg_finish_decompress(&cinfo);
	jpeg_destroy_decompress(&cinfo);
	fclose(infile);
	return true;
}

