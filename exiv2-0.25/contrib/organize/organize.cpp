// ***************************************************************** -*- C++ -*-
/*
 * Copyright (C) 2009 Brad Schick <schickb@gmail.com>
 *
 * This file is part of the organize tool.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, 5th Floor, Boston, MA 02110-1301 USA.
 */
/*
 File:      organize.cpp
 Version:   $Rev: 3603 $
 Author(s): Brad Schick (brad) <schickb@gmail.com>
 History:   19-Jan-09, brad: created
*/
// *****************************************************************************

#include <boost/program_options.hpp>
#include <boost/regex.hpp>
#include <boost/array.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <exiv2/image.hpp>
#include <exiv2/error.hpp>
#include <exiv2/basicio.hpp>
#include <iostream>
#include <iomanip>
#include <cassert>
#include <limits>
#include "MD5.h"
#include "helpers.hpp"

typedef Exiv2::byte md5digest[16];

namespace po = boost::program_options;

bool g_verbose = false;
bool g_neednewline = false;

// Array size should match number of SLOTs
boost::array<int,4> g_run_order = {{-1, -1, -1, -1}};
const int EXIF_SLOT = 0;
const int IPTC_SLOT = 1;
const int XMP_SLOT  = 2;
const int FILE_SLOT = 3;

const unsigned DOT_EVERY = 55;

struct Pattern {
    std::string pat;
    std::string desc;
    pfunc funcs[4];  // order should always be exif, iptc, xmp, file
};

struct PathPart {
    std::string pre;
    const Pattern *pat;
    std::string post;
    PathPart(std::string pre_, const Pattern *pat_, std::string post_)
         : pre(pre_), pat(pat_), post(post_) {}
};

std::vector<PathPart> g_path_parts;

// Instead of making these all global
struct ProcessParams {
    const fs::path &dest_dir;
    const bool dry_run;
    const bool ignore_dups;
    const bool ignore_unsorted;
    const bool force;
    const bool rename;
    const bool symlink;
    const bool verify;
    const bool move;
    const long limit_depth;
    const fs::path &dups_dir;
    const fs::path &unsorted_dir;
    const std::vector<std::string> &excludes;
    unsigned dups_count;
    unsigned unsorted_count;
    unsigned dir_err_count;
    unsigned file_err_count;
    unsigned ok_count;
    unsigned dups_ignored_count;
    unsigned unsorted_ignored_count;
    unsigned dir_ex_count;
    unsigned file_ex_count;
};

void process_directory(const fs::path &directory, const long depth, 
    ProcessParams &params);

const Pattern g_patterns[] = {
    {"@date", "date captured (2009-01-19)", 
        {exif_date, iptc_date, NULL, file_date} },
    {"@year", "year captured (2009)",
        {exif_year, iptc_year, NULL, file_year} },
    {"@month", "month captured (01)",
        {exif_month, iptc_month, NULL, file_month} },
    {"@day", "day captured (19)",
        {exif_day, iptc_day, NULL, file_day} },
    {"@time", "time captured (14-35-27)", 
        {exif_time, iptc_time, NULL, file_time} },
    {"@hour", "hour captured (14)",
        {exif_hour, iptc_hour, NULL, file_hour} },
    {"@min", "minute captured (35)",
        {exif_minute, iptc_minute, NULL, file_minute} },
    {"@sec", "second captured (27)",
        {exif_second, iptc_second, NULL, file_second} },
    {"@dim", "pixel dimension (2272-1704)",
        {exif_dimension, NULL, NULL, file_dimension} },
    {"@x", "pixel width (2272)",
        {exif_width, NULL, NULL, file_width} },
    {"@y", "pixel height (1704)",
        {exif_height, NULL, NULL, file_height} },
    {"@make", "device make (Canon)",
        {exif_make, NULL, NULL, NULL} },
    {"@model", "device model (Canon PowerShot S40)",
        {exif_model, NULL, NULL, NULL} },
    {"@speed", "shutter speed (1-60)",
        {exif_speed, NULL, NULL, NULL} },
    {"@aper", "aperture (F3.2)",
        {exif_aperture, NULL, NULL, NULL} },
    {"@iso", "iso speed (400)",
        {exif_iso, NULL, NULL, NULL} },
    {"@focal", "focal length (8.6 mm)",
        {exif_focal, NULL, NULL, NULL} },
    {"@dist", "subject distance (1.03 m)",
        {exif_distance, NULL, NULL, NULL} },
    {"@meter", "meter mode (multi-segment)",
        {exif_meter, NULL, NULL, NULL} },
    {"@macro", "macro mode (Off)",
        {exif_macro, NULL, NULL, NULL} },
    {"@orient", "orientation (top_left)",
        {exif_orientation, NULL, NULL, NULL} },
    {"@lens", "lens name (Tamron 90mm f-2.8)",
        {exif_lens, NULL, NULL, NULL} },
    {"@key", "first keyword (Family)",
        {exif_keyword, iptc_keyword, NULL, NULL} },

    {"", "", {NULL, NULL, NULL, NULL} }
};


// Check that 'opt1' and 'opt2' are not specified at the same time. 
void conflicting(const po::variables_map& vm, 
    const char* opt1, const char* opt2)
{
    if (vm.count(opt1) && !vm[opt1].defaulted() 
        && vm.count(opt2) && !vm[opt2].defaulted()) {
        throw std::logic_error(std::string("conflicting options '") 
            + opt1 + "' and '" + opt2 + "'");
    }
}

// Check that 'required' is present
void required(const po::variables_map& vm, const char* required)
{
    if (!vm.count(required) || vm[required].defaulted()) {
        throw std::logic_error(std::string("required parameter '") + required
            + "' is missing");
    }
}

void info(const std::string &msg)
{
    if(g_verbose) {
        std::cout << msg << "\n";
        g_neednewline = false;
    }
}

void error(const std::exception &e, const std::string &msg)
{
    if(g_neednewline) {
        std::cout << "\n";
        g_neednewline = false;
    }
    std::cerr << e.what() << "\n";
    std::cerr << msg << std::endl;
}

void usage_header(const char* exname)
{
    std::cout << "Usage: " << exname << " [options] source-dir dest-dir pattern\n";
}

void usage_full(const po::options_description &options, const char* exname)
{
    usage_header(exname);
    std::cout << "\n  Creates groups of files in new directories defined by a metadata 'pattern'.\n" <<
        "  Files are copied, moved, or linked from 'source-dir' to 'dest-dir'.\n" <<
        "  The destination directory should not be within the source directory.\n\n";
    std::cout << options;

    std::cout << "\nPattern values:\n";
    for( const Pattern *pattern = g_patterns; pattern->pat.length(); ++pattern) {
        std::cout << "  " << std::setw(8) << std::left << pattern->pat;
        std::cout << pattern->desc << "\n";
    }

    std::cout << "\nExamples:\n";
    std::cout << "  `" <<  exname << " -m mess clean @year-@month'\n";
    std::cout << "     Moves files from 'mess' into directories of 'clean' according to\n" <<
                 "     year-month the file was captured (clean/2006-11/...)\n\n";
    std::cout << "  `" <<  exname << " -o ie source find width-@x/height-@y'\n";
    std::cout << "     Copies files into directories according first to pixel width then pixel\n" <<
                 "     height. Check iptc then exif metadata (find/width-2272/height-1704/...)\n\n";
    std::cout << "  `" <<  exname << " -lf source find @aper/@hour'\n";
    std::cout << "     Force create symlinks in directories according first to aperture then\n" <<
                 "     hour captured (find/F3.2/15/...)\n";

    std::cout << std::endl;
}

void version()
{
    std::cout << "organized 0.1\n" <<
        "Copyright (C) 2009 Brad Schick. <schickb@gmail.com>\n\n" <<
        "This program is free software; you can redistribute it and/or\n"
        "modify it under the terms of the GNU General Public License\n"
        "as published by the Free Software Foundation; either version 2\n"
        "of the License, or (at your option) any later version.\n"
        "\n"
        "This program is distributed in the hope that it will be useful,\n"
        "but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
        "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"
        "GNU General Public License for more details.\n"
        "\n"
        "You should have received a copy of the GNU General Public\n"
        "License along with this program; if not, write to the Free\n"
        "Software Foundation, Inc., 51 Franklin Street, Fifth Floor,\n"
        "Boston, MA 02110-1301 USA" << std::endl;
}

// Returns empty string if the destination subdirectory could not be determined
// for the supplied source file.
std::string build_dest(const fs::path &source_file) 
{
    std::string dest;

    Exiv2::Image::AutoPtr image;
    try {
        image = Exiv2::ImageFactory::open(source_file.string());
        image->readMetadata();
    } 
    catch(const Exiv2::AnyError&) {
        // No metadata, let things continue to try file info
    }

    std::vector<PathPart>::iterator iter = g_path_parts.begin();
    std::vector<PathPart>::iterator end = g_path_parts.end();
    for( ; iter != end; ++iter) {
        dest += iter->pre;
        std::string result;

        const Pattern *pat = iter->pat;
        for(unsigned fx = 0; fx < g_run_order.size(); ++fx) {
            if(g_run_order[fx] != -1 && pat->funcs[g_run_order[fx]]) {
                if(g_run_order[fx] == FILE_SLOT) {
                    // Always run file operations
                    result = pat->funcs[g_run_order[fx]](image.get(), source_file);
                }
                else if(image.get()) {
                    // No point in running metadata operations without an image
                    result = pat->funcs[g_run_order[fx]](image.get(), source_file);
                }
                if(result.length())
                    break;
            }
        }
        // If we found no data, even for part of pattern, give up and 
        // return no destination
        if(!result.length())
            return result;
    
        dest += (result + iter->post);
    }
    return dest;
}

bool md5sum(const fs::path &path, md5digest &digest)
{
    try {
        Exiv2::FileIo io(path.string());
        if (io.open() != 0)
            return false;
        Exiv2::IoCloser closer(io);

        Exiv2::byte buff[4096];
        MD5_CTX context;
        MD5Init(&context);

        long read_count = io.read(buff, 4096);
        while(read_count) {
            MD5Update(&context, buff, read_count);
            read_count = io.read(buff, 4096);
        }
        MD5Final(digest, &context);
        return true;
    }
    catch (std::exception& ) {
        return false;
    }
}


int main(int argc, char* argv[])
{
    po::options_description options("Options");
    // Don't use default values because the help print it ugly and too wide
    options.add_options()
        ("move,m", "move files rather than copy")
        ("symlink,s", "symlink files rather than copy (posix only)")
        ("order,o", po::value<std::string>(), 
            "order and types of metadata to read\ne=exif, i=iptc, f=file (default: eif)")
        ("unsorted,u", po::value<std::string>(), 
            "special directory to store unsorted files (default: unsorted)")
        ("dups,d", po::value<std::string>(), 
            "special directory to store files with duplicate names (default: duplicates)")
        ("force,f", "overwrite duplicate files instead of using special directory")
        ("rename,r", "rename duplicate files instead of using special directory")
        ("ignore,i", "ignore both unsorted and duplicate files instead of using special directories")
        ("ignore-unsorted", "ignore unsorted files instead of using special directory")
        ("ignore-dups", "ignore duplicate files instead of using special directory")
        ("verify", "verify copied or moved files and exit if incorrect")
        ("exclude,x", po::value< std::vector<std::string> >(), 
            "exclude directories and files that contain arg (case sensitive on all platforms)")
        ("limit-depth,l", po::value<long>(), 
            "limit recursion to specified depth (0 disables recursion)")
        ("verbose,v", "prints operations as they happen")
        ("dry-run,n", "do not make actual changes (implies verbose)")
        ("help,h", "show this help message then exit")
        ("version,V", "show program version then exit")
        ;

    po::options_description hidden("Hidden Options");
    hidden.add_options()
        ("source-dir", po::value< std::string >(), "directory of files to organize, may end in file wildcard")
        ("dest-dir", po::value< std::string >(), "designation directory for files, may not be within source-dir")
        ("pattern", po::value< std::string >(), "subdirectory pattern for grouping files within dest-dir")
        ;

    po::options_description cmdline;
    cmdline.add(options).add(hidden);

    po::positional_options_description positional;
    positional.add("source-dir", 1);
    positional.add("dest-dir", 1);
    positional.add("pattern", 1);

    try {
        po::variables_map vm;
        po::store(po::command_line_parser(argc, argv).
        options(cmdline).positional(positional).run(), vm);
        po::notify(vm);

        if (vm.count("help")) {
            usage_full(options, argv[0]);
            return 0;
        }
    
        if (vm.count("version")) {
            version();
            return 0;
        }
    
        conflicting(vm, "verify", "symlink");
        conflicting(vm, "move", "symlink");
        conflicting(vm, "unsorted", "ignore");
        conflicting(vm, "unsorted", "ignore-unsorted");
        conflicting(vm, "dups", "ignore");
        conflicting(vm, "dups", "ignore-dups");
        conflicting(vm, "force", "ignore");
        conflicting(vm, "force", "ignore-dups");
        conflicting(vm, "force", "rename");
        conflicting(vm, "rename", "ignore");
        conflicting(vm, "rename", "ignore-dups");
        required(vm, "source-dir");
        required(vm, "dest-dir");
        required(vm, "pattern");
    
        const bool dry_run = vm.count("dry-run") != 0;
        g_verbose = (vm.count("verbose") != 0 || dry_run);
    
        std::string order = "eif";
        if(vm.count("order")) {
            order = vm["order"].as<std::string>();
    
            boost::to_lower(order);
            if(order.length() > 3) {
                throw std::logic_error(std::string("order is longer than 4 characters"));
            }
        }
    
        unsigned i = 0;
        std::string::iterator end = order.end();
        for(std::string::iterator iter = order.begin(); iter != end && i < 4; ++iter, ++i) {
            switch(*iter) {
                case 'e': 
                    g_run_order[i] = EXIF_SLOT;
                    break;
                case 'i': 
                    g_run_order[i] = IPTC_SLOT;
                    break;
                case 'x': 
                    throw std::logic_error(std::string("xmp not implemented yet '") + 
                        *iter + "'");
                    break;
                case 'f': 
                    g_run_order[i] = FILE_SLOT;
                    break;
                default:
                    throw std::logic_error(std::string("unknown order character '") + 
                        *iter + "'");
            }
        }
    
        const fs::path source_dir( vm["source-dir"].as<std::string>() );
        if( !exists(source_dir) || !is_directory(source_dir) ) {
            throw std::logic_error(std::string("source '") + 
                source_dir.string() + "' must exist and be a directory");
        }
    
        const fs::path dest_dir( vm["dest-dir"].as<std::string>() );
        if( exists(dest_dir) && !is_directory(dest_dir) ) {
            throw std::logic_error(std::string("destination '") + 
                dest_dir.string() + "' must be a directory");
        }
    
        // Boost doesn't seem to have a way to get a canonical path, so this
        // simple test is easy to confuse with some ../../'s in the paths. Oh
        // well, this is good enough for now.
        fs::path test_dest(dest_dir);
        for(; !test_dest.empty(); test_dest = test_dest.parent_path()) {
            if(fs::equivalent(source_dir, test_dest)) {
                throw std::logic_error(std::string("dest-dir must not be within source-dir"));
            }
        }
    
        // Disect the pattern
        std::string pattern = vm["pattern"].as<std::string>();
        boost::regex regex( "([^@]*)(@[[:alpha:]]+)([^@]*)");
        boost::sregex_iterator m_iter = make_regex_iterator(pattern, regex);
        boost::sregex_iterator m_end;
        for( ; m_iter != m_end; ++m_iter) {
            const boost::smatch &match = *m_iter;
            const std::string &pre = match[1];
            const std::string &pat = match[2];
            const std::string &post = match[3];
    
            // Should put this in a map, but there aren't that many options now
            bool found = false;
            for( const Pattern *pattern = g_patterns; pattern->pat.length(); ++pattern) {
                if(pattern->pat == pat) {
                    PathPart part(pre, pattern, post);
                    g_path_parts.push_back(part);
                    found = true;
                    break;
                }
            }
        
            if(!found) {
                throw std::logic_error(std::string("unknown pattern '") + pat + "'");
            }
        }
    
        // Assign defaults to params that need them
        const bool ignore = vm.count("ignore") != 0;
        std::vector<std::string> excludes;
        if(vm.count("exclude"))
            excludes = vm["exclude"].as< std::vector<std::string> >();
        long limit_depth = LONG_MAX;
        if(vm.count("limit-depth")) {
            limit_depth = vm["limit-depth"].as<long>();
            // Boost program_options doesn't work with unsigned, so do it manually
            if( limit_depth < 0 )
                throw std::logic_error(std::string("recursion depth limit must be positive"));
        }
        std::string dups = "duplicates";
        if(vm.count("dups"))
            dups = vm["dups"].as<std::string>();
        const fs::path dups_dir = dest_dir / dups;
    
        std::string unsorted = "unsorted";
        if(vm.count("unsorted"))
            unsorted = vm["unsorted"].as<std::string>();
        const fs::path unsorted_dir = dest_dir / unsorted;
    
        ProcessParams params = {
            dest_dir, 
            dry_run,
            (vm.count("ignore-dups") != 0 || ignore), 
            (vm.count("ignore-unsorted") != 0 || ignore), 
            vm.count("force") != 0,
            vm.count("rename") != 0,
            vm.count("symlink") != 0, 
            vm.count("verify") != 0, 
            vm.count("move") != 0, 
            limit_depth, 
            dups_dir, 
            unsorted_dir, 
            excludes,
            0, 0, 0, 0, 0, 0, 0, 0, 0
        };
    
        process_directory(source_dir, 0, params);
    
        std::string op = "copied";
        if(params.symlink)
            op = "linked";
        else if(params.move)
            op = "moved";
    
        if(dry_run)
            op = std::string("would be ") + op;
    
        if(g_neednewline)
            std::cout << "\n";
    
        std::cout << "\n" << params.ok_count << " files " << op << "\n";
        std::cout << "   " << params.dups_count << " duplicates\n";
        std::cout << "   " << params.unsorted_count << " unsorted\n";
        if(params.dups_ignored_count)
            std::cout << params.dups_ignored_count << " duplicates ignored\n";
        if(params.unsorted_ignored_count)
            std::cout << params.unsorted_ignored_count << " unsorted ignored\n";
        if(params.dir_ex_count)
            std::cout << params.dir_ex_count << " directories excluded\n";
        if(params.file_ex_count)
            std::cout << params.file_ex_count << " files excluded\n";
        if(params.dir_err_count)
            std::cout << params.dir_err_count << " directory errors\n";
        if(params.file_err_count)
            std::cout << params.file_err_count << " file errors\n";
    
        return 0;
    }
    catch (Exiv2::AnyError& e) {
        error(e, std::string("Aborting"));
        return -1;
    }
    catch(std::logic_error& e) {
        error(e, "");
        usage_header(argv[0]);
        std::cout << argv[0] << " -h    for more help" << std::endl;
        return -2;
    }
    catch(std::exception& e) {
        error(e, "Aborting");
        return -3;
    }
}

boost::regex uregex("(.*?)\\(([[:digit:]]{1,2})\\)$");

fs::path uniquify(const fs::path &dest)
{
    std::string ext = dest.extension().string();
    std::string fname = dest.stem().string();
    fs::path parent = dest.parent_path();

    unsigned number = 1;
    std::string newfname;
    fs::path newdest;

    boost::smatch match;
    if(boost::regex_search(fname, match, uregex)) {
        // Matches are indexes into fname, so don't change it while reading values
        newfname = match[1];
        number = boost::lexical_cast<short>(match[2]);
        fname = newfname;
    }

    do { 
        newfname = fname + "(" + boost::lexical_cast<std::string>(++number) + ")" + ext;
        newdest = parent / newfname;
    } while(fs::exists(newdest));

    return newdest;
}

void process_directory(const fs::path &directory, const long depth, 
    ProcessParams &params)
{
    // Exclude entire directories
    bool exclude = false;
    std::vector<std::string>::const_iterator x_iter = params.excludes.begin();
    std::vector<std::string>::const_iterator x_end = params.excludes.end();
    for( ; x_iter != x_end; ++x_iter ) {
        if(boost::contains(directory.string(), *x_iter)) {
            exclude = true;
            break;
        }
    }
    if(exclude) {
        info(std::string("excluding directory: ") + directory.string() +
            " matched: " + *x_iter);
        ++params.dir_ex_count;
        return;
    }

    try {
        fs::directory_iterator p_iter(directory), p_end; 
        for( ; p_iter != p_end; ++p_iter) {
            if( is_directory(*p_iter) ) {
                // recurse if we haven't hit the limit
                if(depth < params.limit_depth)
                    process_directory(p_iter->path(), depth + 1, params);
                else {
                    info(std::string("depth reached, skipping: ") +
                        p_iter->path().string());
                }
            }
            else if( is_regular_file(*p_iter) ) {
        
                // Check again for excluding file names
                exclude = false;
                x_iter = params.excludes.begin();
                for( ; x_iter != x_end; ++x_iter ) {
                    if(boost::contains(p_iter->path().string(), *x_iter)) {
                        exclude = true;
                        break;
                    }
                }
                if(exclude) {
                    info(std::string("excluding file: ") + p_iter->path().string() +
                        " matched: " + *x_iter);
                    ++params.file_ex_count;
                    continue;
                }
            
                try {
                    const fs::path dest_subdir = build_dest(*p_iter);
                    fs::path dest_file;
                    if(!dest_subdir.empty())
                        dest_file = params.dest_dir / dest_subdir;
                    else if(params.ignore_unsorted) {
                        info(std::string("ignoring unsorted: ") + p_iter->path().string());
                        ++params.unsorted_ignored_count;
                        continue;
                    }
                    else {
                        info(std::string("unsorted file (missing metadata): ") + p_iter->path().string());
                        dest_file = params.unsorted_dir;
                        ++params.unsorted_count;
                    }
            
                    dest_file /= p_iter->path().filename();
                
                    if(fs::exists(dest_file)) {
                        if(params.ignore_dups) {
                            info(std::string("ignoring: ") + p_iter->path().string() +
                                " duplicates: " +  dest_file.string());
                            ++params.dups_ignored_count;
                            continue;
                        }
                        else {
                            if(params.force) {
                                info(std::string("force removing: ") + dest_file.string() + " for: "
                                    + p_iter->path().string());
                                if(!params.dry_run)
                                    fs::remove(dest_file);
                            }
                            else if(params.rename) {
                                info(std::string("renaming: ") + p_iter->path().string() +
                                    " duplicates: " +  dest_file.string());
                                dest_file = uniquify(dest_file);
                            }
                            else {
                                info(std::string("duplicate file: ") + p_iter->path().string() +
                                    " of: " +  dest_file.string());
                                dest_file = params.dups_dir / dest_subdir / p_iter->path().filename();
                                // Ugh, more dup possibilities
                                if(fs::exists(dest_file)) {
                                    info(std::string("renaming: ") + p_iter->path().string() +
                                        " duplicates: " +  dest_file.string());
                                    dest_file = uniquify(dest_file);
                                }
                            }
                            ++params.dups_count;
                        }
                    }
                
                    if(!params.dry_run)
                        fs::create_directories(dest_file.parent_path());
                
                    if(params.symlink) {
                        info(std::string("linking from: ") + p_iter->path().string() + 
                            " to: " + dest_file.string());
                        if(!params.dry_run) {
                            // The target of a symlink must be either absolute (aka complete) or
                            // relative to the location of the link. Easiest solution is to make
                            // a complete path.
                            fs::path target;
                            if(p_iter->path().is_complete()) 
                                target = p_iter->path();
                            else 
                                target = fs::initial_path() / p_iter->path();
                            fs::create_symlink(target, dest_file);
                        }
                    }
                    else {
                        info(std::string("copying from: ") + p_iter->path().string() +
                            " to: " + dest_file.string());
                        if(!params.dry_run) {
                            // Copy the file and restore its write time (needed for posix)
                            std::time_t time = fs::last_write_time(*p_iter);
                            fs::copy_file(*p_iter, dest_file);
                            fs::last_write_time(dest_file, time);
                            if(params.verify) {
                                md5digest src_digest, dst_digest;
                                bool ok = md5sum(p_iter->path(), src_digest);
                                if(ok)
                                    ok = md5sum(dest_file, dst_digest);
                                if(ok)
                                    ok = (memcmp(src_digest,dst_digest, sizeof(md5digest))==0);
                                if(!ok) {
                                    // Should probably find a more appropriate exception for this
                                    throw std::runtime_error(std::string("File verification failed: '") 
                                        + p_iter->path().string() + "' differs from '" + 
                                        dest_file.string() + "'");
                                } 
                                else {
                                    info(std::string("verification passed"));
                                }
                            }
                        }
                    }
                    if(params.move) {
                        info(std::string("removing: ") + p_iter->path().string());
                        if(!params.dry_run)
                            fs::remove(*p_iter);
                    }
                
                    if(!g_verbose && (params.ok_count % DOT_EVERY)==0) {
                        std::cout << "." << std::flush;
                        g_neednewline = true;
                    }
                    ++params.ok_count;
                }
                catch(fs::filesystem_error& e) {
                    error(e, std::string("skipping file: " + p_iter->path().string()));
                    ++params.file_err_count;
                }
            }
        }
    }
    catch(fs::filesystem_error& e) {
        error(e, std::string("skipping directory: " + directory.string()));
        ++params.dir_err_count;
    }
}

