/* -*- C++ -*-
 * 
 *  AVIWrapper.cpp - avifile library wrapper class to play AVI video & audio stream
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

#include "AVIWrapper.h"
#include <SDL_mixer.h>
#include <audiodecoder.h>
#include <avm_cpuinfo.h>
#include <avm_output.h>
#include <stdlib.h>
#include <string.h>

#define DEFAULT_AUDIOBUF 4096
#define AVI_FINISH_EVENT 12345

AVIWrapper::AVIWrapper()
{
    screen_overlay = NULL;
    i_avi = NULL;
    v_stream = NULL;
    a_stream = NULL;
    remaining_buffer = new char[DEFAULT_AUDIOBUF*4];
    remaining_count = 0;
    avm::out.resetDebugLevels(-1);
}

AVIWrapper::~AVIWrapper()
{
    if ( v_stream )
        v_stream->StopStreaming();
    if ( a_stream )
        a_stream->StopStreaming();
    if ( i_avi ) delete i_avi;
    if ( screen_overlay ) SDL_FreeYUVOverlay( screen_overlay );
    if ( remaining_buffer ) delete[] remaining_buffer;
}

int AVIWrapper::init( char *filename, SDL_Surface *surface, bool audio_open_flag )
{
    i_avi = CreateIAviReadFile( filename );
    if ( i_avi == NULL ){
        fprintf( stderr, "can't CreateIAviReadFile from %s\n", filename );
        return -1;
    }

    v_stream = i_avi->GetStream(0, AviStream::Video );
    if ( v_stream == NULL ){
        fprintf( stderr, "Video Stream is NULL\n" );
        return -1;
    }

    width  = v_stream->GetStreamInfo()->GetVideoWidth();
    height = v_stream->GetStreamInfo()->GetVideoHeight();

    screen_rect.x = screen_rect.y = 0;
    screen_rect.w = surface->w;
    screen_rect.h = surface->h;

    v_stream->StartStreaming();
    IVideoDecoder::CAPS cap = v_stream->GetVideoDecoder()->GetCapabilities();
    //printf("cap %x\n", cap );

    if ( cap & IVideoDecoder::CAP_YV12 ){
        v_stream->GetVideoDecoder()->SetDestFmt( 0, fccYV12 );
        screen_overlay = SDL_CreateYUVOverlay( width, height, SDL_YV12_OVERLAY, surface );
    }
    else if ( cap & IVideoDecoder::CAP_YUY2 ){
        v_stream->GetVideoDecoder()->SetDestFmt( 0, fccYUY2 );
        screen_overlay = SDL_CreateYUVOverlay( width, height, SDL_YUY2_OVERLAY, surface );
    }
    else{
        screen_overlay = SDL_CreateYUVOverlay( width, height, SDL_YV12_OVERLAY, surface );
    }

    if ( !audio_open_flag ) return 0;
    a_stream = i_avi->GetStream(0, AviStream::Audio);
    if ( a_stream == NULL ){
        //fprintf( stderr, "Audio Stream is NULL\n" );
        return 0;
    }

    a_stream->StartStreaming();
    WAVEFORMATEX wave_fmt;
    a_stream->GetAudioDecoder()->GetOutputFormat( &wave_fmt );
    //printf(" format %d ch %d sample %d bit %d avg Bps %d\n", wave_fmt.wFormatTag, wave_fmt.nChannels, wave_fmt.nSamplesPerSec, wave_fmt.wBitsPerSample, wave_fmt.nAvgBytesPerSec );
    
    if ( Mix_OpenAudio( wave_fmt.nSamplesPerSec, MIX_DEFAULT_FORMAT, wave_fmt.nChannels, DEFAULT_AUDIOBUF ) < 0 ){
        fprintf( stderr, "can't open audio device\n" );
        a_stream->StopStreaming();
        delete a_stream;
        a_stream = NULL;
        return 0;
    }

    return 0;
}

static void audioCallback( void *userdata, Uint8 *stream, int len )
{
    AVIWrapper &avi = *(AVIWrapper*)userdata;

    if ( len == 0 )
        avi.status = AVIWrapper::AVI_STOP;

    uint_t ocnt;
    uint_t samples;
    uint_t count = 0;

    if ( avi.remaining_count > 0 ){
        if ( avi.remaining_count <= len ){
            memcpy( stream, avi.remaining_buffer, avi.remaining_count );
            count = avi.remaining_count;
            len -= avi.remaining_count;
            avi.remaining_count = 0;
        }
        else{
            memmove( stream, avi.remaining_buffer, len );
            count = len;
            len = 0;
            avi.remaining_count -= len;
            return;
        }
    }
        
    while ( len > 0 && !avi.a_stream->Eof() ){
        avi.a_stream->ReadFrames( avi.remaining_buffer, len, len, samples, ocnt );
        if ( ocnt <= len ){
            memcpy( stream+count, avi.remaining_buffer, ocnt );
            len -= ocnt;
        }
        else{
            memcpy( stream+count, avi.remaining_buffer, len );
            memmove( avi.remaining_buffer, avi.remaining_buffer+len, ocnt-len );
            avi.remaining_count = ocnt-len;
            len = 0;
        }
        count += ocnt;
    }
}

double AVIWrapper::getVideoTime()
{
    if ( time_start == 0 )
    {
        frame_start = (v_stream) ? v_stream->GetTime() : 0.;
        last_frame_start = frame_start;
        time_start = longcount();
    }

    double current_time;
    if ( a_stream )
        current_time = a_stream->GetTime();
    else
        current_time = frame_start + to_float(longcount(), time_start);

    return current_time - v_stream->GetTime();
}

static int playVideo( void *userdata )
{
    AVIWrapper &avi = *(AVIWrapper*)userdata;

    while ( avi.status == AVIWrapper::AVI_PLAYING && !avi.v_stream->Eof() ){
        float async = avi.getVideoTime();
        if ( async > -0.016 ){
            avi.v_stream->ReadFrame();
            CImage *image = avi.v_stream->GetFrame();
            if ( async > 0.016 ) continue;
            avi.drawFrame( image );
        }
        else{
            SDL_Delay( 33 );
        }
    }
    avi.status = AVIWrapper::AVI_STOP;

    return 0;
}

int AVIWrapper::play( bool click_flag )
{
    time_start = 0;
    status = AVI_PLAYING;
    if ( v_stream )
        thread_id = SDL_CreateThread( playVideo, this );
    if ( a_stream )
        Mix_HookMusic( audioCallback, this );
    
    bool done_flag = false;
    while( !(done_flag & click_flag) && status == AVI_PLAYING ){
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

    status = AVI_STOP;
    if ( v_stream )
        SDL_WaitThread( thread_id, NULL );
    if ( a_stream )
        Mix_HookMusic( NULL, NULL );
    
    return 0;
}

int AVIWrapper::drawFrame( CImage *image )
{
    if ( image == NULL ) return -1;

    unsigned int i, j;
    uint32_t comp = image->GetFmt()->biCompression;
        
    unsigned char *buf = image->Data();
    SDL_LockYUVOverlay( screen_overlay );
    Uint8 *dst_y = screen_overlay->pixels[0];
    Uint8 *dst_u = screen_overlay->pixels[1];
    Uint8 *dst_v = screen_overlay->pixels[2];

    if ( comp == 0 ){ // BGR
        image->ToYUV();
        for ( i=0 ; i<width*height ; i++ )
            *dst_y++ = buf[i*3];

        for ( i=0 ; i<height/2 ; i++ ){
            for ( j=0 ; j<width/2 ; j++ ){
                *dst_v++ = buf[(i*width*2+j*2)*3+1];
                *dst_u++ = buf[(i*width*2+j*2)*3+2];
            }
        }
    }
    else if ( comp == IMG_FMT_YUY2 ){
        memcpy( dst_y, buf, width*height*2 );
    }
    else if ( comp == IMG_FMT_YV12 ){ 
        memcpy( dst_y, buf, width*height+width*height/2 );
    }
    SDL_UnlockYUVOverlay( screen_overlay );
    SDL_DisplayYUVOverlay( screen_overlay, &screen_rect );

    return 0;
}
