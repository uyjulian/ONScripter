/* -*- C++ -*-
 *
 *  SarReader.cpp - Reader from a SAR archive
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

#include "SarReader.h"

SarReader::SarReader()
{
    root_archive_info = last_archive_info = &archive_info;
    num_of_sar_archives = 0;
}

SarReader::~SarReader()
{
    close();
}

int SarReader::open( char *name )
{
    printf("SarReader::open %s\n",name);
    ArchiveInfo* info = new ArchiveInfo();

    if ( (info->file_handle = fopen( name, "rb" ) ) == NULL ){
        fprintf( stderr, "can't open file %s\n", name );
        delete info;
        return -1;
    }

    readArchive( info );

    last_archive_info->next = info;
    last_archive_info = last_archive_info->next;
    num_of_sar_archives++;

    return 0;
}

int SarReader::readArchive( struct ArchiveInfo *ai, bool nsa_flag )
{
    int i;
    unsigned long base_offset;
    
    /* Read header */
    ai->num_of_files = readShort( ai->file_handle );
    ai->fi_list = new struct FileInfo[ ai->num_of_files ];
    
    base_offset = readLong( ai->file_handle );
    
    for ( i=0 ; i<ai->num_of_files ; i++ ){
        unsigned char ch;
        int count = 0;

        while( (ch = fgetc( ai->file_handle ) ) ){
            if ( 'a' <= ch && ch <= 'z' ) ch += 'A' - 'a';
            ai->fi_list[i].name[count++] = ch;
        }
        ai->fi_list[i].name[count] = ch;

        if ( nsa_flag )
            ai->fi_list[i].compressed_no = readChar( ai->file_handle );
        ai->fi_list[i].offset = readLong( ai->file_handle ) + base_offset;
        ai->fi_list[i].length = readLong( ai->file_handle );

        /* NBZ check */
        ai->fi_list[i].nbz_flag = false;
        if ( count >= 3 && !strncmp( &ai->fi_list[i].name[count-3], "NBZ", 3 ) ){
            fpos_t pos;
            fgetpos( ai->file_handle, &pos );
            fseek( ai->file_handle, ai->fi_list[i].offset, SEEK_SET );
            ai->fi_list[i].original_length = readLong( ai->file_handle );
            if ( readChar( ai->file_handle ) == 'B' && readChar( ai->file_handle ) == 'Z' ) ai->fi_list[i].nbz_flag = true;
            fsetpos( ai->file_handle, &pos );
        }
        
        if ( nsa_flag ){
            if ( ai->fi_list[i].nbz_flag ) readLong( ai->file_handle );
            else                           ai->fi_list[i].original_length = readLong( ai->file_handle );
        }
        else if ( !ai->fi_list[i].nbz_flag )
            ai->fi_list[i].original_length = ai->fi_list[i].length;
        ai->fi_list[i].access_flag = false;
    }
    ai->num_of_accessed = 0;

    return 0;
}

int SarReader::close()
{
    ArchiveInfo *info = archive_info.next;
    
    for ( int i=0 ; i<num_of_sar_archives ; i++ ){
        if ( info->file_handle ){
            fclose( info->file_handle );
            delete[] info->fi_list;
        }
        last_archive_info = info;
        info = info->next;
        delete last_archive_info;
    }
    return 0;
}

char *SarReader::getArchiveName() const
{
    return "sar";
}

int SarReader::getNumFiles(){
    ArchiveInfo *info = archive_info.next;
    int num = 0;
    
    for ( int i=0 ; i<num_of_sar_archives ; i++ ){
        num += info->num_of_files;
        info = info->next;
    }
    
    return num;
}

int SarReader::getNumAccessed(){
    ArchiveInfo *info = archive_info.next;
    int num = 0;
    
    for ( int i=0 ; i<num_of_sar_archives ; i++ ){
        num += info->num_of_accessed;
        info = info->next;
    }
    
    return num;
}

bool SarReader::getAccessFlag( char *file_name )
{
    ArchiveInfo *info = archive_info.next;
    int j;
    
    for ( int i=0 ; i<num_of_sar_archives ; i++ ){
        j = getIndexFromFile( info, file_name );
        if ( j != info->num_of_files ) break;
        info = info->next;
    }

    if ( !info ) return false;
    
    return info->fi_list[j].access_flag;
}

int SarReader::getIndexFromFile( ArchiveInfo *ai, char *file_name )
{
    unsigned int i, len;

    len = strlen( file_name );
    if ( len > MAX_FILE_NAME_LENGTH ) len = MAX_FILE_NAME_LENGTH;
    memcpy( capital_name, file_name, len );
    capital_name[ len ] = '\0';

    for ( i=0 ; i<len ; i++ ){
        if ( 'a' <= capital_name[i] && capital_name[i] <= 'z' ) capital_name[i] += 'A' - 'a';
        else if ( capital_name[i] == '/' ) capital_name[i] = '\\';
    }
    for ( i=0 ; i<ai->num_of_files ; i++ ){
        if ( !strcmp( capital_name, ai->fi_list[i].name ) ) break;
    }

    return i;
}

size_t SarReader::getFileLength( char *file_name )
{
    size_t ret;
    if ( ( ret = DirectReader::getFileLength( file_name ) ) ) return ret;

    ArchiveInfo *info = archive_info.next;
    int j;
    for ( int i=0 ; i<num_of_sar_archives ; i++ ){
        j = getIndexFromFile( info, file_name );
        if ( j != info->num_of_files ) break;
        info = info->next;
    }
    if ( !info ) return 0;
    
    if ( !info->fi_list[j].access_flag ){
        info->num_of_accessed++;
        info->fi_list[j].access_flag = true;
    }

    return info->fi_list[j].original_length;
}

size_t SarReader::getFileSub( ArchiveInfo *ai, char *file_name, unsigned char *buf )
{
    int i = getIndexFromFile( ai, file_name );
    if ( i == ai->num_of_files ) return 0;

    if ( ai->fi_list[i].nbz_flag ){
        return decodeNBZ( ai->file_handle, ai->fi_list[i].offset, buf );
    }
    
    fseek( ai->file_handle, ai->fi_list[i].offset, SEEK_SET );
    return fread( buf, 1, ai->fi_list[i].length, ai->file_handle );
}

size_t SarReader::getFile( char *file_name, unsigned char *buf )
{
    size_t ret;
    if ( ( ret = DirectReader::getFile( file_name, buf ) ) ) return ret;

    ArchiveInfo *info = archive_info.next;
    size_t j;
    for ( int i=0 ; i<num_of_sar_archives ; i++ ){
        if ( (j = getFileSub( info, file_name, buf )) > 0 ) break;
        info = info->next;
    }

    return j;
}

struct SarReader::FileInfo SarReader::getFileByIndex( int index )
{
    ArchiveInfo *info = archive_info.next;
    for ( int i=0 ; i<num_of_sar_archives ; i++ ){
        if ( index < info->num_of_files ) return info->fi_list[index];
        index -= info->num_of_files;
        info = info->next;
    }
    fprintf( stderr, "SarReader::getFileByIndex  Index %d is out of range\n", index );

    return archive_info.fi_list[index];
}
