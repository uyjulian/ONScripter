/* -*- C++ -*-
 * 
 *  PonscripterLabel_effect_cascade.cpp
 *    - Emulation of Takashi Toyama's "cascade.dll" NScripter plugin effect
 *
 *  Copyright (c) 2008-2011 "Uncle" Mion Sonozaki
 *
 *  UncleMion@gmail.com
 *
 *  Copyright (c) 2019-2019 Ogapee
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
 *  along with this program; if not, see <http://www.gnu.org/licenses/>
 *  or write to the Free Software Foundation, Inc.,
 *  59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "ONScripter.h"

void ONScripter::effectCascade( char *params, int duration )
{
    enum {
        //some constants for cascade
        CASCADE_DIR   = 1,
        CASCADE_LR    = 2,
        CASCADE_UP    = 0,
        CASCADE_DOWN  = 1,
        CASCADE_LEFT  = 2,
        CASCADE_RIGHT = 3,
        CASCADE_CROSS = 4,
        CASCADE_IN    = 8
    };

    SDL_Surface *src_surface;
    SDL_Rect src_rect = screen_rect;
    SDL_Rect dst_rect = screen_rect;
    int mode, width, start, end;

    while (*params != 0 && *params != '/') params++;
    if (*params == '/') params++;
    
    if (params[0] == 'u')
        mode = CASCADE_UP;
    else if (params[0] == 'd')
        mode = CASCADE_DOWN;
    else if (params[0] == 'r')
        mode = CASCADE_RIGHT;
    else
        mode = CASCADE_LEFT;

    if (params[1] == 'i')
        mode |= CASCADE_IN;
    else if (params[1] == 'x')
        mode |= CASCADE_IN | CASCADE_CROSS;

    if (mode & CASCADE_IN)
        src_surface = effect_dst_surface;
    else
        src_surface = effect_src_surface;

    bool dir_flag = false;
    if ((!(mode & CASCADE_DIR) && !(mode & CASCADE_IN)) ||
        ((mode & CASCADE_DIR) && (mode & CASCADE_IN)))
        dir_flag = true;
    
    if (mode & CASCADE_LR) {
        // moves left-right
        width = screen_width * effect_counter / duration;
        if (!(mode & CASCADE_IN))
            width = screen_width - width;

        src_rect.y = dst_rect.y = 0;
        src_rect.h = dst_rect.h = screen_height;
        src_rect.w = dst_rect.w = 1;
        if ((mode & CASCADE_CROSS) && (width > 0)) {
            // need to cascade-out the src
            if (mode & CASCADE_DIR) {
                // moves right
                start = 0;
                end = width;
                dst_rect.x = end;
            } else {
                // moves left
                start = screen_width - width;
                end = screen_width;
                dst_rect.x = start;
            }
            src_rect.x = 0;
            SDL_BlitSurface(effect_src_surface, &dst_rect, accumulation_surface, &src_rect);
            for (int i=start; i<end; i++) {
                dst_rect.x = i;
                SDL_BlitSurface(accumulation_surface, &src_rect, effect_src_surface, &dst_rect);
            }
        }
        
        if (dir_flag){
            start = width;
            end = screen_width;
            src_rect.x = start;
        } else {
            start = 0;
            end = screen_width - width;
            src_rect.x = end;
        }
        for (int i=start; i<end; i++) {
            dst_rect.x = i;
            SDL_BlitSurface(src_surface, &src_rect, accumulation_surface, &dst_rect);
        }
        
        if (dir_flag)
            src_rect.x = 0;
        else
            src_rect.x = screen_width - width;
        dst_rect.x = src_rect.x;
        src_rect.w = dst_rect.w = width;
        SDL_BlitSurface(src_surface, &src_rect, accumulation_surface, &dst_rect);
    } else {
        // moves up-down
        width = screen_height * effect_counter / duration;
        if (!(mode & CASCADE_IN))
            width = screen_height - width;

        src_rect.x = dst_rect.x = 0;
        src_rect.h = dst_rect.h = 1;
        src_rect.w = dst_rect.w = screen_width;
        if ((mode & CASCADE_CROSS) && (width > 0)) {
            // need to cascade-out the src
            if (mode & CASCADE_DIR) {
                // moves down
                start = 0;
                end = width;
                dst_rect.y = end;
            } else {
                // moves up
                start = screen_height - width;
                end = screen_height;
                dst_rect.y = start;
            }
            src_rect.y = 0;
            SDL_BlitSurface(effect_src_surface, &dst_rect, accumulation_surface, &src_rect);
            for (int i=start; i<end; i++) {
                dst_rect.y = i;
                SDL_BlitSurface(accumulation_surface, &src_rect, effect_src_surface, &dst_rect);
            }
        }
        
        if (dir_flag){
            start = width;
            end = screen_height;
            src_rect.y = start;
        } else {
            start = 0;
            end = screen_height - width;
            src_rect.y = end;
        }
        for (int i=start; i<end; i++) {
            dst_rect.y = i;
            SDL_BlitSurface(src_surface, &src_rect, accumulation_surface, &dst_rect);
        }
        
        if (dir_flag)
            src_rect.y = 0;
        else
            src_rect.y = screen_height - width;
        dst_rect.y = src_rect.y;
        src_rect.h = dst_rect.h = width;
        SDL_BlitSurface(src_surface, &src_rect, accumulation_surface, &dst_rect);
    }
    if (mode & CASCADE_CROSS) {
        // do crossfade
        width = 256 * effect_counter / duration;
        SDL_Surface *tmp = effect_dst_surface;
        effect_dst_surface = accumulation_surface;
        alphaBlend(NULL, ALPHA_BLEND_CONST, width, &dirty_rect.bounding_box);
        effect_dst_surface = tmp;
    }
}
