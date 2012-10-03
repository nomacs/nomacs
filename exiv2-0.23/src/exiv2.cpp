// ***************************************************************** -*- C++ -*-
/*
 * Copyright (C) 2004-2012 Andreas Huggel <ahuggel@gmx.net>
 *
 * This program is part of the Exiv2 distribution.
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
  Abstract:  Command line program to display and manipulate image metadata.

  File:      exiv2.cpp
  Version:   $Rev: 2681 $
  Author(s): Andreas Huggel (ahu) <ahuggel@gmx.net>
  History:   10-Dec-03, ahu: created
 */
// *****************************************************************************
#include "rcsid_int.hpp"
EXIV2_RCSID("@(#) $Id: exiv2.cpp 2681 2012-03-22 15:19:35Z ahuggel $")

// *****************************************************************************
// included header files
#ifdef _MSC_VER
# include "exv_msvc.h"
#else
# include "exv_conf.h"
#endif

#include "exiv2app.hpp"
#include "actions.hpp"
#include "utils.hpp"
#include "convert.hpp"
#include "i18n.h"      // NLS support.
#include "xmp.hpp"

#include <string>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <cstring>
#include <cassert>
#include <cctype>

// *****************************************************************************
// local declarations
namespace {

    //! List of all command identifiers and corresponding strings
    static const CmdIdAndString cmdIdAndString[] = {
        { add, "add" },
        { set, "set" },
        { del, "del" },
        { reg, "reg" },
        { invalidCmdId, "invalidCmd" }          // End of list marker
    };

    // Return a command Id for a command string
    CmdId commandId(const std::string& cmdString);

    // Evaluate [-]HH[:MM[:SS]], returns true and sets time to the value
    // in seconds if successful, else returns false.
    bool parseTime(const std::string& ts, long& time);

    /*!
      @brief Parse the oparg string into a bitmap of common targets.
      @param optarg Option arguments
      @param action Action being processed
      @return A bitmap of common targets or -1 in case of a parse error
     */
    int parseCommonTargets(const std::string& optarg,
                           const std::string& action);

    /*!
      @brief Parse numbers separated by commas into container
      @param previewNumbers Container for the numbers
      @param optarg Option arguments
      @param j Starting index into optarg
      @return Number of characters processed
     */
    int parsePreviewNumbers(Params::PreviewNumbers& previewNumbers,
                            const std::string& optarg,
                            int j);

    /*!
      @brief Parse metadata modification commands from multiple files
      @param modifyCmds Reference to a structure to store the parsed commands
      @param cmdFiles Container with the file names
     */
    bool parseCmdFiles(ModifyCmds& modifyCmds,
                       const Params::CmdFiles& cmdFiles);

    /*!
      @brief Parse metadata modification commands from a container of commands
      @param modifyCmds Reference to a structure to store the parsed commands
      @param cmdLines Container with the commands
     */
    bool parseCmdLines(ModifyCmds& modifyCmds,
                       const Params::CmdLines& cmdLines);

    /*!
      @brief Parse one line of the command file
      @param modifyCmd Reference to a command structure to store the parsed
             command
      @param line Input line
      @param num Line number (used for error output)
     */
    bool parseLine(ModifyCmd& modifyCmd,
                   const std::string& line, int num);

    /*!
      @brief Parses a string containing backslash-escapes
      @param input Input string, assumed to be UTF-8
     */
    std::string parseEscapes(const std::string& input);
}

// *****************************************************************************
// Main
int main(int argc, char* const argv[])
{
#ifdef EXV_ENABLE_NLS
    setlocale(LC_ALL, "");
    bindtextdomain(EXV_PACKAGE, EXV_LOCALEDIR);
    textdomain(EXV_PACKAGE);
#endif

    // Handle command line arguments
    Params& params = Params::instance();
    if (params.getopt(argc, argv)) {
        params.usage();
        return 1;
    }
    if (params.help_) {
        params.help();
        return 0;
    }
    if (params.version_) {
        params.version();
        return 0;
    }

    // Create the required action class
    Action::TaskFactory& taskFactory = Action::TaskFactory::instance();
    Action::Task::AutoPtr task
        = taskFactory.create(Action::TaskType(params.action_));
    assert(task.get());

    // Process all files
    int rc = 0;
    int n = 1;
    int s = static_cast<int>(params.files_.size());
    int w = s > 9 ? s > 99 ? 3 : 2 : 1;
    for (Params::Files::const_iterator i = params.files_.begin();
         i != params.files_.end(); ++i) {
        if (params.verbose_) {
            std::cout << _("File") << " " << std::setw(w) << std::right << n++ << "/" << s << ": "
                      << *i << std::endl;
        }
        int ret = task->run(*i);
        if (rc == 0) rc = ret;
    }

    taskFactory.cleanup();
    params.cleanup();
    Exiv2::XmpParser::terminate();

    // Return a positive one byte code for better consistency across platforms
    return static_cast<unsigned int>(rc) % 256;
} // main

// *****************************************************************************
// class Params
Params* Params::instance_ = 0;

const Params::YodAdjust Params::emptyYodAdjust_[] = {
    { false, "-Y", 0 },
    { false, "-O", 0 },
    { false, "-D", 0 },
};

Params& Params::instance()
{
    if (0 == instance_) {
        instance_ = new Params;
    }
    return *instance_;
}

void Params::cleanup()
{
    delete instance_;
    instance_ = 0;
}

void Params::version(std::ostream& os) const
{
    bool  b64    = sizeof(void*)==8;
    const char* sBuild = b64 ? "(64 bit build)" : "(32 bit build)" ;
    os << EXV_PACKAGE_STRING << " " << Exiv2::versionNumberHexString() << " " << sBuild << "\n"
       << _("Copyright (C) 2004-2012 Andreas Huggel.\n")
       << "\n"
       << _("This program is free software; you can redistribute it and/or\n"
            "modify it under the terms of the GNU General Public License\n"
            "as published by the Free Software Foundation; either version 2\n"
            "of the License, or (at your option) any later version.\n")
       << "\n"
       << _("This program is distributed in the hope that it will be useful,\n"
            "but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
            "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"
            "GNU General Public License for more details.\n")
       << "\n"
       << _("You should have received a copy of the GNU General Public\n"
            "License along with this program; if not, write to the Free\n"
            "Software Foundation, Inc., 51 Franklin Street, Fifth Floor,\n"
            "Boston, MA 02110-1301 USA\n");
}

void Params::usage(std::ostream& os) const
{
    os << _("Usage:") << " " << progname()
       << " " << _("[ options ] [ action ] file ...\n\n")
       << _("Manipulate the Exif metadata of images.\n");
}

void Params::help(std::ostream& os) const
{
    usage(os);
    os << _("\nActions:\n")
       << _("  ad | adjust   Adjust Exif timestamps by the given time. This action\n"
            "                requires at least one of the -a, -Y, -O or -D options.\n")
       << _("  pr | print    Print image metadata.\n")
       << _("  rm | delete   Delete image metadata from the files.\n")
       << _("  in | insert   Insert metadata from corresponding *.exv files.\n"
            "                Use option -S to change the suffix of the input files.\n")
       << _("  ex | extract  Extract metadata to *.exv, *.xmp and thumbnail image files.\n")
       << _("  mv | rename   Rename files and/or set file timestamps according to the\n"
            "                Exif create timestamp. The filename format can be set with\n"
            "                -r format, timestamp options are controlled with -t and -T.\n")
       << _("  mo | modify   Apply commands to modify (add, set, delete) the Exif and\n"
            "                IPTC metadata of image files or set the JPEG comment.\n"
            "                Requires option -c, -m or -M.\n")
       << _("  fi | fixiso   Copy ISO setting from the Nikon Makernote to the regular\n"
            "                Exif tag.\n")
       << _("  fc | fixcom   Convert the UNICODE Exif user comment to UCS-2. Its current\n"
            "                character encoding can be specified with the -n option.\n")
       << _("\nOptions:\n")
       << _("   -h      Display this help and exit.\n")
       << _("   -V      Show the program version and exit.\n")
       << _("   -v      Be verbose during the program run.\n")
       << _("   -q      Silence warnings and error messages during the program run (quiet).\n")
       << _("   -Q lvl  Set log-level to d(ebug), i(nfo), w(arning), e(rror) or m(ute).\n")
       << _("   -b      Show large binary values.\n")
       << _("   -u      Show unknown tags.\n")
       << _("   -g key  Only output info for this key (grep).\n")
       << _("   -n enc  Charset to use to decode UNICODE Exif user comments.\n")
       << _("   -k      Preserve file timestamps (keep).\n")
       << _("   -t      Also set the file timestamp in 'rename' action (overrides -k).\n")
       << _("   -T      Only set the file timestamp in 'rename' action, do not rename\n"
            "           the file (overrides -k).\n")
       << _("   -f      Do not prompt before overwriting existing files (force).\n")
       << _("   -F      Do not prompt before renaming files (Force).\n")
       << _("   -a time Time adjustment in the format [-]HH[:MM[:SS]]. This option\n"
            "           is only used with the 'adjust' action.\n")
       << _("   -Y yrs  Year adjustment with the 'adjust' action.\n")
       << _("   -O mon  Month adjustment with the 'adjust' action.\n")
       << _("   -D day  Day adjustment with the 'adjust' action.\n")
       << _("   -p mode Print mode for the 'print' action. Possible modes are:\n")
       << _("             s : print a summary of the Exif metadata (the default)\n")
       << _("             a : print Exif, IPTC and XMP metadata (shortcut for -Pkyct)\n")
       << _("             t : interpreted (translated) Exif data (-PEkyct)\n")
       << _("             v : plain Exif data values (-PExgnycv)\n")
       << _("             h : hexdump of the Exif data (-PExgnycsh)\n")
       << _("             i : IPTC data values (-PIkyct)\n")
       << _("             x : XMP properties (-PXkyct)\n")
       << _("             c : JPEG comment\n")
       << _("             p : list available previews\n")
       << _("   -P flgs Print flags for fine control of tag lists ('print' action):\n")
       << _("             E : include Exif tags in the list\n")
       << _("             I : IPTC datasets\n")
       << _("             X : XMP properties\n")
       << _("             x : print a column with the tag number\n")
       << _("             g : group name\n")
       << _("             k : key\n")
       << _("             l : tag label\n")
       << _("             n : tag name\n")
       << _("             y : type\n")
       << _("             c : number of components (count)\n")
       << _("             s : size in bytes\n")
       << _("             v : plain data value\n")
       << _("             t : interpreted (translated) data\n")
       << _("             h : hexdump of the data\n")
       << _("   -d tgt  Delete target(s) for the 'delete' action. Possible targets are:\n")
       << _("             a : all supported metadata (the default)\n")
       << _("             e : Exif section\n")
       << _("             t : Exif thumbnail only\n")
       << _("             i : IPTC data\n")
       << _("             x : XMP packet\n")
       << _("             c : JPEG comment\n")
       << _("   -i tgt  Insert target(s) for the 'insert' action. Possible targets are\n"
            "           the same as those for the -d option, plus a modifier:\n"
            "             X : Insert metadata from an XMP sidecar file <file>.xmp\n"
            "           Only JPEG thumbnails can be inserted, they need to be named\n"
            "           <file>-thumb.jpg\n")
       << _("   -e tgt  Extract target(s) for the 'extract' action. Possible targets\n"
            "           are the same as those for the -d option, plus a target to extract\n"
            "           preview images and a modifier to generate an XMP sidecar file:\n"
            "             p[<n>[,<m> ...]] : Extract preview images.\n"
            "             X : Extract metadata to an XMP sidecar file <file>.xmp\n")
       << _("   -r fmt  Filename format for the 'rename' action. The format string\n"
            "           follows strftime(3). The following keywords are supported:\n")
       << _("             :basename:   - original filename without extension\n")
       << _("             :dirname:    - name of the directory holding the original file\n")
       << _("             :parentname: - name of parent directory\n")
       << _("           Default filename format is ")
       <<               format_ << ".\n"
       << _("   -c txt  JPEG comment string to set in the image.\n")
       << _("   -m file Command file for the modify action. The format for commands is\n"
            "           set|add|del <key> [[<type>] <value>].\n")
       << _("   -M cmd  Command line for the modify action. The format for the\n"
            "           commands is the same as that of the lines of a command file.\n")
       << _("   -l dir  Location (directory) for files to be inserted from or extracted to.\n")
       << _("   -S .suf Use suffix .suf for source files for insert command.\n\n");
} // Params::help

int Params::option(int opt, const std::string& optarg, int optopt)
{
    int rc = 0;
    switch (opt) {
    case 'h': help_ = true; break;
    case 'V': version_ = true; break;
    case 'v': verbose_ = true; break;
    case 'q': Exiv2::LogMsg::setLevel(Exiv2::LogMsg::mute); break;
    case 'Q': rc = setLogLevel(optarg); break;
    case 'k': preserve_ = true; break;
    case 'b': binary_ = false; break;
    case 'u': unknown_ = false; break;
    case 'f': force_ = true; fileExistsPolicy_ = overwritePolicy; break;
    case 'F': force_ = true; fileExistsPolicy_ = renamePolicy; break;
    case 'g': keys_.push_back(optarg); printMode_ = pmList; break;
    case 'n': charset_ = optarg; break;
    case 'r': rc = evalRename(opt, optarg); break;
    case 't': rc = evalRename(opt, optarg); break;
    case 'T': rc = evalRename(opt, optarg); break;
    case 'a': rc = evalAdjust(optarg); break;
    case 'Y': rc = evalYodAdjust(yodYear, optarg); break;
    case 'O': rc = evalYodAdjust(yodMonth, optarg); break;
    case 'D': rc = evalYodAdjust(yodDay, optarg); break;
    case 'p': rc = evalPrint(optarg); break;
    case 'P': rc = evalPrintFlags(optarg); break;
    case 'd': rc = evalDelete(optarg); break;
    case 'e': rc = evalExtract(optarg); break;
    case 'i': rc = evalInsert(optarg); break;
    case 'c': rc = evalModify(opt, optarg); break;
    case 'm': rc = evalModify(opt, optarg); break;
    case 'M': rc = evalModify(opt, optarg); break;
    case 'l': directory_ = optarg; break;
    case 'S': suffix_ = optarg; break;
    case ':':
        std::cerr << progname() << ": " << _("Option") << " -" << static_cast<char>(optopt)
                   << " " << _("requires an argument\n");
        rc = 1;
        break;
    case '?':
        std::cerr << progname() << ": " << _("Unrecognized option") << " -"
                  << static_cast<char>(optopt) << "\n";
        rc = 1;
        break;
    default:
        std::cerr << progname()
                  << ": " << _("getopt returned unexpected character code") << " "
                  << std::hex << opt << "\n";
        rc = 1;
        break;
    }
    return rc;
} // Params::option

int Params::setLogLevel(const std::string& optarg)
{
    int rc = 0;
    const char logLevel = tolower(optarg[0]);
    switch (logLevel) {
    case 'd': Exiv2::LogMsg::setLevel(Exiv2::LogMsg::debug); break;
    case 'i': Exiv2::LogMsg::setLevel(Exiv2::LogMsg::info); break;
    case 'w': Exiv2::LogMsg::setLevel(Exiv2::LogMsg::warn); break;
    case 'e': Exiv2::LogMsg::setLevel(Exiv2::LogMsg::error); break;
    case 'm': Exiv2::LogMsg::setLevel(Exiv2::LogMsg::mute); break;
    default:
        std::cerr << progname() << ": " << _("Option") << " -Q: "
                  << _("Invalid argument") << " \"" << optarg << "\"\n";
        rc = 1;
        break;
    }
    return rc;
} // Params::setLogLevel

int Params::evalRename(int opt, const std::string& optarg)
{
    int rc = 0;
    switch (action_) {
    case Action::none:
        action_ = Action::rename;
        switch (opt) {
        case 'r':
            format_ = optarg;
            formatSet_ = true;
            break;
        case 't': timestamp_ = true; break;
        case 'T': timestampOnly_ = true; break;
        }
        break;
    case Action::rename:
        if (opt == 'r' && (formatSet_ || timestampOnly_)) {
            std::cerr << progname()
                      << ": " << _("Ignoring surplus option") << " -r \"" << optarg << "\"\n";
        }
        else {
            format_ = optarg;
            formatSet_ = true;
        }
        break;
    default:
        std::cerr << progname()
                  << ": " << _("Option") << " -" << (char)opt
                  << " " << _("is not compatible with a previous option\n");
        rc = 1;
        break;
    }
    return rc;
} // Params::evalRename

int Params::evalAdjust(const std::string& optarg)
{
    int rc = 0;
    switch (action_) {
    case Action::none:
    case Action::adjust:
        if (adjust_) {
            std::cerr << progname()
                      << ": " << _("Ignoring surplus option -a")  << " " << optarg << "\n";
            break;
        }
        action_ = Action::adjust;
        adjust_ = parseTime(optarg, adjustment_);
        if (!adjust_) {
            std::cerr << progname() << ": " << _("Error parsing -a option argument") << " `"
                      << optarg << "'\n";
            rc = 1;
        }
        break;
    default:
        std::cerr << progname()
                  << ": " << _("Option -a is not compatible with a previous option\n");
        rc = 1;
        break;
    }
    return rc;
} // Params::evalAdjust

int Params::evalYodAdjust(const Yod& yod, const std::string& optarg)
{
    int rc = 0;
    switch (action_) {
    case Action::none: // fall-through
    case Action::adjust:
        if (yodAdjust_[yod].flag_) {
            std::cerr << progname()
                      << ": " << _("Ignoring surplus option") << " "
                      << yodAdjust_[yod].option_ << " " << optarg << "\n";
            break;
        }
        action_ = Action::adjust;
        yodAdjust_[yod].flag_ = true;
        if (!Util::strtol(optarg.c_str(), yodAdjust_[yod].adjustment_)) {
            std::cerr << progname() << ": " << _("Error parsing") << " "
                      << yodAdjust_[yod].option_ << " "
                      << _("option argument") << " `" << optarg << "'\n";
            rc = 1;
        }
        break;
    default:
        std::cerr << progname()
                  << ": " << _("Option") << " "
                  << yodAdjust_[yod].option_ << " "
                  << _("is not compatible with a previous option\n");
        rc = 1;
        break;
    }
    return rc;
} // Params::evalYodAdjust

int Params::evalPrint(const std::string& optarg)
{
    int rc = 0;
    switch (action_) {
    case Action::none:
        switch (optarg[0]) {
        case 's': action_ = Action::print; printMode_ = pmSummary; break;
        case 'a': rc = evalPrintFlags("kyct"); break;
        case 't': rc = evalPrintFlags("Ekyct"); break;
        case 'v': rc = evalPrintFlags("Exgnycv"); break;
        case 'h': rc = evalPrintFlags("Exgnycsh"); break;
        case 'i': rc = evalPrintFlags("Ikyct"); break;
        case 'x': rc = evalPrintFlags("Xkyct"); break;
        case 'c': action_ = Action::print; printMode_ = pmComment; break;
        case 'p': action_ = Action::print; printMode_ = pmPreview; break;
        default:
            std::cerr << progname() << ": " << _("Unrecognized print mode") << " `"
                      << optarg << "'\n";
            rc = 1;
            break;
        }
        break;
    case Action::print:
        std::cerr << progname() << ": "
                  << _("Ignoring surplus option -p") << optarg << "\n";
        break;
    default:
        std::cerr << progname() << ": "
                  << _("Option -p is not compatible with a previous option\n");
        rc = 1;
        break;
    }
    return rc;
} // Params::evalPrint

int Params::evalPrintFlags(const std::string& optarg)
{
    int rc = 0;
    switch (action_) {
    case Action::none:
        action_ = Action::print;
        printMode_ = pmList;
        for (std::size_t i = 0; i < optarg.length(); ++i) {
            switch (optarg[i]) {
            case 'E': printTags_  |= Exiv2::mdExif; break;
            case 'I': printTags_  |= Exiv2::mdIptc; break;
            case 'X': printTags_  |= Exiv2::mdXmp;  break;
            case 'x': printItems_ |= prTag;   break;
            case 'g': printItems_ |= prGroup; break;
            case 'k': printItems_ |= prKey;   break;
            case 'l': printItems_ |= prLabel; break;
            case 'n': printItems_ |= prName;  break;
            case 'y': printItems_ |= prType;  break;
            case 'c': printItems_ |= prCount; break;
            case 's': printItems_ |= prSize;  break;
            case 'v': printItems_ |= prValue; break;
            case 't': printItems_ |= prTrans; break;
            case 'h': printItems_ |= prHex;   break;
            default:
                std::cerr << progname() << ": " << _("Unrecognized print item") << " `"
                          << optarg[i] << "'\n";
                rc = 1;
                break;
            }
        }
        break;
    case Action::print:
        std::cerr << progname() << ": "
                  << _("Ignoring surplus option -P") << optarg << "\n";
        break;
    default:
        std::cerr << progname() << ": "
                  << _("Option -P is not compatible with a previous option\n");
        rc = 1;
        break;
    }
    return rc;
} // Params::evalPrintFlags

int Params::evalDelete(const std::string& optarg)
{
    int rc = 0;
    switch (action_) {
    case Action::none:
        action_ = Action::erase;
        target_ = 0;
        // fallthrough
    case Action::erase:
        rc = parseCommonTargets(optarg, "erase");
        if (rc > 0) {
            target_ |= rc;
            rc = 0;
        }
        else {
            rc = 1;
        }
        break;
    default:
        std::cerr << progname() << ": "
                  << _("Option -d is not compatible with a previous option\n");
        rc = 1;
        break;
    }
    return rc;
} // Params::evalDelete

int Params::evalExtract(const std::string& optarg)
{
    int rc = 0;
    switch (action_) {
    case Action::none:
    case Action::modify:
        action_ = Action::extract;
        target_ = 0;
        // fallthrough
    case Action::extract:
        rc = parseCommonTargets(optarg, "extract");
        if (rc > 0) {
            target_ |= rc;
            rc = 0;
        }
        else {
            rc = 1;
        }
        break;
    default:
        std::cerr << progname() << ": "
                  << _("Option -e is not compatible with a previous option\n");
        rc = 1;
        break;
    }
    return rc;
} // Params::evalExtract

int Params::evalInsert(const std::string& optarg)
{
    int rc = 0;
    switch (action_) {
    case Action::none:
    case Action::modify:
        action_ = Action::insert;
        target_ = 0;
        // fallthrough
    case Action::insert:
        rc = parseCommonTargets(optarg, "insert");
        if (rc > 0) {
            target_ |= rc;
            rc = 0;
        }
        else {
            rc = 1;
        }
        break;
    default:
        std::cerr << progname() << ": "
                  << _("Option -i is not compatible with a previous option\n");
        rc = 1;
        break;
    }
    return rc;
} // Params::evalInsert

int Params::evalModify(int opt, const std::string& optarg)
{
    int rc = 0;
    switch (action_) {
    case Action::none:
        action_ = Action::modify;
        // fallthrough
    case Action::modify:
    case Action::extract:
    case Action::insert:
        if (opt == 'c') jpegComment_ = parseEscapes(optarg);
        if (opt == 'm') cmdFiles_.push_back(optarg);  // parse the files later
        if (opt == 'M') cmdLines_.push_back(optarg);  // parse the commands later
        break;
    default:
        std::cerr << progname() << ": "
                  << _("Option") << " -" << (char)opt << " "
                  << _("is not compatible with a previous option\n");
        rc = 1;
        break;
    }
    return rc;
} // Params::evalModify

int Params::nonoption(const std::string& argv)
{
    int rc = 0;
    bool action = false;
    if (first_) {
        // The first non-option argument must be the action
        first_ = false;
        if (argv == "ad" || argv == "adjust") {
            if (action_ != Action::none && action_ != Action::adjust) {
                std::cerr << progname() << ": "
                          << _("Action adjust is not compatible with the given options\n");
                rc = 1;
            }
            action = true;
            action_ = Action::adjust;
        }
        if (argv == "pr" || argv == "print") {
            if (action_ != Action::none && action_ != Action::print) {
                std::cerr << progname() << ": "
                          << _("Action print is not compatible with the given options\n");
                rc = 1;
            }
            action = true;
            action_ = Action::print;
        }
        if (argv == "rm" || argv == "delete") {
            if (action_ != Action::none && action_ != Action::erase) {
                std::cerr << progname() << ": "
                          << _("Action delete is not compatible with the given options\n");
                rc = 1;
            }
            action = true;
            action_ = Action::erase;
        }
        if (argv == "ex" || argv == "extract") {
            if (   action_ != Action::none
                && action_ != Action::extract
                && action_ != Action::modify) {
                std::cerr << progname() << ": "
                          << _("Action extract is not compatible with the given options\n");
                rc = 1;
            }
            action = true;
            action_ = Action::extract;
        }
        if (argv == "in" || argv == "insert") {
            if (   action_ != Action::none
                && action_ != Action::insert
                && action_ != Action::modify) {
                std::cerr << progname() << ": "
                          << _("Action insert is not compatible with the given options\n");
                rc = 1;
            }
            action = true;
            action_ = Action::insert;
        }
        if (argv == "mv" || argv == "rename") {
            if (action_ != Action::none && action_ != Action::rename) {
                std::cerr << progname() << ": "
                          << _("Action rename is not compatible with the given options\n");
                rc = 1;
            }
            action = true;
            action_ = Action::rename;
        }
        if (argv == "mo" || argv == "modify") {
            if (action_ != Action::none && action_ != Action::modify) {
                std::cerr << progname() << ": "
                          << _("Action modify is not compatible with the given options\n");
                rc = 1;
            }
            action = true;
            action_ = Action::modify;
        }
        if (argv == "fi" || argv == "fixiso") {
            if (action_ != Action::none && action_ != Action::fixiso) {
                std::cerr << progname() << ": "
                          << _("Action fixiso is not compatible with the given options\n");
                rc = 1;
            }
            action = true;
            action_ = Action::fixiso;
        }
        if (argv == "fc" || argv == "fixcom" || argv == "fixcomment") {
            if (action_ != Action::none && action_ != Action::fixcom) {
                std::cerr << progname() << ": "
                          << _("Action fixcom is not compatible with the given options\n");
                rc = 1;
            }
            action = true;
            action_ = Action::fixcom;
        }
        if (action_ == Action::none) {
            // if everything else fails, assume print as the default action
            action_ = Action::print;
        }
    }
    if (!action) {
        files_.push_back(argv);
    }
    return rc;
} // Params::nonoption

int Params::getopt(int argc, char* const argv[])
{
    int rc = Util::Getopt::getopt(argc, argv, optstring_);
    // Further consistency checks
    if (help_ || version_) return 0;
    if (action_ == Action::none) {
        // This shouldn't happen since print is taken as default action
        std::cerr << progname() << ": " << _("An action must be specified\n");
        rc = 1;
    }
    if (   action_ == Action::adjust
        && !adjust_
        && !yodAdjust_[yodYear].flag_
        && !yodAdjust_[yodMonth].flag_
        && !yodAdjust_[yodDay].flag_) {
        std::cerr << progname() << ": "
                  << _("Adjust action requires at least one -a, -Y, -O or -D option\n");
        rc = 1;
    }
    if (   action_ == Action::modify
        && cmdFiles_.empty() && cmdLines_.empty() && jpegComment_.empty()) {
        std::cerr << progname() << ": "
                  << _("Modify action requires at least one -c, -m or -M option\n");
        rc = 1;
    }
    if (0 == files_.size()) {
        std::cerr << progname() << ": " << _("At least one file is required\n");
        rc = 1;
    }
    if (rc == 0 && !cmdFiles_.empty()) {
        // Parse command files
        if (!parseCmdFiles(modifyCmds_, cmdFiles_)) {
            std::cerr << progname() << ": " << _("Error parsing -m option arguments\n");
            rc = 1;
        }
    }
    if (rc == 0 && !cmdLines_.empty()) {
        // Parse command lines
        if (!parseCmdLines(modifyCmds_, cmdLines_)) {
            std::cerr << progname() << ": " << _("Error parsing -M option arguments\n");
            rc = 1;
        }
    }
    if (rc == 0 && (!cmdFiles_.empty() || !cmdLines_.empty())) {
        // We'll set them again, after reading the file
        Exiv2::XmpProperties::unregisterNs();
    }
    if (   !directory_.empty()
        && !(action_ == Action::insert || action_ == Action::extract)) {
        std::cerr << progname() << ": "
                  << _("-l option can only be used with extract or insert actions\n");
        rc = 1;
    }
    if (!suffix_.empty() && !(action_ == Action::insert)) {
        std::cerr << progname() << ": "
                  << _("-S option can only be used with insert action\n");
        rc = 1;
    }
    if (timestamp_ && !(action_ == Action::rename)) {
        std::cerr << progname() << ": "
                  << _("-t option can only be used with rename action\n");
        rc = 1;
    }
    if (timestampOnly_ && !(action_ == Action::rename)) {
        std::cerr << progname() << ": "
                  << _("-T option can only be used with rename action\n");
        rc = 1;
    }
    return rc;
} // Params::getopt

// *****************************************************************************
// local implementations
namespace {

    bool parseTime(const std::string& ts, long& time)
    {
        std::string hstr, mstr, sstr;
        char *cts = new char[ts.length() + 1];
        strcpy(cts, ts.c_str());
        char *tmp = ::strtok(cts, ":");
        if (tmp) hstr = tmp;
        tmp = ::strtok(0, ":");
        if (tmp) mstr = tmp;
        tmp = ::strtok(0, ":");
        if (tmp) sstr = tmp;
        delete[] cts;

        int sign = 1;
        long hh(0), mm(0), ss(0);
        // [-]HH part
        if (!Util::strtol(hstr.c_str(), hh)) return false;
        if (hh < 0) {
            sign = -1;
            hh *= -1;
        }
        // check for the -0 special case
        if (hh == 0 && hstr.find('-') != std::string::npos) sign = -1;
        // MM part, if there is one
        if (mstr != "") {
            if (!Util::strtol(mstr.c_str(), mm)) return false;
            if (mm > 59) return false;
            if (mm < 0) return false;
        }
        // SS part, if there is one
        if (sstr != "") {
            if (!Util::strtol(sstr.c_str(), ss)) return false;
            if (ss > 59) return false;
            if (ss < 0) return false;
        }

        time = sign * (hh * 3600 + mm * 60 + ss);
        return true;
    } // parseTime

    int parseCommonTargets(const std::string& optarg,
                           const std::string& action)
    {
        int rc = 0;
        int target = 0;
        for (size_t i = 0; rc == 0 && i < optarg.size(); ++i) {
            switch (optarg[i]) {
            case 'e': target |= Params::ctExif; break;
            case 'i': target |= Params::ctIptc; break;
            case 'x': target |= Params::ctXmp; break;
            case 'c': target |= Params::ctComment; break;
            case 't': target |= Params::ctThumb; break;
            case 'a': target |=   Params::ctExif
                                | Params::ctIptc
                                | Params::ctComment
                                | Params::ctXmp; break;
            case 'X':
                target |= Params::ctXmpSidecar;
                if (optarg == "X") target |= Params::ctExif | Params::ctIptc | Params::ctXmp;
                break;
            case 'p':
            {
                if (strcmp(action.c_str(), "extract") == 0) {
                    i += (size_t) parsePreviewNumbers(Params::instance().previewNumbers_, optarg, (int) i + 1);
                    target |= Params::ctPreview;
                    break;
                }
                // fallthrough
            }
            default:
                std::cerr << Params::instance().progname() << ": " << _("Unrecognized ")
                          << action << " " << _("target") << " `"  << optarg[i] << "'\n";
                rc = -1;
                break;
            }
        }
        return rc ? rc : target;
    } // parseCommonTargets

    int parsePreviewNumbers(Params::PreviewNumbers& previewNumbers,
                            const std::string& optarg,
                            int j)
    {
        size_t k = j;
        for (size_t i = j; i < optarg.size(); ++i) {
            std::ostringstream os;
            for (k = i; k < optarg.size() && isdigit(optarg[k]); ++k) {
                os << optarg[k];
            }
            if (k > i) {
                bool ok = false;
                int num = Exiv2::stringTo<int>(os.str(), ok);
                if (ok && num >= 0) {
                    previewNumbers.insert(num);
                }
                else {
                    std::cerr << Params::instance().progname() << ": "
                              << _("Invalid preview number") << ": " << num << "\n";
                }
                i = k;
            }
            if (!(k < optarg.size() && optarg[i] == ',')) break;
        }
        int ret = static_cast<int>(k - j);
        if (ret == 0) {
            previewNumbers.insert(0);
        }
#ifdef DEBUG
        std::cout << "\nThe set now contains: ";
        for (Params::PreviewNumbers::const_iterator i = previewNumbers.begin();
             i != previewNumbers.end();
             ++i) {
            std::cout << *i << ", ";
        }
        std::cout << std::endl;
#endif
        return (int) (k - j);
    } // parsePreviewNumbers

    bool parseCmdFiles(ModifyCmds& modifyCmds,
                       const Params::CmdFiles& cmdFiles)
    {
        Params::CmdFiles::const_iterator end = cmdFiles.end();
        Params::CmdFiles::const_iterator filename = cmdFiles.begin();
        for ( ; filename != end; ++filename) {
            try {
                std::ifstream file(filename->c_str());
                if (!file) {
                    std::cerr << *filename << ": "
                              << _("Failed to open command file for reading\n");
                    return false;
                }
                int num = 0;
                std::string line;
                while (std::getline(file, line)) {
                    ModifyCmd modifyCmd;
                    if (parseLine(modifyCmd, line, ++num)) {
                        modifyCmds.push_back(modifyCmd);
                    }
                }
            }
            catch (const Exiv2::AnyError& error) {
                std::cerr << *filename << ", " << _("line") << " " << error << "\n";
                return false;
            }
        }
        return true;
    } // parseCmdFile

    bool parseCmdLines(ModifyCmds& modifyCmds,
                       const Params::CmdLines& cmdLines)
    {
        try {
            int num = 0;
            Params::CmdLines::const_iterator end = cmdLines.end();
            Params::CmdLines::const_iterator line = cmdLines.begin();
            for ( ; line != end; ++line) {
                ModifyCmd modifyCmd;
                if (parseLine(modifyCmd, *line, ++num)) {
                    modifyCmds.push_back(modifyCmd);
                }
            }
            return true;
        }
        catch (const Exiv2::AnyError& error) {
            std::cerr << _("-M option") << " " << error << "\n";
            return false;
	}
    } // parseCmdLines

    bool parseLine(ModifyCmd& modifyCmd, const std::string& line, int num)
    {
        const std::string delim = " \t";

        // Skip empty lines and comments
        std::string::size_type cmdStart = line.find_first_not_of(delim);
        if (cmdStart == std::string::npos || line[cmdStart] == '#') return false;

        // Get command and key
        std::string::size_type cmdEnd = line.find_first_of(delim, cmdStart+1);
        std::string::size_type keyStart = line.find_first_not_of(delim, cmdEnd+1);
        std::string::size_type keyEnd = line.find_first_of(delim, keyStart+1);
        if (   cmdStart == std::string::npos
            || cmdEnd == std::string::npos
            || keyStart == std::string::npos) {
            throw Exiv2::Error(1, Exiv2::toString(num)
                               + ": " + _("Invalid command line"));
        }

        std::string cmd(line.substr(cmdStart, cmdEnd-cmdStart));
        CmdId cmdId = commandId(cmd);
        if (cmdId == invalidCmdId) {
            throw Exiv2::Error(1, Exiv2::toString(num)
                               + ": " + _("Invalid command") + " `" + cmd + "'");
        }

        Exiv2::TypeId defaultType = Exiv2::invalidTypeId;
        std::string key(line.substr(keyStart, keyEnd-keyStart));
        MetadataId metadataId = invalidMetadataId;
        if (cmdId != reg) {
            try {
                Exiv2::IptcKey iptcKey(key);
                metadataId = iptc;
                defaultType = Exiv2::IptcDataSets::dataSetType(iptcKey.tag(),
                                                               iptcKey.record());
            }
            catch (const Exiv2::AnyError&) {}
            if (metadataId == invalidMetadataId) {
                try {
                    Exiv2::ExifKey exifKey(key);
                    metadataId = exif;
                    defaultType = exifKey.defaultTypeId();
                }
                catch (const Exiv2::AnyError&) {}
            }
            if (metadataId == invalidMetadataId) {
                try {
                    Exiv2::XmpKey xmpKey(key);
                    metadataId = xmp;
                    defaultType = Exiv2::XmpProperties::propertyType(xmpKey);
                }
                catch (const Exiv2::AnyError&) {}
            }
            if (metadataId == invalidMetadataId) {
                throw Exiv2::Error(1, Exiv2::toString(num)
                                   + ": " + _("Invalid key") + " `" + key + "'");
            }
        }
        std::string value;
        Exiv2::TypeId type = defaultType;
        bool explicitType = false;
        if (cmdId != del) {
            // Get type and value
            std::string::size_type typeStart = std::string::npos;
            if (keyEnd != std::string::npos) typeStart = line.find_first_not_of(delim, keyEnd+1);
            std::string::size_type typeEnd = std::string::npos;
            if (typeStart != std::string::npos) typeEnd = line.find_first_of(delim, typeStart+1);
            std::string::size_type valStart = typeStart;
            std::string::size_type valEnd = std::string::npos;
            if (valStart != std::string::npos) valEnd = line.find_last_not_of(delim);

            if (   cmdId == reg
                && (   keyEnd == std::string::npos
                    || valStart == std::string::npos)) {
                throw Exiv2::Error(1, Exiv2::toString(num)
                                   + ": " + _("Invalid command line") + " " );
            }

            if (   cmdId != reg
                && typeStart != std::string::npos
                && typeEnd != std::string::npos) {
                std::string typeStr(line.substr(typeStart, typeEnd-typeStart));
                Exiv2::TypeId tmpType = Exiv2::TypeInfo::typeId(typeStr);
                if (tmpType != Exiv2::invalidTypeId) {
                    valStart = line.find_first_not_of(delim, typeEnd+1);
                    if (valStart == std::string::npos) {
                        throw Exiv2::Error(1, Exiv2::toString(num)
                                           + ": " + _("Invalid command line") + " " );
                    }
                    type = tmpType;
                    explicitType = true;
                }
            }

            if (valStart != std::string::npos) {
                value = parseEscapes(line.substr(valStart, valEnd+1-valStart));
                std::string::size_type last = value.length()-1;
                if (   (value[0] == '"' && value[last] == '"')
                       || (value[0] == '\'' && value[last] == '\'')) {
                    value = value.substr(1, value.length()-2);
                }
            }
        }

        modifyCmd.cmdId_ = cmdId;
        modifyCmd.key_ = key;
        modifyCmd.metadataId_ = metadataId;
        modifyCmd.typeId_ = type;
        modifyCmd.explicitType_ = explicitType;
        modifyCmd.value_ = value;

        if (cmdId == reg) {
            // Registration needs to be done immediately as the new namespaces are
            // looked up during parsing of subsequent lines (to validate XMP keys).
            Exiv2::XmpProperties::registerNs(modifyCmd.value_, modifyCmd.key_);
        }

        return true;
    } // parseLine

    CmdId commandId(const std::string& cmdString)
    {
        int i = 0;
        for (;   cmdIdAndString[i].cmdId_ != invalidCmdId
                 && cmdIdAndString[i].cmdString_ != cmdString; ++i) {}
        return cmdIdAndString[i].cmdId_;
    }

    std::string parseEscapes(const std::string& input) 
    {
        std::string result = "";
        for (unsigned int i = 0; i < input.length(); ++i) {
            char ch = input[i];
            if (ch != '\\') {
                result.push_back(ch);
                continue;
            }
            int escapeStart = i;
            if (!(input.length() - 1 > i)) {
                result.push_back(ch);
                continue;
            }
            ++i;
            ch = input[i];
            switch (ch) {
            case '\\':                          // Escaping of backslash
                result.push_back('\\');
                break;
            case 'r':                           // Escaping of carriage return
                result.push_back('\r');
                break;
            case 'n':                           // Escaping of newline
                result.push_back('\n');
                break;
            case 't':                           // Escaping of tab
                result.push_back('\t');
                break;
            case 'u':                           // Escaping of unicode
                if (input.length() - 4 > i) {
                    int acc = 0;
                    for (int j = 0; j < 4; ++j) {
                        ++i;
                        acc <<= 4;
                        if (input[i] >= '0' && input[i] <= '9') {
                            acc |= input[i] - '0';
                        }
                        else if (input[i] >= 'a' && input[i] <= 'f') {
                            acc |= input[i] - 'a' + 10;
                        }
                        else if (input[i] >= 'A' && input[i] <= 'F') {
                            acc |= input[i] - 'A' + 10;
                        }
                        else {
                            acc = -1;
                            break;
                        }
                    }
                    if (acc == -1) {
                        result.push_back('\\');
                        i = escapeStart;
                        break;
                    }

                    std::string ucs2toUtf8 = "";
                    ucs2toUtf8.push_back((char) ((acc & 0xff00) >> 8));
                    ucs2toUtf8.push_back((char) (acc & 0x00ff));

                    if (Exiv2::convertStringCharset (ucs2toUtf8, "UCS-2BE", "UTF-8")) {
                        result.append (ucs2toUtf8);
                    }
                }
                else {
                    result.push_back('\\');
                    result.push_back(ch);
                }
                break;
            default:
                result.push_back('\\');
                result.push_back(ch);
            }
        }
        return result;
    }

}
        
