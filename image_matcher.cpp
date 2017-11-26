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

#include <unordered_set>
#include <iostream>
#include "read_jpeg.h"
#include "read_png.h"
#include "read_bmp.h"
#include "image_matcher.h"
#include "bitmap_image.hpp"

const std::vector<std::string> image_matcher::jpeg_suffixes
{
	".jpg",
	".jpeg",
	".JPG",
	".JPEG"
};

const std::vector<std::string> image_matcher::png_suffixes
{
	".png",
	".PNG"
};

const std::vector<std::string> image_matcher::bmp_suffixes
{
	".bmp",
	".BMP"
};

bool image_matcher::split_filename(std::string const& fname, std::string& base, std::string& suffix) const
{
	auto pos = fname.find_last_of('.');
	if (pos != std::string::npos)
	{
		base = fname.substr(0, pos);
		suffix = fname.substr(pos);
		return true;
	}
	else
	{
		return false;
	}
}

void
image_matcher::find_matches(fs::path const& target_path,
							rgb_image_hist const& target_hist,
							image_dir const& search_dir,
							match_result_set& matches) const
{
	if (verbose_ > 0)
	{
		std::cout << std::endl << "searching for images matching "
				<< target_path << " in " << search_dir.dir_path
				<< ":" << std::endl;
	}

	for (auto i = 0ul; i < search_dir.image_paths.size(); ++i)
	{
		if (search_dir.image_histograms[i].is_valid())
		{
			auto& candidate_path = search_dir.image_paths[i];
			if (!fs::equivalent(target_path, candidate_path))
			{
				auto distance = target_hist.chi_sqr_dist(search_dir.image_histograms[i]);
				if (verbose_ > 1)
				{
					std::cout << "compared " << target_path << " with "
							<< candidate_path << ": " << distance << std::endl;
				}
				if (distance <= match_threshold_)
				{
					matches.emplace(target_path,
									match_result{candidate_path, distance});
					if (verbose_ > 0)
					{
						std::cout << "found match -- " << target_path << " and "
								<< candidate_path << ": " << distance << std::endl;
					}
				}
			}
		}
	}
}

void
image_matcher::find_matches(image_dir const& search_dir,
							match_result_set& results) const
{
	// keep track of entries that have matched to avoid redundant comparisons
	std::unordered_set<std::size_t> matched_items;

	if (verbose_ > 0)
	{
		std::cout << "finding matches within: "
				<< search_dir.dir_path << std::endl;
	}

	auto& histograms = search_dir.image_histograms;
	auto& image_paths = search_dir.image_paths;

	for (auto i = 0ul; i < histograms.size(); ++i)
	{
		if (histograms[i].is_valid() && matched_items.count(i) == 0)
		{
			for (auto j = i + 1; j < histograms.size(); ++j)
			{
				if (histograms[j].is_valid() && matched_items.count(j) == 0)
				{
					auto distance = histograms[i].chi_sqr_dist(histograms[j]);
					if (verbose_ > 1)
					{
						std::cout << "compared " << image_paths[i].filename()
								<< " with " << image_paths[j].filename()
								<< ": " << distance << std::endl;
					}
					if (distance <= match_threshold_)
					{
						results.emplace(image_paths[i],
										match_result{image_paths[j], distance});
						if (verbose_ > 0)
						{
							std::cout << "found match -- "
									<< image_paths[i].filename()
									<< " and " << image_paths[j].filename()
									<< ": " << distance << std::endl;
						}

						matched_items.insert(i);
						matched_items.insert(j);
					}
				}
			}
		}
	}
}

void
image_matcher::initialize(image_dir& dir) const
{
	find_images(dir);
	build_histograms(dir);
}

void
image_matcher::find_images(image_dir& dir) const
{
	dir.image_paths.clear();

	fs::directory_iterator end_iter;

	if (verbose_ > 0)
	{
		std::cout << "searching for image files in "
				<< dir.dir_path << ":" << std::endl;
	}

	// first, count them so we can reserve the vector 
	// (to avoid expensive resizing)

	std::size_t count = 0ul;

	for (fs::directory_iterator dir_itr(dir.dir_path);
		dir_itr != end_iter;
		++dir_itr)
	{
		try
		{
			fs::path canonical_path(fs::canonical(dir_itr->path()));
			if (fs::is_regular_file(canonical_path)
				&& (is_image_file(canonical_path)))
			{
				++count;
			}
		}
		catch (const std::exception & ex)
		{
			std::cerr << "'" << dir_itr->path().filename() << "' "
					<< ex.what() << std::endl;
		}
	}

	if (limit_ > 0)
	{
		count = limit_;
	}
	
	dir.image_paths.reserve(count);

	count = 0;
	
	for (fs::directory_iterator dir_itr(dir.dir_path);
		dir_itr != end_iter;
		++dir_itr)
	{
		if (limit_ < 0 || count < limit_)
		{
			try
			{
				fs::path canonical_path(fs::canonical(dir_itr->path()));
				if (fs::is_regular_file(canonical_path)
					&& (is_image_file(canonical_path)))
				{
					if (verbose_ > 1)
					{
						std::cout << "found image file: "
								<< canonical_path.filename() << std::endl;
					}
					dir.image_paths.emplace_back(canonical_path);
					++count;
				}
			}
			catch (const std::exception & ex)
			{
				std::cerr << "'" << dir_itr->path().filename()
						<< "' " << ex.what() << std::endl;
			}
		}
		else
		{
			std::cout << "comparison limit reached in directory " << dir.dir_path << std::endl;
			break;
		}
	}
}

bool
image_matcher::read_image_file(fs::path const& image_path, bitmap_image& image) const
{
	std::string suffix = this->filename_suffix(image_path);
	bool result = true;

	if (is_jpeg_suffix(suffix))
	{
		if (!read_jpeg_file(image_path.string(), image))
		{
			if (verbose_ == 1) std::cout << std::endl;
			std::cerr << "could not read " << image_path
					<< " as a jpeg image" << std::endl;
			result = false;
		}
	}
	else if (is_png_suffix(suffix))
	{
		if (!read_png_file(image_path.string(), image))
		{
			if (verbose_ == 1) std::cout << std::endl;
			std::cerr << "could not read " << image_path
					<< " as a png image" << std::endl;
			result = false;
		}
	}
	else if (is_bmp_suffix(suffix))
	{
		if (!read_bmp_file(image_path.string(), image))
		{
			if (verbose_ == 1) std::cout << std::endl;
			std::cerr << "could not read " << image_path
					<< " as a bmp image" << std::endl;
			result = false;
		}
	}
	else
	{
		if (verbose_ == 1) std::cout << std::endl;
		std::cerr << "unrecognized file extension: "
				<< image_path << std::endl;
		result = false;
	}

	return result;
}

void
image_matcher::build_histograms(image_dir& dir) const
{
	if (verbose_ > 0)
	{
		std::cout << "building histograms for "
				<< dir.image_paths.size() << " images in " << dir.dir_path
				<< ":" << std::endl;
	}

	// avoid reallocation and copying
	dir.image_histograms.reserve(dir.image_paths.size());

	for (auto& image_path : dir.image_paths)
	{
		if (verbose_ > 1)
		{
			std::cout << "building histogram for "
					<< image_path.filename() << std::endl;
		}
		else if (verbose_ == 1)
		{
			std::cout << ".";
			std::cout.flush();
		}
		bitmap_image img;
		
		if (!read_image_file(image_path, img))
		{
			// append a null-constructed histogram, 
			// to keep histograms and paths aligned

			dir.image_histograms.emplace_back();
		}
		else
		{
			dir.image_histograms.emplace_back(img);
		}
	}
	if (verbose_ == 1)
	{
		std::cout << std::endl;
	}
}

void 
image_matcher::set_annotate_links(bool value)
{
	annotate_links_ = value;
}

bool
image_matcher::set_target(std::string const& target_string)
{
	fs::path target_path = fs::system_complete(fs::path(target_string));

	if (!fs::exists(target_path))
	{
		std::cerr << "target " << target_string << " not found" << std::endl;
		return false;
	}

	target_path = fs::canonical(target_path);

	if (fs::is_directory(target_path))
	{
		target_is_dir_ = true;
		if (verbose_ > 0)
		{
			std::cout << "target " << target_string << " is a directory"
					<< std::endl;
		}
	}
	else
	{
		if (!fs::is_regular_file(target_path))
		{
			std::cerr << "target file " << target_path
					<< " is not a regular file" << std::endl;
			return false;
		}

		if (!is_image_file(target_path.filename().string()))
		{
			std::cerr << "target file " << target_path
					<< " is not an image file" << std::endl;
			return false;
		}
	}

	target_path_ = target_path;
	use_target_ = true;
	return true;
}

bool
image_matcher::set_results_path(std::string const& results_dir_string)
{
	fs::path 
	results_parent(fs::system_complete(results_dir_string).parent_path());

	fs::path 
	results_dir_filename(fs::system_complete(results_dir_string).filename());

	if (!fs::exists(results_parent))
	{
		std::cout << "parent of results directory (" << results_dir_string
				<< ") doesn't exist" << std::endl;
		return false;
	}

	if (!fs::is_directory(results_parent))
	{
		std::cout << "parent of results directory (" << results_dir_string
				<< ") isn't a directory" << std::endl;
		return false;
	}

	fs::path results_path(fs::canonical(results_parent));
	results_path /= results_dir_filename;

	if (fs::exists(results_path))
	{
		// directory results_dirname_ already exisits, use it if empty
		if (!fs::is_empty(results_path))
		{
			results_path = find_available_name(results_parent,
											results_dir_filename);
		}
	}
	results_path_ = results_path;
	return true;
}

bool
image_matcher::set_search_paths(string_vec const& search_dir_strings)
{
	search_paths_.reserve(search_dir_strings.size());

	for (auto it = search_dir_strings.begin();
		it != search_dir_strings.end();
		++it)
	{

		fs::path dir_path(fs::system_complete(fs::path(*it)));

		if (!fs::exists(dir_path))
		{
			std::cerr << *it << " not found" << std::endl;
			return false;
		}
		if (!fs::is_directory(dir_path))
		{
			std::cerr << *it << " is not a directory" << std::endl;
			return false;
		}

		search_paths_.emplace_back(fs::canonical(dir_path));
	}
	if (search_paths_.size() < 1)
	{
		std::cerr << "no valid search directories were specified" << std::endl;
		return false;
	}
	return true;
}

void
image_matcher::execute() const
{
	/*
	 * preconditions:
	 *      if use_target_ is true then
	 *          target_path_ exists
	 *          if target_path_ is a directory then
	 *              target_is_dir_ is true
	 *          otherwise, target_path_ appears to be a image file
	 *      search_dir_paths_.size() > 0
	 *      for each element of search_dir_paths_
	 *          element exists
	 *          element is a directory
	 *      either:
	 *          results_dir_path_ exists and is a directory
	 *      or
	 *          results_dir_path_ does not exist, but parent is a directory
	 * 
	 *      match_threshold_ is set to a legitimate
	 *      limit_ is either -1 (no limit) or a value > 0
	 * 
	 *      all paths are canonical (except for results_dir_path_, 
	 *		if it doesn't exist)
	 *       
	 */

	match_result_set matches;

	if (use_target_)
	{
		if (target_is_dir_)
		{
			/*
			 * match all of the images in the target directory against 
			 * all of the images in search directories
			 */

			if (verbose_ > 0)
			{
				std::cout << "target " << target_path_ << " is a directory"
						<< std::endl;
			}

			image_dir target_dir{target_path_};

			initialize(target_dir);

			if (target_dir.image_histograms.size() < 1)
			{
				std::cout << "target directory " << target_path_
						<< " contains no image files" << std::endl;
				return;
			}

			for (auto it = search_paths_.begin();
				it != search_paths_.end();
				++it)
			{
				if (!fs::equivalent(target_path_, *it))
				{
					image_dir search_dir{*it};
					initialize(search_dir);

					if (search_dir.image_paths.size() < 1)
					{
						if (verbose_ > 0)
						{
							std::cout << "search directory " << *it
									<< " is empty" << std::endl;
						}
					}
					else
					{
						for (auto i = 0ul;
							 i < target_dir.image_histograms.size();
							 ++i)
						{
							auto& target_path = target_dir.image_paths[i];
							auto& target_hist = target_dir.image_histograms[i];
							if (target_hist.is_valid())
							{
								find_matches(target_path,
											 target_hist,
											 search_dir,
											 matches);
							}
						}
					}
				}
			}
		}
		else
		{
			/*
			 * if the target is an image file, match it against 
			 * all of the images in search directories
			 */
			bitmap_image target_image;
			if (!read_image_file(target_path_, target_image))
			{
				std::cerr << "could not read " << target_path_
						<< " as an image" << std::endl;
				return;
			}

			rgb_image_hist target_hist{target_image};

			for (auto it = search_paths_.begin();
				it != search_paths_.end();
				++it)
			{
				image_dir search_dir{*it};
				initialize(search_dir);
				find_matches(target_path_, target_hist, search_dir, matches);
			}
		}
	}
	else
	{
		/*
		 * no target -- look for matches within each of the search directories;
		 * don't match between directories
		 */

		for (auto it = search_paths_.begin();
			it != search_paths_.end();
			++it)
		{
			image_dir search_dir{*it};
			initialize(search_dir);
			find_matches(search_dir, matches);
		}
	}

	generate_symlinks(matches);
}

bool
is_acceptable(fs::path const& results_path)
{
	if (fs::exists(results_path))
	{
		if (fs::is_directory(results_path))
		{
			if (fs::is_empty(results_path))
			{
				return true;
			}
		}
	}
	return false;
}

fs::path
image_matcher::find_available_name(fs::path const& results_parent,
								   fs::path const& dir_filename) const
{
	fs::path dir_path(results_parent);
	dir_path /= dir_filename;
	if (!fs::exists(dir_path))
	{
		return dir_path;
	}

	std::size_t count = 0ul;

	while (true)
	{
		std::string possible(dir_filename.string());
		possible.append(std::to_string(count));
		dir_path = results_parent;
		dir_path /= possible;
		if (!fs::exists(dir_path))
		{
			return dir_path;
		}
		count++;
	}
}

bool
image_matcher::create_symlink(fs::path const& link_target,
							  std::string const& link_name,
							  fs::path const& link_parent_dir_path) const
{
	// link names must be unique        
	std::size_t dup_count = 0;

	fs::path link_path(link_parent_dir_path);
	//    std::string link_name(link_target.filename().string());
	link_path /= link_name;
	while (fs::exists(link_path))
	{
		std::string dup_link_name("dup");
		dup_link_name.append(std::to_string(dup_count++))
				.append("_")
				.append(link_name);
		link_path = link_parent_dir_path;
		link_path /= dup_link_name;
	}
	boost::system::error_code ec;
	fs::create_symlink(link_target, link_path, ec);
	if (!ec)
	{
		return true;
	}
	else
	{
		std::cout << "could not create symbolic link to " << link_target
				<< ", error: " << ec.message() << std::endl;
		return false;
	}
}

bool
image_matcher::create_dir(fs::path const& dir_path) const
{
	bool result = false;
	if (!fs::exists(dir_path))
	{
		boost::system::error_code ec;
		fs::create_directory(dir_path, ec);
		if (!ec)
		{
			result = true;
		}
		else
		{
			std::cout << "couldn't create directory " << dir_path 
					<< ", error: " << ec.message() << std::endl;
		}
	}
	else
	{
		std::cout << "could not create " << dir_path << ", already exists"
				<< std::endl;
	}
	return result;
}

void
image_matcher::generate_symlinks(match_result_set const& results) const
{
	if (results.size() > 0)
	{
		if (!fs::exists(results_path_))
		{
			if (!create_dir(results_path_)) return;
		}

		bool assertion = fs::exists(results_path_)
				&& fs::is_directory(results_path_)
				&& fs::is_empty(results_path_);

		if (!assertion)
		{
			std::cout << "results_path assertion failed" << std::endl;
			return;
		}

		std::size_t match_count = 0;

		auto it = results.begin();
		auto end_matches = results.end();

		while (it != end_matches)
		{
			it = create_matching_links(it,
									end_matches,
									results_path_,
									match_count++);
		}
		std::cout << match_count << " match sets were found, containing " 
				<< results.size() + match_count << " matching files" 
				<< std::endl;
	}
	else
	{
		std::cout << "no matches found" << std::endl;
	}
}

image_matcher::match_result_set::const_iterator
image_matcher::create_matching_links(match_result_set::const_iterator it,
									 match_result_set::const_iterator end_it,
									 fs::path const& results_path,
									 std::size_t set_count) const
{
	fs::path match_dir_path(results_path);
	std::string match_dir_name{"m"};
	match_dir_name.append(std::to_string(set_count));
	match_dir_path /= match_dir_name;
	if (!create_dir(match_dir_path)) return end_it;

	fs::path key_path(it->first);
	fs::path match_path(it->second.first);
	double distance = it->second.second;

	std::string link_base;
	std::string link_suffix;
	split_filename(key_path.filename().string(), link_base, link_suffix);
	if (annotate_links_)
	{
		link_base.append("_ref");
	}
	std::string ref_link_name(link_base);
	ref_link_name.append(link_suffix);

	if (!create_symlink(key_path, ref_link_name, match_dir_path))
		return end_it;

	split_filename(match_path.filename().string(), link_base, link_suffix);
	if (annotate_links_)
	{
		link_base.append("_").append(std::to_string(distance));
	}
	std::string match_link_name(link_base);
	match_link_name.append(link_suffix);

	if (!create_symlink(match_path, match_link_name, match_dir_path))
		return end_it;
	++it;

	while (it != end_it && it->first == key_path)
	{
		match_path = it->second.first;
		distance = it->second.second;
		match_link_name = match_path.filename().string();
		match_link_name.append("_").append(std::to_string(distance));
		if (!create_symlink(match_path, match_link_name, match_dir_path))
			return end_it;
		++it;
	}

	return it;
}

void
image_matcher::show_options() const
{
	static const std::string indent("    ");
	
	std::cout << "configuration:" << std::endl;

	std::cout << indent << "match threshold: " << match_threshold_ << std::endl;
	if (use_target_)
	{
		std::cout << indent << "target path: " << target_path_ << std::endl;
	}
	else
	{
		std::cout << indent << "target path: none" << std::endl;
	}

	for (auto dir_path : search_paths_)
	{
		std::cout << indent << "search path: " << dir_path << std::endl;
	}

	std::cout << indent << "results path: " << results_path_ << std::endl;

	if (limit_ < 0)
	{
		std::cout << indent << "limit: none" << std::endl;
	}
	else
	{
		std::cout << indent << "limit: " << limit_ << std::endl;
	}
	
	std::cout << "link annotation is " << ( annotate_links_ ? "on" : "off") 
			<< std::endl;
}

void
image_matcher::set_match_threshold(double match_threshold)
{
	if (match_threshold < 0.0)
	{
		std::cout << "invalid match threshold value, using default: "
				<< default_match_threshold_display()
				<< std::endl;
		match_threshold_ = default_match_threshold();
	}
	else
	{
		match_threshold_ = match_threshold;
	}
}

void
image_matcher::set_verbose(int verbose)
{
	if ((verbose < 0) || (verbose > 2))
	{
		std::cout << "invalid verbose value, using default: 0" << std::endl;
		verbose_ = 0;
	}
	else
	{
		verbose_ = verbose;
	}
}

void
image_matcher::set_limit(int limit)
{
	if (limit < 0)
	{
		limit = -1; // signals no limit
	}
	else if (limit == 0)
	{
		std::cout << "invalid limit, using default: " << default_limit()
				<< std::endl;
		limit = default_limit();
	}
	limit_ = limit;
}
