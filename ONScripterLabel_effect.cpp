/* -*- C++ -*-
 * 
 *  ONScripterLabel_effect.cpp - Effect executer of ONScripter
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

#include "ONScripterLabel.h"

#define EFFECT_STRIPE_WIDTH (16 * screen_ratio1 / screen_ratio2)
#define EFFECT_STRIPE_CURTAIN_WIDTH (24 * screen_ratio1 / screen_ratio2)
#define EFFECT_QUAKE_AMP (12 * screen_ratio1 / screen_ratio2)

int ONScripterLabel::setEffect( int effect_no )
{
    if ( effect_no == 0 ){
        return RET_CONTINUE;
    }

    effect_counter = 0;
    event_mode = EFFECT_EVENT_MODE;
    advancePhase();
    //return RET_WAIT_NEXT;
    return RET_WAIT; // RET_WAIT de yoi?
}

int ONScripterLabel::doEffect( int effect_no, AnimationInfo *anim, int effect_image )
{
    effect_start_time = SDL_GetTicks();
    if ( effect_counter == 0 ) effect_start_time_old = effect_start_time - 1;
    //printf("effect_counter %d timer between %d %d\n",effect_counter,effect_start_time,effect_start_time_old);
    effect_timer_resolution = effect_start_time - effect_start_time_old;
    effect_start_time_old = effect_start_time;
    
    int i;
    int width, width2;
    int height, height2;
    SDL_Rect src_rect, dst_rect, clipped_rect;

    EffectLink *effect = getEffect( effect_no );

    effect_no = effect->effect;
    if ( effect_cut_flag && skip_flag ) effect_no = 1;
    
    if ( effect_counter == 0 ){
        SDL_BlitSurface( text_surface, NULL, effect_src_surface, NULL );

        if ( need_refresh_flag ) refreshSurfaceParameters();
        switch( effect_image ){
          case DIRECT_EFFECT_IMAGE:
            break;
            
          case COLOR_EFFECT_IMAGE:
          case BG_EFFECT_IMAGE:
          case TACHI_EFFECT_IMAGE:
            refreshSurface( effect_dst_surface,
                            NULL,
                            isTextVisible()?REFRESH_SHADOW_MODE:REFRESH_NORMAL_MODE );
            if ( isTextVisible() )
                restoreTextBuffer( effect_dst_surface );
            break;
        }

        /* Load mask image */
        if ( effect_no == 15 || effect_no == 18 ){
            if ( !effect->anim.image_surface ){
                parseTaggedString( &effect->anim );
                setupAnimationInfo( &effect->anim );
            }
        }
        if ( effect_no == 16 || effect_no == 17 )
            dirty_rect.fill( screen_width, screen_height );
    }

    /* ---------------------------------------- */
    /* Execute effect */
    //printf("Effect number %d %d\n", effect_no, effect->duration );
    switch ( effect_no ){
      case 0: // Instant display
      case 1: // Instant display
        SDL_BlitSurface( effect_dst_surface, &dirty_rect.bounding_box, text_surface, &dirty_rect.bounding_box );
        break;

      case 2: // Left shutter
        width = EFFECT_STRIPE_WIDTH * effect_counter / effect->duration;
        for ( i=0 ; i<screen_width/EFFECT_STRIPE_WIDTH ; i++ ){
            src_rect.x = i * EFFECT_STRIPE_WIDTH;
            src_rect.y = 0;
            src_rect.w = width;
            src_rect.h = screen_height;
            doClipping( &src_rect, &dirty_rect.bounding_box );
            SDL_BlitSurface( effect_dst_surface, &src_rect, text_surface, &src_rect );
        }
        break;

      case 3: // Right shutter
        width = EFFECT_STRIPE_WIDTH * effect_counter / effect->duration;
        for ( i=1 ; i<=screen_width/EFFECT_STRIPE_WIDTH ; i++ ){
            src_rect.x = i * EFFECT_STRIPE_WIDTH - width - 1;
            src_rect.y = 0;
            src_rect.w = width;
            src_rect.h = screen_height;
            doClipping( &src_rect, &dirty_rect.bounding_box );
            SDL_BlitSurface( effect_dst_surface, &src_rect, text_surface, &src_rect );
        }
        break;

      case 4: // Top shutter
        height = EFFECT_STRIPE_WIDTH * effect_counter / effect->duration;
        for ( i=0 ; i<screen_height/EFFECT_STRIPE_WIDTH ; i++ ){
            src_rect.x = 0;
            src_rect.y = i * EFFECT_STRIPE_WIDTH;
            src_rect.w = screen_width;
            src_rect.h = height;
            doClipping( &src_rect, &dirty_rect.bounding_box );
            SDL_BlitSurface( effect_dst_surface, &src_rect, text_surface, &src_rect );
        }
        break;

      case 5: // Bottom shutter
        height = EFFECT_STRIPE_WIDTH * effect_counter / effect->duration;
        for ( i=1 ; i<=screen_height/EFFECT_STRIPE_WIDTH ; i++ ){
            src_rect.x = 0;
            src_rect.y = i * EFFECT_STRIPE_WIDTH - height - 1;
            src_rect.w = screen_width;
            src_rect.h = height;
            doClipping( &src_rect, &dirty_rect.bounding_box );
            SDL_BlitSurface( effect_dst_surface, &src_rect, text_surface, &src_rect );
        }
        break;

      case 6: // Left curtain
        width = EFFECT_STRIPE_CURTAIN_WIDTH * effect_counter * 2 / effect->duration;
        for ( i=0 ; i<=screen_width/EFFECT_STRIPE_CURTAIN_WIDTH ; i++ ){
            width2 = width - EFFECT_STRIPE_CURTAIN_WIDTH * EFFECT_STRIPE_CURTAIN_WIDTH * i / screen_width;
            if ( width2 >= 0 ){
                src_rect.x = i * EFFECT_STRIPE_CURTAIN_WIDTH;
                src_rect.y = 0;
                src_rect.w = width2;
                src_rect.h = screen_height;
                doClipping( &src_rect, &dirty_rect.bounding_box );
                SDL_BlitSurface( effect_dst_surface, &src_rect, text_surface, &src_rect );
            }
        }
        break;

      case 7: // Right curtain
        width = EFFECT_STRIPE_CURTAIN_WIDTH * effect_counter * 2 / effect->duration;
        for ( i=0 ; i<=screen_width/EFFECT_STRIPE_CURTAIN_WIDTH ; i++ ){
            width2 = width - EFFECT_STRIPE_CURTAIN_WIDTH * EFFECT_STRIPE_CURTAIN_WIDTH * i / screen_width;
            if ( width2 >= 0 ){
                if ( width2 > EFFECT_STRIPE_CURTAIN_WIDTH ) width2 = EFFECT_STRIPE_CURTAIN_WIDTH;
                src_rect.x = screen_width - i * EFFECT_STRIPE_CURTAIN_WIDTH - width2;
                src_rect.y = 0;
                src_rect.w = width2;
                src_rect.h = screen_height;
                doClipping( &src_rect, &dirty_rect.bounding_box );
                SDL_BlitSurface( effect_dst_surface, &src_rect, text_surface, &src_rect );
            }
        }
        break;

      case 8: // Top curtain
        height = EFFECT_STRIPE_CURTAIN_WIDTH * effect_counter * 2 / effect->duration;
        for ( i=0 ; i<=screen_height/EFFECT_STRIPE_CURTAIN_WIDTH ; i++ ){
            height2 = height - EFFECT_STRIPE_CURTAIN_WIDTH * EFFECT_STRIPE_CURTAIN_WIDTH * i / screen_height;
            if ( height2 >= 0 ){
                src_rect.x = 0;
                src_rect.y = i * EFFECT_STRIPE_CURTAIN_WIDTH;
                src_rect.w = screen_width;
                src_rect.h = height2;
                doClipping( &src_rect, &dirty_rect.bounding_box );
                SDL_BlitSurface( effect_dst_surface, &src_rect, text_surface, &src_rect );
            }
        }
        break;

      case 9: // Bottom curtain
        height = EFFECT_STRIPE_CURTAIN_WIDTH * effect_counter * 2 / effect->duration;
        for ( i=0 ; i<=screen_height/EFFECT_STRIPE_CURTAIN_WIDTH ; i++ ){
            height2 = height - EFFECT_STRIPE_CURTAIN_WIDTH * EFFECT_STRIPE_CURTAIN_WIDTH * i / screen_height;
            if ( height2 >= 0 ){
                src_rect.x = 0;
                src_rect.y = screen_height - i * EFFECT_STRIPE_CURTAIN_WIDTH - height2;
                src_rect.w = screen_width;
                src_rect.h = height2;
                doClipping( &src_rect, &dirty_rect.bounding_box );
                SDL_BlitSurface( effect_dst_surface, &src_rect, text_surface, &src_rect );
            }
        }
        break;

      default:
        printf("effect No. %d is not implemented. Crossfade is substituted for that.\n",effect_no);
        
      case 10: // Cross fade
        height = 256 * effect_counter / effect->duration;
        dst_rect.x = dst_rect.y = 0;
        dst_rect.w = screen_width;
        dst_rect.h = screen_height;
        alphaBlend( text_surface, dst_rect,
                    effect_src_surface, 0, 0,
                    effect_dst_surface, 0, 0,
                    NULL, 0, AnimationInfo::TRANS_COPY, height, &dirty_rect.bounding_box );
        break;
        
      case 11: // Left scroll
        width = screen_width * effect_counter / effect->duration;
        src_rect.x = 0;
        dst_rect.x = width;
        src_rect.y = dst_rect.y = 0;
        src_rect.w = dst_rect.w = screen_width - width;
        src_rect.h = dst_rect.h = screen_height;
        doClipping( &dst_rect, &dirty_rect.bounding_box, &clipped_rect );
        shiftRect( src_rect, clipped_rect );
        SDL_BlitSurface( effect_src_surface, &src_rect, text_surface, &dst_rect );
        
        src_rect.x = screen_width - width - 1;
        dst_rect.x = 0;
        src_rect.y = dst_rect.y = 0;
        src_rect.w = dst_rect.w = width;
        src_rect.h = dst_rect.h = screen_height;
        doClipping( &dst_rect, &dirty_rect.bounding_box, &clipped_rect );
        shiftRect( src_rect, clipped_rect );
        SDL_BlitSurface( effect_dst_surface, &src_rect, text_surface, &dst_rect );
        break;

      case 12: // Right scroll
        width = screen_width * effect_counter / effect->duration;
        src_rect.x = width;
        dst_rect.x = 0;
        src_rect.y = dst_rect.y = 0;
        src_rect.w = dst_rect.w = screen_width - width;
        src_rect.h = dst_rect.h = screen_height;
        doClipping( &dst_rect, &dirty_rect.bounding_box, &clipped_rect );
        shiftRect( src_rect, clipped_rect );
        SDL_BlitSurface( effect_src_surface, &src_rect, text_surface, &dst_rect );

        src_rect.x = 0;
        dst_rect.x = screen_width - width - 1;
        src_rect.y = dst_rect.y = 0;
        src_rect.w = dst_rect.w = width;
        src_rect.h = dst_rect.h = screen_height;
        doClipping( &dst_rect, &dirty_rect.bounding_box, &clipped_rect );
        shiftRect( src_rect, clipped_rect );
        SDL_BlitSurface( effect_dst_surface, &src_rect, text_surface, &dst_rect );
        break;

      case 13: // Top scroll
        width = screen_height * effect_counter / effect->duration;
        src_rect.x = dst_rect.x = 0;
        src_rect.y = 0;
        dst_rect.y = width;
        src_rect.w = dst_rect.w = screen_width;
        src_rect.h = dst_rect.h = screen_height - width;
        doClipping( &dst_rect, &dirty_rect.bounding_box, &clipped_rect );
        shiftRect( src_rect, clipped_rect );
        SDL_BlitSurface( effect_src_surface, &src_rect, text_surface, &dst_rect );

        src_rect.x = dst_rect.x = 0;
        src_rect.y = screen_height - width - 1;
        dst_rect.y = 0;
        src_rect.w = dst_rect.w = screen_width;
        src_rect.h = dst_rect.h = width;
        doClipping( &dst_rect, &dirty_rect.bounding_box, &clipped_rect );
        shiftRect( src_rect, clipped_rect );
        SDL_BlitSurface( effect_dst_surface, &src_rect, text_surface, &dst_rect );
        break;

      case 14: // Bottom scroll
        width = screen_height * effect_counter / effect->duration;
        src_rect.x = dst_rect.x = 0;
        src_rect.y = width;
        dst_rect.y = 0;
        src_rect.w = dst_rect.w = screen_width;
        src_rect.h = dst_rect.h = screen_height - width;
        doClipping( &dst_rect, &dirty_rect.bounding_box, &clipped_rect );
        shiftRect( src_rect, clipped_rect );
        SDL_BlitSurface( effect_src_surface, &src_rect, text_surface, &dst_rect );

        src_rect.x = dst_rect.x = 0;
        src_rect.y = 0;
        dst_rect.y = screen_height - width - 1;
        src_rect.w = dst_rect.w = screen_width;
        src_rect.h = dst_rect.h = width;
        SDL_BlitSurface( effect_dst_surface, &src_rect, text_surface, &dst_rect );
        doClipping( &dst_rect, &dirty_rect.bounding_box, &clipped_rect );
        shiftRect( src_rect, clipped_rect );
        break;

      case 15: // Fade with mask
        dst_rect.x = dst_rect.y = 0;
        dst_rect.w = screen_width;
        dst_rect.h = screen_height;
        alphaBlend( text_surface, dst_rect,
                    effect_src_surface, 0, 0,
                    effect_dst_surface, 0, 0,
                    effect->anim.image_surface, 0, AnimationInfo::TRANS_FADE_MASK, 256 * effect_counter / effect->duration, &dirty_rect.bounding_box );
        break;

      case 16: // Mosaic out
        generateMosaic( text_surface, effect_src_surface, 5 - 6 * effect_counter / effect->duration );
        break;
        
      case 17: // Mosaic in
        generateMosaic( text_surface, effect_dst_surface, 6 * effect_counter / effect->duration );
        break;
        
      case 18: // Cross fade with mask
        dst_rect.x = dst_rect.y = 0;
        dst_rect.w = screen_width;
        dst_rect.h = screen_height;
        alphaBlend( text_surface, dst_rect,
                    effect_src_surface, 0, 0,
                    effect_dst_surface, 0, 0,
                    effect->anim.image_surface, 0, AnimationInfo::TRANS_CROSSFADE_MASK, 256 * effect_counter * 2 / effect->duration, &dirty_rect.bounding_box );
        break;

      case (CUSTOM_EFFECT_NO + 0 ): // quakey
        if ( effect_timer_resolution > effect->duration / 4 / effect->num )
            effect_timer_resolution = effect->duration / 4 / effect->num;
        dst_rect.x = 0;
        dst_rect.y = (Sint16)(sin(M_PI * 2.0 * effect->num * effect_counter / effect->duration) *
                              EFFECT_QUAKE_AMP * effect->num * (effect->duration -  effect_counter) / effect->duration);
        SDL_FillRect( text_surface, NULL, SDL_MapRGBA( text_surface->format, 0, 0, 0, 0 ) );
        SDL_BlitSurface( effect_dst_surface, NULL, text_surface, &dst_rect );
        break;
        
      case (CUSTOM_EFFECT_NO + 1 ): // quakex
        if ( effect_timer_resolution > effect->duration / 4 / effect->num )
            effect_timer_resolution = effect->duration / 4 / effect->num;
        dst_rect.x = (Sint16)(sin(M_PI * 2.0 * effect->num * effect_counter / effect->duration) *
                              EFFECT_QUAKE_AMP * effect->num * (effect->duration -  effect_counter) / effect->duration);
        dst_rect.y = 0;
        SDL_FillRect( text_surface, NULL, SDL_MapRGBA( text_surface->format, 0, 0, 0, 0 ) );
        SDL_BlitSurface( effect_dst_surface, NULL, text_surface, &dst_rect );
        break;
        
      case (CUSTOM_EFFECT_NO + 2 ): // quake
        dst_rect.x = effect->num*((int)(3.0*rand()/(RAND_MAX+1.0)) - 1) * 2;
        dst_rect.y = effect->num*((int)(3.0*rand()/(RAND_MAX+1.0)) - 1) * 2;
        SDL_FillRect( text_surface, NULL, SDL_MapRGBA( text_surface->format, 0, 0, 0, 0 ) );
        SDL_BlitSurface( effect_dst_surface, NULL, text_surface, &dst_rect );
        break;
    }

    //printf("effect conut %d / dur %d\n", effect_counter, effect->duration);
        
    effect_counter += effect_timer_resolution;
    if ( effect_counter < effect->duration ){
        if ( effect_no != 0 ){
            flush( NULL, false );
        }
        return RET_WAIT;
    }
    else{
        //monocro_flag = false;
        SDL_BlitSurface( effect_dst_surface, NULL, text_surface, NULL );
        refreshSurface( accumulation_surface, NULL, REFRESH_NORMAL_MODE );

        if ( effect_no != 0 ) flush();
        if ( effect_no == 1 ) effect_counter = 0;
        event_mode = IDLE_EVENT_MODE;
        return RET_CONTINUE;
    }
}

void ONScripterLabel::generateMosaic( SDL_Surface *dst_surface, SDL_Surface *src_surface, int level )
{
    int i, j, ii, jj;
    int width = 160 * screen_ratio1 / screen_ratio2;

    for ( i=0 ; i<level ; i++ ) width >>= 1;

    SDL_LockSurface( src_surface );
    SDL_LockSurface( dst_surface );
    Uint32 *src_buffer = (Uint32 *)src_surface->pixels;

    for ( i=0 ; i<screen_height ; i+=width ){
        for ( j=0 ; j<screen_width ; j+=width ){
            Uint32 p = src_buffer[ (i+width-1)*screen_width+j ];
            Uint32 *dst_buffer = (Uint32 *)dst_surface->pixels + i*screen_width + j;

            for ( ii=0 ; ii<width ; ii++ ){
                for ( jj=0 ; jj<width ; jj++ ){
                    *dst_buffer++ = p;
                }
                dst_buffer += screen_width - width;
            }
        }
    }
    
    SDL_UnlockSurface( dst_surface );
    SDL_UnlockSurface( src_surface );
}
