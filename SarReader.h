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

#ifndef __SAR_READER_H__
#define __SAR_READER_H__

#include "DirectReader.h"

class SarReader : public DirectReader
{
public:
    SarReader( char *path=NULL );
    ~SarReader();

    int open( char *name=NULL );
    int close();
    char *getArchiveName() const;
    int getNumFiles();
    int getNumAccessed();
    
    bool getAccessFlag( const char *file_name );
    size_t getFileLength( const char *file_name );
    size_t getFile( const char *file_name, unsigned char *buf );
    struct FileInfo getFileByIndex( int index );

    int writeHeader( FILE *fp );
    void putFile( FILE *fp, int no, size_t offset, size_t length, bool modified_flag, unsigned char *buffer );
    
protected:
    struct ArchiveInfo archive_info;
    struct ArchiveInfo *root_archive_info, *last_archive_info;
    int num_of_sar_archives;

    int readArchive( ArchiveInfo *ai, bool nsa_flag = false );
    int getIndexFromFile( ArchiveInfo *ai, const char *file_name );
    size_t getFileSub( ArchiveInfo *ai, const char *file_name, unsigned char *buf );

    int writeHeaderSub( ArchiveInfo *ai, FILE *fp, bool nsa_flag );
    void putFileSub( ArchiveInfo *ai, FILE *fp, int no, size_t offset, size_t length, bool modified_flag, unsigned char *buffer );
};

#endif // __SAR_READER_H__
