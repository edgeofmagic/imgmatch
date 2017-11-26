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

#ifndef IMAGE_MATCHER_H
#define IMAGE_MATCHER_H

#define BOOST_FILESYSTEM_NO_DEPRECATED

#include <vector>
#include <string>
#include <unordered_map>
#include <algorithm>
#include "boost/filesystem/operations.hpp"
#include "boost/filesystem/path.hpp"
#include <boost/functional/hash.hpp>
#include "image_hist.h"

namespace fs = boost::filesystem;

class image_matcher
{
public:

	using string_vec = std::vector<std::string>;
	
	static constexpr double
	default_match_threshold()
	{
		return 0.1;
	}

	static std::string const&
	default_match_threshold_display()
	{
		static const std::string str("0.1");
		return str;
	}

	static constexpr int
	default_limit()
	{
		return 1000;
	}

	using match_result = std::pair<fs::path, double>;
	using match_result_set = 
		std::unordered_multimap<fs::path, match_result, 
								boost::hash<boost::filesystem::path>>;

	using path_vector = std::vector<fs::path>;
	using hist_vector = std::vector<rgb_image_hist>;

	struct image_dir
	{
		image_dir(fs::path const& path)
		: dir_path{path}
		{
		}
		fs::path dir_path;
		path_vector image_paths;
		hist_vector image_histograms;
	};

	/*
	 *	Initial values will all be set from command-line options.
	 */
	inline
	image_matcher()
	:
	match_threshold_{0.0},
	limit_{0},
	verbose_{0},
	results_path_{},
	target_path_{},
	use_target_{false},
	target_is_dir_{false},
	search_paths_{},
	annotate_links_{false}
	{
	}

	void set_annotate_links(bool value);
	
	void set_match_threshold(double match_threshold);

	void set_verbose(int verbose);

	bool set_target(std::string const& target_string);

	inline double
	match_threshold() const
	{
		return match_threshold_;
	}

	bool set_search_paths(string_vec const& search_path_strings);

	void set_limit(int limit);

	bool set_results_path(std::string const& results_path_string);

	void show_options() const;
	
	inline int
	verbose() const
	{
		return verbose_;
	}

	void execute() const;

	bool split_filename(std::string const& fname,
						std::string& base,
						std::string& suffix) const;

	inline std::string 
	filename_suffix(std::string const& fname) const
	{
		auto pos = fname.find_last_of('.');
		if (pos != std::string::npos)
		{
			return fname.substr(pos);
		}
		else
		{
			return std::string{};
		}
	}

	inline std::string 
	filename_suffix(fs::path const& fpath) const
	{
		return filename_suffix(fpath.filename().string());
	}

	bool
	is_image_file(fs::path const& fpath) const
	{
		std::string suffix = filename_suffix(fpath);
		return is_jpeg_suffix(suffix)
				|| is_png_suffix(suffix)
				|| is_bmp_suffix(suffix);
	}
	
	inline bool 
	is_jpeg_suffix(std::string const& suffix) const
	{
		return std::find(jpeg_suffixes.begin(), 
						 jpeg_suffixes.end(), 
						 suffix) != jpeg_suffixes.end();
	}

	inline bool
	is_png_suffix(std::string const& suffix) const
	{
		return std::find(png_suffixes.begin(), 
						 png_suffixes.end(), 
						 suffix) != png_suffixes.end();
	}

	bool
	is_bmp_suffix(std::string const& suffix) const
	{
		return std::find(bmp_suffixes.begin(), 
						 bmp_suffixes.end(), 
						 suffix) != bmp_suffixes.end();
	}
	
	void initialize(image_dir& dir) const;

	bool read_image_file(fs::path const& fpath, bitmap_image& image) const;
	
	void find_matches(fs::path const& target, 
					  rgb_image_hist const& target_hist,
					  image_dir const& search, 
					  match_result_set& results) const;

	void find_matches(image_dir const& search, match_result_set& results) const;

	void find_images(image_dir& idir) const;

	void build_histograms(image_dir& idir) const;

	void generate_symlinks(match_result_set const& results) const;

	bool create_dir(fs::path const& dir_path) const;

	bool create_symlink(fs::path const& link_target,
						std::string const& link_name,
						fs::path const& link_dir) const;

	fs::path find_available_name(fs::path const& results_parent,
								 fs::path const& dir_filename) const;

	match_result_set::const_iterator
	create_matching_links(match_result_set::const_iterator it,
						  match_result_set::const_iterator end_it,
						  fs::path const& results_path,
						  std::size_t set_count) const;

	static const std::vector<std::string> jpeg_suffixes;

	static const std::vector<std::string> png_suffixes;

	static const std::vector<std::string> bmp_suffixes;

private:

	double match_threshold_;
	int limit_;
	int verbose_;
	fs::path results_path_;
	fs::path target_path_;
	bool use_target_;
	bool target_is_dir_;
	std::vector<fs::path> search_paths_;
	bool annotate_links_;

};

#endif /* IMAGE_MATCHER_H */

