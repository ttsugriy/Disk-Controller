#ifdef __cplusplus
extern "C" {
#endif

/* Registers - readable */
#define BUSY		0 /* status of disk */
#define STAT		1 /* reports status of last operation */

/* Int Registers - writable */
#define OP		2 /* code for operation */
#define CYL		3 /* cylinder number */
#define SURF		4 /* surface number */
#define SECT		5 /* sector number */

/* Ptr Registers - writable */
#define MEM		6 /* memory pointer */

/* possible values for register BUSY */
#define TRUE		1
#define FALSE		0

/* possible values for register OP */
#define SEEK		0 /* generates interupt */
#define READ		1 /* generates interupt */
#define WRITE		2 /* generates interupt */
#define REPORT		3

/* possible values for register STAT */
#define OK		0
#define ILL_CYL		1
#define ILL_SURF	2
#define ILL_SECT	3
#define ILL_OP		4
#define ERROR		5

/* possible return values of the interface functions */
#define OK		0
#define ILL_VAL		-1 /* illegal input */
#define ILL_REG		-2 /* illegal register value */
#define MEM_ERR		-3 /* run out of memory */
#define DS		-4 /* disk size is too large */
#define UNINIT		-5 /* unitialised disk */
#define NO_FILE		-6 /* no disk contents file available */


int init_disk(int, int, int, int, char []);
int save_disk(char []);
int read_reg(int);
int write_int_reg(int, int);
int write_mem_reg(char*);
int await_intrpt();

#ifdef __cplusplus
}
#endif
