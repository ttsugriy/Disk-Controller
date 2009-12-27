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

void block_op (int op, int i, char* p) {
    struct disk_addr addr;
    int status;
    do {
        block_to_addr( i, &addr, &status );
        if (status)
            return;
        seek_addr( &addr );
        write_int_reg ( SURF, addr.surf );
        write_int_reg ( SECT, addr.sect );
        if ( write_mem_reg( p ) == UNINIT )
            return;
        write_int_reg ( OP, op );
        await_intrpt();
        status = check_status();
    } while (status);
}

void read_block(int i, char* p) {
    block_op( READ, i, p );
}

void write_block(int i, char* p) {
    block_op( WRITE, i, p );
}

void seek_addr( struct disk_addr* addr) {
    int status;
    do {
        write_int_reg( CYL, addr->cyl );
        write_int_reg( OP, SEEK );
        await_intrpt();
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

int main(int argc, char* argv) {
    printf("Initalizing disk with following values:\n\tno_cyl=%d, no_surf=%d, no_sect=%d, sect_len=%d, disk_cont=%s\n",
            no_cyl, no_surf, no_sect, sect_len, disk_cont);
    init_disk(no_cyl, no_surf, no_sect, sect_len, disk_cont);
    int op, block;
    char* input = malloc(sizeof(char) * sect_len);
    while ( 1 ) {
        printf("Choose operation (READ(%d), WRITE(%d), QUIT(9)): ", READ, WRITE);
        scanf("%d", &op);
        if (op != READ && op != WRITE)
            break;
        printf("Input block number: ");
        scanf("%d", &block);
        if (op == WRITE) {
            printf("Input content(no more than %d chars: ", sect_len);
            scanf("%s", input);
        }
        block_op( op, block, input );
        if (op == READ)
            printf("Content under %dth block: %s\n", block, input);
    }
    save_disk(disk_cont);
    return 0;
}
