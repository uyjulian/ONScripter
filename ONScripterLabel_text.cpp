/* -*- C++ -*-
 * 
 *  ONScripterLabel_text.cpp - Text parser of ONScripter
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

#include "ONScripterLabel.h"

extern unsigned short convSJIS2UTF16( unsigned short in );

#define IS_KINSOKU(x)	\
        ( ( *(x) & 0x80 ) && \
          ( *(x) == (char)0x81 && *((x)+1) == (char)0x41 ) || \
          ( *(x) == (char)0x81 && *((x)+1) == (char)0x42 ) || \
          ( *(x) == (char)0x81 && *((x)+1) == (char)0x48 ) || \
          ( *(x) == (char)0x81 && *((x)+1) == (char)0x49 ) || \
          ( *(x) == (char)0x81 && *((x)+1) == (char)0x76 ) || \
          ( *(x) == (char)0x81 && *((x)+1) == (char)0x78 ) || \
          ( *(x) == (char)0x81 && *((x)+1) == (char)0x5b ) )

void ONScripterLabel::drawGlyph( SDL_Surface *dst_surface, char *text, FontInfo *info, SDL_Color &color, unsigned short unicode, int xy[2], int minx, int maxy, bool text_cache_flag, SDL_Rect *clip, SDL_Rect &dst_rect )
{
    dst_rect.x = xy[0] + minx;
    if ( !(text[0] & 0x80) && text[1] ) dst_rect.x += info->pitch_xy[0] / 2 * screen_ratio1 / screen_ratio2;
    dst_rect.y = xy[1] + TTF_FontAscent( (TTF_Font*)info->ttf_font ) - maxy;
    
    SDL_Surface *tmp_surface = TTF_RenderGlyph_Blended( (TTF_Font*)info->ttf_font, unicode, color );

    if ( tmp_surface ){
        dst_rect.w = tmp_surface->w;
        dst_rect.h = tmp_surface->h;
        if ( dst_surface ){
            SDL_Rect src_rect;
            src_rect.x = src_rect.y = 0;
            SDL_Surface *tmp2_surface = tmp_surface;
            if ( text_cache_flag ){
                alphaBlend( text_surface, dst_rect,
                            text_surface, dst_rect.x, dst_rect.y,
                            tmp_surface, 0, 0,
                            NULL, 0, AnimationInfo::TRANS_ALPHA_MULTIPLE, 255, clip );
                src_rect.x = dst_rect.x;
                src_rect.y = dst_rect.y;
                tmp2_surface = text_surface;
            }
            alphaBlend( dst_surface, dst_rect,
                        dst_surface,  dst_rect.x, dst_rect.y,
                        tmp2_surface, src_rect.x, src_rect.y,
                        NULL, src_rect.x, AnimationInfo::TRANS_ALPHA_PRESERVE, 255, clip );
        }
        SDL_FreeSurface( tmp_surface );
    }
}

void ONScripterLabel::drawChar( char* text, FontInfo *info, bool flush_flag, bool lookback_flag, SDL_Surface *surface, bool text_cache_flag, SDL_Rect *clip )
{
    int xy[2];
    SDL_Color color;
    unsigned short index, unicode;
    int minx, maxx, miny, maxy, advanced;

    //printf("draw %x-%x[%s] %d, %d\n", text[0], text[1], text, info->xy[0], info->xy[1] );
    
    if ( info->ttf_font == NULL ){
        if ( info->openFont( font_file, screen_ratio1, screen_ratio2 ) == NULL ){
            fprintf( stderr, "can't open font file: %s\n", font_file );
            quit();
            exit(-1);
        }
    }

    if ( text[0] & 0x80 ){
        index = ((unsigned char*)text)[0];
        index = index << 8 | ((unsigned char*)text)[1];
        unicode = convSJIS2UTF16( index );
        TTF_GlyphMetrics( (TTF_Font*)info->ttf_font, unicode,
                          &minx, &maxx, &miny, &maxy, &advanced );
    }
    else if ( text[1] ){
        unicode = text[1];
        TTF_GlyphMetrics( (TTF_Font*)info->ttf_font, text[1],
                          &minx, &maxx, &miny, &maxy, &advanced );
    }
    else{
        unicode = text[0];
        TTF_GlyphMetrics( (TTF_Font*)info->ttf_font, text[0],
                          &minx, &maxx, &miny, &maxy, &advanced );
    }

    //printf("minx %d maxx %d miny %d maxy %d ace %d\n",minx, maxx, miny, maxy, advanced );

    if ( info->xy[0] >= info->num_xy[0] ) info->newLine();

    xy[0] = info->x() * screen_ratio1 / screen_ratio2;
    xy[1] = info->y() * screen_ratio1 / screen_ratio2;

    SDL_Rect dst_rect;
    if ( info->is_shadow ){
        color.r = color.g = color.b = 0;
        drawGlyph( surface, text, info, color, unicode, xy, minx+1, maxy-1, text_cache_flag, clip, dst_rect );
    }
    color.r = info->color[0];
    color.g = info->color[1];
    color.b = info->color[2];
    drawGlyph( surface, text, info, color, unicode, xy, minx, maxy, text_cache_flag, clip, dst_rect );

    if ( surface == accumulation_surface &&
         !flush_flag &&
         (!clip || doClipping( &dst_rect, clip ) == 0) ){
        dirty_rect.add( dst_rect );
    }
    else if ( flush_flag ){
        dst_rect.w++;
        dst_rect.h++;
        flush( &dst_rect, false, true );
    }

    /* ---------------------------------------- */
    /* Update text buffer */
    info->xy[0] ++;
    if ( lookback_flag ){
        current_text_buffer->addBuffer( text[0] );
        if ( text[0] & 0x80 || text[1] )
            current_text_buffer->addBuffer( text[1] );
        if ( info->xy[0] >= info->num_xy[0] )
            current_text_buffer->addBuffer( 0x0a );
        if ( internal_saveon_flag ){
            internal_saveon_flag = false;
            saveSaveFile(-1);
        }
    }
}

void ONScripterLabel::drawString( const char *str, uchar3 color, FontInfo *info, bool flush_flag, SDL_Surface *surface, SDL_Rect *rect )
{
    int i;
    uchar3 org_color;
    char text[3] = { '\0', '\0', '\0' };
    SDL_Rect clipped_rect;

    int tmp_xy[2];
    tmp_xy[0] = info->xy[0];
    tmp_xy[1] = info->xy[1];

    /* ---------------------------------------- */
    /* Draw selected characters */
    for ( i=0 ; i<3 ; i++ ) org_color[i] = info->color[i];
    for ( i=0 ; i<3 ; i++ ) info->color[i] = color[i];

    while( *str ){
        if ( *str == ' ' ){
            str++;
            continue;
        }
        if ( *str & 0x80 ){
            /* Kinsoku process */
            if ( info->xy[0] + 1 == info->num_xy[0] &&
                 IS_KINSOKU( str+2 ) ) {
                info->newLine();
            }
            text[0] = *str++;
            text[1] = *str++;
            drawChar( text, info, false, false, surface, false );
        }
        else{
            text[0] = *str++;
            text[1] = '\0';
            drawChar( text, info, false, false, surface, false );
            info->xy[0]--;
            if ( *str ){
                text[1] = *str++;
                drawChar( text, info, false, false, surface, false );
            }
        }
    }
    for ( i=0 ; i<3 ; i++ ) info->color[i] = org_color[i];

    /* ---------------------------------------- */
    /* Calculate the area of selection */
    if ( info->getTateyokoMode() == FontInfo::TATE_MODE ){
        clipped_rect.x = (info->top_xy[0] + (info->num_xy[1] - tmp_xy[1] - 1) * info->pitch_xy[0]) * screen_ratio1 / screen_ratio2;
        clipped_rect.w = (info->pitch_xy[0] * (info->xy[1] - tmp_xy[1] + 1)) * screen_ratio1 / screen_ratio2;
        if ( tmp_xy[1] == info->xy[1] ){
            clipped_rect.y = (info->top_xy[1] + tmp_xy[0] * info->pitch_xy[1]) * screen_ratio1 / screen_ratio2;
            clipped_rect.h = (info->pitch_xy[1] * (info->xy[0] - tmp_xy[0] + 1)) * screen_ratio1 / screen_ratio2;
        }
        else{
            clipped_rect.y = info->top_xy[1] * screen_ratio1 / screen_ratio2;
            clipped_rect.h = info->pitch_xy[1] * info->num_xy[0] * screen_ratio1 / screen_ratio2;
        }
    }
    else{
        if ( tmp_xy[1] == info->xy[1] ){
            clipped_rect.x = (info->top_xy[0] + tmp_xy[0] * info->pitch_xy[0]) * screen_ratio1 / screen_ratio2;
            clipped_rect.w = (info->pitch_xy[0] * (info->xy[0] - tmp_xy[0] + 1)) * screen_ratio1 / screen_ratio2;
        }
        else{
            clipped_rect.x = info->top_xy[0] * screen_ratio1 / screen_ratio2;
            clipped_rect.w = info->pitch_xy[0] * info->num_xy[0] * screen_ratio1 / screen_ratio2;
        }
        clipped_rect.y = (tmp_xy[1] * info->pitch_xy[1] + info->top_xy[1]) * screen_ratio1 / screen_ratio2;
        clipped_rect.h = (info->xy[1] - tmp_xy[1] + 1) * info->pitch_xy[1] * screen_ratio1 / screen_ratio2;
    }
    if ( info->is_shadow ){
        clipped_rect.w++;
        clipped_rect.h++;
    }
    
    if ( flush_flag ) flush( &clipped_rect );
    
    if ( rect ) *rect = clipped_rect;
}

void ONScripterLabel::restoreTextBuffer( SDL_Surface *surface, SDL_Rect *clip, bool text_cache_flag )
{
    if ( current_text_buffer == cached_text_buffer ){
        SDL_Rect rect;
        if ( clip ){
            rect = *clip;
        }
        else{
            rect.x = rect.y = 0;
            rect.w = screen_width;
            rect.h = screen_height;
        }
    
        alphaBlend( surface, rect,
                    surface, rect.x, rect.y,
                    text_surface, rect.x, rect.y,
                    NULL, rect.x, AnimationInfo::TRANS_ALPHA_PRESERVE );
    }
    else{
        char out_text[3] = { '\0','\0','\0' };
        FontInfo f_info = sentence_font;
        f_info.clear();
        for ( int i=0 ; i<current_text_buffer->buffer2_count ; i++ ){
            if ( current_text_buffer->buffer2[i] == 0x0a ){
                f_info.newLine();
            }
            else{
                out_text[0] = current_text_buffer->buffer2[i];
                if ( !(out_text[0] & 0x80) ){
                    out_text[1] = '\0';
                    drawChar( out_text, &f_info, false, false, surface, text_cache_flag, clip );
                    f_info.xy[0]--;
                }
                out_text[1] = current_text_buffer->buffer2[i+1];
                i++;
                drawChar( out_text, &f_info, false, false, surface, text_cache_flag, clip );
            }
        }
    }
}

int ONScripterLabel::clickWait( char *out_text )
{
    if ( (skip_flag || draw_one_page_flag || ctrl_pressed_status) && !textgosub_label ){
        clickstr_state = CLICK_NONE;
        if ( out_text ){
            drawChar( out_text, &sentence_font, false, true, accumulation_surface, true );
            num_chars_in_sentence++;
            string_buffer_offset += 2;
        }
        else{ // called on '@'
            flush();
            string_buffer_offset++;
        }
        num_chars_in_sentence = 0;
        // ... below is ugly work-around ...
        if ( script_h.getStringBuffer()[string_buffer_offset] == '\0' )
            return RET_CONTINUE;

        return RET_CONTINUE_NOREAD;
    }
    else{
        clickstr_state = CLICK_WAIT;
        key_pressed_flag = false;
        if ( sentence_font.wait_time == 0 ||
             ( sentence_font.wait_time == -1 && default_text_speed[text_speed_no] == 0 ) ) flush();
        if ( out_text ){
            drawChar( out_text, &sentence_font, true, true, accumulation_surface, true );
            num_chars_in_sentence++;
        }
        if ( textgosub_label ){
            saveoffCommand();
            if ( out_text ) string_buffer_offset += 2;
            else            string_buffer_offset++;
            if ( script_h.getStringBuffer()[string_buffer_offset] == '\0' ){
                script_h.setKidokuskip( false );
                script_h.readToken();
                script_h.setKidokuskip( kidokuskip_flag );
                string_buffer_offset = 0;
            }
            gosubReal( textgosub_label, true, script_h.getCurrent() );
            return RET_JUMP;
        }
        if ( automode_flag ){
            event_mode = WAIT_INPUT_MODE | WAIT_VOICE_MODE;
            if ( automode_time < 0 )
                startTimer( -automode_time * num_chars_in_sentence );
            else
                startTimer( automode_time );
        }
        else if ( autoclick_time > 0 ){
            event_mode = WAIT_SLEEP_MODE;
            startTimer( autoclick_time );
        }
        else{
            event_mode = WAIT_INPUT_MODE | WAIT_TIMER_MODE;
            advancePhase();
        }
        num_chars_in_sentence = 0;
        return RET_WAIT_NOREAD;
    }
}

int ONScripterLabel::clickNewPage( char *out_text )
{
    clickstr_state = CLICK_NEWPAGE;
    if ( out_text ){
        drawChar( out_text, &sentence_font, false, true, accumulation_surface, true );
        num_chars_in_sentence++;
    }
    if ( skip_flag || draw_one_page_flag || sentence_font.wait_time == 0 ||
         ( sentence_font.wait_time == -1 && default_text_speed[text_speed_no] == 0 ) ||
         ctrl_pressed_status ) flush();
    
    if ( (skip_flag || ctrl_pressed_status) && !textgosub_label  ){
        event_mode = WAIT_SLEEP_MODE;
        advancePhase();
        num_chars_in_sentence = 0;
    }
    else{
        key_pressed_flag = false;
        if ( textgosub_label ){
            saveoffCommand();
            if ( out_text ) string_buffer_offset += 2;
            else            string_buffer_offset++;
            if ( script_h.getStringBuffer()[string_buffer_offset] == '\0' ){
                script_h.setKidokuskip( false );
                script_h.readToken();
                script_h.setKidokuskip( kidokuskip_flag );
                string_buffer_offset = 0;
            }
            gosubReal( textgosub_label, false, script_h.getCurrent() );
            new_line_skip_flag = true;
            return RET_JUMP;
        }
        if ( automode_flag ){
            event_mode = WAIT_INPUT_MODE | WAIT_VOICE_MODE;
            if ( automode_time < 0 )
                startTimer( -automode_time * num_chars_in_sentence );
            else
                startTimer( automode_time );
        }
        else if ( autoclick_time > 0 ){
            event_mode = WAIT_SLEEP_MODE;
            startTimer( autoclick_time );
        }
        else /*if ( cursor_info[ CURSOR_NEWPAGE_NO ].num_of_cells > 0 )*/{
            event_mode = WAIT_INPUT_MODE | WAIT_TIMER_MODE;
            advancePhase();
        }
        num_chars_in_sentence = 0;
    }
    return RET_WAIT_NOREAD;
}

int ONScripterLabel::textCommand()
{
    int i, j, ret = enterTextDisplayMode();// RET_WAIT_NOREAD );
    if ( ret != RET_NOMATCH ) return ret;
    
    char out_text[3]= {'\0', '\0', '\0'};

    if ( event_mode & (WAIT_INPUT_MODE | WAIT_SLEEP_MODE) ){
        if ( clickstr_state == CLICK_WAIT ){
            event_mode = IDLE_EVENT_MODE;
            if ( script_h.getStringBuffer()[ string_buffer_offset ] != '@' ) string_buffer_offset++;
            string_buffer_offset++;
            clickstr_state = CLICK_NONE;
            //return RET_CONTINUE;
        }
        else if ( clickstr_state == CLICK_NEWPAGE ){
            event_mode = IDLE_EVENT_MODE;
            if ( script_h.getStringBuffer()[ string_buffer_offset ] != '\\' ) string_buffer_offset++;
            string_buffer_offset++;
            newPage( true );
            new_line_skip_flag = true;
            clickstr_state = CLICK_NONE;
            return RET_CONTINUE_NOREAD;
        }
        else if ( script_h.getStringBuffer()[ string_buffer_offset ] & 0x80 ){
            string_buffer_offset += 2;
        }
        else if ( script_h.getStringBuffer()[ string_buffer_offset ] == '!' ){
            string_buffer_offset++;
            if ( script_h.getStringBuffer()[ string_buffer_offset ] == 'w' || script_h.getStringBuffer()[ string_buffer_offset ] == 'd' ){
                string_buffer_offset++;
                while ( script_h.getStringBuffer()[ string_buffer_offset ] >= '0' &&
                        script_h.getStringBuffer()[ string_buffer_offset ] <= '9' )
                    string_buffer_offset++;
            }
        }
        else if ( script_h.getStringBuffer()[ string_buffer_offset+1 ] ){
            string_buffer_offset += 2;
        }
        else
            string_buffer_offset++;

        event_mode = IDLE_EVENT_MODE;
    }

    if ( script_h.getStringBuffer()[string_buffer_offset] == '\0' ) return RET_CONTINUE;
    new_line_skip_flag = false;
    
    //printf("*** textCommand %d (%d,%d)\n", string_buffer_offset, sentence_font.xy[0], sentence_font.xy[1]);
    
    while( script_h.getStringBuffer()[ string_buffer_offset ] == ' ' ||
           script_h.getStringBuffer()[ string_buffer_offset ] == '\t' ) string_buffer_offset ++;
    char ch = script_h.getStringBuffer()[string_buffer_offset];
    if ( ch & 0x80 ){ // Shift jis
        /* ---------------------------------------- */
        /* Kinsoku process */
        if ( sentence_font.xy[0] + 1 == sentence_font.num_xy[0] &&
             IS_KINSOKU( script_h.getStringBuffer() + string_buffer_offset + 2 ) ){
            sentence_font.newLine();
            current_text_buffer->addBuffer( 0x0a );
        }
        
        out_text[0] = script_h.getStringBuffer()[string_buffer_offset];
        out_text[1] = script_h.getStringBuffer()[string_buffer_offset+1];
        if ( clickstr_state == CLICK_IGNORE ){
            clickstr_state = CLICK_NONE;
        }
        else{
            clickstr_state = CLICK_NONE;
            for ( i=0 ; i<clickstr_num ; i++ ){
                if ( clickstr_list[i*2] == out_text[0] && clickstr_list[i*2+1] == out_text[1] ){
                    if ( sentence_font.xy[1] >= sentence_font.num_xy[1] - clickstr_line ){
                        if ( script_h.getStringBuffer()[string_buffer_offset+2] != '@' && script_h.getStringBuffer()[string_buffer_offset+2] != '\\' ){
                            clickstr_state = CLICK_NEWPAGE;
                        }
                    }
                    else{
                        if ( script_h.getStringBuffer()[string_buffer_offset+2] != '@' && script_h.getStringBuffer()[string_buffer_offset+2] != '\\' ){
                            clickstr_state = CLICK_WAIT;
                        }
                    }
                    for ( j=0 ; j<clickstr_num ; j++ ){
                        if ( clickstr_list[j*2] == script_h.getStringBuffer()[string_buffer_offset+2] &&
                             clickstr_list[j*2+1] == script_h.getStringBuffer()[string_buffer_offset+3] ){
                            clickstr_state = CLICK_NONE;
                        }
                    }
                    if ( script_h.getStringBuffer()[string_buffer_offset+2] == '!' ){
                        if ( script_h.getStringBuffer()[string_buffer_offset+3] == 'w' ||
                             script_h.getStringBuffer()[string_buffer_offset+3] == 'd' )
                            clickstr_state = CLICK_NONE;
                    }
                }
            }
        }
        if ( clickstr_state == CLICK_WAIT ){
            return clickWait( out_text );
        }
        else if ( clickstr_state == CLICK_NEWPAGE ){
            return clickNewPage( out_text );
        }
        else{
            if ( skip_flag || draw_one_page_flag || sentence_font.wait_time == 0 ||
                 ( sentence_font.wait_time == -1 && default_text_speed[text_speed_no] == 0 ) ||
                 ctrl_pressed_status ){
                drawChar( out_text, &sentence_font, false, true, accumulation_surface, true );
                num_chars_in_sentence++;
                
                string_buffer_offset += 2;
                return RET_CONTINUE_NOREAD;
            }
            else{
                drawChar( out_text, &sentence_font, true, true, accumulation_surface, true );
                num_chars_in_sentence++;
                event_mode = WAIT_SLEEP_MODE;
                if ( sentence_font.wait_time == -1 )
                    advancePhase( default_text_speed[text_speed_no] );
                else
                    advancePhase( sentence_font.wait_time );
                return RET_WAIT_NOREAD;
            }
        }
    }
    else if ( ch == '(' ){
        int i=0;

        while( (ch = script_h.getStringBuffer()[string_buffer_offset+i]) != '\0' ){
            if ( ch == '(' ){
                ruby_struct.stage = RubyStruct::BODY;
                ruby_font = sentence_font;
                ruby_font.ttf_font = NULL;
                if ( ruby_struct.font_size_xy[0] != -1 )
                    ruby_font.font_size_xy[0] = ruby_struct.font_size_xy[0];
                else
                    ruby_font.font_size_xy[0] = sentence_font.font_size_xy[0]/2;
                if ( ruby_struct.font_size_xy[1] != -1 )
                    ruby_font.font_size_xy[1] = ruby_struct.font_size_xy[1];
                else
                    ruby_font.font_size_xy[1] = sentence_font.font_size_xy[1]/2;
                
                ruby_struct.body_count = 0;
                ruby_struct.ruby_count = 0;
                ruby_struct.ruby_end = 0;
            }
            else if ( ch == '/' ){
                ruby_struct.stage = RubyStruct::RUBY;
                ruby_struct.ruby_start = string_buffer_offset + i + 1;
            }
            else if ( ch == ')' ){
                ruby_struct.ruby_end = string_buffer_offset + i;
                break;
            }
            else{
                if ( ruby_struct.stage == RubyStruct::BODY )
                    ruby_struct.body_count++;
                else if ( ruby_struct.stage == RubyStruct::RUBY )
                    ruby_struct.ruby_count++;
            }
            i++;
        }
        ruby_struct.stage = RubyStruct::BODY;
        
        ruby_font.top_xy[0] = sentence_font.x();
        ruby_font.top_xy[1] = sentence_font.y() - ruby_font.font_size_xy[1];
        ruby_font.y_offset = 0;
        ruby_font.num_xy[0] = ruby_struct.ruby_count/2;
        ruby_font.num_xy[1] = 1;
        ruby_font.clear();
        if ( ruby_struct.ruby_count/2*ruby_font.font_size_xy[0] >= ruby_struct.body_count/2*sentence_font.pitch_xy[0] ){
            ruby_struct.x_margin = (ruby_struct.ruby_count/2*ruby_font.font_size_xy[0] - ruby_struct.body_count/2*sentence_font.pitch_xy[0] + 1)/2;
            ruby_font.x_offset = 0;
            ruby_font.pitch_xy[0] = ruby_font.font_size_xy[0];
        }
        else{
            ruby_struct.x_margin = 0;
            ruby_font.x_offset = (ruby_struct.body_count/2*sentence_font.pitch_xy[0] - ruby_struct.ruby_count/2*ruby_font.font_size_xy[0] + ruby_struct.ruby_count/2)
                / ruby_struct.ruby_count;
            ruby_font.pitch_xy[0] = ruby_font.x_offset*2+ruby_font.font_size_xy[0];
        }
        sentence_font.x_offset += ruby_struct.x_margin;
        string_buffer_offset++;
        return RET_CONTINUE_NOREAD;
    }
    else if ( ch == ')' ){
        if ( rubyon_flag ){
            bool flush_flag = true;
            if ( skip_flag || draw_one_page_flag || sentence_font.wait_time == 0 ||
                 ( sentence_font.wait_time == -1 && default_text_speed[text_speed_no] == 0 ) ||
                 ctrl_pressed_status )
                flush_flag = false;
        
            ruby_font.xy[0] = ruby_font.xy[1] = 0;
            for ( i=ruby_struct.ruby_start ; i<string_buffer_offset ; i++ ){
                out_text[0] = script_h.getStringBuffer()[i];
                if ( script_h.getStringBuffer()[i] & 0x80 ){
                    out_text[1] = script_h.getStringBuffer()[i+1];
                    drawChar( out_text, &ruby_font, flush_flag, false, accumulation_surface, true );
                    i++;
                }
                else{
                    out_text[1] = '\0';
                    drawChar( out_text, &ruby_font, flush_flag, false, accumulation_surface, true );
                    if ( script_h.getStringBuffer()[i+1] ){
                        out_text[1] = script_h.getStringBuffer()[i+1];
                        drawChar( out_text, &ruby_font, flush_flag, false, accumulation_surface, true );
                        i++;
                    }
                }
            }
        }
        ruby_struct.stage = RubyStruct::NONE;
        string_buffer_offset++;
        return RET_CONTINUE_NOREAD;
    }
    else if ( ch == '@' ){ // wait for click
        return clickWait( NULL );
    }
    else if ( ch == '/' ){
        if ( ruby_struct.stage == RubyStruct::BODY ){
            sentence_font.x_offset += ruby_struct.x_margin;
            if ( ruby_struct.ruby_end != 0 ){
                ruby_struct.stage = RubyStruct::RUBY;
                string_buffer_offset = ruby_struct.ruby_end;
            }
            else{
                ruby_struct.stage = RubyStruct::NONE;
                while( script_h.getStringBuffer()[string_buffer_offset] != '\0' )
                    string_buffer_offset++;
            }
        }
        else{ // skip new line
            new_line_skip_flag = true;
            string_buffer_offset++;
        }
        return RET_CONTINUE_NOREAD;
    }
    else if ( ch == '\\' ){ // new page
        return clickNewPage( NULL );
    }
    else if ( ch == '!' ){
        string_buffer_offset++;
        if ( script_h.getStringBuffer()[ string_buffer_offset ] == 's' ){
            string_buffer_offset++;
            if ( script_h.getStringBuffer()[ string_buffer_offset ] == 'd' ){
                sentence_font.wait_time = -1;
                string_buffer_offset++;
            }
            else{
                int t = 0;
                while( script_h.getStringBuffer()[ string_buffer_offset ] >= '0' &&
                       script_h.getStringBuffer()[ string_buffer_offset ] <= '9' ){
                    t = t*10 + script_h.getStringBuffer()[ string_buffer_offset ] - '0';
                    string_buffer_offset++;
                }
                sentence_font.wait_time = t;
            }
        }
        else if ( script_h.getStringBuffer()[ string_buffer_offset ] == 'w' || script_h.getStringBuffer()[ string_buffer_offset ] == 'd' ){
            bool flag = false;
            if ( script_h.getStringBuffer()[ string_buffer_offset ] == 'd' ) flag = true;
            string_buffer_offset++;
            int tmp_string_buffer_offset = string_buffer_offset;
            int t = 0;
            while( script_h.getStringBuffer()[ string_buffer_offset ] >= '0' &&
                   script_h.getStringBuffer()[ string_buffer_offset ] <= '9' ){
                t = t*10 + script_h.getStringBuffer()[ string_buffer_offset ] - '0';
                string_buffer_offset++;
            }
            if ( skip_flag || draw_one_page_flag || ctrl_pressed_status ){
                return RET_CONTINUE_NOREAD;
            }
            else{
                event_mode = WAIT_SLEEP_MODE;
                if ( flag ) event_mode |= WAIT_INPUT_MODE;
                key_pressed_flag = false;
                startTimer( t );
                string_buffer_offset = tmp_string_buffer_offset - 2;
                return RET_WAIT;
            }
        }
        return RET_CONTINUE_NOREAD;
    }
    else if ( ch == '_' ){ // Ignore following forced return
        clickstr_state = CLICK_IGNORE;
        string_buffer_offset++;
        return RET_CONTINUE_NOREAD;
    }
    else if ( ch == '\0' ){ // End of line
        printf("end of text\n");
        return RET_CONTINUE;
    }
    else if ( ch == '#' ){
        readColor( &sentence_font.color, script_h.getStringBuffer() + string_buffer_offset );
        readColor( &ruby_font.color, script_h.getStringBuffer() + string_buffer_offset );
        string_buffer_offset += 7;
        return RET_CONTINUE_NOREAD;
    }
    else{
        bool flush_flag = true;
        if ( skip_flag || draw_one_page_flag || sentence_font.wait_time == 0 ||
             ( sentence_font.wait_time == -1 && default_text_speed[text_speed_no] == 0 ) ||
             ctrl_pressed_status )
            flush_flag = false;
        out_text[0] = ch;
        drawChar( out_text, &sentence_font, flush_flag, true, accumulation_surface, true );
        sentence_font.xy[0]--;
        if ( script_h.getStringBuffer()[ string_buffer_offset + 1 ] ){
            out_text[1] = script_h.getStringBuffer()[ string_buffer_offset + 1 ];
            drawChar( out_text, &sentence_font, flush_flag, true, accumulation_surface, true );
        }
        num_chars_in_sentence++;
        if ( skip_flag || draw_one_page_flag || sentence_font.wait_time == 0 ||
             ( sentence_font.wait_time == -1 && default_text_speed[text_speed_no] == 0 ) ||
             ctrl_pressed_status ){
            if ( script_h.getStringBuffer()[ string_buffer_offset + 1 ] ) string_buffer_offset++;
            string_buffer_offset++;
            return RET_CONTINUE_NOREAD;
        }
        else{
            event_mode = WAIT_SLEEP_MODE;
            if ( sentence_font.wait_time == -1 )
                advancePhase( default_text_speed[text_speed_no] );
            else
                advancePhase( sentence_font.wait_time );
            return RET_WAIT_NOREAD;
        }
    }

    return RET_NOMATCH;
}
