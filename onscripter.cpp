/* -*- C++ -*-
 * 
 *  onscripter.cpp -- main function of ONScripter
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
#include "version.h"

void optionHelp()
{
    printf( "Usage: onscripter [option ...]\n" );
    printf( "      --cdaudio\t\tuse CD audio if available\n");
    printf( "  -f, --font file\tuse file as a default font\n");
    printf( "      --registry file\tuse file as a default registry file\n");
    printf( "      --dll file\tuse file as a default dll file\n");
    printf( "  -r, --root path\tuse path as a root path to the archives\n");
    printf( "      --force-button-shortcut\tignore useescspc and getenter command\n");
    printf( "      --disable-rescale\tdo not rescale the images in the archives when compiled with -DPDA\n");
    printf( "      --edit\t\tenable editing the volumes and the variables when 'z' is pressed\n");
    printf( "      --key-exe file\tuse file (EXE) which contains a key table\n");
    printf( "  -h, --help\t\tshow this help and exit\n");
    printf( "  -v, --version\t\tshow the version information and exit\n");
    exit(0);
}

void optionVersion()
{
    printf("ONScripter version %s\n", ONS_VERSION );
    printf("Written by Ogapee <ogapee@aqua.dti2.ne.jp>\n\n");
    printf("Copyright (c) 2001-2004 Ogapee.\n");
    printf("This is free software; see the source for copying conditions.\n");
    exit(0);
}
#if defined(QWS)
int SDL_main( int argc, char **argv )
#else
int main( int argc, char **argv )
#endif
{
    bool cdaudio_flag = false;
    char *default_font = NULL;
    char *default_registry = NULL;
    char *default_dll = NULL;
    char *default_archive_path = NULL;
    char *default_key_exe = NULL;
    bool force_button_shortcut_flag = false;
    bool disable_rescale_flag = false;
    bool edit_flag = false;
    
    /* ---------------------------------------- */
    /* Parse options */
    argv++;
    while( argc > 1 ){
        if ( argv[0][0] == '-' ){
            if ( !strcmp( argv[0]+1, "h" ) || !strcmp( argv[0]+1, "-help" ) ){
                optionHelp();
            }
            else if ( !strcmp( argv[0]+1, "v" ) || !strcmp( argv[0]+1, "-version" ) ){
                optionVersion();
            }
            else if ( !strcmp( argv[0]+1, "-cdaudio" ) ){
                cdaudio_flag = true;
            }
            else if ( !strcmp( argv[0]+1, "f" ) || !strcmp( argv[0]+1, "-font" ) ){
                argc--;
                argv++;
                if ( default_font ) delete[] default_font;
                default_font = new char[ strlen( argv[0] ) + 1 ];
                memcpy( default_font, argv[0], strlen( argv[0] ) + 1 );
            }
            else if ( !strcmp( argv[0]+1, "-registry" ) ){
                argc--;
                argv++;
                if ( default_registry ) delete[] default_registry;
                default_registry = new char[ strlen( argv[0] ) + 1 ];
                memcpy( default_registry, argv[0], strlen( argv[0] ) + 1 );
            }
            else if ( !strcmp( argv[0]+1, "-dll" ) ){
                argc--;
                argv++;
                if ( default_dll ) delete[] default_dll;
                default_dll = new char[ strlen( argv[0] ) + 1 ];
                memcpy( default_dll, argv[0], strlen( argv[0] ) + 1 );
            }
            else if ( !strcmp( argv[0]+1, "r" ) || !strcmp( argv[0]+1, "-root" ) ){
                argc--;
                argv++;
                if ( default_archive_path ) delete[] default_archive_path;
                default_archive_path = new char[ strlen( argv[0] ) + 1 ];
                memcpy( default_archive_path, argv[0], strlen( argv[0] ) + 1 );
            }
            else if ( !strcmp( argv[0]+1, "-force-button-shortcut" ) ){
                force_button_shortcut_flag = true;
            }
            else if ( !strcmp( argv[0]+1, "-disable-rescale" ) ){
                disable_rescale_flag = true;
            }
            else if ( !strcmp( argv[0]+1, "-edit" ) ){
                edit_flag = true;
            }
            else if ( !strcmp( argv[0]+1, "-key-exe" ) ){
                argc--;
                argv++;
                if ( default_key_exe ) delete[] default_key_exe;
                default_key_exe = new char[ strlen( argv[0] ) + 1 ];
                memcpy( default_key_exe, argv[0], strlen( argv[0] ) + 1 );
            }
            else{
                printf(" unknown option %s\n", argv[0] );
            }
        }
        else{
            optionHelp();
        }
        argc--;
        argv++;
    }
    
    /* ---------------------------------------- */
    /* Run ONScripter */
    ONScripterLabel *ons = new ONScripterLabel( cdaudio_flag, default_font, default_registry, default_dll, default_archive_path, force_button_shortcut_flag, disable_rescale_flag, edit_flag, default_key_exe );
    ons->eventLoop();
    
    exit(0);
}
