/* -*- C++ -*-
 * 
 *  ONScripterLabel_sound.cpp - Methods to play sound
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
#if defined(LINUX)
#include <signal.h>
#endif

#if defined(EXTERNAL_MUSIC_PLAYER)
extern bool ext_music_play_once_flag;
#endif

extern void mp3callback( void *userdata, Uint8 *stream, int len );
extern Uint32 cdaudioCallback( Uint32 interval, void *param );
extern void midiCallback( int sig );
#if defined(EXTERNAL_MUSIC_PLAYER)
extern void musicCallback( int sig );
#endif
extern SDL_TimerID timer_cdaudio_id;

#define TMP_MIDI_FILE "tmp.mid"
#define TMP_MUSIC_FILE "tmp.mus"

int ONScripterLabel::playMIDIFile()
{
    if ( !audio_open_flag ) return -1;

    FILE *fp;

    if ( (fp = fopen( TMP_MIDI_FILE, "wb" )) == NULL ){
        fprintf( stderr, "can't open temporaly MIDI file %s\n", TMP_MIDI_FILE );
        return -1;
    }

    unsigned long length = script_h.cBR->getFileLength( midi_file_name );
    if ( length == 0 ){
        fprintf( stderr, " *** can't find file [%s] ***\n", midi_file_name );
        return -1;
    }
    unsigned char *buffer = new unsigned char[length];
    script_h.cBR->getFile( midi_file_name, buffer );
    fwrite( buffer, 1, length, fp );
    delete[] buffer;

    fclose( fp );

    return playMIDI();
}

int ONScripterLabel::playMIDI()
{
    int midi_looping = midi_play_loop_flag ? -1 : 0;
    char *midi_file = new char[ strlen(archive_path) + strlen(TMP_MIDI_FILE) + 1 ];
    sprintf( midi_file, "%s%s", archive_path, TMP_MIDI_FILE );

    char *music_cmd = getenv( "MUSIC_CMD" );

#if defined(EXTERNAL_MIDI_PROGRAM)
    FILE *com_file;
    if ( midi_play_loop_flag ){
        if( (com_file = fopen("play_midi", "wb")) != NULL )
            fclose(com_file);
    }
    else{
        if( (com_file = fopen("playonce_midi", "wb")) != NULL )
            fclose(com_file);
    }
#endif

#if defined(LINUX)
    signal( SIGCHLD, midiCallback );
    if ( music_cmd ) midi_looping = 0;
#endif

    Mix_SetMusicCMD( music_cmd );

    if ( (midi_info = Mix_LoadMUS( midi_file )) == NULL ) {
        fprintf( stderr, "can't load MIDI file %s\n", midi_file );
        delete[] midi_file;
        return -1;
    }

    delete[] midi_file;
    Mix_VolumeMusic( mp3_volume );
    Mix_PlayMusic( midi_info, midi_looping );
    current_cd_track = -2; 
    
    return 0;
}

#if defined(EXTERNAL_MUSIC_PLAYER)
int ONScripterLabel::playMusicFile()
{
    if ( !audio_open_flag ) return -1;
    if ( music_file_name == NULL ) return -1;

    char *player_cmd = getenv( "PLAYER_CMD" );
    if ( player_cmd == NULL ){
        playMP3( 0 );
        return 0;
    }

    FILE *fp;

    if ( (fp = fopen( TMP_MUSIC_FILE, "wb" )) == NULL ){
        fprintf( stderr, "can't open temporaly Music file %s\n", TMP_MUSIC_FILE );
        return -1;
    }

    unsigned long length = script_h.cBR->getFileLength( music_file_name );
    if ( length == 0 ){
        fprintf( stderr, " *** can't find file [%s] ***\n", music_file_name );
        return -1;
    }
    unsigned char *buffer = new unsigned char[length];
    script_h.cBR->getFile( music_file_name, buffer );
    fwrite( buffer, 1, length, fp );
    delete[] buffer;

    fclose( fp );

    ext_music_play_once_flag = !music_play_loop_flag;
    
    return playMusic();
}

int ONScripterLabel::playMusic()
{
    if ( !audio_open_flag ) return -1;
    
    int music_looping = music_play_loop_flag ? -1 : 0;
    char *music_file = new char[ strlen(archive_path) + strlen(TMP_MUSIC_FILE) + 1 ];
    sprintf( music_file, "%s%s", archive_path, TMP_MUSIC_FILE );

    char *player_cmd = getenv( "PLAYER_CMD" );

#if defined(LINUX)
    signal( SIGCHLD, musicCallback );
    if ( player_cmd ) music_looping = 0;
#endif

    Mix_SetMusicCMD( player_cmd );

    if ( (music_info = Mix_LoadMUS( music_file )) == NULL ) {
        fprintf( stderr, "can't load Music file %s\n", music_file );
        delete[] music_file;
        return -1;
    }

    delete[] music_file;
    // Mix_VolumeMusic( mp3_volume );
    Mix_PlayMusic( music_info, music_looping );

    return 0;
}
#endif

int ONScripterLabel::playMP3( int cd_no )
{
    if ( !audio_open_flag ) return -1;

    if ( music_file_name == NULL ){
        char file_name[128];

        sprintf( file_name, "%scd%ctrack%2.2d.mp3", archive_path, DELIMITER, cd_no );
        mp3_sample = SMPEG_new( file_name, NULL, 0 );
    }
    else{
        unsigned long length;
    
        length = script_h.cBR->getFileLength( music_file_name );
        mp3_buffer = new unsigned char[length];
        script_h.cBR->getFile( music_file_name, mp3_buffer );
        mp3_sample = SMPEG_new_rwops( SDL_RWFromMem( mp3_buffer, length ), NULL, 0 );
    }

    if ( SMPEG_error( mp3_sample ) ){
        //printf(" failed. [%s]\n",SMPEG_error( mp3_sample ));
        // The line below fails. ?????
        //SMPEG_delete( mp3_sample );
        mp3_sample = NULL;
    }
    else{
#ifndef MP3_MAD        
        SMPEG_enableaudio( mp3_sample, 0 );
        if ( audio_open_flag ){
            SMPEG_actualSpec( mp3_sample, &audio_format );
            SMPEG_enableaudio( mp3_sample, 1 );
        }
#endif
        SMPEG_play( mp3_sample );
        SMPEG_setvolume( mp3_sample, mp3_volume );

        Mix_HookMusic( mp3callback, mp3_sample );
    }

    return 0;
}

void ONScripterLabel::playMPEG( const char *filename, bool click_flag )
{
#ifndef MP3_MAD        
    unsigned long length = script_h.cBR->getFileLength( filename );
    unsigned char *mpeg_buffer = new unsigned char[length];
    script_h.cBR->getFile( filename, mpeg_buffer );
    SMPEG *mpeg_sample = SMPEG_new_rwops( SDL_RWFromMem( mpeg_buffer, length ), NULL, 0 );

    if ( !SMPEG_error( mpeg_sample ) ){
        SMPEG_enableaudio( mpeg_sample, 0 );

        if ( audio_open_flag ){
            SMPEG_actualSpec( mpeg_sample, &audio_format );
            SMPEG_enableaudio( mpeg_sample, 1 );
        }

        SMPEG_enablevideo( mpeg_sample, 1 );
        SMPEG_setdisplay( mpeg_sample, screen_surface, NULL, NULL );
        SMPEG_setvolume( mpeg_sample, mp3_volume );

        Mix_HookMusic( mp3callback, mpeg_sample );
        SMPEG_play( mpeg_sample );

        bool done_flag = false;
        while( !(done_flag & click_flag) && SMPEG_status(mpeg_sample) == SMPEG_PLAYING ){
            SDL_Event event;

            while( SDL_PollEvent( &event ) ){
                switch (event.type){
                  case SDL_KEYDOWN:
                    if ( ((SDL_KeyboardEvent *)&event)->keysym.sym == SDLK_RETURN ||
                         ((SDL_KeyboardEvent *)&event)->keysym.sym == SDLK_SPACE ||
                         ((SDL_KeyboardEvent *)&event)->keysym.sym == SDLK_ESCAPE )
                        done_flag = true;
                    break;
                  case SDL_MOUSEBUTTONDOWN:
                    done_flag = true;
                    break;
                  default:
                    break;
                }
            }
            SDL_Delay( 100 );
        }

        Mix_HookMusic( NULL, NULL );
        SMPEG_stop( mpeg_sample );
        SMPEG_delete( mpeg_sample );
    }
    delete[] mpeg_buffer;
#endif
}

int ONScripterLabel::playCDAudio( int cd_no )
{
    int length = cdrom_info->track[cd_no - 1].length / 75;

    SDL_CDPlayTracks( cdrom_info, cd_no - 1, 0, 1, 0 );
    timer_cdaudio_id = SDL_AddTimer( length * 1000, cdaudioCallback, NULL );

    return 0;
}

int ONScripterLabel::playWave( const char *file_name, bool loop_flag, int channel, int play_mode )
{
    unsigned long length = 0;
    unsigned char *buffer;

    if ( !audio_open_flag ) return -1;
    
    if ( channel >= ONS_MIX_CHANNELS ) channel = ONS_MIX_CHANNELS - 1;

    Mix_Pause( channel );
    if ( !(play_mode & WAVE_PLAY_LOADED) ){
        length = script_h.cBR->getFileLength( file_name );
        if ( length==0 ) return -1;
        buffer = new unsigned char[length];
        script_h.cBR->getFile( file_name, buffer );

        if ( wave_sample[channel] ){
            Mix_FreeChunk( wave_sample[channel] );
        }
        wave_sample[channel] = Mix_LoadWAV_RW(SDL_RWFromMem( buffer, length ), 1);
        delete[] buffer;
    }

    if ( !wave_sample[channel] ) return -1; // if not pre-loaded or the format is MP3
    
    if ( channel == 0 ) Mix_Volume( channel, voice_volume * 128 / 100 );
    else                Mix_Volume( channel, se_volume * 128 / 100 );

    if ( debug_level > 0 )
        printf("playWave %s %ld at vol %d\n", file_name, length, (channel==0)?voice_volume:se_volume );
    
    if ( !(play_mode & WAVE_PRELOAD) ){
        Mix_PlayChannel( channel, wave_sample[channel], loop_flag?-1:0 );
    }

    return 0;
}

void ONScripterLabel::stopBGM( bool continue_flag )
{
#if defined(EXTERNAL_MIDI_PROGRAM)
    FILE *com_file;
    if( (com_file = fopen("stop_bgm", "wb")) != NULL )
        fclose(com_file);
#endif

    if ( cdaudio_flag && cdrom_info ){
        extern SDL_TimerID timer_cdaudio_id;

        if ( timer_cdaudio_id ){
            SDL_RemoveTimer( timer_cdaudio_id );
            timer_cdaudio_id = NULL;
        }
        if (SDL_CDStatus( cdrom_info ) >= CD_PLAYING )
            SDL_CDStop( cdrom_info );
    }

    mp3save_flag = false;
    if ( mp3_sample ){
        Mix_HookMusic( NULL, NULL );
        SMPEG_stop( mp3_sample );
        SMPEG_delete( mp3_sample );
        mp3_sample = NULL;

        if ( mp3_buffer ){
            delete[] mp3_buffer;
            mp3_buffer = NULL;
        }
    }
    if ( !continue_flag )
        setStr( &music_file_name, NULL );

    if ( midi_info ){
        midi_play_loop_flag = false;
        Mix_HaltMusic();
        Mix_FreeMusic( midi_info );
        midi_info = NULL;
    }
    setStr( &music_file_name, NULL );

#if defined(EXTERNAL_MUSIC_PLAYER)
    if ( music_info ){
        ext_music_play_once_flag = true;
        Mix_HaltMusic();
        Mix_FreeMusic( music_info );
        music_info = NULL;
    }
    setStr( &music_file_name, NULL );
#endif
    if ( !continue_flag ) current_cd_track = -1;
}

void ONScripterLabel::playClickVoice()
{
    if      ( clickstr_state == CLICK_NEWPAGE ){
        if ( clickvoice_file_name[CLICKVOICE_NEWPAGE] )
            playWave( clickvoice_file_name[CLICKVOICE_NEWPAGE], false, DEFAULT_WAVE_CHANNEL );
    }
    else if ( clickstr_state == CLICK_WAIT ){
        if ( clickvoice_file_name[CLICKVOICE_NORMAL] )
            playWave( clickvoice_file_name[CLICKVOICE_NORMAL], false, DEFAULT_WAVE_CHANNEL );
    }
}

