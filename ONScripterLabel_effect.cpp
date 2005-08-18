/* -*- C++ -*-
 * 
 *  ONScripterLabel_effect.cpp - Effect executer of ONScripter
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

#define EFFECT_STRIPE_WIDTH (16 * screen_ratio1 / screen_ratio2)
#define EFFECT_STRIPE_CURTAIN_WIDTH (24 * screen_ratio1 / screen_ratio2)
#define EFFECT_QUAKE_AMP (12 * screen_ratio1 / screen_ratio2)

int ONScripterLabel::setEffect( int effect_no )
{
    if ( effect_no == 0 ) return RET_CONTINUE;

    effect_counter = 0;
    event_mode = EFFECT_EVENT_MODE;
    advancePhase();
    
    return RET_WAIT | RET_REREAD;
}

int ONScripterLabel::doEffect( int effect_no, AnimationInfo *anim, int effect_image )
{
    effect_start_time = SDL_GetTicks();
    if ( effect_counter == 0 ) effect_start_time_old = effect_start_time - 1;
    //printf("effect_counter %d timer between %d %d\n",effect_counter,effect_start_time,effect_start_time_old);
    effect_timer_resolution = effect_start_time - effect_start_time_old;
    effect_start_time_old = effect_start_time;
    
    EffectLink *effect = getEffect( effect_no );

    effect_no = effect->effect;
    if ( effect_cut_flag && skip_flag ) effect_no = 1;
    
    if ( effect_counter == 0 ){
        blitSurface( accumulation_surface, NULL, effect_src_surface, NULL );
        copyTexture(effect_src_id);
        if ( effect_no == 15 || effect_no == 18 ){
            saveTexture( effect_src_surface );
        }
        
        if ( need_refresh_flag ) refreshSurfaceParameters();
        switch( effect_image ){
          case DIRECT_EFFECT_IMAGE:
            copyTexture(effect_dst_id);
            break;
            
          case COLOR_EFFECT_IMAGE:
          case BG_EFFECT_IMAGE:
          case TACHI_EFFECT_IMAGE:
            if (effect_no == 1){
                refreshSurface( effect_dst_surface, &dirty_rect.bounding_box, refreshMode() );
                copyTexture(effect_dst_id);
            }
            else{
                refreshSurface( effect_dst_surface, NULL, refreshMode() );
                copyTexture(effect_dst_id);
                if ( effect_no == 15 || effect_no == 18 ){
                    saveTexture( effect_dst_surface );
                }
            }
            break;
        }

        /* Load mask image */
        if ( effect_no == 15 || effect_no == 18 ){
            if ( !effect->anim.image_surface ){
                parseTaggedString( &effect->anim );
                setupAnimationInfo( &effect->anim );
            }
        }
        if ( effect_no == 11 || effect_no == 12 || effect_no == 13 || effect_no == 14 ||
             effect_no == 16 || effect_no == 17 )
            dirty_rect.fill( screen_width, screen_height );
    }

#ifdef USE_OPENGL
    glMatrixMode(GL_MODELVIEW) ;
    glPushMatrix();
	glLoadIdentity() ;
#endif
    
    int i;
    int width, width2;
    int height, height2;
    SDL_Rect src_rect={0, 0, screen_width, screen_height};
    SDL_Rect dst_rect={0, 0, screen_width, screen_height};

    /* ---------------------------------------- */
    /* Execute effect */
    //printf("Effect number %d %d\n", effect_no, effect->duration );

    switch ( effect_no ){
      case 0: // Instant display
      case 1: // Instant display
        //drawEffect( &src_rect, &src_rect, effect_dst_surface );
        break;

      case 2: // Left shutter
#ifdef USE_OPENGL        
        drawEffect(&src_rect, &src_rect, effect_src_surface);
#endif        
        width = EFFECT_STRIPE_WIDTH * effect_counter / effect->duration;
        for ( i=0 ; i<screen_width/EFFECT_STRIPE_WIDTH ; i++ ){
            src_rect.x = i * EFFECT_STRIPE_WIDTH;
            src_rect.y = 0;
            src_rect.w = width;
            src_rect.h = screen_height;
            drawEffect(&src_rect, &src_rect, effect_dst_surface);
        }
        break;

      case 3: // Right shutter
#ifdef USE_OPENGL        
        drawEffect(&src_rect, &src_rect, effect_src_surface);
#endif        
        width = EFFECT_STRIPE_WIDTH * effect_counter / effect->duration;
        for ( i=1 ; i<=screen_width/EFFECT_STRIPE_WIDTH ; i++ ){
            src_rect.x = i * EFFECT_STRIPE_WIDTH - width - 1;
            src_rect.y = 0;
            src_rect.w = width;
            src_rect.h = screen_height;
            drawEffect(&src_rect, &src_rect, effect_dst_surface);
        }
        break;

      case 4: // Top shutter
#ifdef USE_OPENGL        
        drawEffect(&src_rect, &src_rect, effect_src_surface);
#endif        
        height = EFFECT_STRIPE_WIDTH * effect_counter / effect->duration;
        for ( i=0 ; i<screen_height/EFFECT_STRIPE_WIDTH ; i++ ){
            src_rect.x = 0;
            src_rect.y = i * EFFECT_STRIPE_WIDTH;
            src_rect.w = screen_width;
            src_rect.h = height;
            drawEffect(&src_rect, &src_rect, effect_dst_surface);
        }
        break;

      case 5: // Bottom shutter
#ifdef USE_OPENGL        
        drawEffect(&src_rect, &src_rect, effect_src_surface);
#endif        
        height = EFFECT_STRIPE_WIDTH * effect_counter / effect->duration;
        for ( i=1 ; i<=screen_height/EFFECT_STRIPE_WIDTH ; i++ ){
            src_rect.x = 0;
            src_rect.y = i * EFFECT_STRIPE_WIDTH - height - 1;
            src_rect.w = screen_width;
            src_rect.h = height;
            drawEffect(&src_rect, &src_rect, effect_dst_surface);
        }
        break;

      case 6: // Left curtain
#ifdef USE_OPENGL        
        drawEffect(&src_rect, &src_rect, effect_src_surface);
#endif        
        width = EFFECT_STRIPE_CURTAIN_WIDTH * effect_counter * 2 / effect->duration;
        for ( i=0 ; i<=screen_width/EFFECT_STRIPE_CURTAIN_WIDTH ; i++ ){
            width2 = width - EFFECT_STRIPE_CURTAIN_WIDTH * EFFECT_STRIPE_CURTAIN_WIDTH * i / screen_width;
            if ( width2 >= 0 ){
                src_rect.x = i * EFFECT_STRIPE_CURTAIN_WIDTH;
                src_rect.y = 0;
                src_rect.w = width2;
                src_rect.h = screen_height;
                drawEffect(&src_rect, &src_rect, effect_dst_surface);
            }
        }
        break;

      case 7: // Right curtain
#ifdef USE_OPENGL        
        drawEffect(&src_rect, &src_rect, effect_src_surface);
#endif        
        width = EFFECT_STRIPE_CURTAIN_WIDTH * effect_counter * 2 / effect->duration;
        for ( i=0 ; i<=screen_width/EFFECT_STRIPE_CURTAIN_WIDTH ; i++ ){
            width2 = width - EFFECT_STRIPE_CURTAIN_WIDTH * EFFECT_STRIPE_CURTAIN_WIDTH * i / screen_width;
            if ( width2 >= 0 ){
                if ( width2 > EFFECT_STRIPE_CURTAIN_WIDTH ) width2 = EFFECT_STRIPE_CURTAIN_WIDTH;
                src_rect.x = screen_width - i * EFFECT_STRIPE_CURTAIN_WIDTH - width2;
                src_rect.y = 0;
                src_rect.w = width2;
                src_rect.h = screen_height;
                drawEffect(&src_rect, &src_rect, effect_dst_surface);
            }
        }
        break;

      case 8: // Top curtain
#ifdef USE_OPENGL        
        drawEffect(&src_rect, &src_rect, effect_src_surface);
#endif        
        height = EFFECT_STRIPE_CURTAIN_WIDTH * effect_counter * 2 / effect->duration;
        for ( i=0 ; i<=screen_height/EFFECT_STRIPE_CURTAIN_WIDTH ; i++ ){
            height2 = height - EFFECT_STRIPE_CURTAIN_WIDTH * EFFECT_STRIPE_CURTAIN_WIDTH * i / screen_height;
            if ( height2 >= 0 ){
                src_rect.x = 0;
                src_rect.y = i * EFFECT_STRIPE_CURTAIN_WIDTH;
                src_rect.w = screen_width;
                src_rect.h = height2;
                drawEffect(&src_rect, &src_rect, effect_dst_surface);
            }
        }
        break;

      case 9: // Bottom curtain
#ifdef USE_OPENGL        
        drawEffect(&src_rect, &src_rect, effect_src_surface);
#endif        
        height = EFFECT_STRIPE_CURTAIN_WIDTH * effect_counter * 2 / effect->duration;
        for ( i=0 ; i<=screen_height/EFFECT_STRIPE_CURTAIN_WIDTH ; i++ ){
            height2 = height - EFFECT_STRIPE_CURTAIN_WIDTH * EFFECT_STRIPE_CURTAIN_WIDTH * i / screen_height;
            if ( height2 >= 0 ){
                src_rect.x = 0;
                src_rect.y = screen_height - i * EFFECT_STRIPE_CURTAIN_WIDTH - height2;
                src_rect.w = screen_width;
                src_rect.h = height2;
                drawEffect(&src_rect, &src_rect, effect_dst_surface);
            }
        }
        break;

      default:
        printf("effect No. %d is not implemented. Crossfade is substituted for that.\n",effect_no);
        
      case 10: // Cross fade
#ifdef USE_OPENGL
        drawTexture( effect_src_id, (Rect&)src_rect, (Rect&)src_rect );
        
        glBlendColor_ptr(0.0, 0.0, 0.0, (float)effect_counter / effect->duration);
        glBlendFunc(GL_CONSTANT_ALPHA, GL_ONE_MINUS_CONSTANT_ALPHA);
        drawTexture( effect_dst_id, (Rect&)src_rect, (Rect&)src_rect );
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
#else        
        height = 256 * effect_counter / effect->duration;
        alphaBlend( accumulation_surface, dst_rect,
                    effect_src_surface,
                    effect_dst_surface, 0, 0,
                    NULL, ALPHA_BLEND_CONST, height, &dirty_rect.bounding_box );
#endif        
        break;
        
      case 11: // Left scroll
        width = screen_width * effect_counter / effect->duration;
        src_rect.x = 0;
        dst_rect.x = width;
        src_rect.y = dst_rect.y = 0;
        src_rect.w = dst_rect.w = screen_width - width;
        src_rect.h = dst_rect.h = screen_height;
        drawEffect(&dst_rect, &src_rect, effect_src_surface);
        
        src_rect.x = screen_width - width - 1;
        dst_rect.x = 0;
        src_rect.y = dst_rect.y = 0;
        src_rect.w = dst_rect.w = width;
        src_rect.h = dst_rect.h = screen_height;
        drawEffect(&dst_rect, &src_rect, effect_dst_surface);
        break;

      case 12: // Right scroll
        width = screen_width * effect_counter / effect->duration;
        src_rect.x = width;
        dst_rect.x = 0;
        src_rect.y = dst_rect.y = 0;
        src_rect.w = dst_rect.w = screen_width - width;
        src_rect.h = dst_rect.h = screen_height;
        drawEffect(&dst_rect, &src_rect, effect_src_surface);

        src_rect.x = 0;
        dst_rect.x = screen_width - width - 1;
        src_rect.y = dst_rect.y = 0;
        src_rect.w = dst_rect.w = width;
        src_rect.h = dst_rect.h = screen_height;
        drawEffect(&dst_rect, &src_rect, effect_dst_surface);
        break;

      case 13: // Top scroll
        width = screen_height * effect_counter / effect->duration;
        src_rect.x = dst_rect.x = 0;
        src_rect.y = 0;
        dst_rect.y = width;
        src_rect.w = dst_rect.w = screen_width;
        src_rect.h = dst_rect.h = screen_height - width;
        drawEffect(&dst_rect, &src_rect, effect_src_surface);

        src_rect.x = dst_rect.x = 0;
        src_rect.y = screen_height - width - 1;
        dst_rect.y = 0;
        src_rect.w = dst_rect.w = screen_width;
        src_rect.h = dst_rect.h = width;
        drawEffect(&dst_rect, &src_rect, effect_dst_surface);
        break;

      case 14: // Bottom scroll
        width = screen_height * effect_counter / effect->duration;
        src_rect.x = dst_rect.x = 0;
        src_rect.y = width;
        dst_rect.y = 0;
        src_rect.w = dst_rect.w = screen_width;
        src_rect.h = dst_rect.h = screen_height - width;
        drawEffect(&dst_rect, &src_rect, effect_src_surface);

        src_rect.x = dst_rect.x = 0;
        src_rect.y = 0;
        dst_rect.y = screen_height - width - 1;
        src_rect.w = dst_rect.w = screen_width;
        src_rect.h = dst_rect.h = width;
        drawEffect(&dst_rect, &src_rect, effect_dst_surface);
        break;

      case 15: // Fade with mask
        alphaBlend( accumulation_surface, dst_rect,
                    effect_src_surface,
                    effect_dst_surface, 0, 0,
                    effect->anim.image_surface, ALPHA_BLEND_FADE_MASK, 256 * effect_counter / effect->duration, &dirty_rect.bounding_box );
#ifdef USE_OPENGL        
        loadSubTexture(accumulation_surface, effect_dst_id );
        drawTexture( effect_dst_id, (Rect&)dst_rect, (Rect&)dst_rect );
#endif    
        break;

      case 16: // Mosaic out
        generateMosaic( effect_src_surface, 5 - 6 * effect_counter / effect->duration );
#ifdef USE_OPENGL        
        loadSubTexture(accumulation_surface, effect_dst_id );
        drawTexture( effect_dst_id, (Rect&)dst_rect, (Rect&)dst_rect );
#endif    
        break;
        
      case 17: // Mosaic in
        generateMosaic( effect_dst_surface, 6 * effect_counter / effect->duration );
#ifdef USE_OPENGL        
        loadSubTexture(accumulation_surface, effect_dst_id );
        drawTexture( effect_dst_id, (Rect&)dst_rect, (Rect&)dst_rect );
#endif    
        break;
        
      case 18: // Cross fade with mask
        alphaBlend( accumulation_surface, dst_rect,
                    effect_src_surface,
                    effect_dst_surface, 0, 0,
                    effect->anim.image_surface, ALPHA_BLEND_CROSSFADE_MASK, 256 * effect_counter * 2 / effect->duration, &dirty_rect.bounding_box );
#ifdef USE_OPENGL
        loadSubTexture(accumulation_surface, effect_dst_id );
        drawTexture( effect_dst_id, (Rect&)dst_rect, (Rect&)dst_rect );
#endif    
        break;

      case (CUSTOM_EFFECT_NO + 0 ): // quakey
        if ( effect_timer_resolution > effect->duration / 4 / effect->num )
            effect_timer_resolution = effect->duration / 4 / effect->num;
        dst_rect.x = 0;
        dst_rect.y = (Sint16)(sin(M_PI * 2.0 * effect->num * effect_counter / effect->duration) *
                              EFFECT_QUAKE_AMP * effect->num * (effect->duration -  effect_counter) / effect->duration);
        SDL_FillRect( accumulation_surface, NULL, SDL_MapRGBA( accumulation_surface->format, 0, 0, 0, 0xff ) );
        drawEffect(&dst_rect, &src_rect, effect_dst_surface);
        break;
        
      case (CUSTOM_EFFECT_NO + 1 ): // quakex
        if ( effect_timer_resolution > effect->duration / 4 / effect->num )
            effect_timer_resolution = effect->duration / 4 / effect->num;
        dst_rect.x = (Sint16)(sin(M_PI * 2.0 * effect->num * effect_counter / effect->duration) *
                              EFFECT_QUAKE_AMP * effect->num * (effect->duration -  effect_counter) / effect->duration);
        dst_rect.y = 0;
        drawEffect(&dst_rect, &src_rect, effect_dst_surface);
        break;
        
      case (CUSTOM_EFFECT_NO + 2 ): // quake
        dst_rect.x = effect->num*((int)(3.0*rand()/(RAND_MAX+1.0)) - 1) * 2;
        dst_rect.y = effect->num*((int)(3.0*rand()/(RAND_MAX+1.0)) - 1) * 2;
        SDL_FillRect( accumulation_surface, NULL, SDL_MapRGBA( accumulation_surface->format, 0, 0, 0, 0xff ) );
        drawEffect(&dst_rect, &src_rect, effect_dst_surface);
        break;
    }

    //printf("effect conut %d / dur %d\n", effect_counter, effect->duration);
#ifdef USE_OPENGL
    int err;
    if ((err = glGetError()) != GL_NO_ERROR){
        fprintf(stderr, "effect err: %s\n", gluErrorString(err));
    }
#endif
    
    effect_counter += effect_timer_resolution;
    if ( effect_counter < effect->duration && effect_no != 1 ){
        if ( effect_no != 0 ){
#ifdef USE_OPENGL            
            glPopMatrix();
            SDL_GL_SwapBuffers();
#else
            flush( REFRESH_NONE_MODE, NULL, false );
#endif            
        }
        return RET_WAIT | RET_REREAD;
    }
    else{
        //monocro_flag = false;
        blitSurface( effect_dst_surface, &dirty_rect.bounding_box, accumulation_surface, &dirty_rect.bounding_box );

        if ( effect_no != 0 ){
#ifdef USE_OPENGL            
            src_rect.x = src_rect.y = 0;
            src_rect.w = screen_width;
            src_rect.h = screen_height;
            drawTexture( effect_dst_id, (Rect&)src_rect, (Rect&)src_rect );
            glPopMatrix();
            SDL_GL_SwapBuffers();
            dirty_rect.clear();
#else            
            flush(REFRESH_NONE_MODE);
#endif
        }
        if ( effect_no == 1 ) effect_counter = 0;
        event_mode = IDLE_EVENT_MODE;
        return RET_CONTINUE;
    }
}

void ONScripterLabel::copyTexture(unsigned int tex_id)
{
#ifdef USE_OPENGL
#ifdef USE_GL_TEXTURE_RECTANGLE
    glBindTexture(GL_TEXTURE_RECTANGLE_ARB, tex_id);
    glCopyTexImage2D(GL_TEXTURE_RECTANGLE_ARB, 0,
                     GL_RGBA, 0, 0, screen_texture_width, screen_texture_height, 0);
#else
    glBindTexture(GL_TEXTURE_2D, tex_id);
    glCopyTexImage2D(GL_TEXTURE_2D, 0,
                     GL_RGBA, 0, 0, screen_texture_width, screen_texture_height, 0);
#endif
#endif        
}

void ONScripterLabel::drawEffect(SDL_Rect *dst_rect, SDL_Rect *src_rect, SDL_Surface *surface)
{
#ifdef USE_OPENGL
    if (surface == effect_dst_surface)
        drawTexture( effect_dst_id, (Rect&)*dst_rect, (Rect&)*src_rect );
    else
        drawTexture( effect_src_id, (Rect&)*dst_rect, (Rect&)*src_rect );
#else
    SDL_Rect clipped_rect;
    if (AnimationInfo::doClipping(dst_rect, &dirty_rect.bounding_box, &clipped_rect)) return;
    if (src_rect != dst_rect)
        shiftRect(*src_rect, clipped_rect);
    blitSurface(surface, src_rect, accumulation_surface, dst_rect);
#endif    
}

void ONScripterLabel::generateMosaic( SDL_Surface *src_surface, int level )
{
    int i, j, ii, jj;
    int width = screen_height/3 * screen_ratio1 / screen_ratio2;

    for ( i=0 ; i<level ; i++ ) width >>= 1;

    SDL_LockSurface( src_surface );
    SDL_LockSurface( accumulation_surface );
    Uint32 *src_buffer = (Uint32 *)src_surface->pixels;

    for ( i=0 ; i<screen_height ; i+=width ){
        for ( j=0 ; j<screen_width ; j+=width ){
            Uint32 p = src_buffer[ (i+width-1)*screen_width+j ];
            Uint32 *dst_buffer = (Uint32 *)accumulation_surface->pixels + i*screen_width + j;

            for ( ii=0 ; ii<width ; ii++ ){
                for ( jj=0 ; jj<width ; jj++ ){
                    *dst_buffer++ = p;
                }
                dst_buffer += screen_width - width;
            }
        }
    }
    
    SDL_UnlockSurface( accumulation_surface );
    SDL_UnlockSurface( src_surface );
}
