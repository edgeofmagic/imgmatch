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
#include <iomanip>
#include <limits>
#include <string>
#include <vector>
#include <boost/program_options.hpp>
#include "image_matcher.h"
#include "imgmatch_config.h"

namespace po = boost::program_options;

using string_vec = std::vector<std::string>;

int
main(int argc, char** argv)
{
	po::options_description visible("Allowed options");
	visible.add_options()
	
		("help,h", "produce help message")
			
		("spew,s", 
			po::value<int>()->default_value(0)->implicit_value(1),
			"set output verbosity level { 0 | 1 | 2 }")
	
		("version,v",
			po::bool_switch()->default_value(false),
			"show version")
	
		("match,m",
			po::value<double>()->
			default_value(image_matcher::default_match_threshold(),
						  image_matcher::default_match_threshold_display()),
			"set match distance threshold")
						
		("target,t", po::value<std::string>(),
			"set match target file or directory")
			
		("results,r", 
			po::value<std::string>()->default_value("matches", "./matches"),
			"set results directory")
	
		("annotate,a",
			po::bool_switch(),
			"annotate link names with distance")
	
		("exhaustive,x",
			po::bool_switch()->default_value(false),
			"exhaustive match in all search directories")
					
		("limit,l", 
			po::value<int>()->default_value(image_matcher::default_limit()),
			 "set maximum images compared per search directory");
	
	po::options_description hidden("Hidden options");
	hidden.add_options()
	
			("search",
			po::value<string_vec>()->composing(),
			"set search directories");
		
	po::options_description all("Allowed options");
	all.add(visible).add(hidden);

	po::positional_options_description pd;
	pd.add("search", -1);

	po::variables_map vm;
	
	try
	{
		po::store(po::command_line_parser(argc, argv)
						.options(all)
						.positional(pd)
						.run(),
				  vm);
		po::notify(vm);
	}
	catch (boost::program_options::error const& e)
	{
        std::cout << "error parsing command line options: " 
				<< e.what() << std::endl;
		return 0;
	}
	
	if (vm.count("help"))
	{
		std::cout << "Usage: imgmatch [<option>...] [<path>...]\n";
		std::cout << visible;
		return 0;
	}
	
	if (vm["version"].as<bool>())
	{
		std::cout << "imgmatch version " << imgmatch_VERSION_MAJOR << "." 
				<< imgmatch_VERSION_MINOR << std::endl;
	}

	image_matcher matcher;

	if (vm.count("spew"))
	{
		matcher.set_verbose(vm["spew"].as<int>());
	}

	if (vm.count("match"))
	{
		matcher.set_match_threshold(vm["match"].as<double>());
	}

	if (vm.count("target"))
	{
		if (!matcher.set_target(vm["target"].as<std::string>()))
		{
			return 0;
		}
	}

	if (vm.count("search"))
	{
		matcher.set_search_paths(vm["search"].as<string_vec> ());
	}
	else
	{
		matcher.set_search_paths({"."});
	}

	if (vm.count("limit"))
	{
		matcher.set_limit(vm["limit"].as<int>());
	}

	if (vm.count("results"))
	{
		if (!matcher.set_results_path(vm["results"].as<std::string>()))
		{
			return 0;
		}
	}
	
	matcher.set_exhaustive(vm["exhaustive"].as<bool>());
	
	assert(vm.count("annotate") > 0);
	
	matcher.set_annotate_links(vm["annotate"].as<bool>());

	if (matcher.verbose() > 1)
	{
		matcher.show_options();
	}

	matcher.execute();

	return 0;
}

