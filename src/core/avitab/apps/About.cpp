/*
 *   AviTab - Aviator's Virtual Tablet
 *   Copyright (C) 2018-2025 Folke Will
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU Affero General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU Affero General Public License for more details.
 *
 *   You should have received a copy of the GNU Affero General Public License
 *   along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#include "About.h"
#include "avitab/config.h"

namespace avitab {

About::About(FuncsPtr appFuncs):
    App(appFuncs),
    window(std::make_shared<Window>(getUIContainer(), "About AviTab")),
    label(std::make_shared<Label>(window, ""))
{
    window->setOnClose([this] () { exit(); });

    std::string aboutText =
            "AviTab " AVITAB_VERSION_STR " [" AVITAB_COMMIT_ID "] from TeamAvitab.\n"
            "Copyright 2018-2025 Folke Will, and other contributors.\n"
            "Licensed under the AGPL license, see LICENSE for details.\n"
            "\n"
            "Avitab uses and acknowledges the copyright and licensing of the following\n"
            "Open Source or public domain libraries (see LICENSING.md for details):\n"
            "Brotli, bzip2, Curl, Detex, {fmt}, FreeType, GLFW, Gumbo, HarfBuzz, jbig2dec, JPEG,\n"
            "JSON for C++, Little CMS, LERC, LIBGEOTIFF, libpng, libssh2, LunaSVG, LittlevGL,\n"
            "Mbed TLS, MuPDF, OpenJPEG, PROJ, SQLite, stb, TIFF, zlib, Zstandard\n"
            "\n"
            "Icon if_applications-internet_118835 copyright by Tango\n"
            "Icon if_Airport_22906 copyright by Icons-Land\n"
            "Icon if_Help_1493288 copyright by GlyphLab\n"
            "Icon if_ilustracoes_04-11_1519786 copyright by Thalita Torres\n"
            "Icon if_xmag_3617 copyright by Everaldo Coelho\n"
            "Icons if_starthere_18227 and if_txt2_3783 copyright by Everaldo Coelho\n";

    label->setText(aboutText);
}

} /* namespace avitab */
