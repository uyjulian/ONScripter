/* -*- C++ -*-
 * 
 *  FontInfo.cpp - Font information storage class of ONScripter
 *
 *  Copyright (c) 2001-2003 Ogapee. All rights reserved.
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

static struct FontContainer{
    FontContainer *next;
    int size;
    TTF_Font *font;

    FontContainer(){
        size = 0;
        next = NULL;
        font = NULL;
    };
} root_font_container;

FontInfo::FontInfo()
{
    ttf_font = NULL;

    reset();
    resetSelectColor();
}

void FontInfo::reset()
{
    xy[0] = xy[1] = 0;

    color[0] = color[1] = color[2] = 0xff;
    is_bold = true;
    is_shadow = true;
    is_transparent = true;
}

void FontInfo::resetSelectColor()
{
    on_color[0]     = on_color[1]     = on_color[2]     = 0xff;
    off_color[0]    = off_color[1]    = off_color[2]    = 0x80;
    nofile_color[0] = nofile_color[1] = nofile_color[2] = 0x80;
}

void *FontInfo::openFont( char *font_file, int ratio1, int ratio2 )
{
    int font_size;
    if ( font_size_xy[0] < font_size_xy[1] )
        font_size = font_size_xy[0];
    else
        font_size = font_size_xy[1];

    FontContainer *fc = &root_font_container;
    while( fc->next ){
        if ( fc->next->size == font_size ) break;
        fc = fc->next;
    }
    if ( !fc->next ){
        fc->next = new FontContainer();
        fc->next->size = font_size;
        fc->next->font = TTF_OpenFont( font_file, font_size * ratio1 / ratio2 );
    }

    ttf_font = (void*)fc->next->font;
    
    return fc->next->font;
}

int FontInfo::x( int tateyoko_mode )
{
    if ( tateyoko_mode == 1 )
        return (num_xy[1] - xy[1] - 1) * pitch_xy[0] + top_xy[0];
    
    return xy[0] * pitch_xy[0] + top_xy[0];
}

int FontInfo::y( int tateyoko_mode )
{
    if ( tateyoko_mode == 1 )
        return xy[0] * pitch_xy[1] + top_xy[1];
    
    return xy[1] * pitch_xy[1] + top_xy[1];
}
