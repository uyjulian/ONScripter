/* -*- C++ -*-
 * 
 *  DirtyRect.cpp - Invalid region on text_surface which should be updated
 *
 *  Copyright (c) 2001-2004 Ogapee. All rights reserved.
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

#include "DirtyRect.h"

DirtyRect::DirtyRect()
{
    area = 0;
    total_history = 10;
    num_history = 0;
    bounding_box.w = bounding_box.h = 0;
    history = new SDL_Rect[total_history];
};

DirtyRect::DirtyRect( const DirtyRect &d )
{
    area = d.area;
    total_history = d.total_history;
    num_history = d.num_history;
    bounding_box = d.bounding_box;
    history = new SDL_Rect[total_history];

    for ( int i=0 ; i<num_history ; i++ )
        history[i] = d.history[i];
};

DirtyRect& DirtyRect::operator =( const DirtyRect &d )
{
    area = d.area;
    total_history = d.total_history;
    num_history = d.num_history;
    bounding_box = d.bounding_box;
    delete[] history;
    history = new SDL_Rect[total_history];

    for ( int i=0 ; i<num_history ; i++ )
        history[i] = d.history[i];

    return *this;
};

DirtyRect::~DirtyRect()
{
    delete[] history;
}

void DirtyRect::add( SDL_Rect src )
{
    //printf("add %d %d %d %d\n", src.x, src.y, src.w, src.h );
    if ( src.w == 0 || src.h == 0 ) return;

    if (src.x < 0){
        if (src.w < -src.x) return;
        src.w += src.x;
        src.x = 0;
    }
    if (src.y < 0){
        if (src.h < -src.y) return;
        src.h += src.y;
        src.y = 0;
    }

    history[ num_history++ ] = src;
    addBoundingBox( src );
    area += src.w * src.h;
    
    if ( num_history == total_history ){
        total_history += 10;
        SDL_Rect *tmp = history;
        history = new SDL_Rect[ total_history ];
        for ( int i=0 ; i<num_history ; i++ )
            history[i] = tmp[i];

        delete[] tmp;
    }
};

void DirtyRect::addBoundingBox( SDL_Rect &src )
{
    if ( bounding_box.w == 0 || bounding_box.h == 0 ){
        bounding_box = src;
        return;
    }
    if ( bounding_box.x > src.x ){
        bounding_box.w += bounding_box.x - src.x;
        bounding_box.x = src.x;
    }
    if ( bounding_box.y > src.y ){
        bounding_box.h += bounding_box.y - src.y;
        bounding_box.y = src.y;
    }
    if ( bounding_box.x + bounding_box.w < src.x + src.w ){
        bounding_box.w = src.x + src.w - bounding_box.x;
    }
    if ( bounding_box.y + bounding_box.h < src.y + src.h ){
        bounding_box.h = src.y + src.h - bounding_box.y;
    }
}

void DirtyRect::clear()
{
    area = 0;
    num_history = 0;
    bounding_box.w = bounding_box.h = 0;
}

void DirtyRect::fill( int w, int h )
{
    area = w*h;
    bounding_box.x = bounding_box.y = 0;
    bounding_box.w = w;
    bounding_box.h = h;
}
