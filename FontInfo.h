/* -*- C++ -*-
 * 
 *  FontInfo.h - Font information storage class of ONScripter
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

#ifndef __FONT_INFO_H__
#define __FONT_INFO_H__

typedef unsigned char uchar3[3];

class FontInfo{
public:
    void *ttf_font;
    uchar3 color;
    uchar3 on_color, off_color, nofile_color;
    int font_size_xy[2];
    int top_xy[2]; // Top left origin
    int num_xy[2]; // Row and column of the text windows
    int xy[2]; // Current position
    int pitch_xy[2]; // Width and height of a character
    int wait_time;
    bool is_bold;
    bool is_shadow;
    bool is_transparent;
    uchar3  window_color;

    FontInfo();
    void reset();
    void resetSelectColor();
    void *openFont( char *font_file, int ratio1, int ratio2 );
    void closeFont();
    int x( int tatoyoko_mode ); // return current x position
    int y( int tatoyoko_mode ); // return current y position
};

#endif // __FONT_INFO_H__
