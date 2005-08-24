/* -*- C++ -*-
 * 
 *  ONScripterLabel_image.cpp - Image processing in ONScripter
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

#include "ONScripterLabel.h"
#include "resize_image.h"

void ONScripterLabel::blitSurface( SDL_Surface *src, SDL_Rect *src_rect, SDL_Surface *dst, SDL_Rect *dst_rect )
{
    SDL_Rect src_rect2 = {0, 0, src->w, src->h};
    if (src_rect) src_rect2 = *src_rect;
    SDL_Rect dst_rect2 = {0, 0, dst->w, dst->h};
    if (dst_rect) dst_rect2 = *dst_rect;

    if (src_rect2.x >= src->w) return;
    if (src_rect2.y >= src->h) return;
    if (dst_rect2.x >= dst->w) return;
    if (dst_rect2.y >= dst->h) return;
    
    if (src_rect2.x+src_rect2.w >= src->w)
        src_rect2.w = src->w - src_rect2.x;
    if (src_rect2.y+src_rect2.h >= src->h)
        src_rect2.h = src->h - src_rect2.y;
        
    if (dst_rect2.x+src_rect2.w > dst->w)
        src_rect2.w = dst->w - dst_rect2.x;
    if (dst_rect2.y+src_rect2.h > dst->h)
        src_rect2.h = dst->h - dst_rect2.y;
        
    SDL_LockSurface( src );
    SDL_LockSurface( dst );

    for (int i=0 ; i<src_rect2.h ; i++)
        memcpy( (Uint32*)dst->pixels + (dst_rect2.y+i) * dst->w + dst_rect2.x,
                (Uint32*)src->pixels + (src_rect2.y+i) * src->w + src_rect2.x,
                src_rect2.w*4 );
    
    SDL_UnlockSurface( dst );
    SDL_UnlockSurface( src );
}

int ONScripterLabel::resizeSurface( SDL_Surface *src, SDL_Rect *src_rect, SDL_Surface *dst, SDL_Rect *dst_rect )
{
    SDL_Rect src_rect2, dst_rect2;

    if ( src_rect ){
        src_rect2 = *src_rect;
    }
    else{
        src_rect2.x = src_rect2.y = 0;
        src_rect2.w = src->w;
        src_rect2.h = src->h;
    }
    
    if ( dst_rect ){
        dst_rect2 = *dst_rect;
    }
    else{
        dst_rect2.x = dst_rect2.y = 0;
        dst_rect2.w = dst->w;
        dst_rect2.h = dst->h;
    }
    
    SDL_LockSurface( dst );
    SDL_LockSurface( src );
    Uint32 *src_buffer = (Uint32 *)src->pixels + src->w * src_rect2.y + src_rect2.x;
    Uint32 *dst_buffer = (Uint32 *)dst->pixels + dst->w * dst_rect2.y + dst_rect2.x;

    /* size of tmp_buffer must be larger than 16 bytes */
    size_t len = src_rect2.w * (src_rect2.h+1) * 4 + 4;
    if (resize_buffer_size < len){
        delete[] resize_buffer;
        resize_buffer = new unsigned char[len];
        resize_buffer_size = len;
    }
    resizeImage( (unsigned char*)dst_buffer, dst_rect2.w, dst_rect2.h, dst->w * 4,
                 (unsigned char*)src_buffer, src_rect2.w, src_rect2.h, src->w * 4,
                 4, resize_buffer, src_rect2.w * 4, false );

    SDL_UnlockSurface( src );
    SDL_UnlockSurface( dst );

    return 0;
}

int ONScripterLabel::shiftRect( SDL_Rect &dst, SDL_Rect &clip )
{
    dst.x += clip.x;
    dst.y += clip.y;
    dst.w = clip.w;
    dst.h = clip.h;

    return 0;
}

#define blend_pixel(){\
    maskrb =  (((*src1_buffer & 0xff00ff) * mask1 + \
                (*src2_buffer & 0xff00ff) * mask2) >> 8) & 0xff00ff;\
    mask |= (((*src1_buffer++ & 0x00ff00) * mask1 +\
              (*src2_buffer++ & 0x00ff00) * mask2) >> 8) & 0x00ff00;\
    *dst_buffer++ = maskrb | mask;\
}

void ONScripterLabel::alphaBlend( SDL_Surface *dst_surface, SDL_Rect dst_rect,
                                  SDL_Surface *src1_surface,
                                  SDL_Surface *src2_surface, int x2, int y2,
                                  SDL_Surface *mask_surface,
                                  int trans_mode, Uint32 mask_value, SDL_Rect *clip, uchar3 *direct_color )
{
    int i, j;
    SDL_Rect clip_rect, clipped_rect;
    Uint32 mask, mask1, mask2, maskrb;
    Uint32 *src1_buffer, *src2_buffer, *dst_buffer, *mask_buffer=NULL;

    /* ---------------------------------------- */
    /* 1st clipping */
    if ( clip ){
        if ( AnimationInfo::doClipping( &dst_rect, clip, &clipped_rect ) ) return;

        x2 += clipped_rect.x;
        y2 += clipped_rect.y;
    }

    /* ---------------------------------------- */
    /* 2nd clipping */
    clip_rect.x = 0;
    clip_rect.y = 0;
    clip_rect.w = dst_surface->w;
    clip_rect.h = dst_surface->h;

    if ( AnimationInfo::doClipping( &dst_rect, &clip_rect, &clipped_rect ) ) return;
    
    x2 += clipped_rect.x;
    y2 += clipped_rect.y;

    /* ---------------------------------------- */

    SDL_LockSurface( src1_surface );
    SDL_LockSurface( src2_surface );
    if ( dst_surface != src1_surface ) SDL_LockSurface( dst_surface );
    if ( mask_surface ) SDL_LockSurface( mask_surface );
    
    src1_buffer = (Uint32 *)src1_surface->pixels + src1_surface->w * dst_rect.y + dst_rect.x;
    src2_buffer = (Uint32 *)src2_surface->pixels + src2_surface->w * y2 + x2;
    dst_buffer  = (Uint32 *)dst_surface->pixels  + dst_surface->w * dst_rect.y + dst_rect.x;

    Uint32 overflow_mask;
    if ( trans_mode == ALPHA_BLEND_FADE_MASK )
        overflow_mask = 0xffffffff;
    else
        overflow_mask = 0xffffff00;

    mask2 = mask_value;
    mask1 = 256 - mask2;

    Uint32 a_shift1 = src1_surface->format->Ashift;
    Uint32 a_shift2 = src2_surface->format->Ashift;
    Uint32 a_shiftd = dst_surface->format->Ashift;
    for ( i=0; i<dst_rect.h ; i++ ) {
        if (mask_surface) mask_buffer = (Uint32 *)mask_surface->pixels + mask_surface->w * ((y2+i)%mask_surface->h);

        if ( trans_mode == ALPHA_BLEND_NORMAL ){
            for ( j=0 ; j<dst_rect.w ; j++ ){
                mask2 = (*src2_buffer & amask) >> a_shift2;
                mask1 = mask2 ^ 0xff;
                mask = amask;
                blend_pixel();
            }
        }
        else if ( trans_mode == ALPHA_BLEND_MULTIPLE ){
            for ( j=0 ; j<dst_rect.w ; j++ ){
                mask2 = (*src2_buffer & amask) >> a_shift2;
                mask1 = mask2 ^ 0xff;

                Uint32 an_1 = (*src1_buffer & amask) >> a_shift1;
                Uint32 an = 0xff-((0xff-an_1)*(0xff-mask2) >> 8);
                mask = an << a_shiftd;
                mask1 = an_1 * mask1 / an;
                mask2 = (mask2 << 8) / an;
                blend_pixel();
            }
        }
        else if ( trans_mode == ALPHA_BLEND_FADE_MASK ||
                  trans_mode == ALPHA_BLEND_CROSSFADE_MASK ){
            for ( j=0 ; j<dst_rect.w ; j++ ){
                mask = *(mask_buffer + (x2+j)%mask_surface->w) & 0xff;
                if ( mask_value > mask ){
                    mask2 = mask_value - mask;
                    if ( mask2 & overflow_mask ) mask2 = 0xff;
                }
                else{
                    mask2 = 0;
                }
                mask = amask;
                mask1 = mask2 ^ 0xff;
                blend_pixel();
            }
        }
        else{ // ALPHA_BLEND_CONST
            for ( j=0 ; j<dst_rect.w ; j++ ){
                mask = amask;
                blend_pixel();
            }
        }
        src1_buffer += src1_surface->w - dst_rect.w;
        src2_buffer += src2_surface->w - dst_rect.w;
        dst_buffer  += dst_surface->w  - dst_rect.w;
    }
    
    if ( mask_surface ) SDL_UnlockSurface( mask_surface );
    if ( dst_surface != src1_surface ) SDL_UnlockSurface( dst_surface );
    SDL_UnlockSurface( src2_surface );
    SDL_UnlockSurface( src1_surface );
}

void ONScripterLabel::makeNegaSurface( SDL_Surface *surface, SDL_Rect *dst_rect, int refresh_mode )
{
    int i, j;
    SDL_Rect rect;
    Uint32 *buf, cr, cg, cb;

    if ( dst_rect ){
        rect.x = dst_rect->x;
        rect.y = dst_rect->y;
        rect.w = dst_rect->w;
        rect.h = dst_rect->h;
        if ( rect.x + rect.w > surface->w ) rect.w = surface->w - rect.x;
        if ( rect.y + rect.h > surface->h ) rect.h = surface->h - rect.y;
    }
    else{
        rect.x = rect.y = 0;
        rect.w = surface->w;
        rect.h = surface->h;
    }

    SDL_LockSurface( surface );
    buf = (Uint32 *)surface->pixels + rect.y * surface->w + rect.x;

    for ( i=rect.y ; i<rect.y + rect.h ; i++ ){
        for ( j=rect.x ; j<rect.x + rect.w ; j++, buf++ ){
            cr = ((*buf >> surface->format->Rshift) & 0xff) ^ 0xff;
            cg = ((*buf >> surface->format->Gshift) & 0xff) ^ 0xff;
            cb = ((*buf >> surface->format->Bshift) & 0xff) ^ 0xff;
            *buf = cr << surface->format->Rshift |
                cg << surface->format->Gshift |
                cb << surface->format->Bshift |
                (*buf & amask);
        }
        buf += surface->w - rect.w;
    }

    SDL_UnlockSurface( surface );
}

void ONScripterLabel::makeMonochromeSurface( SDL_Surface *surface, SDL_Rect *dst_rect, int refresh_mode )
{
    SDL_Rect rect = {0, 0, screen_width, screen_height};

    if ( dst_rect ){
        rect = *dst_rect;
        if ( rect.x + rect.w > surface->w ) rect.w = surface->w - rect.x;
        if ( rect.y + rect.h > surface->h ) rect.h = surface->h - rect.y;
    }

    SDL_LockSurface( surface );
    Uint32 *buf = (Uint32 *)surface->pixels + rect.y * surface->w + rect.x, c;
    
    for ( int i=rect.y ; i<rect.y + rect.h ; i++ ){
        for ( int j=rect.x ; j<rect.x + rect.w ; j++, buf++ ){
            c = (((*buf >> surface->format->Rshift) & 0xff) * 77 +
                 ((*buf >> surface->format->Gshift) & 0xff) * 151 +
                 ((*buf >> surface->format->Bshift) & 0xff) * 28 ) >> 8; 
            *buf = monocro_color_lut[c][0] << surface->format->Rshift |
                monocro_color_lut[c][1] << surface->format->Gshift |
                monocro_color_lut[c][2] << surface->format->Bshift |
                (*buf & amask);
        }
        buf += surface->w - rect.w;
    }

    SDL_UnlockSurface( surface );
}

void ONScripterLabel::refreshSurfaceParameters()
{
    int i;
    
    monocro_flag = monocro_flag_new;
    for ( i=0 ; i<3 ; i++ )
        monocro_color[i] = monocro_color_new[i];
    for ( i=0 ; i<256 ; i++ ){
        monocro_color_lut[i][0] = (monocro_color[0] * i) >> 8;
        monocro_color_lut[i][1] = (monocro_color[1] * i) >> 8;
        monocro_color_lut[i][2] = (monocro_color[2] * i) >> 8;
    }
    need_refresh_flag = false;
}

void ONScripterLabel::refreshSurface( SDL_Surface *surface, SDL_Rect *clip, int refresh_mode )
{
    if (refresh_mode == REFRESH_NONE_MODE) return;
    
    int i, top;

    drawTaggedSurface( surface, &bg_info, clip, refresh_mode );
    
    if ( !all_sprite_hide_flag ){
        if ( z_order < 10 && refresh_mode & REFRESH_SAYA_MODE )
            top = 9;
        else
            top = z_order;
        for ( i=MAX_SPRITE_NUM-1 ; i>top ; i-- ){
            if ( sprite_info[i].image_surface && sprite_info[i].visible ){
                drawTaggedSurface( surface, &sprite_info[i], clip, refresh_mode );
            }
        }
    }

    for ( i=0 ; i<3 ; i++ ){
        if (human_order[2-i] >= 0 && tachi_info[human_order[2-i]].image_surface){
            drawTaggedSurface( surface, &tachi_info[human_order[2-i]], clip, refresh_mode );
        }
    }

    if ( windowback_flag ){
        if ( nega_mode == 1 ) makeNegaSurface( surface, clip, refresh_mode );
        if ( monocro_flag ) makeMonochromeSurface( surface, clip, refresh_mode );
        if ( nega_mode == 2 ) makeNegaSurface( surface, clip, refresh_mode );
        shadowTextDisplay( surface, clip, refresh_mode );
        refreshText( surface, clip, refresh_mode );
    }

    if ( !all_sprite_hide_flag ){
        if ( refresh_mode & REFRESH_SAYA_MODE )
            top = 10;
        else
            top = 0;
        for ( i=z_order ; i>=top ; i-- ){
            if ( sprite_info[i].image_surface && sprite_info[i].visible ){
                drawTaggedSurface( surface, &sprite_info[i], clip, refresh_mode );
            }
        }
    }

    if ( !windowback_flag ){
        if ( nega_mode == 1 ) makeNegaSurface( surface, clip, refresh_mode );
        if ( monocro_flag ) makeMonochromeSurface( surface, clip, refresh_mode );
        if ( nega_mode == 2 ) makeNegaSurface( surface, clip, refresh_mode );
    }
    
    if ( !( refresh_mode & REFRESH_SAYA_MODE ) ){
        for ( i=0 ; i<MAX_PARAM_NUM ; i++ ){
            if ( bar_info[i] ) {
                drawTaggedSurface( surface, bar_info[i], clip, refresh_mode );
            }
        }
        for ( i=0 ; i<MAX_PARAM_NUM ; i++ ){
            if ( prnum_info[i] ){
                drawTaggedSurface( surface, prnum_info[i], clip, refresh_mode );
            }
        }
    }

    if ( !windowback_flag ){
        shadowTextDisplay( surface, clip, refresh_mode );
        refreshText( surface, clip, refresh_mode );
    }

    if ( refresh_mode & REFRESH_CURSOR_MODE && !textgosub_label ){
        if ( clickstr_state == CLICK_WAIT )
            drawTaggedSurface( surface, &cursor_info[CURSOR_WAIT_NO], clip, refresh_mode );
        else if ( clickstr_state == CLICK_NEWPAGE )
            drawTaggedSurface( surface, &cursor_info[CURSOR_NEWPAGE_NO], clip, refresh_mode );
    }

    ButtonLink *p_button_link = root_button_link.next;
    while( p_button_link ){
        if (p_button_link->show_flag > 0){
            drawTaggedSurface( surface, p_button_link->anim[p_button_link->show_flag-1], clip, refresh_mode );
        }
        p_button_link = p_button_link->next;
    }
}

void ONScripterLabel::refreshSprite( SDL_Surface *surface, int sprite_no,
                                     bool active_flag, int cell_no,
                                     SDL_Rect *check_src_rect, SDL_Rect *check_dst_rect )
{
    if ( sprite_info[sprite_no].image_name && 
         ( sprite_info[ sprite_no ].visible != active_flag ||
           (cell_no >= 0 && sprite_info[ sprite_no ].current_cell != cell_no ) ||
           AnimationInfo::doClipping(check_src_rect, &sprite_info[ sprite_no ].pos) == 0 ||
           AnimationInfo::doClipping(check_dst_rect, &sprite_info[ sprite_no ].pos) == 0) )
    {
        if ( cell_no >= 0 )
            sprite_info[ sprite_no ].setCell(cell_no);

        sprite_info[ sprite_no ].visible = active_flag;

        dirty_rect.add( sprite_info[ sprite_no ].pos );
    }
}

void ONScripterLabel::createBackground()
{
    bg_effect_image = COLOR_EFFECT_IMAGE;

    if ( !strcmp( bg_info.file_name, "white" ) ){
        bg_info.color[0] = bg_info.color[1] = bg_info.color[2] = 0xff;
    }
    else if ( !strcmp( bg_info.file_name, "black" ) ){
        bg_info.color[0] = bg_info.color[1] = bg_info.color[2] = 0x00;
    }
    else{
        if ( bg_info.file_name[0] == '#' ){
            readColor( &bg_info.color, bg_info.file_name );
        }
        else{
            bg_info.color[0] = bg_info.color[1] = bg_info.color[2] = 0x00;
            setStr( &bg_info.image_name, bg_info.file_name );
            parseTaggedString( &bg_info );
            bg_info.trans_mode = AnimationInfo::TRANS_COPY;
            setupAnimationInfo( &bg_info );
            if (bg_info.image_surface){
                bg_info.pos.x = (screen_width - bg_info.image_surface->w) / 2;
                bg_info.pos.y = (screen_height - bg_info.image_surface->h) / 2;
            }
            bg_effect_image = BG_EFFECT_IMAGE;
        }
    }

    if (bg_effect_image == COLOR_EFFECT_IMAGE){
        SDL_Surface *surface = SDL_CreateRGBSurface( DEFAULT_SURFACE_FLAG,
                                                     screen_width, screen_height,
                                                     32, rmask, gmask, bmask, amask );
        SDL_FillRect( surface, NULL,
                      SDL_MapRGB( surface->format, bg_info.color[0], bg_info.color[1], bg_info.color[2]) );

        bg_info.pos.x = 0;
        bg_info.pos.y = 0;
        bg_info.num_of_cells = 1;
        bg_info.trans_mode = AnimationInfo::TRANS_COPY;
        setupAnimationInfo(&bg_info, NULL, surface);
    }
}

SDL_Surface *ONScripterLabel::rotateSurface90CW(SDL_Surface *surface)
{
    if ( surface == NULL ) return NULL;
    
    SDL_Surface *tmp = SDL_CreateRGBSurface( DEFAULT_SURFACE_FLAG, surface->h, surface->w, 32, rmask, gmask, bmask, amask );
        
    SDL_LockSurface( surface );
    SDL_LockSurface( tmp );

    Uint32 *src = (Uint32 *)surface->pixels;
    for (int i=0 ; i<surface->h ; i++){
        Uint32 *dst = (Uint32 *)tmp->pixels + surface->h - i - 1;
        for (int j=0 ; j<surface->w ; j++){
            *dst = *src++;
            dst += surface->h;
        }
    }
        
    SDL_UnlockSurface( tmp );
    SDL_UnlockSurface( surface );
        
    //SDL_FreeSurface( surface );
    return tmp;
}
