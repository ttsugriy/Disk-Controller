#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <limits.h>
#include "internal.h"
#include "interface.h"

int *info = NULL;
int *i_reg = NULL;
char *p_reg = NULL;
int *i_buf = NULL;
char *p_buf = NULL;
char *disk = NULL;
int cur_poll;
int maxpoll;
int cyl_pos;
int last_op_report;
FILE *fp;
int i;
int ch;


int init_disk(int no_cyl, int no_surf, int no_sect, int sect_len, char disk_cont[]){
	int d = sect_len * no_sect * no_cyl * no_surf;

	cyl_pos = 0;
	last_op_report = OK;

	if(no_cyl < 1 || no_surf < 1 || no_sect < 1 || sect_len < 1) return ILL_VAL;
	if(d > INT_MAX) return DS;

        i_reg = (int *)calloc(NO_REG, sizeof(int));        
        if(i_reg == NULL) return MEM_ERR;

        p_reg = (char *)calloc(1, sizeof(double));        
        if(p_reg == NULL) return MEM_ERR;

	i_buf = (int *)calloc(NO_REG, sizeof(int));
	if(i_buf == NULL) return MEM_ERR;

	p_buf = (char *)calloc(1, sizeof(double));
	if(p_buf == NULL) return MEM_ERR;

        disk = (char *)calloc(d, sizeof(char));
        if(disk == NULL) return MEM_ERR;

	info = (int *)calloc(NO_INFO, sizeof(int));
	if(info == NULL) return MEM_ERR;
	info[SECT_LEN] = sect_len;
	info[NO_SECT] = no_sect;
	info[NO_CYL] = no_cyl;
	info[NO_SURF] = no_surf;
	info[TRK_LEN] = no_sect * sect_len;
	info[CYL_LEN] = info[TRK_LEN] * no_surf;
	info[DISK_LEN] = info[CYL_LEN] * no_cyl;

	for (i = 0; i < info[DISK_LEN]; i++) 
	  disk[i] = 0;

	fp = fopen(disk_cont,"rb");
	if (fp==NULL) 
	  printf("Warning: file %s does not exist; disk initialized to empty\n", disk_cont); 
	else {
	  for (i = 0; i < info[DISK_LEN]; i++) {
	    ch = getc(fp);
	    if (ch == EOF) break;
	    disk[i] = ch; 
	  }
	}
	if ( fp != NULL ) {
		fclose(fp);
	}

	/* srand(SEED); constant SEED replaced by call to time() */
	
	srand(time(NULL));
	if  (fp==NULL) 
	  return NO_FILE;
	else 
	  return OK;
}

/*====================================================================*/

int save_disk(char disk_cont[]){

	fp = fopen(disk_cont,"wb");
	if (fp==NULL) {
	  printf("Warning: file %s could not be opened for writing\n", 
		 disk_cont); 
	  return ILL_VAL;
	}
	else 
	  for (i = 0; i < info[DISK_LEN]; i++) putc(disk[i], fp);
	  
	fclose(fp); 

	return OK;
}

/*====================================================================*/

int read_reg(int reg_no){
	if(disk == NULL) return UNINIT;

	if(reg_no != BUSY && reg_no != STAT) return ILL_REG;

	/* for polling the busy register */
	if((reg_no == BUSY) && (i_reg[BUSY] == TRUE)){
		if(cur_poll < maxpoll) cur_poll++;
		else{
			if(i_buf[OP] == REPORT){
				i_reg[STAT] = last_op_report;
				i_reg[BUSY] = FALSE;
			}
			if(i_buf[OP] < SEEK || i_buf[OP] > REPORT){
				last_op_report = ILL_OP;
				i_reg[BUSY] = FALSE;
			}
			if(i_buf[OP] >= SEEK && i_buf[OP] <= WRITE){
				await_intrpt();
			}
		}
	}

	return i_reg[reg_no];
}


/*====================================================================*/

int write_int_reg(int reg_no, int v){
  int start();
	if(disk == NULL) return UNINIT;

	if((reg_no < OP) || (reg_no > SECT)) return ILL_REG;

	i_reg[reg_no] = v;

	if (reg_no == OP) start();

	return OK;
}


/*====================================================================*/

int write_mem_reg(char* v){
	if(disk == NULL) return UNINIT;

	p_reg = v;

	return OK;
}


/*====================================================================*/

int start(){
	int i;

	if(i_reg[BUSY] == TRUE) return OK; /* start ignored cos disk is busy */

	i_reg[BUSY] = TRUE;
	cur_poll = 0;
	if(i_reg[OP] == SEEK) maxpoll = thispoll(SEEK_POLL);
	else if(i_reg[OP] == READ) maxpoll = thispoll(READ_POLL);
	else if(i_reg[OP] == WRITE) maxpoll = thispoll(WRITE_POLL);
	else if(i_reg[OP] == REPORT) maxpoll = thispoll(REPORT_POLL);
	else maxpoll = thispoll(ILL_OP_POLL); /* illegal opcode */

	/* buffer the writable registers */
	for(i = OP; i <= SECT; i++) i_buf[i] = i_reg[i];
	p_buf = p_reg;
	return OK;
}


/*====================================================================*/

int await_intrpt(){
	int i, n;
	int j; /*remove*/

	if(disk == NULL) return UNINIT;
	if(i_buf[OP] > WRITE || i_buf[OP] < SEEK || i_reg[BUSY] == FALSE) 
		for(;1;); /* hang system */

	if((i_buf[OP] == READ) || (i_buf[OP] == WRITE)){
		if((i_buf[SURF] >= info[NO_SURF]) || (i_buf[SURF] < 0)){
			last_op_report = ILL_SURF;
			i_reg[BUSY] = FALSE;
			return OK;
		}
		if((i_buf[SECT] >= info[NO_SECT]) || (i_buf[SECT] < 0)){
			last_op_report = ILL_SECT;
			i_reg[BUSY] = FALSE;
			return OK;
		}

		n = disk_offset(); 
		if(i_buf[OP] == READ){
			if((last_op_report = err(READ_ERROR)) == ERROR){
			  i_reg[BUSY] = FALSE;
				return OK;
			}
			for(i = 0; i < info[SECT_LEN]; i++){
			  p_buf[i] = disk[n+i];
			}
		}
		if(i_buf[OP] == WRITE){
			if((last_op_report = err(WRITE_ERROR)) == ERROR){
				i_reg[BUSY] = FALSE;
				return OK;
			}
			for(i = 0; i < info[SECT_LEN]; i++){
			  disk[n+i] = p_buf[i];
			}
		}
	}

	if(i_buf[OP] == SEEK){
		if((i_buf[CYL] >= info[NO_CYL]) || (i_buf[CYL] < 0)){
			last_op_report = ILL_CYL;
			i_reg[BUSY] = FALSE;
			return OK;
		}
		if((last_op_report = err(SEEK_ERROR)) == ERROR){
			i_reg[BUSY] = FALSE;
			return OK;
		}
		cyl_pos = i_buf[CYL];
	}

	i_reg[BUSY] = FALSE;
	return OK;
}



/*====================================================================*/

/*
Takes a error probability in %
Returns OK, ERROR
*/

int err(int p){
	int r = (int)(((float)rand() / RAND_MAX) * 100); /* gets 0-99 */
	if(r < p) return ERROR;
	else return OK;
}

/*
Takes a poll number
Returns the actual poll number for this instance using POLL_VARY
*/

int thispoll(int p){
	int vary = (int)(((float)POLL_VARY / (float)100) * (float)p);
	int r = (int)(((float)rand() / RAND_MAX) * vary); /* gets 0-vary */

	return p + r;
}

/*
Returns the byte offset in the disk requested
must use the cyl_pos rather than the value in the register
*/

int disk_offset(){
	return 
	  info[CYL_LEN] * cyl_pos + 
	  info[TRK_LEN] * i_buf[SURF] + 
	  info[SECT_LEN] * i_buf[SECT]; 
}

/*====================================================================*/
