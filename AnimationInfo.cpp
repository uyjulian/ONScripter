/* -*- C++ -*-
 * 
 *  AnimationInfo.cpp - General image storage class of ONScripter
 *
 *  Copyright (c) 2001-2005 Ogapee. All rights reserved.
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

#include "AnimationInfo.h"
#include <math.h>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

//#define ENABLE_BILINEAR

AnimationInfo::AnimationInfo()
{
    // the mask is the same as the one used in TTF_RenderGlyph_Blended
    rmask = 0x00ff0000;
    gmask = 0x0000ff00;
    bmask = 0x000000ff;
    amask = 0xff000000;
    rgbmask = (rmask | gmask | bmask);

    image_name = NULL;
    image_surface = NULL;
    tex_id = 0;

    duration_list = NULL;
    color_list = NULL;
    file_name = NULL;
    mask_file_name = NULL;

    trans_mode = TRANS_TOPLEFT;

    reset();
}

AnimationInfo::~AnimationInfo()
{
    reset();
}

void AnimationInfo::reset()
{
    remove();

    pos.x = pos.y = 0;
    visible = false;

    font_size_xy[0] = font_size_xy[1] = -1;
    font_pitch = -1;
}

void AnimationInfo::deleteImageName(){
    if ( image_name ) delete[] image_name;
    image_name = NULL;
}

void AnimationInfo::setImageName( const char *name ){
    deleteImageName();
    image_name = new char[ strlen(name) + 1 ];
    strcpy( image_name, name );
}

void AnimationInfo::deleteSurface(){
    if ( image_surface ) SDL_FreeSurface( image_surface );
    image_surface = NULL;
#ifdef USE_OPENGL
    if (tex_id != 0) glDeleteTextures(1, (const GLuint*)&tex_id);
    tex_id = 0;
#endif    
}

void AnimationInfo::remove(){
    trans = 256;
    deleteImageName();
    deleteSurface();
    removeTag();
}

void AnimationInfo::removeTag(){
    if ( duration_list ){
        delete[] duration_list;
        duration_list = NULL;
    }
    if ( color_list ){
        delete[] color_list;
        color_list = NULL;
    }
    if ( file_name ){
        delete[] file_name;
        file_name = NULL;
    }
    if ( mask_file_name ){
        delete[] mask_file_name;
        mask_file_name = NULL;
    }
    current_cell = 0;
    num_of_cells = 0;
    remaining_time = 0;
    is_animatable = false;
    is_single_line = true;
    direction = 1;

    color[0] = color[1] = color[2] = 0;
}

// 0 ... restart at the end
// 1 ... stop at the end
// 2 ... reverse at the end
// 3 ... no animation
bool AnimationInfo::proceedAnimation()
{
    bool is_changed = false;
    
    if ( loop_mode != 3 && num_of_cells > 1 ){
        current_cell += direction;
        is_changed = true;
    }

    if ( current_cell < 0 ){ // loop_mode must be 2
        current_cell = 1;
        direction = 1;
    }
    else if ( current_cell >= num_of_cells ){
        if ( loop_mode == 0 ){
            current_cell = 0;
        }
        else if ( loop_mode == 1 ){
            current_cell = num_of_cells - 1;
            is_changed = false;
        }
        else{
            current_cell = num_of_cells - 2;
            direction = -1;
        }
    }

    remaining_time = duration_list[ current_cell ];

    return is_changed;
}

void AnimationInfo::setCell(int cell)
{
    if (cell < 0) cell = 0;
    else if (cell >= num_of_cells) cell = num_of_cells - 1;

    current_cell = cell;
}

int AnimationInfo::doClipping( SDL_Rect *dst, SDL_Rect *clip, SDL_Rect *clipped )
{
    if ( clipped ) clipped->x = clipped->y = 0;

    if ( !dst ||
         dst->x >= clip->x + clip->w || dst->x + dst->w <= clip->x ||
         dst->y >= clip->y + clip->h || dst->y + dst->h <= clip->y )
        return -1;

    if ( dst->x < clip->x ){
        dst->w -= clip->x - dst->x;
        if ( clipped ) clipped->x = clip->x - dst->x;
        dst->x = clip->x;
    }
    if ( clip->x + clip->w < dst->x + dst->w ){
        dst->w = clip->x + clip->w - dst->x;
    }
    
    if ( dst->y < clip->y ){
        dst->h -= clip->y - dst->y;
        if ( clipped ) clipped->y = clip->y - dst->y;
        dst->y = clip->y;
    }
    if ( clip->y + clip->h < dst->y + dst->h ){
        dst->h = clip->y + clip->h - dst->y;
    }
    if ( clipped ){
        clipped->w = dst->w;
        clipped->h = dst->h;
    }

    return 0;
}

void AnimationInfo::blendOnSurface( SDL_Surface *dst_surface, int dst_x, int dst_y,
                                    SDL_Rect *clip, int alpha )
{
    if ( image_surface == NULL ) return;
    
    SDL_Rect dst_rect = {dst_x, dst_y, pos.w, pos.h};
    SDL_Rect src_rect = {0, 0, 0, 0};
    SDL_Rect clip_rect, clipped_rect;

    /* ---------------------------------------- */
    /* 1st clipping */
    if ( clip ){
        if ( doClipping( &dst_rect, clip, &clipped_rect ) ) return;

        src_rect.x += clipped_rect.x;
        src_rect.y += clipped_rect.y;
    }
    
    /* ---------------------------------------- */
    /* 2nd clipping */
    clip_rect.x = 0;
    clip_rect.y = 0;
    clip_rect.w = dst_surface->w;
    clip_rect.h = dst_surface->h;

    if ( doClipping( &dst_rect, &clip_rect, &clipped_rect ) ) return;
    
    src_rect.x += clipped_rect.x;
    src_rect.y += clipped_rect.y;

    /* ---------------------------------------- */
    
    // lock surface
    SDL_LockSurface( dst_surface );
    SDL_LockSurface( image_surface );

    Uint32 *src_buffer = (Uint32 *)image_surface->pixels + image_surface->w * src_rect.y + image_surface->w*current_cell/num_of_cells + src_rect.x;
    Uint32 *dst_buffer = (Uint32 *)dst_surface->pixels  + dst_surface->w * dst_rect.y + dst_rect.x;
    Uint32 mask1, mask2, mask_rb, mask_g;
    
    Uint32 a_shift = image_surface->format->Ashift;
    for (int i=0 ; i<dst_rect.h ; i++){
        for (int j=0 ; j<dst_rect.w ; j++, src_buffer++, dst_buffer++){

            mask2 = (((*src_buffer & amask) >> a_shift) * alpha) >> 8;
            mask1 = 256 - mask2;
            
            mask_rb = (((*dst_buffer & 0xff00ff) * mask1 +
                        (*src_buffer & 0xff00ff) * mask2) >> 8) & 0xff00ff; // red and blue pixel
            mask_g = (((*dst_buffer & 0x00ff00) * mask1 +
                       (*src_buffer & 0x00ff00) * mask2) >> 8) & 0x00ff00; // green pixel

            *dst_buffer = mask_rb | mask_g | amask;
        }
        src_buffer += image_surface->w - dst_rect.w;
        dst_buffer += dst_surface->w  - dst_rect.w;
    }
    
    // unlock surface
    SDL_UnlockSurface( image_surface );
    SDL_UnlockSurface( dst_surface );
}

void AnimationInfo::blendOnSurface2( SDL_Surface *dst_surface, int dst_x, int dst_y,
                                     SDL_Rect *clip, int alpha, 
                                     int scale_x, int scale_y, int rot,
                                     bool do_interpolation )
{
    if ( image_surface == NULL ) return;
    if ( scale_x == 0 || scale_y == 0 ) return;
    
    int i, x, y;
    SDL_Rect dst_rect = {dst_x, dst_y, pos.w, pos.h};

    // for integer arithmetic operation
    int cos_i = 256, sin_i = 0;
    if (rot != 0){
        cos_i = (int)(256.0 * cos(-M_PI*rot/180));
        sin_i = (int)(256.0 * sin(-M_PI*rot/180));
    }

    // project corner point and calculate bounding box
    int dst_corner_xy[4][2];
    int dst_center_xy[2] = {dst_rect.x + dst_rect.w/2, dst_rect.y + dst_rect.h/2};
    int min_xy[2]={dst_surface->w-1, dst_surface->h-1}, max_xy[2]={0,0};
    for (i=0 ; i<4 ; i++){
        int c_x = ((((i+1)%4)/2)*2-1)*dst_rect.w/2 - (((i+1)%4)/2) * (1-dst_rect.w%2);
        int c_y = ((i/2)*2-1)*dst_rect.h/2 - (i/2) * (1-dst_rect.h%2);
        dst_corner_xy[i][0] = (cos_i * scale_x * c_x - sin_i * scale_y * c_y) / (100*256) + dst_center_xy[0];
        dst_corner_xy[i][1] = (sin_i * scale_x * c_x + cos_i * scale_y * c_y) / (100*256) + dst_center_xy[1];

        if (min_xy[0] > dst_corner_xy[i][0]) min_xy[0] = dst_corner_xy[i][0];
        if (max_xy[0] < dst_corner_xy[i][0]) max_xy[0] = dst_corner_xy[i][0];
        if (min_xy[1] > dst_corner_xy[i][1]) min_xy[1] = dst_corner_xy[i][1];
        if (max_xy[1] < dst_corner_xy[i][1]) max_xy[1] = dst_corner_xy[i][1];
    }

    // clip bounding box
    if (max_xy[0] < 0) return;
    if (max_xy[0] >= dst_surface->w) max_xy[0] = dst_surface->w - 1;
    if (min_xy[0] >= dst_surface->w) return;
    if (min_xy[0] < 0) min_xy[0] = 0;
    if (max_xy[1] < 0) return;
    if (max_xy[1] >= dst_surface->h) max_xy[1] = dst_surface->h - 1;
    if (min_xy[1] >= dst_surface->h) return;
    if (min_xy[1] < 0) min_xy[1] = 0;

    // extra clipping
    if ( clip ){
        if (max_xy[0] < clip->x) return;
        else if (max_xy[0] >= clip->x + clip->w) max_xy[0] = clip->x + clip->w - 1;
        if (min_xy[0] >= clip->x + clip->w) return;
        else if (min_xy[0] < clip->x) min_xy[0] = clip->x;
        if (max_xy[1] < clip->y) return;
        else if (max_xy[1] >= clip->y + clip->h) max_xy[1] = clip->y + clip->h - 1;
        if (min_xy[1] >= clip->y + clip->h) return;
        else if (min_xy[1] < clip->y) min_xy[1] = clip->y;
    }

    // lock surface
    SDL_LockSurface( dst_surface );
    SDL_LockSurface( image_surface );

    Uint32 a_shift = image_surface->format->Ashift;
    // set pixel by inverse-projection with raster scanning
    for (y=min_xy[1] ; y<= max_xy[1] ; y++){
        // calculate start and end point for each raster scanning
        int raster_min = min_xy[0], raster_max = max_xy[0];
        if (rot != 0){
            for (i=0 ; i<4 ; i++){
                if (dst_corner_xy[i][1] == dst_corner_xy[(i+1)%4][1]) continue;
                x = (dst_corner_xy[(i+1)%4][0] - dst_corner_xy[i][0])*(y-dst_corner_xy[i][1])/(dst_corner_xy[(i+1)%4][1] - dst_corner_xy[i][1]) + dst_corner_xy[i][0];
                if (dst_corner_xy[(i+1)%4][1] - dst_corner_xy[i][1] > 0){
                    if (raster_max > x) raster_max = x;
                }
                else{
                    if (raster_min < x) raster_min = x;
                }
            }
        }

        Uint32 *dst_buffer = (Uint32 *)dst_surface->pixels + dst_surface->w * y + raster_min;

        // inverse-projection
        for (x=raster_min ; x<=raster_max ; x++, dst_buffer++){
#ifdef ENABLE_BILINEAR            
            int x2 = ( cos_i * (x-dst_center_xy[0]) + sin_i * (y-dst_center_xy[1])) * 100 / scale_x + (dst_rect.w/2)*256;
            int y2 = (-sin_i * (x-dst_center_xy[0]) + cos_i * (y-dst_center_xy[1])) * 100 / scale_y + (dst_rect.h/2)*256;
            int dx = x2 % 256;
            int dy = y2 % 256;
            
            x2 = x2 / 256;
            y2 = y2 / 256;
            
#else
            int x2 = ( cos_i * (x-dst_center_xy[0]) + sin_i * (y-dst_center_xy[1])) * 100 / (scale_x*256) + dst_rect.w/2;
            int y2 = (-sin_i * (x-dst_center_xy[0]) + cos_i * (y-dst_center_xy[1])) * 100 / (scale_y*256) + dst_rect.h/2;
#endif            
            if (x2 < 0) x2 = 0;
            else if (x2 >= image_surface->w) x2 = image_surface->w-1;
            x2 += image_surface->w*current_cell/num_of_cells;
            
            // one last line is omitted to accelerate the interpolation code
            if (y2 < 0) y2 = 0;
            else if (y2 >= image_surface->h-1) y2 = image_surface->h-2;
            
            Uint32 *sp_buffer  = (Uint32 *)image_surface->pixels + image_surface->w * y2 + x2;
            Uint32 pixel, mask_rb, mask_g;
#ifdef ENABLE_BILINEAR            
            Uint32 mask_rb2, mask_g2;
            if (do_interpolation){
                // bi-linear interpolation
                mask_rb = (((*sp_buffer & 0xff00ff) * (256-dx) + (*(sp_buffer+1) & 0xff00ff) * dx) >> 8) & 0xff00ff;
                mask_g  = (((*sp_buffer & 0x00ff00) * (256-dx) + (*(sp_buffer+1) & 0x00ff00) * dx) >> 8) & 0x00ff00;

                sp_buffer += image_surface->w;
                mask_rb2 = (((*sp_buffer & 0xff00ff) * (256-dx) + (*(sp_buffer+1) & 0xff00ff) * dx) >> 8) & 0xff00ff;
                mask_g2  = (((*sp_buffer & 0x00ff00) * (256-dx) + (*(sp_buffer+1) & 0x00ff00) * dx) >> 8) & 0x00ff00;

                pixel = (((mask_rb * (256-dy) + mask_rb2 * dy) >> 8) & 0xff00ff) |
                    (((mask_g  * (256-dy) + mask_g2  * dy) >> 8) & 0x00ff00) | (*sp_buffer & amask);
            }
            else
#endif                
            {
                pixel = *sp_buffer;
            }

            Uint32 mask2 = (((pixel & amask) >> a_shift) * alpha) >> 8;
            Uint32 mask1 = 256 - mask2;
            
            mask_rb = (((*dst_buffer & 0xff00ff) * mask1 +
                        (pixel & 0xff00ff) * mask2) >> 8) & 0xff00ff; // red and blue pixel
            mask_g = (((*dst_buffer & 0x00ff00) * mask1 +
                       (pixel & 0x00ff00) * mask2) >> 8) & 0x00ff00; // green pixel

            *dst_buffer = mask_rb | mask_g | amask;
        }
    }
    
    // unlock surface
    SDL_UnlockSurface( image_surface );
    SDL_UnlockSurface( dst_surface );
}

void AnimationInfo::setupImage( SDL_Surface *surface, SDL_Surface *surface_m )
{
    if (surface == NULL) return;
    SDL_LockSurface( surface );
    Uint32 *buffer = (Uint32 *)surface->pixels;

    int w = surface->w;
    int h = surface->h;
    int w2 = w / num_of_cells;
    if (trans_mode == TRANS_ALPHA)
        w = (w2/2) * num_of_cells;

    image_surface = SDL_CreateRGBSurface(SDL_SWSURFACE, w, h, 32, rmask, gmask, bmask, amask );
    Uint32 *buffer_dst = (Uint32 *)image_surface->pixels;
    
    Uint32 ref_color=0;
    if ( trans_mode == TRANS_TOPLEFT ){
        ref_color = *buffer;
    }
    else if ( trans_mode == TRANS_TOPRIGHT ){
        ref_color = *(buffer + surface->w - 1);
    }
    else if ( trans_mode == TRANS_DIRECT ) {
        ref_color = direct_color[0] << surface->format->Rshift |
            direct_color[1] << surface->format->Gshift |
            direct_color[2] << surface->format->Bshift;
    }
    ref_color &= rgbmask;

    int i, j, c;
    Uint32 mask;
    if ( trans_mode == TRANS_ALPHA ){
        for (i=0 ; i<h ; i++){
            for (c=0 ; c<num_of_cells ; c++){
                for (j=0 ; j<w2/2 ; j++){
                    mask = ((*(buffer+(w2/2)+j) & rmask) >> surface->format->Rshift) ^ 0xff;
                    *buffer_dst++ = (*(buffer+j) & rgbmask) | (mask << surface->format->Ashift);
                }
                buffer += w2;
            }
            buffer += surface->w - w2*num_of_cells;
        }
    }
    else if ( trans_mode == TRANS_MASK ){
        if (surface_m){
            SDL_LockSurface( surface_m );
            int mw = surface->w;
            int mh = surface->h;

            for (i=0 ; i<h ; i++){
                Uint32 *buffer_m = (Uint32 *)surface_m->pixels + mw*(i%mh);
                for (c=0 ; c<num_of_cells ; c++){
                    for (j=0 ; j<w2 ; j++, buffer++){
                        *buffer_dst = *buffer & rgbmask;
                        if (surface_m){
                            mask = ((*(buffer_m + j%mw) & rmask) >> surface->format->Rshift) ^ 0xff;
                            *buffer_dst++ |= mask << surface->format->Ashift;
                        }
                        else
                            *buffer_dst++ |= amask;
                    }
                }
            }
            SDL_UnlockSurface( surface_m );
        }
    }
    else if ( trans_mode == TRANS_TOPLEFT ||
              trans_mode == TRANS_TOPRIGHT ||
              trans_mode == TRANS_DIRECT ){
        for (i=0 ; i<h*w ; i++, buffer++){
            if ( (*buffer & rgbmask) == ref_color )
                *buffer_dst++ = *buffer & rgbmask;
            else
                *buffer_dst++ = *buffer | amask;
        }
    }
    else if ( trans_mode == TRANS_STRING ){
        for (i=0 ; i<h*w ; i++, buffer++)
            *buffer_dst++ = *buffer;
    }
    else { // TRANS_COPY
        for (i=0 ; i<h*w ; i++, buffer++)
            *buffer_dst++ = *buffer | amask;
    }

    SDL_UnlockSurface( surface );

    pos.w = w / num_of_cells;
    pos.h = h;

#ifdef USE_GL_TEXTURE_RECTANGLE
    texture_width  = image_surface->w;
    texture_height = image_surface->h;
#else
    texture_width = 1;
    while (texture_width  < image_surface->w) texture_width <<= 1;
    texture_height = 1;
    while (texture_height < image_surface->h) texture_height <<= 1;
#endif
    bindTexture();
}

void AnimationInfo::bindTexture()
{
#ifdef USE_OPENGL
    if (tex_id == 0) glGenTextures(1, (GLuint*)&tex_id);
#ifdef USE_GL_TEXTURE_RECTANGLE
    glBindTexture(GL_TEXTURE_RECTANGLE_ARB, tex_id);
    glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
#else
    glBindTexture(GL_TEXTURE_2D, tex_id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
#endif
#endif    
}
