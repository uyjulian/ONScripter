/* -*- C++ -*-
 * 
 *  ONScripterLabel_sound.cpp - Methods for playing sound
 *
 *  Copyright (c) 2001-2006 Ogapee. All rights reserved.
 *
 *  ogapee@aqua.dti2.ne.jp
 *
 *  A part of this code is derived from decoder_example.c included
 *  in libvorbis-1.0.tar.gz.
 *  (Copyright (c) 1994-2002 by the Xiph.Org Foundation)
 *
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

#if defined(USE_AVIFILE)
#include "AVIWrapper.h"
#endif

#if defined(USE_OGG_VORBIS)
#if defined(INTEGER_OGG_VORBIS)
#include <tremor/ivorbiscodec.h>
#else
#include <vorbis/codec.h>
#endif
#define OGG_BUFFER 4096
ogg_int16_t convbuffer[OGG_BUFFER];
#endif
struct WAVE_HEADER{
    char chunk_riff[4];
    char riff_length[4];
    char fmt_id[8];
    char fmt_size[4];
    char data_fmt[2];
    char channels[2];
    char frequency[4];
    char byte_size[4];
    char sample_byte_size[2];
    char sample_bit_size[2];
             
    char chunk_id[4];
    char data_length[4];
} header;

#if defined(EXTERNAL_MUSIC_PLAYER)
extern bool ext_music_play_once_flag;
#endif

extern "C"{
    extern void mp3callback( void *userdata, Uint8 *stream, int len );
    extern Uint32 cdaudioCallback( Uint32 interval, void *param );
}
extern void midiCallback( int sig );
#if defined(EXTERNAL_MUSIC_PLAYER)
extern void musicCallback( int sig );
#endif
extern SDL_TimerID timer_cdaudio_id;

#define TMP_MIDI_FILE "tmp.mid"
#define TMP_MUSIC_FILE "tmp.mus"

int ONScripterLabel::playMIDIFile(const char* filename)
{
    if ( !audio_open_flag ) return -1;

    FILE *fp;

    if ( (fp = fopen( TMP_MIDI_FILE, "wb" )) == NULL ){
        fprintf( stderr, "can't open temporaly MIDI file %s\n", TMP_MIDI_FILE );
        return -1;
    }

    unsigned long length = script_h.cBR->getFileLength( filename );
    if ( length == 0 ){
        fprintf( stderr, " *** can't find file [%s] ***\n", filename );
        return -1;
    }
    unsigned char *buffer = new unsigned char[length];
    script_h.cBR->getFile( filename, buffer );
    fwrite( buffer, 1, length, fp );
    delete[] buffer;

    fclose( fp );

    return playMIDI();
}

int ONScripterLabel::playMIDI()
{
    int midi_looping = internal_midi_play_loop_flag ? -1 : 0;
    char *midi_file = new char[ strlen(archive_path) + strlen(TMP_MIDI_FILE) + 1 ];
    sprintf( midi_file, "%s%s", archive_path, TMP_MIDI_FILE );

    char *music_cmd = getenv( "MUSIC_CMD" );

#if defined(EXTERNAL_MIDI_PROGRAM)
    FILE *com_file;
    if ( internal_midi_play_loop_flag ){
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

    char filename[128];
    char *tmp_music_file_name = music_file_name;
    if (tmp_music_file_name == NULL){
        sprintf( filename, "cd\\track%2.2d.mp3", cd_no );
        tmp_music_file_name = filename;
    }

    unsigned long length = script_h.cBR->getFileLength( tmp_music_file_name );
    mp3_buffer = new unsigned char[length];
    script_h.cBR->getFile( tmp_music_file_name, mp3_buffer );
    if ( mp3_buffer[0] == 0x30 && mp3_buffer[1] == 0x26 &&
         mp3_buffer[2] == 0xb2 && mp3_buffer[3] == 0x75 ){
        /* WMA */
        delete [] mp3_buffer;
        mp3_buffer = NULL;
        return -1;
    }
    mp3_sample = SMPEG_new_rwops( SDL_RWFromMem( mp3_buffer, length ), NULL, 0 );

    if ( SMPEG_error( mp3_sample ) ){
        //printf(" failed. [%s]\n",SMPEG_error( mp3_sample ));
        // The line below fails. ?????
        //SMPEG_delete( mp3_sample );
        mp3_sample = NULL;
        return -1;
    }
    else{
#ifndef MP3_MAD        
        SMPEG_enableaudio( mp3_sample, 0 );
        if ( audio_open_flag ){
            SMPEG_actualSpec( mp3_sample, &audio_format );
            SMPEG_enableaudio( mp3_sample, 1 );
        }
#endif
        SMPEG_setvolume( mp3_sample, mp3_volume );
        Mix_HookMusic( mp3callback, mp3_sample );
        SMPEG_play( mp3_sample );
    }

    return 0;
}

int ONScripterLabel::playMPEG( const char *filename, bool click_flag )
{
    int ret = 0;
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
                  case SDL_QUIT:
                    ret = 1;
                  case SDL_MOUSEBUTTONDOWN:
                    done_flag = true;
                    break;
                  default:
                    break;
                }
            }
            SDL_Delay( 10 );
        }

        SMPEG_stop( mpeg_sample );
        Mix_HookMusic( NULL, NULL );
        SMPEG_delete( mpeg_sample );

    }
    delete[] mpeg_buffer;
#else
    fprintf( stderr, "mpegplay command is disabled.\n" );
#endif

    return ret;
}

void ONScripterLabel::playAVI( const char *filename, bool click_flag )
{
#if defined(USE_AVIFILE)
    char *absolute_filename = new char[ strlen(archive_path) + strlen(filename) + 1 ];
    sprintf( absolute_filename, "%s%s", archive_path, filename );
    for ( unsigned int i=0 ; i<strlen( absolute_filename ) ; i++ )
        if ( absolute_filename[i] == '/' ||
             absolute_filename[i] == '\\' )
            absolute_filename[i] = DELIMITER;

    if ( audio_open_flag ) Mix_CloseAudio();

    AVIWrapper *avi = new AVIWrapper();
    if ( avi->init( absolute_filename, false ) == 0 &&
         avi->initAV( screen_surface, audio_open_flag ) == 0 ){
        if (avi->play( click_flag )) endCommand();
    }
    delete avi;
    delete[] absolute_filename;

    if ( audio_open_flag ){
        Mix_CloseAudio();
        openAudio();
    }
#else
    fprintf( stderr, "avi command is disabled.\n" );
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
    
    Mix_Pause( channel );
    if ( !(play_mode & WAVE_PLAY_LOADED) ){
        length = script_h.cBR->getFileLength( file_name );
        if ( length==0 ) return -1;
        buffer = new unsigned char[length];
        script_h.cBR->getFile( file_name, buffer );

        // check Ogg Vorbis
        int channels, rate;
        unsigned long length2 = decodeOggVorbis( buffer, NULL, length, channels, rate );
        if ( length2 > 0 ){
            unsigned char *buffer2 = new unsigned char[ sizeof(WAVE_HEADER) + length2 ];
            decodeOggVorbis( buffer, buffer2+sizeof(WAVE_HEADER), length, channels, rate );
            setupWaveHeader( buffer2, channels, rate, length2 );
            delete[] buffer;
            buffer = buffer2;
            length = sizeof(WAVE_HEADER) + length2;
        }
        
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

    if ( mp3_sample ){
        SMPEG_stop( mp3_sample );
        Mix_HookMusic( NULL, NULL );
        SMPEG_delete( mp3_sample );
        mp3_sample = NULL;

        if ( mp3_buffer ){
            delete[] mp3_buffer;
            mp3_buffer = NULL;
        }
    }

    if ( wave_sample[MIX_BGM_CHANNEL] ){
        Mix_Pause( MIX_BGM_CHANNEL );
        Mix_FreeChunk( wave_sample[MIX_BGM_CHANNEL] );
        wave_sample[MIX_BGM_CHANNEL] = NULL;
    }

    if ( !continue_flag ){
        setStr( &music_file_name, NULL );
        music_play_loop_flag = false;
    }

    if ( midi_info ){
        Mix_HaltMusic();
        Mix_FreeMusic( midi_info );
        midi_info = NULL;
    }
    if ( !continue_flag ){
        setStr( &midi_file_name, NULL );
        midi_play_loop_flag = false;
        internal_midi_play_loop_flag = false;
    }

#if defined(EXTERNAL_MUSIC_PLAYER)
    if ( music_info ){
        ext_music_play_once_flag = true;
        Mix_HaltMusic();
        Mix_FreeMusic( music_info );
        music_info = NULL;
    }
#endif
    if ( !continue_flag ) current_cd_track = -1;
}

void ONScripterLabel::playClickVoice()
{
    if      ( clickstr_state == CLICK_NEWPAGE ){
        if ( clickvoice_file_name[CLICKVOICE_NEWPAGE] )
            playWave( clickvoice_file_name[CLICKVOICE_NEWPAGE], false, MIX_WAVE_CHANNEL );
    }
    else if ( clickstr_state == CLICK_WAIT ){
        if ( clickvoice_file_name[CLICKVOICE_NORMAL] )
            playWave( clickvoice_file_name[CLICKVOICE_NORMAL], false, MIX_WAVE_CHANNEL );
    }
}

void ONScripterLabel::setupWaveHeader( unsigned char *buffer, int channels, int rate, unsigned long data_length )
{
    memcpy( header.chunk_riff, "RIFF", 4 );
    int riff_length = sizeof(WAVE_HEADER) + data_length - 8;
    header.riff_length[0] = riff_length & 0xff;
    header.riff_length[1] = (riff_length >> 8) & 0xff;
    header.riff_length[2] = (riff_length >> 16) & 0xff;
    header.riff_length[3] = (riff_length >> 24) & 0xff;
    memcpy( header.fmt_id, "WAVEfmt ", 8 );
    header.fmt_size[0] = 0x10;
    header.fmt_size[1] = header.fmt_size[2] = header.fmt_size[3] = 0;
    header.data_fmt[0] = 1; header.data_fmt[1] = 0; // PCM format
    header.channels[0] = channels; header.channels[1] = 0;
    header.frequency[0] = rate & 0xff;
    header.frequency[1] = (rate >> 8) & 0xff;
    header.frequency[2] = (rate >> 16) & 0xff;
    header.frequency[3] = (rate >> 24) & 0xff;

    int sample_byte_size = 2 * channels; // 16bit * channels
    int byte_size = sample_byte_size * rate;
    header.byte_size[0] = byte_size & 0xff;
    header.byte_size[1] = (byte_size >> 8) & 0xff;
    header.byte_size[2] = (byte_size >> 16) & 0xff;
    header.byte_size[3] = (byte_size >> 24) & 0xff;
    header.sample_byte_size[0] = sample_byte_size;
    header.sample_byte_size[1] = 0;
    header.sample_bit_size[0] = 16; // 16bit
    header.sample_bit_size[1] = 0;

    memcpy( header.chunk_id, "data", 4 );
    header.data_length[0] = (char)(data_length & 0xff);
    header.data_length[1] = (char)((data_length >> 8) & 0xff);
    header.data_length[2] = (char)((data_length >> 16) & 0xff);
    header.data_length[3] = (char)((data_length >> 24) & 0xff);

    memcpy( buffer, &header, sizeof(header) );
}

#if defined(USE_OGG_VORBIS)
inline int ogg_sync_sub( ogg_sync_state *oy, unsigned char **buffer_in, unsigned long &length )
{
    int  bytes;
#if defined(INTEGER_OGG_VORBIS)    
    unsigned char *buffer = ogg_sync_bufferin( oy, OGG_BUFFER );
#else
    char *buffer = ogg_sync_buffer( oy, OGG_BUFFER );
#endif    
    if ( length > OGG_BUFFER ){
        memcpy( buffer, *buffer_in, OGG_BUFFER );
        *buffer_in += OGG_BUFFER;
        bytes = OGG_BUFFER;
        length -= OGG_BUFFER;
    }
    else{
        memcpy( buffer, *buffer_in, length );
        *buffer_in += length;
        bytes = length;
        length -= length;
    }
    if ( bytes > 0 )
        ogg_sync_wrote( oy, bytes );

    return bytes;
}
#endif

unsigned long ONScripterLabel::decodeOggVorbis( unsigned char *buffer_in, unsigned char *buffer_out, unsigned long length, int &channels, int &rate )
{
#if defined(USE_OGG_VORBIS)
    ogg_sync_state *oy=NULL;
    ogg_stream_state *os=NULL;

    ogg_page         og = {0,0,0,0};
    ogg_packet       op = {0,0,0,0,0,0};

    vorbis_info      vi;

    vorbis_comment   vc;
    vorbis_dsp_state vd;
    vorbis_block     vb;

#if defined(INTEGER_OGG_VORBIS)    
    oy = ogg_sync_create();
#else    
    ogg_sync_state   oy_org;
    ogg_sync_init(&oy_org);
    oy = &oy_org;
#endif
    ogg_sync_sub( oy, &buffer_in, length );
    if ( ogg_sync_pageout(oy,&og) != 1 )
        return 0;
  
#if defined(INTEGER_OGG_VORBIS)    
    os = ogg_stream_create(ogg_page_serialno(&og));
#else    
    ogg_stream_state os_org;
    ogg_stream_init(&os_org,ogg_page_serialno(&og));
    os = &os_org;
#endif
    
    vorbis_info_init(&vi);
    vorbis_comment_init(&vc);
    if ( ogg_stream_pagein(os,&og) < 0 ) return 0;
    if ( ogg_stream_packetout(os,&op) != 1 ) return 0;
    if ( vorbis_synthesis_headerin(&vi,&vc,&op ) < 0) return 0;

    channels = vi.channels;
    rate = vi.rate;
    
    int length_out = 0;
    int i=0;
    while(i<2){
        while(i<2){
            int result = ogg_sync_pageout(oy,&og);
            if ( result == 0) break;
            if ( result == 1){
                ogg_stream_pagein( os, &og );

                while(i<2){
                    result=ogg_stream_packetout(os,&op);
                    if ( result == 0 ) break;
                    if ( result < 0 ) return 0;
                    vorbis_synthesis_headerin(&vi,&vc,&op);
                    i++;
                }
            }
        }

        int bytes = ogg_sync_sub( oy, &buffer_in, length );
        if (bytes==0 && i<2){
            fprintf(stderr,"End of file before finding all Vorbis headers!\n");
            return 0;
        }
    }
    
    int convsize = OGG_BUFFER/vi.channels;

    vorbis_synthesis_init(&vd,&vi);
    vorbis_block_init(&vd,&vb);

    int eos = 0;
    while(!eos){
        while(!eos){
            int result=ogg_sync_pageout(oy,&og);
            if ( result == 0 ) break;
            if ( result > 0 ){
                ogg_stream_pagein( os, &og );
                                              
                while(1){
                    result = ogg_stream_packetout(os,&op);

                    if ( result == 0 ) break;
                    if ( result > 0 ){
#if defined(INTEGER_OGG_VORBIS)                        
                        ogg_int32_t **pcm;
#else                        
                        float **pcm;
#endif                        
                        int samples;
	      
#if defined(INTEGER_OGG_VORBIS)                        
                        if ( vorbis_synthesis(&vb,&op,1) == 0 )
#else
                        if ( vorbis_synthesis(&vb,&op) == 0 )
#endif
                            vorbis_synthesis_blockin(&vd,&vb);
	      
                        while((samples=vorbis_synthesis_pcmout(&vd,&pcm))>0){
                            int bout=(samples<convsize?samples:convsize);

                            length_out += bout;
                            if ( buffer_out != NULL ){
                                for ( i=0 ; i<vi.channels ; i++ ){
                                    ogg_int16_t *ptr=convbuffer+i;
#if defined(INTEGER_OGG_VORBIS)
                                    ogg_int32_t *mono=pcm[i];
#else                        
                                    float  *mono=pcm[i];
#endif                                    
                                    for ( int j=0 ; j<bout ; j++){
#if defined(INTEGER_OGG_VORBIS)
                                        int x=mono[j]>>9;
                                        int val=x;
                                        val-= ((x<=32767)-1)&(x-32767);
                                        val-= ((x>=-32768)-1)&(x+32768);
#else
                                        int val = (int)(mono[j]*32767.f);
                                        if (val > 32767 )  val = 32767;
                                        if (val < -32768 ) val = -32768;
#endif                                        
#if SDL_BYTEORDER == SDL_LIL_ENDIAN
                                        *ptr=val;
#else
                                        *ptr=(val>>8 & 0xff)|(val<<8 & 0xff00);
#endif
                                        ptr+=vi.channels;
                                    }
                                }
		
                                memcpy( buffer_out, convbuffer, 2*vi.channels*bout );
                                buffer_out += 2*vi.channels*bout;
                            }
		
                            vorbis_synthesis_read(&vd,bout);
                        }	    
                    }
                }
#if !defined(INTEGER_OGG_VORBIS)                
                if (ogg_page_eos(&og)) eos=1;
#endif                
            }
        }
        if (!eos)
            if ( ogg_sync_sub( oy, &buffer_in, length ) == 0 ) eos = 1;
    }
    
#if !defined(INTEGER_OGG_VORBIS)
    ogg_stream_clear(os);
#endif
    
    vorbis_block_clear(&vb);
    vorbis_dsp_clear(&vd);
    vorbis_comment_clear(&vc);
    vorbis_info_clear(&vi);
#if !defined(INTEGER_OGG_VORBIS)
    ogg_sync_clear(oy);
#endif    
  
    return length_out * channels * 2;
#else
    return 0;
#endif    
}
