/* This file is part of the Avitab project. See the README and LICENSE for details. */

#include <string>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>
#include <filesystem>
#include <lunasvg.h>


using namespace lunasvg;

std::vector<std::string> args;
std::string outfile;
std::uint32_t width = 64, height = 64;
std::uint32_t bgColor = 0x00000000; // transparent

bool parseArgs(int argc, const char** argv);

int main(int argc, const char** argv)
{
    if (!parseArgs(argc, argv)) exit(1);
    if (args.size() != 1) exit(2);
  
    std::filesystem::path input(args.front());
    
    auto document = Document::loadFromFile(input.string());
    if(!document) exit(3);

    auto bitmap = document->renderToBitmap(width, height, bgColor);
    if(!bitmap.valid()) exit(4);
    bitmap.convertToRGBA();

    std::ostringstream bitmapName;
    bitmapName << input.stem().string() << '_' << width << 'x' << height;
    
    // create the output file
    std::ofstream output(outfile);
    output << "// GENERATED FILE: modifications liable to be overwritten" << std::endl;
    output << "#pragma once" << std::endl;
    output << "#include <stdint.h>" << std::endl;
    output << "const size_t " << bitmapName.str() << "_WIDTH = " << width << ";" << std::endl;
    output << "const size_t " << bitmapName.str() << "_HEIGHT = " << height << ";" << std::endl;
    output << "const uint32_t " << bitmapName.str() << "[] = {" << std::endl;

    auto bw = bitmap.width();
    auto bh = bitmap.height();
    auto stride = bitmap.stride();
    auto rowData = bitmap.data();

    for(std::uint32_t y = 0; y < bh; y++) {
        auto data = rowData;
        std::ostringstream line;
        for(std::uint32_t x = 0; x < bw; x++) {
            line << "0x" << std::hex << *(reinterpret_cast<uint32_t*>(data)) << ", ";
            data += 4;
        }
        output << "    " << line.str() << std::endl;
        rowData += stride;
    }

    output << "}; // " << bitmapName.str() << std::endl;

    if (output.fail()) exit(5);

    return 0;
}

bool parseArgs(int argc, const char** argv)
{
    ++argv; --argc; // dump the program name
    for (; argc > 0; ++argv, --argc) {
        const char *a = *argv;
        const char *n = (argc > 1) ? *(argv + 1) : "";
        if (*a != '-') {
            // not an option, add to list of files
            args.push_back(a);
            continue;
        }
        std::string opt(a);
        if (opt.size() == 2) {
            // all of our single letter options consume the next argument
            opt += n;
            ++argv; --argc;
        }
        if (opt[1] == 'o') {
            outfile = opt.substr(2);
        } else if (opt[1] == 'w') {
            std::stringstream ss;
            ss << opt.substr(2);
            ss >> width;
            if (ss.fail()) return false;
        } else if (opt[1] == 'h') {
            std::stringstream ss;
            ss << opt.substr(2);
            ss >> height;
            if (ss.fail()) return false;
        } else if (opt[1] == 'b') {
            std::stringstream ss;
            ss << std::hex << opt.substr(2);
            ss >> std::hex >> bgColor;
            if (ss.fail()) return false;
        }
    }
    return true;
}
