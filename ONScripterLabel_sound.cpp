/* -*- C++ -*-
 * 
 *  ONScripterLabel_sound.cpp - Methods to play sound
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
#include <signal.h>
#endif

extern bool midi_play_once_flag;

extern void mp3callback( void *userdata, Uint8 *stream, int len );
extern Uint32 cdaudioCallback( Uint32 interval, void *param );
extern void midiCallback( int sig );
extern SDL_TimerID timer_cdaudio_id;

#define TMP_MIDI_FILE "tmp.mid"

int ONScripterLabel::playMIDIFile()
{
    if ( !audio_open_flag ) return -1;

    FILE *fp;

    if ( (fp = fopen( TMP_MIDI_FILE, "wb" )) == NULL ){
        fprintf( stderr, "can't open temporaly MIDI file %s\n", TMP_MIDI_FILE );
        return -1;
    }

    unsigned long length = cBR->getFileLength( music_file_name );
    if ( length == 0 ){
        fprintf( stderr, " *** can't find file [%s] ***\n", music_file_name );
        return -1;
    }
    unsigned char *buffer = new unsigned char[length];
    cBR->getFile( music_file_name, buffer );
    fwrite( buffer, 1, length, fp );
    delete[] buffer;

    fclose( fp );

    midi_play_once_flag = music_play_once_flag;
    
    return playMIDI();
}

int ONScripterLabel::playMIDI()
{
    int midi_looping = music_play_once_flag ? 0 : -1;
    char *midi_file = new char[ strlen(archive_path) + strlen(TMP_MIDI_FILE) + 1 ];
    sprintf( midi_file, "%s%s", archive_path, TMP_MIDI_FILE );

    char *music_cmd = getenv( "MUSIC_CMD" );

#if defined(LINUX)
    signal( SIGCHLD, midiCallback );
    if ( music_cmd ) midi_looping = 0;
#endif

    Mix_SetMusicCMD( music_cmd );

    if ( (midi_info = Mix_LoadMUS( midi_file )) == NULL ) {
        printf( "can't load MIDI file %s\n", midi_file );
        return -1;
    }

    Mix_VolumeMusic( mp3_volume );
    Mix_PlayMusic( midi_info, midi_looping );
    current_cd_track = -2; 
    
    return 0;
}

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
    
        length = cBR->getFileLength( music_file_name );
        mp3_buffer = new unsigned char[length];
        cBR->getFile( music_file_name, mp3_buffer );
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
        SMPEG_actualSpec( mp3_sample, &audio_format );

        SMPEG_enableaudio( mp3_sample, 1 );
#endif
        SMPEG_play( mp3_sample );
        SMPEG_setvolume( mp3_sample, mp3_volume );

        Mix_HookMusic( mp3callback, mp3_sample );
    }

    return 0;
}

int ONScripterLabel::playCDAudio( int cd_no )
{
    int length = cdrom_info->track[cd_no - 1].length / 75;

    printf("playCDAudio %d\n", cd_no );
    SDL_CDPlayTracks( cdrom_info, cd_no - 1, 0, 1, 0 );
    timer_cdaudio_id = SDL_AddTimer( length * 1000, cdaudioCallback, NULL );

    return 0;
}

int ONScripterLabel::playWave( char *file_name, bool loop_flag, int channel )
{
    unsigned long length;
    unsigned char *buffer;

    if ( !audio_open_flag ) return -1;
    
    if ( channel >= MIX_CHANNELS ) channel = MIX_CHANNELS - 1;

    length = cBR->getFileLength( file_name );
    buffer = new unsigned char[length];
    cBR->getFile( file_name, buffer );

    if ( wave_sample[channel] ){
        Mix_Pause( channel );
        Mix_FreeChunk( wave_sample[channel] );
    }
    wave_sample[channel] = Mix_LoadWAV_RW(SDL_RWFromMem( buffer, length ), 1);
    delete[] buffer;

    if ( channel == 0 ) Mix_Volume( channel, voice_volume * 128 / 100 );
    else                Mix_Volume( channel, se_volume * 128 / 100 );

    if ( debug_level > 0 )
        printf("playWave %s at vol %d\n", file_name, (channel==0)?voice_volume:se_volume );
    
    Mix_PlayChannel( channel, wave_sample[channel], loop_flag?-1:0 );

    return 0;
}

void ONScripterLabel::stopBGM( bool continue_flag )
{
    if ( cdaudio_flag && cdrom_info ){
        extern SDL_TimerID timer_cdaudio_id;

        if ( timer_cdaudio_id ){
            SDL_RemoveTimer( timer_cdaudio_id );
            timer_cdaudio_id = NULL;
        }
        if (SDL_CDStatus( cdrom_info ) >= CD_PLAYING )
            SDL_CDStop( cdrom_info );
    }

    if ( mp3_sample ){
        Mix_HookMusic( NULL, NULL );
        SMPEG_stop( mp3_sample );
        SMPEG_delete( mp3_sample );
        mp3_sample = NULL;

        if ( mp3_buffer ){
            delete[] mp3_buffer;
            mp3_buffer = NULL;
        }
        if ( !continue_flag ) setStr( &music_file_name, NULL );
    }

    if ( midi_info ){
        midi_play_once_flag = true;
        Mix_HaltMusic();
        Mix_FreeMusic( midi_info );
        midi_info = NULL;
        setStr( &music_file_name, NULL );
    }

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
