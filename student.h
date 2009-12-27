#include "interface.h"
#include <stdio.h>
#include <stdlib.h>

#define no_cyl 5
#define no_surf 2
#define no_sect 10
#define sect_len 10
#define disk_cont "disk_cont.dat"

struct disk_addr {
    long long cyl;
    long long surf;
    long long sect;
};

void read_block(int i, char *p);
void write_block(int i, char *p);
void block_to_addr(int i, struct disk_addr* addr, int* status);
void seek_addr(struct disk_addr* addr);
int check_status( void );
void block_op(int op, int i, char* p);
