/*
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "interface.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define no_cyl 5
#define no_surf 2
#define no_sect 10
#define sect_len 10
#define disk_cont "disk_cont.dat"
//#define USE_DMA
#define WRITE_TEXT 3
#define DEBUG

struct disk_addr {
    long long cyl;
    long long surf;
    long long sect;
};

int read_block(int i, char *p);
int write_block(int i, char *p);
void block_to_addr(int i, struct disk_addr* addr, int* status);
void seek_addr(struct disk_addr* addr);
int check_status( void );
int block_op(int op, int i, char* p);
void _wait_intrpt();
int write_text( int fblock, char* text );
