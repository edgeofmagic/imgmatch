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

bool image_matcher::split_filename(std::string const& fname, 
								   std::string& base, 
								   std::string& suffix) const
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

bool
image_matcher::read_image_file(fs::path const& image_path, 
							   bitmap_image& image) const
{
	std::string suffix = this->filename_suffix(image_path);
	bool result = true;

	if (is_jpeg_suffix(suffix))
	{
		if (!read_jpeg_file(image_path.string(), image))
		{
			if (verbose_ == 1) std::cout << std::endl;
			std::cerr << "error: could not read " << image_path
					<< " as a jpeg image" << std::endl;
			result = false;
		}
	}
	else if (is_png_suffix(suffix))
	{
		if (!read_png_file(image_path.string(), image))
		{
			if (verbose_ == 1) std::cout << std::endl;
			std::cerr << "error: could not read " << image_path
					<< " as a png image" << std::endl;
			result = false;
		}
	}
	else if (is_bmp_suffix(suffix))
	{
		if (!read_bmp_file(image_path.string(), image))
		{
			if (verbose_ == 1) std::cout << std::endl;
			std::cerr << "error: could not read " << image_path
					<< " as a bmp image" << std::endl;
			result = false;
		}
	}
	else
	{
		if (verbose_ == 1) std::cout << std::endl;
		std::cerr << "error: unrecognized file extension: "
				<< image_path << std::endl;
		result = false;
	}

	return result;
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
		std::cerr << "error: target " << target_string << " not found" << std::endl;
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
			std::cerr << "error: target file " << target_path
					<< " is not a regular file" << std::endl;
			return false;
		}

		if (!is_image_file(target_path.filename().string()))
		{
			std::cerr << "error: target file " << target_path
					<< " is not an image file" << std::endl;
			return false;
		}
	}

	if (exhaustive_)
	{
		std::cerr << "warning: exhaustive/x option is incompatible with target," 
				<< "ignoring exhaustive" << std::endl;
		exhaustive_ = false;
	}

	target_path_ = target_path;
	use_target_ = true;
	return true;
}

void image_matcher::set_exhaustive(bool value)
{	
	if (value && use_target_)
	{
		std::cerr << "warning: exhaustive/x option is incompatible with target," 
				<< " ignoring exhaustive" << std::endl;
		exhaustive_ = false;
	}
	else
	{
		exhaustive_ = value;
	}
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
		std::cerr << "error: parent of results directory (" << results_dir_string
				<< ") doesn't exist" << std::endl;
		return false;
	}

	if (!fs::is_directory(results_parent))
	{
		std::cerr << "error: parent of results directory (" << results_dir_string
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
			std::cerr << "error: " << *it << " not found" << std::endl;
			return false;
		}
		if (!fs::is_directory(dir_path))
		{
			std::cerr << "error: " << *it << " is not a directory" << std::endl;
			return false;
		}
		search_paths_.emplace_back(fs::canonical(dir_path));
	}
	if (search_paths_.size() < 1)
	{
		std::cerr << "error: no valid search directories were specified" << std::endl;
		return false;
	}
	return true;
}

void
image_matcher::execute()
{
	path_hist_map search_hist_map;
	
	if (use_target_)
	{
		path_hist_map target_hist_map;
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
			
			build_histograms(target_path_, target_hist_map);
			
			if (target_hist_map.size() < 1)
			{
				std::cerr << "warning: target directory " << target_path_
						<< " contains no image files" << std::endl;
				return;
			}
			
		}
		else
		{
			/*
			 * if the target is an image file, match it against 
			 * all of the images in search directories
			 */

			path_ptr target_path_ptr = std::make_shared<fs::path>(target_path_);
			
			auto insert_result = unique_paths_.insert(target_path_ptr);
			if (!insert_result.second)
			{
				target_path_ptr = *insert_result.first;
			}
			
			build_histogram(target_path_ptr, target_hist_map);

			if (target_hist_map.size() < 1)
			{
				std::cerr << "error: no target image found at " << target_path_
						<< std::endl;
				return;
			}
		}
		
		for (auto it = search_paths_.begin();
			it != search_paths_.end();
			++it)
		{
			build_histograms(*it, search_hist_map);
		}

		find_matches(target_hist_map, search_hist_map);
		
		for (auto it = target_hist_map.begin(); 
			 it != target_hist_map.end(); 
			 ++it)
		{
			search_hist_map.emplace(*std::make_move_iterator(it));
		}

		generate_symlinks(search_hist_map);
		
	}
	else if (exhaustive_)
	{
		for (auto it = search_paths_.begin();
			it != search_paths_.end();
			++it)
		{
			build_histograms(*it, search_hist_map);
		}

		find_matches(search_hist_map);
		generate_symlinks(search_hist_map);
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
			path_hist_map hmap;
			build_histograms(*it, hmap);
			find_matches(hmap);
			for (auto it = hmap.begin(); it != hmap.end(); ++it)
			{
				search_hist_map.emplace(*std::make_move_iterator(it));
			}
		}
		generate_symlinks(search_hist_map);
	}
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
		std::cerr << "error: could not create symbolic link to " << link_target
				<< ", error code message: " << ec.message() << std::endl;
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
			std::cerr << "error: couldn't create directory " << dir_path 
					<< ", error code message: " << ec.message() << std::endl;
		}
	}
	else
	{
		std::cerr << "error: could not create " << dir_path << ", already exists"
				<< std::endl;
	}
	return result;
}

void
image_matcher::create_matching_links(std::vector<path_ptr> const& paths,
									 std::vector<double> const& distances,
									 std::size_t set_index) const
{
	fs::path match_dir_path(results_path_);
	std::string match_dir_name{"m"};
	match_dir_name.append(std::to_string(set_index));
	match_dir_path /= match_dir_name;
	if (!create_dir(match_dir_path))
	{
		std::cerr << "error: couldn't create match set directory " 
				<< match_dir_path << std::endl;
		return;
	}
	
	for (auto i = 0u; i < paths.size(); ++i)
	{
		std::string link_base;
		std::string link_suffix;
		split_filename(paths[i]->filename().string(), link_base, link_suffix);
		if (annotate_links_)
		{
			link_base.append("_").append(std::to_string(distances[i]));
		}
		std::string link_name(link_base);
		link_name.append(link_suffix);

		if (!create_symlink(*paths[i], link_name, match_dir_path))
		{
			std::cerr << "error: couldn't create symbolic link " 
					<< link_name << " in match set directory "
					<< match_dir_path << " to link target " << *paths[i] 
					<< std::endl;
 			return;
		}
	}
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
	
	std::cout << "exhaustive is " << std::boolalpha << exhaustive_ << std::endl;
}

void
image_matcher::set_match_threshold(double match_threshold)
{
	if (match_threshold < 0.0)
	{
		std::cerr << "warning: invalid match threshold value, using default: "
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
		std::cerr << "warning: invalid verbose value, using default: 0" << std::endl;
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
		std::cerr << "warning: invalid limit, using default: " << default_limit()
				<< std::endl;
		limit = default_limit();
	}
	limit_ = limit;
}

void
image_matcher::build_histograms(fs::path const& dir, path_hist_map& hmap)
{
	fs::directory_iterator end_iter;

	if (verbose_ > 0)
	{
		std::cout << "building histograms for image files in "
				<< dir << ":" << std::endl;
	}

	for (fs::directory_iterator dir_itr(dir);
		dir_itr != end_iter;
		++dir_itr)
	{
		try
		{
			fs::path canonical_path(fs::canonical(dir_itr->path()));
			if (fs::is_regular_file(canonical_path)
				&& (is_image_file(canonical_path)))
			{
				if (limit_ < 0 || unique_paths_.size() < limit_)
				{
					if (verbose_ > 1)
					{
						std::cout << "found image file: "
								<< canonical_path.filename() << std::endl;
					}
					path_ptr pth = std::make_shared<fs::path>(canonical_path);
					auto insert_result = unique_paths_.insert(pth);
					if (insert_result.second)
					{
						if (verbose_ > 1)
						{
							std::cout << "building histogram for "
									<< pth->filename() << std::endl;
						}
						else if (verbose_ == 1)
						{
							std::cout << ".";
							std::cout.flush();
						}
						build_histogram(pth, hmap);
					}
				}
				else
				{
					std::cout << "image count limit reached" << std::endl;
					break;
				}
			}
		}
		catch (const std::exception & ex)
		{
			std::cerr << "error: '" << dir_itr->path().filename() << "' "
					<< ex.what() << std::endl;
		}
	}
	if (verbose_ == 1)
	{
		std::cout << std::endl;
	}
}

void image_matcher::find_matches(path_hist_map const& target_hmap, 
								 path_hist_map const& search_hmap)
{
	for (auto it_targets = target_hmap.begin(); 
		 it_targets != target_hmap.end(); 
		 ++it_targets)
	{
		for (auto it_search = search_hmap.begin(); 
			 it_search != search_hmap.end(); 
			 ++it_search)
		{
			compare(it_targets, it_search);
		}
	}
}

void image_matcher::find_matches(path_hist_map const& hmap)
{
	for (auto it_outer = hmap.begin(); it_outer != hmap.end(); ++it_outer)
	{
		auto it_inner = it_outer;
		while (++it_inner != hmap.end())
		{
			compare(it_outer, it_inner);
		}
	}
}

void image_matcher::compare(path_hist_map::const_iterator a, 
							path_hist_map::const_iterator b)
{
	if (path_at(a) == path_at(b))
	{
		return;
	}
	auto distance = histogram_at(a).chi_sqr_dist(histogram_at(b));
	if (verbose_ > 1)
	{
		std::cout << "compared " << *path_at(a) << " with "
				<< *path_at(b) << ": " << distance << std::endl;
	}
	if (distance <= match_threshold_)
	{
		if (verbose_ > 0)
		{
			std::cout << "found match -- " << *path_at(a) << " and "
					<< *path_at(b) << ": " << distance << std::endl;
		}
		
		auto match_set_end = match_set_map_.end();
		
		auto a_match_set = match_set_map_.find(path_at(a));
		auto b_match_set = match_set_map_.find(path_at(b));
		if (is_end(a_match_set) && is_end(b_match_set))
		{
			match_set_ptr new_set = std::make_shared<match_set>();
			new_set->insert(path_at(a));
			new_set->insert(path_at(b));
			match_set_map_.emplace(path_at(a), new_set);
			match_set_map_.emplace(path_at(b), new_set);
			match_sets_.insert(new_set);
		}
		else if (!is_end(a_match_set) && is_end(b_match_set))
		{
			match_set_map_.emplace(path_at(b), match_set_at(a_match_set));
			match_set_at(a_match_set)->insert(path_at(b));
		}
		else if (is_end(a_match_set) && !is_end(b_match_set))
		{
			match_set_map_.emplace(path_at(a), match_set_at(b_match_set));
			match_set_at(b_match_set)->insert(path_at(a));
		}
		else // a_it != end && b_it != end
		{
			if (match_set_at(a_match_set) != match_set_at(b_match_set))
			{
				// coalesce b's match set into a's match set
//				std::cout << *(path_at(a_match_set)) << " matched " 
//						<< *(path_at(b_match_set)) 
//						<< ", but they belong to disjoint match sets; coalescing" 
//						<< std::endl;
				for (auto b_match_set_member = match_set_at(b_match_set)->begin(); 
					 b_match_set_member != match_set_at(b_match_set)->end(); 
					 ++ b_match_set_member)
				{
					match_set_at(a_match_set)->insert(*b_match_set_member);
				}
				match_sets_.erase(match_set_at(b_match_set));
				match_set_at(b_match_set) = match_set_at(a_match_set);
			} // else nothing -- both are already in the same match set
		}
	}
}

void image_matcher::generate_symlinks(path_hist_map const& hist_map) const
{
	if (match_sets_.size() > 0)
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
			std::cerr << "error: results_path assertion failed" << std::endl;
			return;
		}
		
		std::size_t match_set_count = 0ul;
		
		for (auto match_set_it = match_sets_.begin(); 
			 match_set_it != match_sets_.end(); 
			 ++match_set_it)
		{
			match_set_ptr match_set = *match_set_it;
			std::vector<path_ptr> path_vec;
			for (auto pit = match_set->begin(); pit != match_set->end(); ++pit)
			{
				path_vec.push_back(*pit);
			}
			std::size_t n = match_set->size();
			std::vector<std::vector<double>> dist_matrix;
			dist_matrix.reserve(n);

			for (auto i = 0u; i < n; ++i)
			{
				dist_matrix.emplace_back(n, 0.0);
			}

			for (auto i = 0u; i < n - 1; ++i)
			{
				for (auto j = i + 1; j < n; ++j)
				{
					auto i_hist = hist_map.find(path_vec[i]);
					if (i_hist == hist_map.end())
					{
						std::cerr << "error: histogram for " 
								<< *(path_vec[i]) << " not found in map" 
								<< std::endl;
						return;
					}

					auto j_hist = hist_map.find(path_vec[j]);
					if (j_hist == hist_map.end())
					{
						std::cerr << "error: histogram for " 
								<< *(path_vec[j]) << " not found in map" 
								<< std::endl;
						return;
					}
					auto dist = 
						histogram_at(i_hist).chi_sqr_dist(histogram_at(j_hist));
					dist_matrix[i][j] = dist;
					dist_matrix[j][i] = dist;					
				}
			}
			
			std::vector<double> sum_sq_err;
			sum_sq_err.reserve(n);
			for (auto i = 0u; i < n; ++i)
			{
				double sum = 0.0;
				for (auto j = 0u; j < n; ++j)
				{
					double d = dist_matrix[i][j];
					sum += d * d;
				}
				sum_sq_err.push_back(sum);
			}
			
			auto min_index = 0u;
			auto min_value = sum_sq_err[min_index];
			for (auto i = 1u; i < n; ++i)
			{
				if (sum_sq_err[i] < min_value)
				{
					min_value = sum_sq_err[i];
					min_index = i;
				}
			}

			create_matching_links(path_vec,
								 dist_matrix[min_index],
								 match_set_count++);
			
		}
		
		std::cout << match_set_count << " match sets were found, containing " 
				<< match_set_map_.size() << " matching files" 
				<< std::endl;
	}
	else
	{
		std::cout << "no matches found" << std::endl;
	}
}

void 
image_matcher::build_histogram(path_ptr p, path_hist_map& hmap)
{
	if (hmap.count(p) == 0)
	{
		bitmap_image img;

		if (read_image_file(*p, img))
		{
			hmap.emplace(p,img);
		}			
	}
}
