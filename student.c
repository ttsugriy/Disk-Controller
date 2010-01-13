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

#include "student.h"

void block_to_addr(int i, struct disk_addr * addr, int* status) {
    addr->cyl = addr->surf = addr->sect = 0;
    while ( i > 0 ) {
        addr->sect++;
        if (addr->sect == no_sect) {
            addr->sect = 0;
            addr->surf++;
        }
        if (addr->surf == no_surf) {
            addr->surf = 0;
            addr->cyl++;
        }
        if (addr->cyl >= no_cyl) {
            *status = ILL_CYL;
            return;
        }
        i--;
    }
    *status = OK;
}

void _wait_intrpt() {
#ifdef USE_DMA
    await_intrpt();
#else
    int busy;
    do {
        busy = read_reg( BUSY );
    } while (busy);
#endif
}

int block_op (int op, int i, char* p) {
    struct disk_addr addr;
    int status;
    do {
        block_to_addr( i, &addr, &status );
        if (status)
            break;
        seek_addr( &addr );
        write_int_reg ( SURF, addr.surf );
        write_int_reg ( SECT, addr.sect );
        if ( write_mem_reg( p ) == UNINIT )
            return UNINIT;
        write_int_reg ( OP, op );
        _wait_intrpt();
        status = check_status();
#ifdef DEBUG
        printf("Done %d op with status %d\n", op, status);
#endif
    } while (status);
    return status;
}

int read_block(int i, char* p) {
    return block_op( READ, i, p );
}

int write_block(int i, char* p) {
    return block_op( WRITE, i, p );
}

void seek_addr( struct disk_addr* addr) {
    int status;
    do {
        write_int_reg( CYL, addr->cyl );
        write_int_reg( OP, SEEK );
        _wait_intrpt();
        status = check_status();
    } while (status);
}

int check_status( void ) {
   write_int_reg ( OP, REPORT );
   int busy;
   do {
       busy = read_reg( BUSY );
   } while (busy);
   return read_reg( STAT );
}

int write_text( int fblock, char* text ) {
    int text_len = strlen(text);
    int blck_num = text_len / sect_len;
    int i, j;
    int written = 0;
    char* part = malloc(sizeof(char) * sect_len);
    for (i = 0; i < blck_num; i++) {
        for (j = 0; j < sect_len; j++) {
            if (i * sect_len + j > text_len)
                break;
            part[j] = text[i * sect_len + j];
            written++;
        }
        write_block( fblock + i, part );
#ifdef DEBUG
        printf("Writing '%s' to the %dth block\n", part, fblock + i);
#endif
    }
    return written;
}

void process_read(char* content) {
    int index = ask_for_block_idx();
    read_block( index, content );
    printf("Read '%s' from block #%d\n", content, index);
}

void process_write(char* content) {
    int index = ask_for_block_idx();
    ask_for_content( content );
    write_block( index, content );
    printf("Wrote '%s' to block #%d\n", content, index);
}

int ask_for_content( char* buffer ) {
    printf("Input content: ");
    scanf("%s", buffer);
}

int ask_for_block_idx( void ) {
    int index;
    printf("Input block index: ");
    scanf("%d", &index);
    return index;
}

int main(int argc, char* argv) {
    printf("Initalizing disk with following values:" 
            "\n\tno_cyl=%d, no_surf=%d, no_sect=%d, sect_len=%d, disk_cont=%s\n",
            no_cyl, no_surf, no_sect, sect_len, disk_cont);
    if (init_disk(no_cyl, no_surf, no_sect, sect_len, disk_cont) != OK) {
        printf("One of the initialization parameters is invalid."
                "Please try again providing correct one\n");
        return -1;
    }
    int op, block;
    int max_symbs = sect_len * no_sect * no_surf * no_cyl;
    char* input;
    while ( 1 ) {
        printf("Choose block operation: READ(%d), WRITE(%d) or WRITE TEXT(%d). QUIT(9)): ",
                READ, WRITE, WRITE_TEXT);
        scanf("%d", &op);
        int symsToWrite = op == WRITE ? max_symbs : sect_len;
        input = malloc(sizeof(char) * symsToWrite);
        switch ( op ) {
            case READ:
                process_read(input);
                break;
            case WRITE:
                process_write(input);
                break;
            default:
                printf("Leaving program...\n");
                return 0;
        }
    }
    save_disk(disk_cont);
    return 0;
}
