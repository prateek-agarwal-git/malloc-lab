#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <assert.h>
#include <float.h>
#include <time.h>
#include "mm.h"
#include "memlib.h"
#include "fsecs.h"
#include "config.h"
#define MAXLINE     1024 /* max string size */
#define HDRLINES       4 /* number of header lines in a trace file */
#define LINENUM(i) (i+5) /* cnvt trace request nums to linenums (origin 1) */
#define IS_ALIGNED(p)  ((((unsigned int)(p)) % ALIGNMENT) == 0)
typedef struct range_t {  char *lo;  char *hi;  struct range_t *next;
} range_t;
typedef struct {
    enum {ALLOC, FREE, REALLOC} type; 
    int index;   int size; 
} traceop_t;
typedef struct {int sugg_heapsize;int num_ids;int num_ops;int weight; traceop_t *ops;     
    char **blocks; size_t *block_sizes; } trace_t;
typedef struct { trace_t *trace;  range_t *ranges; } speed_t;

typedef struct { double ops; int valid;  double secs; double util;} stats_t; 

int verbose = 0;    static int errors = 0; char msg[MAXLINE]; static char tracedir[MAXLINE] = TRACEDIR;
static char *default_tracefiles[] = { DEFAULT_TRACEFILES, NULL};
static int add_range(range_t **ranges, char *lo, int size, int tracenum, int opnum);
static void remove_range(range_t **ranges, char *lo);
static void clear_ranges(range_t **ranges);
static trace_t *read_trace(char *tracedir, char *filename);
static void free_trace(trace_t *trace);
static int eval_libc_valid(trace_t *trace, int tracenum);
static void eval_libc_speed(void *ptr);
static int eval_mm_valid(trace_t *trace, int tracenum, range_t **ranges);
static double eval_mm_util(trace_t *trace, int tracenum, range_t **ranges);
static void eval_mm_speed(void *ptr);
static void printresults(int n, stats_t *stats);
static void usage(void);
static void unix_error(char *msg);
static void malloc_error(int tracenum, int opnum, char *msg);
static void app_error(char *msg);
int main(int argc, char **argv){
    int i; char c; char **tracefiles = NULL;int num_tracefiles = 0; trace_t *trace = NULL;  
   /* stores a single trace file in memory */
    range_t *ranges = NULL;   stats_t *libc_stats = NULL;
    stats_t *mm_stats = NULL;  
    speed_t speed_params;    
    int team_check = 1;
    int run_libc = 0;
    int autograder = 0;  
    double secs, ops, util, avg_mm_util, avg_mm_throughput, p1, p2, perfindex;
    int numcorrect;
    while ((c = getopt(argc, argv, "f:t:hvVgal")) != EOF) {
        switch (c) {
	case 'g': autograder = 1; break;
        case 'f': num_tracefiles = 1;
            if ((tracefiles = realloc(tracefiles, 2*sizeof(char *))) == NULL)
		unix_error("ERROR: realloc failed in main");
	    strcpy(tracedir, "./"); tracefiles[0] = strdup(optarg);tracefiles[1] = NULL;break;
	case 't':if (num_tracefiles == 1) break;strcpy(tracedir, optarg);
	    if (tracedir[strlen(tracedir)-1] != '/') 
		strcat(tracedir, "/");break;
        case 'a':team_check = 0;break;
        case 'l':run_libc = 1;break;case 'v': verbose = 1;break;
        case 'V':verbose = 2;break;
        case 'h':usage();exit(0);
        default: usage();exit(1);}
    }
     if (team_check) {	if (!strcmp(team.teamname, "")) { printf("ERROR: Please provide the information about your team in mm.c.\n"); exit(1);
	} else  printf("Team Name:%s\n", team.teamname);
	if ((*team.name1 == '\0') || (*team.id1 == '\0')) {printf("ERROR.  You must fill in all team member 1 fields!\n");
	    exit(1);} 
	else  printf("Member 1 :%s:%s\n", team.name1, team.id1);
	if (((*team.name2 != '\0') && (*team.id2 == '\0')) ||
	    ((*team.name2 == '\0') && (*team.id2 != '\0'))) { 
	    printf("ERROR.  You must fill in all or none of the team member 2 ID fields!\n");	exit(1);}
	else if (*team.name2 != '\0')
	    printf("Member 2 :%s:%s\n", team.name2, team.id2);
    }
    if (tracefiles == NULL) { tracefiles = default_tracefiles;
        num_tracefiles = sizeof(default_tracefiles) / sizeof(char *) - 1;
	printf("Using default tracefiles in %s\n", tracedir);
    }
    init_fsecs();
    if (run_libc) {
	if (verbose > 1)
	    printf("\nTesting libc malloc\n");
	libc_stats = (stats_t *)calloc(num_tracefiles, sizeof(stats_t));
	if (libc_stats == NULL)
	    unix_error("libc_stats calloc in main failed");
	for (i=0; i < num_tracefiles; i++) {
	    trace = read_trace(tracedir, tracefiles[i]);
	    libc_stats[i].ops = trace->num_ops;
	    if (verbose > 1)
		printf("Checking libc malloc for correctness, ");
	    libc_stats[i].valid = eval_libc_valid(trace, i);
	    if (libc_stats[i].valid) {
		speed_params.trace = trace;
		if (verbose > 1)
		    printf("and performance.\n");
		libc_stats[i].secs = fsecs(eval_libc_speed, &speed_params);
	    }
	    free_trace(trace);
	}
	if (verbose) {
	    printf("\nResults for libc malloc:\n");
	    printresults(num_tracefiles, libc_stats);
	}
    }

    if (verbose > 1)
	printf("\nTesting mm malloc\n");

    mm_stats = (stats_t *)calloc(num_tracefiles, sizeof(stats_t));
    if (mm_stats == NULL)
	unix_error("mm_stats calloc in main failed");
    mem_init(); 
    for (i=0; i < num_tracefiles; i++) {
	trace = read_trace(tracedir, tracefiles[i]);
	mm_stats[i].ops = trace->num_ops;
	if (verbose > 1)printf("Checking mm_malloc for correctness, ");
	mm_stats[i].valid = eval_mm_valid(trace, i, &ranges);
	if (mm_stats[i].valid) {
	    if (verbose > 1)printf("efficiency, ");
	    mm_stats[i].util = eval_mm_util(trace, i, &ranges);
	    speed_params.trace = trace;
	    speed_params.ranges = ranges;
	    if (verbose > 1)printf("and performance.\n");
	    mm_stats[i].secs = fsecs(eval_mm_speed, &speed_params);}
	free_trace(trace);    }
    if (verbose) {
	printf("\nResults for mm malloc:\n");
	printresults(num_tracefiles, mm_stats);	printf("\n");
    }
    secs = 0;
    ops = 0;
    util = 0;
    numcorrect = 0;
    for (i=0; i < num_tracefiles; i++) {
	secs += mm_stats[i].secs;
	ops += mm_stats[i].ops;
	util += mm_stats[i].util;
	if (mm_stats[i].valid)
	    numcorrect++;
    }
    avg_mm_util = util/num_tracefiles;

    if (errors == 0) {
	avg_mm_throughput = ops/secs;
	p1 = UTIL_WEIGHT * avg_mm_util;
	if (avg_mm_throughput > AVG_LIBC_THRUPUT) p2 = (double)(1.0 - UTIL_WEIGHT);
	else {
	    p2 = ((double) (1.0 - UTIL_WEIGHT)) * 
		(avg_mm_throughput/AVG_LIBC_THRUPUT);}
	perfindex = (p1 + p2)*100.0;
	printf("Perf index = %.0f (util) + %.0f (thru) = %.0f/100\n", p1*100, 
	       p2*100, perfindex);  }
    else { perfindex = 0.0;
	printf("Terminated with %d errors\n", errors);
    }
    if (autograder) {
	printf("correct:%d\n", numcorrect);
	printf("perfidx:%.0f\n", perfindex);
    }

    exit(0);
}
static int add_range(range_t **ranges, char *lo, int size,int tracenum,
 int opnum){
    char *hi = lo + size - 1;
    range_t *p;
    char msg[MAXLINE];
    assert(size > 0);
    if (!IS_ALIGNED(lo)) {
	sprintf(msg, "Payload address (%p) not aligned to %d bytes", 
		lo, ALIGNMENT);
        malloc_error(tracenum, opnum, msg); return 0;}
    if ((lo < (char *)mem_heap_lo()) || (lo > (char *)mem_heap_hi()) || 
	(hi < (char *)mem_heap_lo()) || (hi > (char *)mem_heap_hi())) {
	sprintf(msg, "Payload (%p:%p) lies outside heap (%p:%p)",
		lo, hi, mem_heap_lo(), mem_heap_hi());
	malloc_error(tracenum, opnum, msg);return 0;
    }
    for (p = *ranges;  p != NULL;  p = p->next) {
        if ((lo >= p->lo && lo <= p-> hi) ||
            (hi >= p->lo && hi <= p->hi)) {
	    sprintf(msg, "Payload (%p:%p) overlaps another payload (%p:%p)\n",
		    lo, hi, p->lo, p->hi);
	    malloc_error(tracenum, opnum, msg);
	    return 0;
        }
    }
    if ((p = (range_t *)malloc(sizeof(range_t))) == NULL)
	unix_error("malloc error in add_range");
	p->next = *ranges;p->lo = lo;p->hi = hi;*ranges = p;return 1;
}
static void remove_range(range_t **ranges, char *lo)
{    range_t *p; range_t **prevpp = ranges;
    int size;
    for (p = *ranges;  p != NULL; p = p->next) {
        if (p->lo == lo) {*prevpp = p->next;size = p->hi - p->lo + 1;
            free(p); break;}
        prevpp = &(p->next);
    }}
static void clear_ranges(range_t **ranges)
{    range_t *p;
    range_t *pnext;
   for (p = *ranges;  p != NULL;  p = pnext) {
        pnext = p->next;free(p);}
        *ranges = NULL;}
static trace_t *read_trace(char *tracedir, char *filename)
{
    FILE *tracefile;
    trace_t *trace;
    char type[MAXLINE];
    char path[MAXLINE];
    unsigned index, size;
    unsigned max_index = 0;
    unsigned op_index;

    if (verbose > 1)
	printf("Reading tracefile: %s\n", filename);
    if ((trace = (trace_t *) malloc(sizeof(trace_t))) == NULL)
	unix_error("malloc 1 failed in read_trance");
    strcpy(path, tracedir);
    strcat(path, filename);
    if ((tracefile = fopen(path, "r")) == NULL) {
	sprintf(msg, "Could not open %s in read_trace", path);
	unix_error(msg);
    }
    fscanf(tracefile, "%d", &(trace->sugg_heapsize)); /* not used */
    fscanf(tracefile, "%d", &(trace->num_ids));     
    fscanf(tracefile, "%d", &(trace->num_ops));     
    fscanf(tracefile, "%d", &(trace->weight));        /* not used */
    if ((trace->ops = 
	 (traceop_t *)malloc(trace->num_ops * sizeof(traceop_t))) == NULL)
	unix_error("malloc 2 failed in read_trace");

    if ((trace->blocks = 
	 (char **)malloc(trace->num_ids * sizeof(char *))) == NULL)
	unix_error("malloc 3 failed in read_trace");
    if ((trace->block_sizes = 
	 (size_t *)malloc(trace->num_ids * sizeof(size_t))) == NULL)
	unix_error("malloc 4 failed in read_trace");
    index = 0;
    op_index = 0;
    while (fscanf(tracefile, "%s", type) != EOF) {
	switch(type[0]) {
	case 'a':
	    fscanf(tracefile, "%u %u", &index, &size);
	    trace->ops[op_index].type = ALLOC;
	    trace->ops[op_index].index = index;
	    trace->ops[op_index].size = size;
	    max_index = (index > max_index) ? index : max_index;
	    break;
	case 'r':
	    fscanf(tracefile, "%u %u", &index, &size);
	    trace->ops[op_index].type = REALLOC;
	    trace->ops[op_index].index = index;
	    trace->ops[op_index].size = size;
	    max_index = (index > max_index) ? index : max_index;
	    break;
	case 'f':
	    fscanf(tracefile, "%ud", &index);
	    trace->ops[op_index].type = FREE;
	    trace->ops[op_index].index = index;
	    break;
	default:
	    printf("Bogus type character (%c) in tracefile %s\n", 
		   type[0], path);
	    exit(1);
	}
	op_index++;
    }
    fclose(tracefile);
    assert(max_index == trace->num_ids - 1);
    assert(trace->num_ops == op_index);
    return trace;}
void free_trace(trace_t *trace)
{
    free(trace->ops);
    free(trace->blocks);      
    free(trace->block_sizes);
    free(trace);}
static int eval_mm_valid(trace_t *trace, int tracenum, range_t **ranges) 
    int i, j;
    int index;
    int size;
    int oldsize;
    char *newp;
    char *oldp;
    char *p;
    mem_reset_brk();
    clear_ranges(ranges);
    if (mm_init() < 0) {
	malloc_error(tracenum, 0, "mm_init failed.");
	return 0;
    }
    for (i = 0;  i < trace->num_ops;  i++) {
	index = trace->ops[i].index;
	size = trace->ops[i].size;
        switch (trace->ops[i].type) {
        case ALLOC:
	    if ((p = mm_malloc(size)) == NULL) {
		malloc_error(tracenum, i, "mm_malloc failed.");
		return 0; }
	    if (add_range(ranges, p, size, tracenum, i) == 0)
		return 0;
	    memset(p, index & 0xFF, size);
	    trace->blocks[index] = p;
	    trace->block_sizes[index] = size;
	    break;
        case REALLOC: /* mm_realloc */
	    oldp = trace->blocks[index];
	    if ((newp = mm_realloc(oldp, size)) == NULL) {
		malloc_error(tracenum, i, "mm_realloc failed.");
		return 0;}
	    remove_range(ranges, oldp);
	    if (add_range(ranges, newp, size, tracenum, i) == 0)
		return 0;
	    oldsize = trace->block_sizes[index];
	    if (size < oldsize) oldsize = size;
	    for (j = 0; j < oldsize; j++) {
	      if (newp[j] != (index & 0xFF)) {
		malloc_error(tracenum, i, "mm_realloc did not preserve the "
			     "data from old block");
		return 0;  }
	    }
	    memset(newp, index & 0xFF, size);
	    trace->blocks[index] = newp;
	    trace->block_sizes[index] = size;
	    break;
        case FREE: /* mm_free */
	    p = trace->blocks[index];
	    remove_range(ranges, p);
	    mm_free(p);
	    break;

	default:
	    app_error("Nonexistent request type in eval_mm_valid");
        }

    }
    return 1;
}
static double eval_mm_util(trace_t *trace, int tracenum, range_t **ranges)
{    int i;
    int index;
    int size, newsize, oldsize;
    int max_total_size = 0;
    int total_size = 0;
    char *p;
    char *newp, *oldp;
    mem_reset_brk();
    if (mm_init() < 0)
	app_error("mm_init failed in eval_mm_util");
    for (i = 0;  i < trace->num_ops;  i++) {
        switch (trace->ops[i].type) {
        case ALLOC: /* mm_alloc */
	    index = trace->ops[i].index;
	    size = trace->ops[i].size;
	    if ((p = mm_malloc(size)) == NULL) 
		app_error("mm_malloc failed in eval_mm_util");
	    trace->blocks[index] = p;
	    trace->block_sizes[index] = size;
	    total_size += size;
	    max_total_size = (total_size > max_total_size) ?
        	total_size : max_total_size;
	    break;
	case REALLOC: /* mm_realloc */
	    index = trace->ops[i].index;
	    newsize = trace->ops[i].size;
	    oldsize = trace->block_sizes[index];
	    oldp = trace->blocks[index];
	    if ((newp = mm_realloc(oldp,newsize)) == NULL)
		app_error("mm_realloc failed in eval_mm_util");
	    trace->blocks[index] = newp;
	    trace->block_sizes[index] = newsize;
	    total_size += (newsize - oldsize);
	    max_total_size = (total_size > max_total_size) ?
		total_size : max_total_size;
	    break;
        case FREE: /* mm_free */
	    index = trace->ops[i].index;
	    size = trace->block_sizes[index];
	    p = trace->blocks[index];
	    mm_free(p);
	    total_size -= size;
	    break;
	default:
	    app_error("Nonexistent request type in eval_mm_util");

        }
    }

    return ((double)max_total_size / (double)mem_heapsize());
}
static void eval_mm_speed(void *ptr)
{
    int i, index, size, newsize;
    char *p, *newp, *oldp, *block;
    trace_t *trace = ((speed_t *)ptr)->trace;

    /* Reset the heap and initialize the mm package */
    mem_reset_brk();
    if (mm_init() < 0) 
	app_error("mm_init failed in eval_mm_speed");

    /* Interpret each trace request */
    for (i = 0;  i < trace->num_ops;  i++)
        switch (trace->ops[i].type) {

        case ALLOC: /* mm_malloc */
            index = trace->ops[i].index;
            size = trace->ops[i].size;
            if ((p = mm_malloc(size)) == NULL)
		app_error("mm_malloc error in eval_mm_speed");
            trace->blocks[index] = p;
            break;

	case REALLOC: /* mm_realloc */
	    index = trace->ops[i].index;
            newsize = trace->ops[i].size;
	    oldp = trace->blocks[index];
            if ((newp = mm_realloc(oldp,newsize)) == NULL)
		app_error("mm_realloc error in eval_mm_speed");
            trace->blocks[index] = newp;
            break;

        case FREE: /* mm_free */
            index = trace->ops[i].index;
            block = trace->blocks[index];
            mm_free(block);
            break;

	default:
	    app_error("Nonexistent request type in eval_mm_valid");
        }
}

/*
 * eval_libc_valid - We run this function to make sure that the
 *    libc malloc can run to completion on the set of traces.
 *    We'll be conservative and terminate if any libc malloc call fails.
 *
 */
static int eval_libc_valid(trace_t *trace, int tracenum)
{
    int i, newsize;
    char *p, *newp, *oldp;

    for (i = 0;  i < trace->num_ops;  i++) {
        switch (trace->ops[i].type) {

        case ALLOC: /* malloc */
	    if ((p = malloc(trace->ops[i].size)) == NULL) {
		malloc_error(tracenum, i, "libc malloc failed");
		unix_error("System message");
	    }
	    trace->blocks[trace->ops[i].index] = p;
	    break;

	case REALLOC: /* realloc */
            newsize = trace->ops[i].size;
	    oldp = trace->blocks[trace->ops[i].index];
	    if ((newp = realloc(oldp, newsize)) == NULL) {
		malloc_error(tracenum, i, "libc realloc failed");
		unix_error("System message");
	    }
	    trace->blocks[trace->ops[i].index] = newp;
	    break;
	    
        case FREE: /* free */
	    free(trace->blocks[trace->ops[i].index]);
	    break;

	default:
	    app_error("invalid operation type  in eval_libc_valid");
	}
    }

    return 1;
}

/* 
 * eval_libc_speed - This is the function that is used by fcyc() to
 *    measure the running time of the libc malloc package on the set
 *    of traces.
 */
static void eval_libc_speed(void *ptr)
{
    int i;
    int index, size, newsize;
    char *p, *newp, *oldp, *block;
    trace_t *trace = ((speed_t *)ptr)->trace;

    for (i = 0;  i < trace->num_ops;  i++) {
        switch (trace->ops[i].type) {
        case ALLOC: /* malloc */
	    index = trace->ops[i].index;
	    size = trace->ops[i].size;
	    if ((p = malloc(size)) == NULL)
		unix_error("malloc failed in eval_libc_speed");
	    trace->blocks[index] = p;
	    break;

	case REALLOC: /* realloc */
	    index = trace->ops[i].index;
	    newsize = trace->ops[i].size;
	    oldp = trace->blocks[index];
	    if ((newp = realloc(oldp, newsize)) == NULL)
		unix_error("realloc failed in eval_libc_speed\n");
	    
	    trace->blocks[index] = newp;
	    break;
	    
        case FREE: /* free */
	    index = trace->ops[i].index;
	    block = trace->blocks[index];
	    free(block);
	    break;
	}
    }
}

/*************************************
 * Some miscellaneous helper routines
 ************************************/


/*
 * printresults - prints a performance summary for some malloc package
 */
static void printresults(int n, stats_t *stats) 
{
    int i;
    double secs = 0;
    double ops = 0;
    double util = 0;

    /* Print the individual results for each trace */
    printf("%5s%7s %5s%8s%10s%6s\n", 
	   "trace", " valid", "util", "ops", "secs", "Kops");
    for (i=0; i < n; i++) {
	if (stats[i].valid) {
	    printf("%2d%10s%5.0f%%%8.0f%10.6f%6.0f\n", 
		   i,
		   "yes",
		   stats[i].util*100.0,
		   stats[i].ops,
		   stats[i].secs,
		   (stats[i].ops/1e3)/stats[i].secs);
	    secs += stats[i].secs;
	    ops += stats[i].ops;
	    util += stats[i].util;
	}
	else {
	    printf("%2d%10s%6s%8s%10s%6s\n", 
		   i,
		   "no",
		   "-",
		   "-",
		   "-",
		   "-");
	}
    }

    /* Print the aggregate results for the set of traces */
    if (errors == 0) {
	printf("%12s%5.0f%%%8.0f%10.6f%6.0f\n", 
	       "Total       ",
	       (util/n)*100.0,
	       ops, 
	       secs,
	       (ops/1e3)/secs);
    }
    else {
	printf("%12s%6s%8s%10s%6s\n", 
	       "Total       ",
	       "-", 
	       "-", 
	       "-", 
	       "-");
    }

}

/* 
 * app_error - Report an arbitrary application error
 */
void app_error(char *msg) 
{
    printf("%s\n", msg);
    exit(1);
}

/* 
 * unix_error - Report a Unix-style error
 */
void unix_error(char *msg) 
{
    printf("%s: %s\n", msg, strerror(errno));
    exit(1);
}

/*
 * malloc_error - Report an error returned by the mm_malloc package
 */
void malloc_error(int tracenum, int opnum, char *msg)
{
    errors++;
    printf("ERROR [trace %d, line %d]: %s\n", tracenum, LINENUM(opnum), msg);
}

/* 
 * usage - Explain the command line arguments
 */
static void usage(void) 
{
    fprintf(stderr, "Usage: mdriver [-hvVal] [-f <file>] [-t <dir>]\n");
    fprintf(stderr, "Options\n");
    fprintf(stderr, "\t-a         Don't check the team structure.\n");
    fprintf(stderr, "\t-f <file>  Use <file> as the trace file.\n");
    fprintf(stderr, "\t-g         Generate summary info for autograder.\n");
    fprintf(stderr, "\t-h         Print this message.\n");
    fprintf(stderr, "\t-l         Run libc malloc as well.\n");
    fprintf(stderr, "\t-t <dir>   Directory to find default traces.\n");
    fprintf(stderr, "\t-v         Print per-trace performance breakdowns.\n");
    fprintf(stderr, "\t-V         Print additional debug info.\n");
}
