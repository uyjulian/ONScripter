/* -*- C++ -*-
 * 
 *  ONScripterLabel_event.cpp - Event handler of ONScripter
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

#include "ONScripterLabel.h"
#if defined(LINUX)
#include <wait.h>
#endif

#define ONS_TIMER_EVENT   (SDL_USEREVENT)
#define ONS_SOUND_EVENT   (SDL_USEREVENT+1)
#define ONS_CDAUDIO_EVENT (SDL_USEREVENT+2)
#define ONS_MIDI_EVENT    (SDL_USEREVENT+3)

#define EDIT_MODE_PREFIX "[EDIT MODE]  "
#define EDIT_SELECT_STRING "MP3 vol (m)  SE vol (s)  Voice vol (v)  Numeric variable (n)"

static SDL_TimerID timer_id = NULL;
SDL_TimerID timer_cdaudio_id = NULL;

/* **************************************** *
 * Callback functions
 * **************************************** */
void mp3callback( void *userdata, Uint8 *stream, int len )
{
    if ( SMPEG_playAudio( (SMPEG*)userdata, stream, len ) == 0 ){
        SDL_Event event;
        event.type = ONS_SOUND_EVENT;
        SDL_PushEvent(&event);
    }
}

Uint32 timerCallback( Uint32 interval, void *param )
{
    SDL_RemoveTimer( timer_id );
    timer_id = NULL;

	SDL_Event event;
	event.type = ONS_TIMER_EVENT;
	SDL_PushEvent( &event );

    return interval;
}

Uint32 cdaudioCallback( Uint32 interval, void *param )
{
    SDL_RemoveTimer( timer_cdaudio_id );
    timer_cdaudio_id = NULL;

    SDL_Event event;
    event.type = ONS_CDAUDIO_EVENT;
    SDL_PushEvent( &event );

    return interval;
}

void ONScripterLabel::startTimer( int count )
{
    if ( timer_id != NULL ){
        SDL_RemoveTimer( timer_id );
    }
    if ( count > MINIMUM_TIMER_RESOLUTION )
        timer_id = SDL_AddTimer( count, timerCallback, NULL );
    else
        timer_id = SDL_AddTimer( MINIMUM_TIMER_RESOLUTION, timerCallback, NULL );
}

void midiCallback( int sig )
{
    printf("waiting for Ext MIDI player terminating\n");
#if defined(LINUX)
    int status;
    wait( &status );
#endif
    SDL_Event event;
    event.type = ONS_MIDI_EVENT;
    SDL_PushEvent(&event);

    printf("Ext MIDI player terminated\n");
}

/* **************************************** *
 * Event handlers
 * **************************************** */
void ONScripterLabel::mouseMoveEvent( SDL_MouseMotionEvent *event )
{
    mouseOverCheck( event->x, event->y );
}

void ONScripterLabel::mousePressEvent( SDL_MouseButtonEvent *event )
{
    if ( variable_edit_mode ) return;
    
    current_button_state.x = event->x;
    current_button_state.y = event->y;
    
    if ( event->button == SDL_BUTTON_RIGHT && rmode_flag ){
        current_button_state.button = -1;
        volatile_button_state.button = -1;
        last_mouse_state.button = -1;
    }
    else if ( event->button == SDL_BUTTON_LEFT ){
        current_button_state.button = current_over_button;
        volatile_button_state.button = current_over_button;
        last_mouse_state.button = current_over_button;
        if ( trap_flag ){
            printf("trap by left mouse\n");
            trap_flag = false;
            current_link_label_info->label_info = lookupLabel( trap_dist );
            current_link_label_info->current_line = 0;
            current_link_label_info->offset = 0;
            if ( !(event_mode & WAIT_BUTTON_MODE) ) endCursor( clickstr_state );
            event_mode = IDLE_EVENT_MODE;
            startTimer( MINIMUM_TIMER_RESOLUTION );
            return;
        }
    }
    else return;
    
    if ( skip_flag ) skip_flag = false;
    
    if ( event_mode & WAIT_INPUT_MODE && volatile_button_state.button == -1 && root_menu_link.next ){
        system_menu_mode = SYSTEM_MENU;
    }
    
    if ( event_mode & (WAIT_INPUT_MODE | WAIT_BUTTON_MODE) ){
        if ( !(event_mode & WAIT_BUTTON_MODE) ) endCursor( clickstr_state );
        startTimer( MINIMUM_TIMER_RESOLUTION );
    }
}

void ONScripterLabel::variableEditMode( SDL_KeyboardEvent *event )
{
    int  i, p;
    char *var_name, var_index[12];

    switch ( event->keysym.sym ) {
      case SDLK_m:
        if ( variable_edit_mode != EDIT_SELECT_MODE ) return;
        variable_edit_mode = EDIT_MP3_VOLUME_MODE;
        variable_edit_num = mp3_volume;
        break;

      case SDLK_s:
        if ( variable_edit_mode != EDIT_SELECT_MODE ) return;
        variable_edit_mode = EDIT_SE_VOLUME_MODE;
        variable_edit_num = se_volume;
        break;

      case SDLK_v:
        if ( variable_edit_mode != EDIT_SELECT_MODE ) return;
        variable_edit_mode = EDIT_VOICE_VOLUME_MODE;
        variable_edit_num = voice_volume;
        break;

      case SDLK_n:
        if ( variable_edit_mode != EDIT_SELECT_MODE ) return;
        variable_edit_mode = EDIT_VARIABLE_INDEX_MODE;
        variable_edit_num = 0;
        break;

      case SDLK_9: case SDLK_KP9: variable_edit_num = variable_edit_num * 10 + 9; break;
      case SDLK_8: case SDLK_KP8: variable_edit_num = variable_edit_num * 10 + 8; break;
      case SDLK_7: case SDLK_KP7: variable_edit_num = variable_edit_num * 10 + 7; break;
      case SDLK_6: case SDLK_KP6: variable_edit_num = variable_edit_num * 10 + 6; break;
      case SDLK_5: case SDLK_KP5: variable_edit_num = variable_edit_num * 10 + 5; break;
      case SDLK_4: case SDLK_KP4: variable_edit_num = variable_edit_num * 10 + 4; break;
      case SDLK_3: case SDLK_KP3: variable_edit_num = variable_edit_num * 10 + 3; break;
      case SDLK_2: case SDLK_KP2: variable_edit_num = variable_edit_num * 10 + 2; break;
      case SDLK_1: case SDLK_KP1: variable_edit_num = variable_edit_num * 10 + 1; break;
      case SDLK_0: case SDLK_KP0: variable_edit_num = variable_edit_num * 10 + 0; break;

      case SDLK_MINUS: case SDLK_KP_MINUS:
        if ( variable_edit_mode == EDIT_VARIABLE_NUM_MODE && variable_edit_num == 0 ) variable_edit_sign = -1;
        break;

      case SDLK_BACKSPACE:
        if ( variable_edit_num ) variable_edit_num /= 10;
        else if ( variable_edit_sign == -1 ) variable_edit_sign = 1;
        break;

      case SDLK_RETURN: case SDLK_KP_ENTER:
        switch( variable_edit_mode ){

          case EDIT_VARIABLE_INDEX_MODE:
            variable_edit_index = variable_edit_num;
            variable_edit_num = num_variables[ variable_edit_index ];
            if ( variable_edit_num < 0 ){
                variable_edit_num = -variable_edit_num;
                variable_edit_sign = -1;
            }
            else{
                variable_edit_sign = 1;
            }
            break;

          case EDIT_VARIABLE_NUM_MODE:
            setNumVariable( variable_edit_index, variable_edit_sign * variable_edit_num );
            break;

          case EDIT_MP3_VOLUME_MODE:
            mp3_volume = variable_edit_num;
            if ( mp3_sample ) SMPEG_setvolume( mp3_sample, mp3_volume );
            break;

          case EDIT_SE_VOLUME_MODE:
            se_volume = variable_edit_num;
            for ( i=1 ; i<MIX_CHANNELS ; i++ )
                if ( wave_sample[i] ) Mix_Volume( i, se_volume * 128 / 100 );
            break;

          case EDIT_VOICE_VOLUME_MODE:
            voice_volume = variable_edit_num;
            if ( wave_sample[0] ) Mix_Volume( 0, se_volume * 128 / 100 );

          default:
            break;
        }
        if ( variable_edit_mode == EDIT_VARIABLE_INDEX_MODE )
            variable_edit_mode = EDIT_VARIABLE_NUM_MODE;
        else
            variable_edit_mode = EDIT_SELECT_MODE;
        break;

      case SDLK_ESCAPE:
        if ( variable_edit_mode == EDIT_SELECT_MODE ){
            variable_edit_mode = NOT_EDIT_MODE;
            SDL_WM_SetCaption( DEFAULT_WM_TITLE, DEFAULT_WM_ICON );
            SDL_Delay( 100 );
            SDL_WM_SetCaption( wm_title_string, wm_icon_string );
            return;
        }
        variable_edit_mode = EDIT_SELECT_MODE;

      default:
        break;
    }

    if ( variable_edit_mode == EDIT_SELECT_MODE ){
        sprintf( wm_edit_string, "%s%s", EDIT_MODE_PREFIX, EDIT_SELECT_STRING );
    }
    else if ( variable_edit_mode == EDIT_VARIABLE_INDEX_MODE ) {
        sprintf( wm_edit_string, "%s%s%d", EDIT_MODE_PREFIX, "Variable Index?  %", variable_edit_sign * variable_edit_num );
    }
    else if ( variable_edit_mode >= EDIT_VARIABLE_NUM_MODE ){
        switch( variable_edit_mode ){

          case EDIT_VARIABLE_NUM_MODE:
            sprintf( var_index, "%%%d", variable_edit_index );
            var_name = var_index; p = num_variables[ variable_edit_index ]; break;

          case EDIT_MP3_VOLUME_MODE:
            var_name = "MP3 Volume"; p = mp3_volume; break;

          case EDIT_VOICE_VOLUME_MODE:
            var_name = "Voice Volume"; p = voice_volume; break;

          case EDIT_SE_VOLUME_MODE:
            var_name = "Sound effect Volume"; p = se_volume; break;

          default:
            var_name = "";
        }
        sprintf( wm_edit_string, "%sCurrent %s=%d  New value? %s%d",
                 EDIT_MODE_PREFIX, var_name, p, (variable_edit_sign==1)?"":"-", variable_edit_num );
    }

    SDL_WM_SetCaption( wm_edit_string, wm_icon_string );
}


void ONScripterLabel::keyPressEvent( SDL_KeyboardEvent *event )
{
    int i;

    if ( variable_edit_mode ){
        variableEditMode( event );
        return;
    }

    if ( edit_flag && event->keysym.sym == SDLK_z ){
        variable_edit_mode = EDIT_SELECT_MODE;
        variable_edit_sign = 1;
        variable_edit_num = 0;
        sprintf( wm_edit_string, "%s%s", EDIT_MODE_PREFIX, EDIT_SELECT_STRING );
        SDL_WM_SetCaption( wm_edit_string, wm_icon_string );
    }

    if ( skip_flag && event->keysym.sym == SDLK_s) skip_flag = false;

    if ( trap_flag && (event->keysym.sym == SDLK_RETURN ||
                       event->keysym.sym == SDLK_SPACE ) ){
        printf("trap by key\n");
        trap_flag = false;
        current_link_label_info->label_info = lookupLabel( trap_dist );
        current_link_label_info->current_line = 0;
        current_link_label_info->offset = 0;
        endCursor( clickstr_state );
        event_mode = IDLE_EVENT_MODE;
        startTimer( MINIMUM_TIMER_RESOLUTION );
        return;
    }
    
    if ( event_mode & WAIT_BUTTON_MODE ){
        if ( event->keysym.sym == SDLK_UP || event->keysym.sym == SDLK_p ){
            if ( --shortcut_mouse_line < 0 ) shortcut_mouse_line = 0;
            struct ButtonLink *p_button_link = root_button_link.next;
            for ( i=0 ; i<shortcut_mouse_line && p_button_link ; i++ ) p_button_link  = p_button_link->next;
            if ( p_button_link ) SDL_WarpMouse( p_button_link->select_rect.x + p_button_link->select_rect.w / 2, p_button_link->select_rect.y + p_button_link->select_rect.h / 2 );
        }
        else if ( event->keysym.sym == SDLK_DOWN || event->keysym.sym == SDLK_n ){
            shortcut_mouse_line++;
            struct ButtonLink *p_button_link = root_button_link.next;
            for ( i=0 ; i<shortcut_mouse_line && p_button_link ; i++ ) p_button_link  = p_button_link->next;
            if ( !p_button_link ){
                shortcut_mouse_line = i-1;
                p_button_link = root_button_link.next;
                for ( i=0 ; i<shortcut_mouse_line ; i++ ) p_button_link  = p_button_link->next;
            }
            if ( p_button_link ) SDL_WarpMouse( p_button_link->select_rect.x + p_button_link->select_rect.w / 2, p_button_link->select_rect.y + p_button_link->select_rect.h / 2 );
        }
        else if ( event->keysym.sym == SDLK_RETURN ||
                  event->keysym.sym == SDLK_SPACE ){
            if ( shortcut_mouse_line >= 0 ){
                if ( event->keysym.sym == SDLK_RETURN ){
                    current_button_state.button = current_over_button;
                    volatile_button_state.button = current_over_button;
                }
                else{
                    current_button_state.button = 0;
                    volatile_button_state.button = 0;
                }
                startTimer( MINIMUM_TIMER_RESOLUTION );
                return;
            }
        }
    }

    if ( event_mode & (WAIT_INPUT_MODE | WAIT_BUTTON_MODE) ){
        if ( event->keysym.sym == SDLK_ESCAPE && rmode_flag ){
            current_button_state.button  = -1;
            volatile_button_state.button = -1;
            if ( event_mode & WAIT_INPUT_MODE && root_menu_link.next ){
                system_menu_mode = SYSTEM_MENU;
                endCursor( clickstr_state );
            }
            startTimer( MINIMUM_TIMER_RESOLUTION );
            return;
        }
    }
    
    if ( event_mode & WAIT_INPUT_MODE && !key_pressed_flag ){
        if (event->keysym.sym == SDLK_RETURN || event->keysym.sym == SDLK_SPACE ){
            skip_flag = false;
            key_pressed_flag = true;
            endCursor( clickstr_state );
            startTimer( MINIMUM_TIMER_RESOLUTION );
        }
    }
    
    if ( event_mode & ( WAIT_INPUT_MODE | WAIT_TEXTBTN_MODE ) && !key_pressed_flag ){
        if (event->keysym.sym == SDLK_s){
            skip_flag = true;
            printf("toggle skip to true\n");
            key_pressed_flag = true;
            endCursor( clickstr_state );
            startTimer( MINIMUM_TIMER_RESOLUTION );
        }
        else if (event->keysym.sym == SDLK_o){
            draw_one_page_flag = !draw_one_page_flag;
            printf("toggle draw one page flag to %s\n", (draw_one_page_flag?"true":"false") );
            if ( draw_one_page_flag ){
                endCursor( clickstr_state );
                startTimer( MINIMUM_TIMER_RESOLUTION );
            }
        }
        else if ( event->keysym.sym == SDLK_1 ){
            text_speed_no = 0;
            sentence_font.wait_time = default_text_speed[ text_speed_no ];
        }
        else if ( event->keysym.sym == SDLK_2 ){
            text_speed_no = 1;
            sentence_font.wait_time = default_text_speed[ text_speed_no ];
        }
        else if ( event->keysym.sym == SDLK_3 ){
            text_speed_no = 2;
            sentence_font.wait_time = default_text_speed[ text_speed_no ];
        }
    }
}

void ONScripterLabel::timerEvent( void )
{
  timerEventTop:

    int ret;
    
    if ( event_mode & WAIT_ANIMATION_MODE ){
        int no;
    
        if ( clickstr_state == CLICK_WAIT )         no = CURSOR_WAIT_NO;
        else if ( clickstr_state == CLICK_NEWPAGE ) no = CURSOR_NEWPAGE_NO;

        showAnimation( &cursor_info[ no ] );
        if ( cursor_info[ no ].tag.duration_list ){
            startTimer( cursor_info[ no ].tag.duration_list[ cursor_info[ no ].tag.current_cell ] );
        }
    }
    else if ( event_mode & EFFECT_EVENT_MODE ){
        if ( display_mode & TEXT_DISPLAY_MODE && erase_text_window_flag ){
            if ( effect_counter == 0 ){
                flush();
                SDL_BlitSurface( accumulation_surface, NULL, effect_dst_surface, NULL );
                SDL_BlitSurface( text_surface, NULL, effect_src_surface, NULL );
            }
            if ( doEffect( WINDOW_EFFECT, NULL, DIRECT_EFFECT_IMAGE ) == RET_CONTINUE ){
                display_mode = NORMAL_DISPLAY_MODE;
                effect_counter = 0;
                event_mode = EFFECT_EVENT_MODE;
            }
            startTimer( MINIMUM_TIMER_RESOLUTION );
            return;
        }
        string_buffer_offset = 0;
        memcpy( string_buffer, effect_command, strlen(effect_command) + 1 );
        ret = this->parseLine();
        if ( ret == RET_CONTINUE ){
            delete[] effect_command;
            if ( effect_blank == 0 || effect_counter == 0 ) goto timerEventTop;
            startTimer( effect_blank );
        }
        else{
            startTimer( MINIMUM_TIMER_RESOLUTION );
        }
        return;
    }
    else{
        if ( system_menu_mode != SYSTEM_NULL || (event_mode & WAIT_INPUT_MODE && volatile_button_state.button == -1)  )
            executeSystemCall();
        else
            executeLabel();
    }
    volatile_button_state.button = 0;
}

/* **************************************** *
 * Event loop
 * **************************************** */
int ONScripterLabel::eventLoop()
{
	SDL_Event event;

    startTimer( MINIMUM_TIMER_RESOLUTION );

	while ( SDL_WaitEvent(&event) ) {
		switch (event.type) {
          case SDL_MOUSEMOTION:
            mouseMoveEvent( (SDL_MouseMotionEvent*)&event );
            break;
            
          case SDL_MOUSEBUTTONDOWN:
            mousePressEvent( (SDL_MouseButtonEvent*)&event );
            break;

          case SDL_KEYDOWN:
            keyPressEvent( (SDL_KeyboardEvent*)&event );
            break;

          case ONS_TIMER_EVENT:
            timerEvent();
            break;
                
          case ONS_SOUND_EVENT:
            if ( !music_play_once_flag ){
                stopBGM( true );
                playMP3( current_cd_track );
            }
            else{
                stopBGM( false );
            }
            break;
                
          case ONS_CDAUDIO_EVENT:
            if ( !music_play_once_flag ){
                stopBGM( true );
                playCDAudio( current_cd_track );
            }
            else{
                stopBGM( false );
            }
            break;

          case ONS_MIDI_EVENT:
            if ( !music_play_once_flag ){
                stopBGM( true );
                playMIDI( );
            }
            else{
                stopBGM( false );
            }
            break;

          case SDL_QUIT:
            saveGlovalData();
            saveFileLog();
            saveLabelLog();
            if ( cdrom_info ){
                SDL_CDStop( cdrom_info );
                SDL_CDClose( cdrom_info );
            }
            if ( midi_info ){
                Mix_HaltMusic();
                SDL_Delay(500);
                Mix_FreeMusic( midi_info );
            }
            return(0);
            
          default:
            break;
		}
	}
    return -1;
}

