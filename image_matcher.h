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
#include <unordered_set>
#include <set>
#include <algorithm>
#include "boost/filesystem/operations.hpp"
#include "boost/filesystem/path.hpp"
#include <boost/functional/hash.hpp>
#include "image_hist.h"

namespace fs = boost::filesystem;

class image_matcher
{
public:

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
	annotate_links_{false},
	exhaustive_{false}
	{
	}

	void set_annotate_links(bool value);
	
	void set_match_threshold(double match_threshold);

	void set_verbose(int verbose);

	bool set_target(std::string const& target_string);

	void set_exhaustive(bool value);
	
	using string_vec = std::vector<std::string>;

	bool set_search_paths(string_vec const& search_path_strings);

	void set_limit(int limit);

	bool set_results_path(std::string const& results_path_string);

	void show_options() const;

	inline int
	verbose() const
	{
		return verbose_;
	}

	inline double
	match_threshold() const
	{
		return match_threshold_;
	}

	inline int
	limit() const
	{
		return limit_;
	}

	inline fs::path const&
	results_path() const
	{
		return results_path_;
	}

	inline fs::path const&
	target_path() const
	{
		return target_path_;
	}

	inline bool
	use_target() const
	{
		return use_target_;
	}

	inline bool
	is_target_dir() const
	{
		return target_is_dir_;
	}

	inline std::vector<fs::path> const&
	search_paths() const
	{
		return search_paths_;
	}

	inline bool
	exhaustive() const
	{
		return exhaustive_;
	}

	void execute();

private:
	
	using path_ptr = std::shared_ptr<fs::path>;
	using path_ptr_vec = std::vector<path_ptr>;
	
	struct path_ptr_equals
	{
		bool operator()(path_ptr const& a, path_ptr const& b) const
		{
			return *a == *b;
		}
	};
	
	struct path_ptr_hash
	{
		std::size_t operator()(path_ptr const& p) const
		{
			return boost::filesystem::hash_value(*p);
		}
	};
	
	struct path_ptr_less
	{
		bool operator()(path_ptr const& a, path_ptr const& b) const
		{
			return a->compare(*b) < 0;
		}
	};
	
	using path_set = std::unordered_set<path_ptr, path_ptr_hash, path_ptr_equals>;
	using path_hist_map = std::unordered_map<path_ptr, rgb_image_hist>;
	using match_set = std::set<path_ptr, path_ptr_less>;
	using match_set_ptr = std::shared_ptr<match_set>;
	using match_set_map = std::unordered_map<path_ptr, match_set_ptr>;
	using match_set_set = std::unordered_set<match_set_ptr>;


	inline rgb_image_hist const& histogram_at(path_hist_map::const_iterator it) const
	{
		return it->second;
	}
	
	inline match_set_ptr match_set_at(match_set_map::iterator it) const
	{
		return it->second;
	}
	
	inline path_ptr path_at(match_set_map::const_iterator it) const
	{
		return it->first;
	}
	
	inline path_ptr path_at(path_hist_map::const_iterator it) const
	{
		return it->first;
	}
	
	inline bool is_end(match_set_map::iterator it) 
	{
		return it == match_set_map_.end();
	}
	

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
	
	bool read_image_file(fs::path const& fpath, bitmap_image& image) const;

	void compare(path_hist_map::const_iterator a, path_hist_map::const_iterator b);
	
	void generate_symlinks(path_hist_map const& hist_map) const;	
	
	bool create_dir(fs::path const& dir_path) const;

	bool create_symlink(fs::path const& link_target,
						std::string const& link_name,
						fs::path const& link_dir) const;

	fs::path find_available_name(fs::path const& results_parent,
								 fs::path const& dir_filename) const;

	void create_matching_links(std::vector<path_ptr> const& paths, 
							   std::vector<double> const& distances, 
							   std::size_t set_index) const;

	static const std::vector<std::string> jpeg_suffixes;

	static const std::vector<std::string> png_suffixes;

	static const std::vector<std::string> bmp_suffixes;

	void build_histograms(fs::path const& dir, path_hist_map& hmap);
	
	void build_histogram(path_ptr p, path_hist_map& hmap);

	void find_matches(path_hist_map const& hmap);
	
	void find_matches(path_hist_map const& target_hmap, path_hist_map const& search_hmap);

	double match_threshold_;
	int limit_;
	int verbose_;
	fs::path results_path_;
	fs::path target_path_;
	bool use_target_;
	bool target_is_dir_;
	std::vector<fs::path> search_paths_;
	bool annotate_links_;
	bool exhaustive_;
	
	path_set unique_paths_;
	match_set_map match_set_map_;
	match_set_set match_sets_;

};

#endif /* IMAGE_MATCHER_H */

