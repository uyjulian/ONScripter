/* -*- C++ -*-
 * 
 *  FontInfo.cpp - Font information storage class of ONScripter
 *
 *  Copyright (c) 2001-2002 Ogapee. All rights reserved.
 *
 *  ogapee@aqua.dti2.ne.jp
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "FontInfo.h"
#include <stdio.h>
#include <SDL_ttf.h>

FontInfo::FontInfo()
{
    ttf_font = NULL;

    reset();
}

void FontInfo::reset()
{
    xy[0] = xy[1] = 0;

    color[0] = color[1] = color[2] = 0xff;
    is_bold = true;
    is_shadow = true;
    is_transparent = true;
    
    on_color[0]     = on_color[1]     = on_color[2]     = 0xff;
    off_color[0]    = off_color[1]    = off_color[2]    = 0x80;
    nofile_color[0] = nofile_color[1] = nofile_color[2] = 0x80;
}

void *FontInfo::openFont( char *font_file, int ratio1, int ratio2 )
{
    if ( ttf_font ) TTF_CloseFont( (TTF_Font*)ttf_font );

    int font_size;
    
    if ( font_size_xy[0] < font_size_xy[1] )
        font_size = font_size_xy[0];
    else
        font_size = font_size_xy[1];

    ttf_font = (void*)TTF_OpenFont( font_file, font_size * ratio1 / ratio2 );

    return ttf_font;
}

void FontInfo::closeFont()
{
    if ( ttf_font ){
        TTF_CloseFont( (TTF_Font*)ttf_font );
        ttf_font = NULL;
    }
}

int FontInfo::x()
{
    return xy[0] * pitch_xy[0] + top_xy[0];
}

int FontInfo::y()
{
    return xy[1] * pitch_xy[1] + top_xy[1];
}
