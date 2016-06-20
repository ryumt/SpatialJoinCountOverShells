#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <x86intrin.h>
#include <assert.h>

#include "atomic.h"
#include "time.h"
#include "scheme.h"

#include "support_functions.h"

#include "GetOption/GetOption.h"

#include "objects/CasQueue/CasQueue.h"
#include "objects/ThreadPool/ThreadPool.h"
#include "objects/SameThreadPool/SameThreadPool.h"
#include "objects/ArraySTR/ArraySTR.h"
#include "objects/ArraySTR/MultiRangeCount.h"

#include "Gadget/Bound.h"
#include "Gadget/BufHalo.h"
#include "Gadget/BufVector3D.h"
#include "Gadget/GadgetHeader.h"
#include "Gadget/GadId.h"

#define TMP_BUFSIZE (1024)

typedef struct JoinThreadContext {
	/* number of threads */
	int nth;
	/* thread id */
	int tid;
	/* number of waiting threads */
	int nwait;
	/* number of finished threads */
	int nfin;
	/* number of processed rows */
	ulong_t nprows;
} JoinThreadContext;

typedef struct JoinContext {
	/* buffer contains halo data */
	BufHalo *bh;
	/* index on particle data */
	ArraySTR *idx;
	JoinThreadContext jtc;
	MultiRangeCountContext *mrcc;
	
	/* neighbor counts for each range of each halo */
	ulong_t *counts;
	
	SameThreadPool *pstp;
} JoinContext;


typedef struct BacklogStuff {
	ThreadPool *tp;
	CasQueue *backlog_queue;
	int *nbacklogs_ptr;
	int maxbacklogs;
	
	int first_nbacklogs;
	int first_nsetups;
	
	int nthreads;
	int nsetups;
		
	JoinContext *jc_template;
	char **particle_files;
	int *file_index_ptr;
	int nparticle_files;
	bool particle_compressed;
	
	SameThreadPool *pstp;
} BacklogStuff;

/* static void *secureGets(char *buf, size_t size); */
static void setErrorFlagAndMessage(bool *flag, char *buf, char *msg);
static void printUsageAndExit(int argc, char *argv[]);

static JoinContext *produceJoinContext(JoinContext *jc_template);
static void scrapJoinContext(JoinContext *jc);

static void *constructIndex(int nsetups, char *fname, bool iscompressed);
static void *constructIndexWithSameThreadPool(SameThreadPool *stp, char *fname, bool iscompressed);
static void destructIndex(void *idx);

static void threadJoin(void *arg);

static void produceBacklogIndex(void *arg);
static void scrapBacklogIndex(void *arg);
static void executeSearch(char *particle_files[], int nparticle_files,
			  bool particle_compressed, int maxbacklogs, 
			  int first_nbacklogs, int first_nsetups, 
			  int nthreads, int nsetups,
			  JoinContext *jc_template);
			  
static int outputResult(JoinContext *jc, char *fname);


static inline 
void 
setErrorFlagAndMessage(bool *flag, char *buf, char *msg)
{
	if (*flag == false) {
		*flag = true;
		strcpy(buf, msg);
	}
}

static inline 
void 
printUsageAndExit(int argc, char *argv[])
{
	fprintf(stderr, "Usage: %s \n"
		"          --thread #_of_threads_used_in_this_process\n"
		"          --next-setup #_of_threads_to_build_next_index_in_background\n"
		"          --particle-files particle_files(1 or more) [--compressed]\n"
		"          --halo-file halo_file [--filter-file filter_id_list_file]\n"
		"          --radius minimum_radius:maximum_radius:#_of_bins\n"
		"          [--backlog max_#_of_backlog_indexes]\n"
		"          [--first-setup #_of_indexes_be_firstly_built:#_of_threads_to_build_each_index]\n"
		"          [--output-file output_file_name]\n", 
		argv[0]);
	exit(EXIT_FAILURE);
}

static inline 
JoinContext *
produceJoinContext(JoinContext *jc_template)
{
	JoinContext *jc;
	
	jc = (void *)malloc(sizeof(*jc));
	if (jc == NULL)
		return NULL;
	memcpy(jc, jc_template, sizeof(*jc));
	return jc;
}

static inline 
void 
scrapJoinContext(JoinContext *jc)
{
	free(jc);
}

static inline 
BacklogStuff *
produceBacklogStuff(BacklogStuff *bs_template)
{
	BacklogStuff *bs;
	
	bs = (void *)malloc(sizeof(*bs));
	if (bs == NULL)
		return NULL;
	memcpy(bs, bs_template, sizeof(*bs));
	return bs;
}

static inline 
void 
scrapBacklogStuff(BacklogStuff *bs)
{
	free(bs);
}


/* ThreadPool worker routine */
static 
void 
produceBacklogIndex(void *arg)
{
	BacklogStuff *bs = arg;
	int nbacklogs = fetch_and_add(bs->nbacklogs_ptr, 1);
	
	if (nbacklogs < bs->maxbacklogs) { 
		int n = fetch_and_add(bs->file_index_ptr, 1);
		JoinContext *jc;
		
		printf("index %d\n", n);
		if (n < bs->nparticle_files) {
			if (bs->nsetups == 1) {
				jc = produceJoinContext(bs->jc_template);
				/* construct index in background */
				jc->idx = constructIndex(1, bs->particle_files[n], bs->particle_compressed);
				/* put search request with index to backlog queue */
				enqCasQueue(bs->backlog_queue, jc);
			}
			else {
				jc = produceJoinContext(bs->jc_template);
				/* wait until partitioning of thread pool has done */
				// wait_while(bs->pstp->nworkers < bs->nsetups);
				/* multi-threaded construction of index in background */
				jc->idx = constructIndexWithSameThreadPool(bs->pstp, bs->particle_files[n], bs->particle_compressed);
				/* set pointer to partitioned thread pool, for later releasing */
				jc->pstp = bs->pstp;
				/* put search request with index to backlog queue */
				enqCasQueue(bs->backlog_queue, jc);
				// callbackThreadPoolWorkerFromPseudoSameThreadPool(bs->pstp);
			}
		}
	}
	else {
		fetch_and_add(bs->nbacklogs_ptr, -1);
	}
	if (bs->pstp != NULL) {
		callbackThreadPoolWorkerFromPseudoSameThreadPool(bs->pstp);
		scrapBacklogStuff(bs);
	}
}

static 
void 
scrapBacklogIndex(void *arg)
{
	JoinContext *jc = arg;
	MEASURE_TIME_START();
	/* destruct index that was already used */
	destructIndex(jc->idx);
	/* release thread pool that was reserved for index construction */
	destructPseudoSameThreadPool(jc->pstp);
	scrapJoinContext(jc);
	MEASURE_TIME_END_PRINTMILLIS(">>>> index destruction time");
}


static inline 
void 
noBacklogExecution(char *particle_files[], int nparticle_files,  bool particle_compressed, 
		   int nthreads, int nsetups, JoinContext *jc_template)
{
	SameThreadPool *stp;
	JoinContext jc;
	int i;
	
	puts("  -- with no backlog execution");
	stp = constructSameThreadPool(nthreads);
	for (i = 0; i < nparticle_files; i++) {
		memcpy(&jc, jc_template, sizeof(jc));
		stp->nworkers = nsetups;
		/* construct index */
		jc.idx = constructIndexWithSameThreadPool(stp, particle_files[i], particle_compressed);
		stp->nworkers = nthreads;
		/* issue search request to thread pool */
		MEASURE_TIME_START();
		requestWorkSameThreadPool(stp, threadJoin, &jc);
		/* wait until all join threads will finish */
		waitCompletionSameThreadPool(stp);
		MEASURE_TIME_END_PRINTMILLIS(">>>> searching time");
		/* destruct index */
		MEASURE_TIME_START();
		destructIndex(jc.idx);
		MEASURE_TIME_END_PRINTMILLIS(">>>> index destruction time");
	}
	destructSameThreadPool(stp);
}

static inline 
void 
withBacklogExecution(char *particle_files[], int nparticle_files,  bool particle_compressed, 
		     int maxbacklogs, int first_nbacklogs, int first_nsetups, 
		     int nthreads, int nsetups, JoinContext *jc_template)
{
	/* number of backlog indexes */
	int nbacklogs = 0;
	/* file index(in particle_files[]) that will be indexed next */
	int file_index = 0;
	/* queue that holds backlog JoinContext(search request) */
	CasQueue *backlog_queue = constructCasQueue();
	/* thread pool: for index processing & search */
	ThreadPool *tp = constructThreadPool(nthreads);
	BacklogStuff bs_template = {
		tp, backlog_queue, 
		&nbacklogs, maxbacklogs, first_nbacklogs, first_nsetups, 
		nthreads, nsetups, 
		jc_template, 
		particle_files, &file_index, 
		nparticle_files, particle_compressed, 
		NULL /* Pseudo-SameThreadPool pointer: will be set later */
	};
	int i, j, k;
	
	
	puts("  -- with backlog execution");
	/* setup arguments for worker thread */
	bs_template.tp = tp;
	bs_template.backlog_queue = backlog_queue;
	
	for (j = 0; j < first_nbacklogs; j++) {
		BacklogStuff *bs = produceBacklogStuff(&bs_template);
		
		/* allocate some worker threads to construct one index in parallel */
		bs->pstp = constructPseudoSameThreadPool(first_nsetups);
		/* request for multi-threaded construction of index */
		requestWorkThreadPool(tp, produceBacklogIndex, bs);
		for (k = 0; k < first_nsetups; k++)
			requestWorkThreadPool(tp, moveThreadPoolWorkerToPseudoSameThreadPool, bs->pstp);
	}
	
	/* search will be performed on every particle files */
	for (i = 0; i < nparticle_files; i++) {
		JoinContext *jc;
		
		/* fetch search request with index */
		MEASURE_TIME_START();
		wait_while(deqCasQueue(backlog_queue, (void **)&jc) == false);
		fetch_and_add(&nbacklogs, -1);
		MEASURE_TIME_END_PRINTMILLIS(">>>> index getting time");
		/* issue search request to thread pool */
		MEASURE_TIME_START();
		for (j = 0; j < tp->nworkers; j++)
			requestWorkThreadPool(tp, threadJoin, jc);
		/* wait until all join threads will finish */
		wait_while(jc->jtc.nfin < tp->nworkers);
		MEASURE_TIME_END_PRINTMILLIS(">>>> searching time");
		/* request for destruct index */
		requestWorkThreadPool(tp, scrapBacklogIndex, jc);
		
		/* construct index in background, to prepare for later search */
		if (nsetups == 1) {
			for (j = nbacklogs; j < maxbacklogs; j++) {
				/* request for single-threaded construction of index */
				requestWorkThreadPool(tp, produceBacklogIndex, &bs_template);
			}
		}
		else {
			for (j = nbacklogs; j < maxbacklogs; j++) {
				BacklogStuff *bs = produceBacklogStuff(&bs_template);
				
				/* allocate some worker threads to construct one index in parallel */
				bs->pstp = constructPseudoSameThreadPool(nsetups);
				for (k = 0; k < nsetups; k++)
					requestWorkThreadPool(tp, moveThreadPoolWorkerToPseudoSameThreadPool, bs->pstp);
				/* request for multi-threaded construction of index */
				requestWorkThreadPool(tp, produceBacklogIndex, bs);
			}
		}
	}
	
	destructThreadPool(tp);
	destructCasQueue(backlog_queue);
}


static inline 
void 
executeSearch(char *particle_files[], int nparticle_files,  bool particle_compressed, 
	      int maxbacklogs, int first_nbacklogs, int first_nsetups, 
	      int nthreads, int nsetups, JoinContext *jc_template)
{
	if (maxbacklogs == 0) {
		noBacklogExecution(particle_files, nparticle_files, particle_compressed, 
				   nthreads, nsetups, jc_template);
	}
	else {
		withBacklogExecution(particle_files, nparticle_files, particle_compressed, 
				     maxbacklogs, first_nbacklogs, first_nsetups, 
				     nthreads, nsetups, jc_template);
	}
}


static inline 
int  
outputResult(JoinContext *jc, char *fname)
{
	MultiRangeCountContext *mrcc = jc->mrcc;
	ulong_t *total;
	char msg[TMP_BUFSIZE];
	FILE *fp;
	ulong_t n;
	int i;
	
	/* set error message in advance */
	snprintf(msg, sizeof(msg), "ERROR: while writing %s in %s()\n", fname, __func__);
	
	/* radiuses were squared for efficiency of search, so sqrt and restore them */
	for (i = 0; i < mrcc->nrads; i++)
		mrcc->radiuses[i] = sqrt(mrcc->radiuses[i]);
	
	/* create output file, if no file specified use stdout */
	if (fname == NULL)
		fp = stdout;
	else {
		fp = fopen(fname, "w");
		if (fp == NULL) {
			fprintf(stderr, "Cannot create %s in %s().\n", fname, __func__);
			return -1;
		}
	}
	
	total = &jc->counts[0];
	for (n = 0; n < jc->bh->nhalos; n++) {
		/* get count array for n-th halo */
		ulong_t *counts = &jc->counts[n * mrcc->nrads];
		
		/* output id */
		fprintf(fp, "id %lu\n", jc->bh->halo[n].id);
		/* output count for each shell of this halo */
		for (i = 0; i < mrcc->nrads; i++) {
			if (i == 0)
				fprintf(fp, "  [0.0 ~ %f]: %lu\n", mrcc->radiuses[0], counts[0]);
			else
				fprintf(fp, "  [%f ~ %f]: %lu\n", mrcc->radiuses[i - 1], mrcc->radiuses[i], counts[i]);
			total[i] += counts[i];
		}
		if (ferror(fp)) {
			perror(msg);
			if (fp != stdout)
				fclose(fp);
			return -1;
		}
	}
	
	/* output total number of particles in each shell */
	fprintf(fp, "\nTotal\n");
	for (i = 0; i < mrcc->nrads; i++) {
		if (i == 0)
			fprintf(fp, "  [0.0 ~ %f]: %lu\n", mrcc->radiuses[0], total[0]);
		else 
			fprintf(fp, "  [%f ~ %f]: %lu\n", mrcc->radiuses[i - 1], mrcc->radiuses[i], total[i]);
		
		if (ferror(fp)) {
			perror(msg);
			if (fp != stdout)
				fclose(fp);
			return -1;
		}
	}
	
	if (fp != stdout)
		fclose(fp);
	return 0;
}


int
main(int argc, char *argv[])
{
	JoinContext jc_template;
	int i;
	
	/* output file name */
	char *outfile_name = NULL;
			
	/* sequential argument indexes in argv[] that represent names of particle files */
	int particle_beg = -1;
	int particle_end = -1;
	/* flag that indicates particle files are compressed */
	bool particle_compressed = false;
	
	/* name of file that contains halo data */
	char *halo_name = NULL;
	/* name of file that contains filtering id to halo data */
	char *halo_filter_name = NULL;
	
	/* # of radiuses */
	int nrads = 0;
	/* minimum, maximum radius */
	double minrad = 0.0;
	double maxrad = 0.0;
	/* string pointer that points to radius info in argv[] */
	char *range_str = NULL;
	
	/* max # of backlog indexes */
	int maxbacklogs = 0;
	
	/* # of threads in thread pool */
	int nthreads = -1;
	/* # of backlog index construction threads */
	int nsetups = -1;
	
	/* # of indexes that will be firstly built */
	int first_nbacklogs = 1;
	/* # of threads that will construct each first indexes */
	int first_nsetups = 1;
	/* string pointer that points to first setup info in argv[] */
	char *setup_str = NULL;
	
	bool errflag = false;
	char errmsg[TMP_BUFSIZE];
	
	
	/*** argument processing ***/
	/* get output file name */
	getOptionValueFromArgs(argc, argv, &outfile_name, VALUE_TYPE_STRING_PTR, "--output-file", 0);
	
	/* get index of particle file names */
	getOptionValueRangeFromArgs(argc, argv, &particle_beg, &particle_end, "--particle-files");
	/* get whether particle file is compressed */
	particle_compressed = getOptionFlagFromArgs(argc, argv, "--compressed");
	
	/* get halo file name */
	getOptionValueFromArgs(argc, argv, &halo_name, VALUE_TYPE_STRING_PTR, "--halo-file", 0);
	/* get filter id list file name */
	getOptionValueFromArgs(argc, argv, &halo_filter_name, VALUE_TYPE_STRING_PTR, "--filter-file", 0);
	
	getOptionValueFromArgs(argc, argv, &range_str, VALUE_TYPE_STRING_PTR, "--radius", 0);
	if (range_str != NULL) {
		char *delimp;
		/* get minimum range */
		minrad = strtod(range_str, NULL);
		if ((delimp = strchr(range_str, ':')) != NULL) {
			/* get maximum range */
			maxrad = strtod(delimp + 1, NULL);
			if ((delimp = strrchr(range_str, ':')) != NULL) {
				/* get # of ranges(# of bins) */
				nrads = strtol(delimp + 1, NULL, 10);
			}
		}
	}
	if (range_str == NULL || nrads <= 0 || minrad <= 0 || maxrad <= 0) {
		setErrorFlagAndMessage(&errflag, errmsg,
				       "# of ranges, minimum range, maximum range must be > 0.\n"
				       "      use --radius and fix.\n");
	}
	else if (minrad == maxrad) {
		setErrorFlagAndMessage(&errflag, errmsg,
				       "minimum range must be <  maximum range.\n"
				       "      use --radius and fix.\n");
	}
			
	/* if minimum radius > maximum radius; probably due to input miss, fix it */
	if (minrad > maxrad)
		SWAP_MACRO(double, &minrad, &maxrad);
	
	/* get max # of backlog indexes */
	getOptionValueFromArgs(argc, argv, &maxbacklogs, VALUE_TYPE_INT, "--backlog", 0);
	if (maxbacklogs < 0) {
		setErrorFlagAndMessage(&errflag, errmsg, 
				       "max # of backlog indexes must be >= 0.\n"
				       "      use --backlog and fix.\n");
	}
	
	/* get # of threads in thread pool */
	getOptionValueFromArgs(argc, argv, &nthreads, VALUE_TYPE_INT, "--thread", 0);
	/* get # of backlog index construction threads */
	getOptionValueFromArgs(argc, argv, &nsetups, VALUE_TYPE_INT, "--next-setup", 0);
	if (nthreads < 1) {
		setErrorFlagAndMessage(&errflag, errmsg,
				       "# of threads must be >= 1.\n"
				       "      use --thread and fix.\n");
	}
	if (nsetups < 1) {
		setErrorFlagAndMessage(&errflag, errmsg, 
				       "# of backlog index construction threads must be >= 1.\n"
				       "      use --next-setup and fix.\n");
	}
		
	getOptionValueFromArgs(argc, argv, &setup_str, VALUE_TYPE_STRING_PTR, "--first-setup", 0);
	if (setup_str != NULL) {
		char *delimp;
		/* get # of indexes that will be firstly built */
		first_nbacklogs = strtol(setup_str, NULL, 10);
		/* get # of threads that will construct each first indexes */
		if ((delimp = strchr(setup_str, ':')) != NULL)
			first_nsetups = strtol(delimp + 1, NULL, 10);
	}
	
	if (maxbacklogs == 0) {
		if (nthreads < nsetups) {
			setErrorFlagAndMessage(&errflag, errmsg,
					       "in noBacklogExecution; # of threads must be"
					       ">= # of index construction threads.\n"
					       "      use --thread, --next-setup and fix.\n");
		}
	}
	else {
		if ((nthreads < (nsetups + 1) || nthreads < (first_nsetups + 1))) {
			setErrorFlagAndMessage(&errflag, errmsg,
					       "in withBacklogExecution; # of threads must be"
					       "> # of index construction threads.\n"
					       "      use --thread, --next-setup, --first-setup and fix.\n");
		}
		
		if (maxbacklogs < first_nbacklogs) {
			setErrorFlagAndMessage(&errflag, errmsg,
					       "in withBacklogExecution; max # of backlog indexes must be"
					       "> # of first backlog indexes.\n"
					       "      use --backlog, --first-setup and fix.\n");
		}
	}
	
	/* check a lack of essential arguments */
	if (particle_beg <= 0) {
		setErrorFlagAndMessage(&errflag, errmsg,
				       "particle file is missing.\n"
				       "      use --particle-files and fix.\n");
	}
	if (halo_name == NULL) {
		setErrorFlagAndMessage(&errflag, errmsg,
				       "halo file is missing.\n"
				       "      use --halo-file and fix.\n");
	}
		
	/* print information that aquired from input arguments */
	puts("\n--execution info--");
	printf("  particle files (compressed = %s): \n", BOOL2STRING(particle_compressed));
	for (i = particle_beg; i < particle_end; i++)
		printf("   |- %s\n", argv[i]);
	printf("  halo file = %s\n"
	       "  halo filter file = %s\n"
	       "  # of threads in thread pool = %d\n"
	       "  # of index construction threads = %d\n"
	       "  # of radiuses = %d\n"
	       "   |- minimum = %f, maximum = %f\n"
	       "  max # of backlog indexes = %d\n"
	       "  %s# of first backlog indexes = %d\n"
	       "  %s# of first index construction threads = %d\n", 
	       halo_name, halo_filter_name, nthreads, nsetups, 
	       nrads, minrad, maxrad, maxbacklogs, 
	       (maxbacklogs == 0) ? "! ignored ! " : "", first_nbacklogs,
	       (maxbacklogs == 0) ? "! ignored ! " : "", first_nsetups);
	
	if (errflag) {
		fprintf(stderr, "\n => %s\n", errmsg);
		printUsageAndExit(argc, argv);
	};
		
	/*** initialize ***/
	/* setup template JoinContext, this will be shared among later multiple times of searches */
	bzero(&jc_template, sizeof(jc_template));
	/* set max # of join threads */
	jc_template.jtc.nth = nthreads;
	
	/* read halo file. if filter file was specified, apply it. */
	jc_template.bh = constructBufHalo();
	MEASURE_TIME_START();
	if (halo_filter_name == NULL)
		jc_template.bh = readHalos(jc_template.bh, halo_name);
	else
		jc_template.bh = readAndFilterHalos(jc_template.bh, halo_name, halo_filter_name);
	MEASURE_TIME_END_PRINTMILLIS(">>>> halo data reading time");
	printf("  # of halos = %lu\n", jc_template.bh->nhalos);
	
	/* setup parameters and buffers for MultiRangeCount() */
	jc_template.mrcc = constructMultiRangeCountContext(nrads, NULL, true, false);
	Logspace(2.0, log2(minrad), log2(maxrad), nrads, jc_template.mrcc->radiuses);
	/* radiuses were squared for efficiency of search */
	for (i = 0; i < nrads; i++)
		jc_template.mrcc->radiuses[i] *= jc_template.mrcc->radiuses[i];
	/* alloc array to store counts */
	jc_template.counts = (void *)calloc(1, (sizeof(*jc_template.counts) * jc_template.mrcc->nrads) * jc_template.bh->nhalos);
				
	/*** execution ***/
	MEASURE_TIME_START();
	executeSearch(&argv[particle_beg], particle_end - particle_beg, 
		      particle_compressed, maxbacklogs, first_nbacklogs, first_nsetups, 
		      nthreads, nsetups, &jc_template);
	MEASURE_TIME_END_PRINTMILLIS(">>>> total searching time");
		
	/*** result processing ***/
	MEASURE_TIME_START();
	if (outputResult(&jc_template, outfile_name) < 0)
		exit(EXIT_FAILURE);
	MEASURE_TIME_END_PRINTMILLIS(">>>> result writing time");
		
	/*** finalyze ***/
	destructBufHalo(jc_template.bh);
	destructMultiRangeCountContext(jc_template.mrcc, true, false);
	free(jc_template.counts);	
	
	return 0;
}



static 
void * 
constructIndex(int nsetups, char *fname, bool iscompressed)
{
	void *idx;
	ulong_t n;
	
	if (iscompressed) {
		BufUshortVector3D *buv;
		GadgetHeader hdr;
		GadgetDecodeParams dparams;
		GadId idsys;
		static const int ntile = 1;
		
		buv = constructBufUshortVector3D();
		MEASURE_TIME_START();
		buv = readCompressedParticles(buv, fname, &hdr);
		MEASURE_TIME_END_PRINTMILLIS(">>>> particles reading time");
		MEASURE_TIME_START();
		initGadgetDecodeParams(&dparams, &hdr, ntile);
		initGadId(&idsys, ntile, dparams.nparticles_partile);
		idx = constructArraySTR(buv->nuvs);
		for(n = 0; n < buv->nuvs; n++) {
			uint16_t *uv = getVectorBufUshortVector3D(buv, n);
			float pos[4];
			
			decodeCompressedParticle(uv, pos, &hdr, &idsys, n + dparams.id_start, dparams.cell_interval, dparams.unitsep);
			insertDataArraySTR(idx, n, pos);
		}
		makeTreeArraySTR(idx, nsetups);
		MEASURE_TIME_END_PRINTMILLIS(">>>> particles decoding + index construction time");
		printf("  # of particles = %llu\n", buv->nuvs);
		destructBufUshortVector3D(buv);
	}
	else {
		BufFloatVector3D *bfv;
		
		bfv = constructBufFloatVector3D();
		MEASURE_TIME_START();
		bfv = readDecodedParticles(bfv, fname);
		MEASURE_TIME_END_PRINTMILLIS(">>>> particles reading time");
		MEASURE_TIME_START();
		idx = constructArraySTR(bfv->nfvs);
		for (n = 0; n < bfv->nfvs; n++) {
			float *fv = getVectorBufFloatVector3D(bfv, n);
			insertDataArraySTR(idx, n, fv);
		}
		makeTreeArraySTR(idx, nsetups);
		MEASURE_TIME_END_PRINTMILLIS(">>>> index construction time");
		printf("  # of particles = %llu\n", bfv->nfvs);
		destructBufFloatVector3D(bfv);
	}
		
	return idx;
}

static 
void * 
constructIndexWithSameThreadPool(SameThreadPool *stp, char *fname, bool iscompressed)
{
	void *idx;
	ulong_t n;
	
	if (iscompressed) {
		BufUshortVector3D *buv;
		GadgetHeader hdr;
		GadgetDecodeParams dparams;
		GadId idsys;
		static const int ntile = 1;
		
		buv = constructBufUshortVector3D();
		MEASURE_TIME_START();
		buv = readCompressedParticles(buv, fname, &hdr);
		MEASURE_TIME_END_PRINTMILLIS(">>>> particles reading time");
		MEASURE_TIME_START();
		initGadgetDecodeParams(&dparams, &hdr, ntile);
		initGadId(&idsys, ntile, dparams.nparticles_partile);
		idx = constructArraySTR(buv->nuvs);
		for(n = 0; n < buv->nuvs; n++) {
			uint16_t *uv = getVectorBufUshortVector3D(buv, n);
			float pos[4];
			
			decodeCompressedParticle(uv, pos, &hdr, &idsys, n + dparams.id_start, dparams.cell_interval, dparams.unitsep);
			insertDataArraySTR(idx, n, pos);
		}
		makeTreeWithSameThreadPoolArraySTR(idx, stp);
		MEASURE_TIME_END_PRINTMILLIS(">>>> particles decoding + index construction time");
		printf("  # of particles = %llu\n", buv->nuvs);
		destructBufUshortVector3D(buv);
	}
	else {
		BufFloatVector3D *bfv;
		
		bfv = constructBufFloatVector3D();
		MEASURE_TIME_START();
		bfv = readDecodedParticles(bfv, fname);
		MEASURE_TIME_END_PRINTMILLIS(">>>> particles reading time");
		MEASURE_TIME_START();
		idx = constructArraySTR(bfv->nfvs);
		for (n = 0; n < bfv->nfvs; n++) {
			float *fv = getVectorBufFloatVector3D(bfv, n);
			insertDataArraySTR(idx, n, fv);
		}
		makeTreeWithSameThreadPoolArraySTR(idx, stp);
		MEASURE_TIME_END_PRINTMILLIS(">>>> index construction time");
		printf("  # of particles = %llu\n", bfv->nfvs);
		destructBufFloatVector3D(bfv);
	}
		
	return idx;
}

static 
void 
destructIndex(void *idx)
{
	destructArraySTR(idx);
}

static 
void 
threadJoin(void *arg)
{
	JoinContext *jc = arg;
	/* int tid = fetch_and_add(&jc->jtc.tid, 1); */
	/* processing granularity per thread */
	const int WIDTH = (jc->bh->nhalos > 100) ? 100 : 10;
	
	while (jc->jtc.nprows < jc->bh->nhalos) {
		ulong_t beg, end, row;
		
		/* get range to process */
		beg = fetch_and_add(&jc->jtc.nprows, WIDTH);
		end = beg + WIDTH;
		if (beg >= jc->bh->nhalos)
			break;
		if (end > jc->bh->nhalos)
			end = jc->bh->nhalos;
		
		for (row = beg; row < end; row++) {
			MultiRangeCountContext mrcc = *jc->mrcc;
			
			/* set row-th point to central search point */
			mrcc.central = jc->bh->halo[row].point;
			/* set up result buffer to contain multiple result counts */
			mrcc.counts = &jc->counts[row * mrcc.nrads];
			multiRangeCountWithPriodicBoundArraySTR(jc->idx, &mrcc);
		} 	
	}
	fetch_and_add(&jc->jtc.nfin, 1);
}


/* static inline */
/* void * */
/* secureGets(char *buf, size_t size) */
/* { */
/*         char *p; */
/*         void *ret; */

/*         ret = fgets(buf, size, stdin); */
/*         p = buf + strlen(buf) - 1; */
/*         if (*p == '\n') */
/*                 *p = '\0'; */
/*         else */
/*                 while (getc(stdin) != '\n'); */
/*         return ret; */
/* } */

