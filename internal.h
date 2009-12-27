/* #define SEED 3 /* seed for the random number generator */

#define NO_REG		7 /* number of registers */

/* items that the info array holds */
#define NO_INFO		7 /* number of info items */

#define NO_CYL  	0
#define NO_SURF  	1
#define NO_SECT  	2
#define SECT_LEN	3
#define TRK_LEN 	4
#define CYL_LEN         5 
#define DISK_LEN	6

/* error rates in percent 0 - 100 */
#define SEEK_ERROR      20
#define READ_ERROR      20
#define WRITE_ERROR     20

/* number of times to poll busy register before a operation is done */
#define SEEK_POLL	100 /* minimum value */
#define READ_POLL	100 /* minimum value */
#define WRITE_POLL	100 /* minimum value */
#define REPORT_POLL	10 /* minimum value */
#define ILL_OP_POLL	10 /* minimum value */
#define POLL_VARY	50 /* percentage polling varies */

extern int *info; /* array to hold the disk information */
extern int *i_reg; /* holds the current int register values */
extern char *p_reg; /* holds the current ptr register value */
extern int *i_buf; /* buffer the int registers when start is called */
extern char *p_buf; /* buffer the ptr register when start is called */
extern char *disk; /* array to simulate the disk */
extern int cur_poll; /* number of times the BUSY register has been polled */
extern int maxpoll; /* number of times the BUSY register has to be polled */
extern int cyl_pos; /* the current position of the cylinder in the disk */
extern int last_op_report; /* the report of the last operation */

int disk_offset();
int err(int);
int thispoll(int);
