/*------------------------------------------------------------------------------------------------------*
 * Program for finding move optimal sokoban solutions                                                   *
 *  Copyright (c) 2015, Gil Dogon                                                                       *
 *                                                                                                      *
 *  Redistribution and use in source and binary forms, with or without modification,                    *
 *  are permitted provided that the following conditions are met:                                       *
 *                                                                                                      *
 *  Redistributions of source code must retain the above copyright notice.                              *
 *                                                                                                      *
 *  Redistributions in binary form must reproduce the above copyright notice.                           *
 *                                                                                                      *
 *  The name of the copyright holder may not be used to endorse or promote products derived from        *
 *  this software without specific prior written permission.                                            *
 *                                                                                                      *
 *  THIS SOFTWARE IS PROVIDED "AS IS" BY THE COPYRIGHT HOLDER WITHOUT ANY WARRANTY WHATSOEVER ...       *
 *------------------------------------------------------------------------------------------------------*/

/*
 * Portability issues:
 * This program assumes sizeof(int) and sizeof(uint32_t)==4 sorry about that ...
 *
 * I tried to write this program using standard ANSI "C" and lowest common denominator Posix/Unix like interfaces
 * However the hash function code assumes support of long longs and Big Endianity! So program will
 * not work correctly on Sparcs without modification to that function.
 *
 * The program uses ints instead of pointers to save space on 64 bit machines. If you do need/want a version
 * of the program to work with a lot of Gigabytes of RAM you will need to do some changes, i.e. convert
 * the ints to pointers and resize some allocations accordingly ...
 *
 * The other major headache assumption is __builtin_clz supported by  gcc/Clang but not in the standard C.
 * If you want to build the program on Windows like environment look at this discussion:
 * http://stackoverflow.com/questions/355967/how-to-use-msvc-intrinsics-to-get-the-equivalent-of-this-gcc-code
 * or just undef (comment) the HAS_CLZ define below
 *
 */

/*
 * A brief outline of the algorithm:
 * We simply do a two sided BFS search on the state space. The forward direction starts from the initial state, and the
 * backward direction starts for the various possible final states (only depending on final sokoban location).
 * A hash keeps all states visited so far, so a new level must consist only of new states. When both searches meet, we
 * know the length of the shortest solution.
 * There is a relatively simple and efficient deadlock detection done (only during forward search), based on impossible location of 
 * a single boxes or a pair of boxes.
 * To print the solution itself (with -s/-S command line) a nifty recursive algorithm is used. 
 */

#include <stdio.h>
#define HAS_CLZ
#ifdef HAS_CLZ
#define BITIND(mask) (31-__builtin_clz(mask))
#else
static int bitind_tab[256];
inline my_bitind(unsigned int m)
{
	int ind=0;
	if (m >= 0x10000) {
		m >>= 16;
		ind=16;
	}
	if (m >= 0x100) {
		m>>=8;
		ind+=8;
	}
	return ind + bitind_tab[m];
}
#define BITIND(mask) my_bitind(mask)
#endif

#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <assert.h>
#include <stdint.h>

#define ASCII_ANIMATE
#ifdef ASCII_ANIMATE
#include <unistd.h>
#endif

#define MEASURE_TIME
#ifdef MEASURE_TIME
#include <sys/resource.h>
#include <sys/time.h>

static double user_time() {
	struct rusage x;
	getrusage(RUSAGE_SELF, &x);
	return x.ru_utime.tv_sec + x.ru_utime.tv_usec * 1E-6;
}

static uint32_t state_hash(uint32_t *state);

double g_time;
static void tm_reset() {
	g_time = user_time();
}
static double tm_used() {
	return user_time() - g_time;
}
#else
static void tm_reset() {
}

static double tm_used() {
	return 0;
}
#endif

#define MAXN 30
#define MAXNS 3  // This is the limit for 'small n' where we use different board representation. (index array instead of mask)
#define MAXBOARD (1 << 11)
// Boards with more than MAXNS boxes are going to be limited to MAXBOARDB places for reasonable search time.
#define MLL 4
#define MAXBOARDB (MLL * 64)
#define DD2_MAXBOARD 256  // This will be max board size for n > 2 (where deadlock2 is used)
#define MMASK (MAXBOARD / 32)
#define DD2_MMASK (DD2_MAXBOARD / 32)

// The following three big constants determine our maximal memory usage ...
// They are tuned below  according to the limits of the sokoban levels competition scorer, but can be easily changed
// if you want to solve bigger/harder levels.

#define HASHBITS 24 
#define LEVEL_MAXMEM 5000000
#define MAXMEM 100000000

static inline void solver_assert(int expression) {
}

typedef unsigned int uint32_t;
#define BIT(arr,bit) ((((uint32_t *)arr)[(bit)>>5]>>((bit)&31))&1)
#define CLRBIT(arr,bit) (((uint32_t *)arr)[(bit)>>5]&= ~(1<<((bit)&31)))
#define SETBIT(arr,bit) (((uint32_t *)arr)[(bit)>>5]|= (1<<((bit)&31)))

// deltas for l,r,u,d neighbors
static int di[4] = { 0, 0, -1, 1 };
static int dj[4] = { -1, 1, 0, 0 };

#define CLEARMASK(mask) (memset(mask,0,g_mask_sz))

typedef short locs_t[4]; // will fit in a long long ...
typedef uint32_t locmask_t[MMASK];

// State access macros. See comment below.
#define BITMASK(state) ((state) + g_bitmask_ind)
#define LPTR(state)    (state[g_lptr_ind])
#define MTITLE 100

/*
 * Admittedly the style of this program is old fashioned, and maybe a bit dirty, but I prefer to use global variables instead of
 * passing a lot of arguments around. Anyway, all those variables are parameters of the level to be solved, and are set
 * by the readin parser routine.
 * If you want to understand the algorithm the next description of the hash state records is crucial:
 * In our hash we keep one state record for each partial board state defined by the positions of  the boxes. You can view it as a 
 * variable sized struct. The state itself is partial because you also need to know position of the sokoban for a full state.
 * There are 3 fields in each state record
 * A. The first field defines the partial state and is either a board bitmask or an array of g_n shorts (for g_n <=3) that contains the locations 
 *    of the boxes on the board for that partial state.
 * B. The second field (accessed by the macro BITMASK above) is actualy two concatenated bitmasks. The first mask contains all positions of the sokoban
 *    that have been reached for this state. The second mask corresponding bit is the search direction from which that particular sokoban location
 *    was found. When we reach the same exact state from both ends, the search for the shortest path is successfull.
 * C. The third field (LPTR) is a pointer to the current level area where a final bitmask is kept, of the newly discovered sokoban locations
 *    in the current level.
 */

static int g_n; // The number of actual boxes;
static int g_state_sz; // The actual size of state in bytes
static int g_mask_sz; // The actual size of a board bitmask in bytes
static int g_mask_isz; // The actual size of a board bitmask in ints
static int g_size; // The size of the board
static int g_hsize; // The size  in ints of hashable part
static int g_hbsize; // The size  in bytes of hashable part
static int g_lstate_sz;
static int g_lstate_isz;
static int g_bitmask_ind;
static int g_lptr_ind;
static int g_smalln; // true if g_n <=3;
static int g_dosol; // !=0 if we want solution printout
static uint32_t *g_middle_state; // records the meet in the middle state
static int g_middle_sloc; // records the meet in the middle state
static int g_verbose = 1;
static int g_dd2;

static int g_moveok = 1; // This global is used (by setting to zero) to enforce a first push in a cyclic board
static int g_force_insert = 0; // This global is used to skip meet solution detection in initializationof cyclic levels
static int g_allstar = 0; // Use to set allstar mode for cyclic levels

struct gboard_t {
	int height; // number of lines in original input board
	short neighbors[MAXBOARD][4]; // l,r,u,d neighboring moves
	short sloc; // initial sokoban position
	short init_b[MAXN + 1]; // initial boxes position + the sokoban
	short target[MAXN]; //final target locations
	unsigned char deadlock1[MAXBOARD]; //One box deadlock detection, 4  bits corresponding to the 4 possible neighbors of a loc.
//If the sokoban is on that neighbor having just pushed to that loc and the bit is set we are deadlocked.
//Two boxes deadlock detection: For each box location  + directions of neighboring sokoan, a bitmask of all other dealocking positions.
	uint32_t deadlock2[DD2_MAXBOARD * 4][DD2_MMASK];
	int is_cyclic;
} g_board;

// Some buffers  used by the input readin routine
#define MAXAREA (1 << 15) //32K seems enough ..
static short g_iboard_area[MAXAREA];
static short* g_iboard[MAXBOARD];  // pointers to input lines
static short g_iboard_len[MAXBOARD]; // the lines length
#define MAXTARGETS (8 * (MAXN * MAXN) / 2) * 3
static short g_targets[MAXTARGETS];

static int g_usleep = 300000; // ASCII animation sleep time ..

static void* safe_malloc(size_t n, uint32_t line, int dozer) {
	void* p = dozer ? calloc(n, 1) : malloc(n);
	if (!p) {
		fprintf(stderr, "[%s:%u]MALLOC: Really out of memory(%d bytes)\n", __FILE__, line, (int) n);
		exit(1);
	}
	return p;
}

#define MALLOC(size) safe_malloc(size,__LINE__,0)
#define CALLOC(size) safe_malloc(size,__LINE__,1)

//
// This is simple flooding algorithm  used to mark squares that are reachable by the sokoban.
//
void flood_connected(int sy, int sx, int size) {
	short *queue = (short *) MALLOC((size + 1) * 2 * sizeof(short));
	queue[0] = sy;
	queue[1] = sx;
	g_iboard[sy][sx] &= ~0x8000; // mark as reachable
	int qs = 0, qe = 2;
	while (qs < qe) {
		int dir;
		int y = queue[qs++], x = queue[qs++];
		for (dir = 0; dir < 4; dir++) {
			int yn = y + di[dir];
			int xn = x + dj[dir];
			if (yn < 0 || yn >= g_board.height)
				continue;
			if (xn < 0 || xn >= g_iboard_len[yn])
				continue;
			if (g_iboard[yn][xn] >= 0)
				continue;
			if ((char) (g_iboard[yn][xn] & 0xff) != '#') {
				solver_assert(qe < size * 2);
				g_iboard[yn][xn] &= ~0x8000; // mark as reachable
				queue[qe++] = yn;
				queue[qe++] = xn;
			}
		}
	}
	free(queue);
	return;
}

/* 
 * A non standard format to print board because I did not bother to print the surrounding walls.
 * Still its easy to figure out wahts happening.
 *  '.' stands for a free space and ' ' is a wall. 'b' is a box and '@' the Sokoban
 */
static void print_board(uint32_t *state, int sloc) {
	//printf("boardlocs hash is (%p): %x\n",state, state_hash(state));
	int i, j;
	for (i = 0; i < g_board.height; i++) {
		for (j = 0; j < g_iboard_len[i]; j++) {
			if (g_iboard[i][j] < 0) {
				putchar(' ');
			} else if (g_iboard[i][j] == sloc) {
				putchar('@');
			} else if (instate(state, g_iboard[i][j])) {
				putchar('b');
			} else {
				putchar('.');
			}
		}
		putchar('\n');
	}
	putchar('\n');
	fflush(stdout);
}

//
// A trivial pool memory allocator , just  allocates until memory is finished. Everytyhings freed at once by alloc_reset
//
typedef struct {
	char *area;
	char *marea;
	char *ptr;   // ptr to end of pool.
	const char *name;
	unsigned size; // size of pool (notice it is int and not long, which means you need to be careful)
} alloc_t;

void alloc_init(alloc_t *a, int size, const char *name) {
	a->name = name;
	a->ptr = a->area = MALLOC(size);
	a->marea = a->area - 8; // This is just  used to have nonzero inds when using the macros below.
	a->size = size;
}

// reset the allocator (essentialy everything is freed ...)
void alloc_reset(alloc_t *a) {
	a->ptr = a->area;
}

inline char *alloc_mem(alloc_t *a, int n) {
	char *ret = a->ptr;
	a->ptr += n;
	if (a->ptr - a->area > a->size) {
		if (g_verbose > 0)
			printf("%s memory exhausted!\n", a->name);
		return 0;
	}
	memset(ret, 0, n);
	return ret;
}

// Those macros will be used to convert pointers to pool objects into integers. they are ugly, but useful
// in 64 bit architectures where memory is at a premium and pointers are twice as heavy (64 bits instead of 32)
// this program is designed now to work on a virtual server and has less than 0.5 gigabytes of memory. If you would
// want to work with more than 4 gigabytes RAM you would need of course to mess with this code ...

#define PTR_TOIND(a,p) ((unsigned)((char *)p - a.marea))
#define IND_TOPTR(type,a,i) ((type *)(a.marea+i))

static int g_debuga = 0;
alloc_t g_state_mem;

#define STATE_TOIND(s) PTR_TOIND(g_state_mem,s)
#define IND_TOSTATE(i) IND_TOPTR(uint32_t,g_state_mem,i)

static inline uint32_t *alloc_state(uint32_t *locs) {
	uint32_t *ret = (uint32_t *) alloc_mem(&g_state_mem, g_state_sz);
	if (!ret)
		return 0;
	memcpy(ret, locs, g_hbsize);
	if (g_debuga) {
		printf("allocated board:\n");
		print_board(ret, -1);
	}
	return ret;
}
//
// This is the hash that is used to hold all our states,
//
typedef struct {
	alloc_t *a;
	uint32_t *area; // The actual pool data where hash objects reside.
	int sizebits;
	int size; // a power of two
	int mask; // size-1 , used for fast modulu with bitwise &
	int n;    // number of elements actually inside
	uint32_t *hash; //indexes of states in underlaying area. I don't use pointers because of their double sizoef.
	int occ_size, *occupied; //Used for faster hash_clear of a sparse hash.
} statehash_t;

static statehash_t *hash_create(alloc_t *a, int sizebits, int fastclear) {
	statehash_t *h = (statehash_t *) CALLOC(sizeof(statehash_t));
	h->a = a;
	h->area = ((uint32_t *) a->area) - 2;  // Subtract 2 here to have only strictly positive indices!
	alloc_reset(a);                   // Notice hash resets the allocator pool !
	h->sizebits = sizebits;
	h->size = 1 << sizebits;
	h->occ_size = fastclear ? h->size >> 4 : 0;
	h->mask = h->size - 1;
	h->n = 0;
	h->hash = (uint32_t *) CALLOC(h->size * sizeof(uint32_t *));
	h->occupied = 0;
	if (h->occ_size)
		h->occupied = (int *) CALLOC(h->occ_size * sizeof(int *));
	return h;
}

static void *hash_clear(statehash_t *h) {
	if (h->n < h->occ_size) {
		int i;
		for (i = 0; i < h->n; i++)
			h->hash[h->occupied[i]] = 0;
	} else {
		memset(h->hash, 0, h->size * sizeof(uint32_t));
	}
	h->n = 0;
	alloc_reset(h->a); //The allocator pool is reset!
}

static void hash_free(statehash_t *h) {
	hash_clear(h);
	free(h->hash);
	free(h->occupied);
	free(h);
}

// An improvised hash functions for board of size upto 256 (which means upto 8 ints bitmask)
// works surprisingly well I think as far as I tested...
static uint32_t state_hash(uint32_t *state) {
	typedef unsigned long long uint64_t;
	uint64_t *l64 = (uint64_t *) state;
	uint64_t hash;
	if (g_hsize <= 4) {
		if (g_hsize == 1) {
			hash = state[0];
		} else {
			hash = l64[0];
			if (g_hsize == 3) {
				hash ^= state[2];
			} else if (g_hsize == 4) {
				hash ^= l64[1];
			}
		}
	} else {
		hash = l64[0] ^ l64[1];
		if (g_hsize == 5) {
			hash ^= state[4];
		} else {
			hash ^= l64[2];
			if (g_hsize == 7) {
				hash ^= state[6];
			} else {
				hash ^= l64[3];
			}
		}
	}
	hash ^= (hash >> 5);
	hash ^= (hash >> 27);
	uint32_t mul = (hash * (hash >> 31)) >> 16;
	return mul + hash;
}

static inline uint32_t *hash_lookup(statehash_t *h, uint32_t *locs, int do_create) {
	uint32_t loc = state_hash((uint32_t *) locs) & h->mask;
	uint32_t jump = 1;
	uint32_t *hash = h->hash;
//g_nlookup++;
	while (hash[loc] != 0) {
		//g_nloop++;
		if (!memcmp(h->area + hash[loc], locs, g_hbsize)) {
			return h->area + hash[loc];
		}
		loc = (loc + jump) & (h->mask);
	}
	if (do_create) {
		if (h->n < h->occ_size) {
			h->occupied[h->n] = loc;
		} else {
			if (h->n >= h->size >> 1) {  //limit is half hash full to avoid excessive collisions in lookup!
				if (g_verbose > 0)
					fprintf(stderr, "hash is too full\n");
				return 0; //error exit
			}
		}
		h->n++;
		uint32_t *n = alloc_state(locs);
		if (!n)
			return 0;
		hash[loc] = n - h->area;
		return n;
	}
	return 0;
}

/*
 * Given the locs array return the index of 'n' if it appears in it or -1 if not.
 */
static inline int getind(uint32_t *state, int n) {
	int i;
	short *locs = (short *) state;
	for (i = 0; i < g_n; i++) {
		if (n == locs[i])
			return i;
	}
	return -1;
}

/* 
 *  Sokoban main input routine : Gets a standard sokoban input format.
 */
static const char *readin(char *inp) {
	int j, i = 0;
	g_board.sloc = -1;
	g_n = 0;
	int starti = -1, startj;
	short *iptr = g_iboard_area;
	g_iboard[0] = iptr;
	char tline[MTITLE];
	char aline[MTITLE];
	static char errbuf[100];
	tline[0] = aline[0] = 0;
	// Skip any  empty lines and read header
	char *g_line = strtok(inp, "\n\r");
	while (g_line) {
		if (!strncmp(g_line, "Title:", 6)) {
			strncpy(tline, g_line + 6, MTITLE);
		} else if (!strncmp(g_line, "Author:", 7)) {
			strncpy(aline, g_line + 7, MTITLE);
		} else if (g_line[0] == ';') {
		} else if (strncmp(g_line, "Score:", 6)) {
			break;
		}
		g_line = strtok(0, "\n\r");
	}

	for (i = 0; g_line && i < MAXBOARD; i++, g_line = strtok(0, "\n\r")) {
		printf("%s\n", g_line);
		if (g_line[0] == ';' || g_line[0] == '\0') {
			--i;
			continue;
		}
		g_iboard[i] = iptr;
		for (j = 0; j < MAXBOARD; j++) {
			int c = ((unsigned char *) g_line)[j];
			if (!c) {
				g_iboard_len[i] = j;
				break;
			}
			*iptr++ = c | 0x8000; //At first mark everything as unconnected ...
			if (iptr - g_iboard_area >= MAXAREA) {
				return "Max input area exceeded!";
			}
			if (c == '@' || c == '+') {
				starti = i;
				startj = j;
			}
		}
	}

	// The next check happens when we had no input, i.e. were at the end of the file...
	if (!(g_board.height = i))
		return "EOF";
	if (starti == -1) {
		return "Missing sokoban";
	}
	flood_connected(starti, startj, iptr - g_iboard_area);

	int ind = 0, nt = 0;
	int nstars = 0;
	for (i = 0; i < g_board.height; i++) {
		for (j = 0; j < g_iboard_len[i]; j++) {
			int c = g_iboard[i][j];
			if (c < 0)
				continue; // not connected
			c = (char) (c & 0xff);
			g_iboard[i][j] = ind;
			ind++;
			if (ind > MAXBOARD) {
				return "Board too big!";
			}
			if (c == ' ')
				continue;
			if (c == '@' || c == '+') {
				if (g_board.sloc != -1) {
					return "Only one sokoban please";
				}
				g_board.sloc = ind - 1;
				if (c == '@')
					continue;
			}
			if (c == '$' || c == '*') {
				g_board.init_b[g_n++] = ind - 1;
				if (g_n > MAXN) {
					return "Too many boxes!";
				}
				if (c == '$')
					continue;
				nstars++;
			}
			if (c == '.' || c == '*' || c == '+') {
				g_board.target[nt++] = ind - 1;
				if (nt > MAXN) {
					return "Too many targets!";
				}
				continue;
			}
			if (g_verbose > 0)
				printf("Line %d plc %d Illegal input character :%c %d!\n", i, j, c, c);
			return "Illegal input character";
		}
	}
	g_size = ind;
	if (g_board.sloc == -1) {
		return "Missing sokoban";
	}
	if (!g_n) {
		return "Missing (reachable) boxes!";
	}
	if (nt != g_n) {
		snprintf(errbuf, 100, "Number of (reachable) boxes is %d, but number of (reachable) goal positions is %d!", g_n, nt);
		return errbuf;
	}
	g_board.init_b[g_n] = g_board.sloc;

	g_smalln = g_n <= MAXNS;
	if (!g_smalln && g_size > MAXBOARDB) {
		return "Board too big for more than  3 boxes!";
	}
	if (g_n > 2 && g_size > DD2_MAXBOARD) {
		return "Board too big for more than  2 boxes!";
	}

	for (i = 0; i < g_board.height; i++) {
		for (j = 0; j < g_iboard_len[i]; j++) {
			int ind = g_iboard[i][j];
			if (ind < 0)
				continue; // not connected
			int dir;
			for (dir = 0; dir < 4; dir++) {
				int ni = i + di[dir];
				int nj = j + dj[dir];
				if (ni < 0 || nj < 0 || ni >= g_board.height || nj >= g_iboard_len[ni] || g_iboard[ni][nj] < 0) {
					g_board.neighbors[ind][dir] = -1; //Illegal move/neighbor
				} else {
					g_board.neighbors[ind][dir] = g_iboard[ni][nj];
				}
			}
		}
	}

	g_board.is_cyclic = nstars == g_n;
	g_board.is_cyclic += g_allstar * g_board.is_cyclic; // for allstar mode we will get a 2 here ...
	printf("%s PARAMS: %d,%d\n", g_board.is_cyclic ? (g_board.is_cyclic == 1 ? "CYCLIC " : "ALLSTAR") : "", g_size, g_n);

	/*
	 * The following code sets all our ugly size globals.
	 */
	g_mask_isz = (g_size + 31) / 32;
	g_mask_sz = g_mask_isz * 4;
	g_lstate_sz = g_mask_sz + 4;
	g_lstate_isz = g_mask_isz + 1;
	int inds_sz = (g_n + 1) / 2;
	g_bitmask_ind = g_smalln ? inds_sz : g_mask_isz;
	g_hsize = g_bitmask_ind;
	solver_assert(g_hsize <= 8); // We support only upto 8 ints in our hash function!
	g_hbsize = g_hsize << 2;
	g_lptr_ind = g_bitmask_ind + (2 * g_size + 31) / 32;
	g_state_sz = g_lptr_ind * 4 + 4; // The size of state in bytes!
	if (g_verbose > 0)
		printf("Will hopefully have available %d states\n", MAXMEM / g_state_sz);
	return 0;
}

static const char *readin_f(FILE *f) {
	static char buf[MAXAREA + 1];
	char *ptr = buf;
	*buf = 0;
	while (fgets(buf, MAXAREA, f)) {
		if (*buf != '\n' && *buf != ';')
			break;
	}
	ptr = buf + strlen(buf);
	int left = buf + MAXAREA - ptr;
	while (fgets(ptr, left, f) && left) {
		if (*ptr == '\n')
			break;
		ptr += strlen(ptr);
		left = buf + MAXAREA - ptr;
	}
	return readin(buf);
}

int instate(uint32_t *s, int loc) {
	if (g_smalln) {
		return getind(s, loc) >= 0;
	} else {
		return BIT(s, loc);
	}
}

typedef struct {
	alloc_t a;
	uint32_t *lstates; // Ugly. We will access directly the allocators area to pass on all lstates
	int n; // Number of lstates
	int nstates; // Total number of postions (for each lstate we have the number of set bits in the mask)
} level_t;

// This just does the initial memory allocation.
static level_init(level_t *lev, int size, const char *name) {
	alloc_init(&(lev->a), size, name);
	lev->lstates = (uint32_t *) lev->a.area; // ugly ...
}

static level_reset(level_t *lev) {
	alloc_reset(&(lev->a));
	lev->n = lev->nstates = 0;
}

//
// This is called when a potentialy new sokoban location state is discovered.
//
inline int state_add_sloc(uint32_t *s, int sloc, int type_bit, level_t *nextlev) {
	int typeloc = sloc + g_size;
	if (BIT(BITMASK(s),sloc) && !g_force_insert) { // already existing...
												   // In a rather unexpected twist we discover the solution right here when a state
												   // is reached from both directions of the search ...
		if (type_bit != BIT(BITMASK(s), typeloc)) {
			g_middle_state = s;
			g_middle_sloc = sloc;
			return 1; // Success exit
		}
		return 0;
	}
	SETBIT(BITMASK(s), sloc);
	if (type_bit)
		SETBIT(BITMASK(s), typeloc);
// Now record this new discovery in the next level
	nextlev->nstates++;
	uint32_t *lstate;
	uint32_t lptr = LPTR(s);
	if (!lptr) { // We enter here the first time this state is entered to nextlev
		lstate = (uint32_t *) alloc_mem(&(nextlev->a), g_lstate_sz);
		if (!lstate)
			return -1; // Memory full exit
		nextlev->n++;
		LPTR(s) = PTR_TOIND(nextlev->a, lstate);
		lstate[0] = STATE_TOIND(s);
	} else {
		lstate = IND_TOPTR(uint32_t, nextlev->a, lptr);
	}
	SETBIT((lstate + 1), sloc);
	return 0;
}

// Insert to the hashset. If out of memory returns -1 Normaly will return 0
// When 1 is returned it signals sucessfull meet in the middle of the search !
int hash_insert(statehash_t *h, uint32_t *locs, int sloc, int type_bit, level_t *lev) {
	uint32_t *s = hash_lookup(h, locs, 1);
	if (!s)
		return -1; //hash full error
	return state_add_sloc(s, sloc, type_bit, lev);
}

typedef int (*addmoves_func_t)(uint32_t *s, int sloc, statehash_t *hash, level_t *lev);
//
//  The basic move generation (backwards) of sokoban state
//
int addmovesb(uint32_t *s, int sloc, statehash_t *hash, level_t *lev) {
	int dir;
	int rc = 0;
	for (dir = 0; dir < 4; dir++) {
		int n = g_board.neighbors[sloc][dir];
		if (n < 0)
			continue;
		if (!BIT(s, n)) { //We have a free space to move (no push/pull)  sokoban. If new, record it.
			if (g_moveok)
				rc |= state_add_sloc(s, n, 1, lev);
		} else { //Try to see if a pull (inverse push) is possible
			int idir = dir ^ 1;
			int in = g_board.neighbors[sloc][idir];
			if (in >= 0 && !BIT(s, in)) {
				locmask_t locs;
				memcpy(locs, s, g_mask_sz);
				SETBIT(locs, sloc);
				CLRBIT(locs, n);
				rc |= hash_insert(hash, locs, in, 1, lev);
			}
		}
	}
	return rc;
}

//
//  The basic move generation (forwards) of sokoban state
//
int addmovesf(uint32_t *s, int sloc, statehash_t *hash, level_t *lev) {
	int i, j, dir;
	int rc = 0;
	for (dir = 0; dir < 4; dir++) {
		int nloc = g_board.neighbors[sloc][dir];
		if (nloc < 0)
			continue;
		if (!BIT(s, nloc)) { // we have a free space to move sokoban. If new, record it.
			if (g_moveok)
				rc |= state_add_sloc(s, nloc, 0, lev);
		} else { // Try to see if a push is possible
			int inloc = g_board.neighbors[nloc][dir];
			if (inloc >= 0 && !BIT(s, inloc)) {
				// First check for simple deadlock
				int idir = dir ^ 1;
				if ((g_board.deadlock1[inloc] >> (idir)) & 1) {
					continue;
				}
				locmask_t locs;
				memcpy(locs, s, g_mask_sz);
				CLRBIT(locs, nloc);
				int dind = (inloc << 2) + idir;
				for (j = 0; j < g_mask_isz; j++) {
					if (g_board.deadlock2[dind][j] & locs[j])
						break;
				}
				if (j < g_mask_isz) {
					continue; //deadlock2 (pair of boxes) discovered ...
				}
				SETBIT(locs, inloc);
				rc |= hash_insert(hash, locs, nloc, 0, lev);
			}
		}
	}
	return rc;
}
//
//  The basic move generation (backwards) of sokoban state for small board (g_n<4)
//
int s_addmovesb(uint32_t *s, int sloc, statehash_t *hash, level_t *lev) {
	int dir;
	int rc = 0;
	for (dir = 0; dir < 4; dir++) {
		int n = g_board.neighbors[sloc][dir];
		if (n < 0)
			continue;
		int ind = getind(s, n);
		if (ind < 0) {
			if (g_moveok)
				rc |= state_add_sloc(s, n, 1, lev);
		} else { //Try to see if a pull (inverse push) is possible
			int idir = dir ^ 1;
			int in = g_board.neighbors[sloc][idir];
			if (in >= 0 && getind(s, in) < 0) {
				locs_t locs;
				memcpy(locs, s, sizeof(locs_t));
				while (ind < g_n - 1 && sloc > locs[ind + 1]) {
					locs[ind] = locs[ind + 1];
					ind++;
				}
				while (ind > 0 && sloc < locs[ind - 1]) {
					locs[ind] = locs[ind - 1];
					ind--;
				}
				locs[ind] = sloc;
				rc |= hash_insert(hash, (uint32_t *) locs, in, 1, lev);
			}
		}
	}
	return rc;
}

//
//  The basic move generation (forwards) of sokoban state for small board
//
int s_addmovesf(uint32_t *s, int sloc, statehash_t *hash, level_t *lev) {
	int i, j, dir;
	int rc = 0;
	for (dir = 0; dir < 4; dir++) {
		int nloc = g_board.neighbors[sloc][dir];
		if (nloc < 0)
			continue;
		int ind = getind(s, nloc);
		if (ind < 0) {
			if (g_moveok)
				rc |= state_add_sloc(s, nloc, 0, lev);
		} else { //Try to see if a push is possible
			int inloc = g_board.neighbors[nloc][dir];
			if (inloc >= 0 && getind(s, inloc) < 0) {
				// First check for deadlock
				int idir = dir ^ 1;
				if ((g_board.deadlock1[inloc] >> (idir)) & 1) {
					continue;
				}
				locs_t locs;
				memcpy(locs, s, sizeof(locs_t));
				int dind = (inloc << 2) + idir;
				locs[ind] = inloc;
				for (j = 0; j < g_dd2; j++) {
					if (BIT(g_board.deadlock2[dind], locs[j]))
						break;
				}
				if (j < g_dd2) {
					continue; // deadlock2 discovered ...
				}
				while (ind < g_n - 1 && inloc > locs[ind + 1]) {
					locs[ind] = locs[ind + 1];
					ind++;
				}
				while (ind > 0 && inloc < locs[ind - 1]) {
					locs[ind] = locs[ind - 1];
					ind--;
				}
				locs[ind] = inloc;
				rc |= hash_insert(hash, (uint32_t *) locs, nloc, 0, lev);
			}
		}
	}
	return rc;
}

static int g_debug = 0;

//
// This is called once level generation was completed to reset the participating states level pointers.
//
void level_complete(level_t *lev) {
	int i;
	uint32_t *lstate = lev->lstates;
	for (i = 0; i < lev->n; i++, lstate += g_lstate_isz) {
		uint32_t *s = IND_TOSTATE(lstate[0]);
		LPTR(s) = 0; //Zero the level pointer of the state
	}
}

//
// This is the main BFS level advance routine. It travereses all states in curr level and uses them to populate next level
//
int level_adv(level_t *curr, level_t *next, statehash_t *shash, int dir) {
	int i, j;
	addmoves_func_t addmoves = (g_smalln) ? (dir ? s_addmovesb : s_addmovesf) : (dir ? addmovesb : addmovesf);
	level_reset(next);
	if (g_debug)
		printf("Debug level with %d hashstates\n", curr->n);

	uint32_t *lstate = curr->lstates;
	for (i = 0; i < curr->n; i++, lstate += g_lstate_isz) {
		uint32_t *s = IND_TOSTATE(lstate[0]);
		int iw, ind0 = 0;
		uint32_t *islocs = lstate + 1;
		for (iw = 0; iw < g_mask_isz; iw++, ind0 += 32) { // This is a loop similar to getlocs getting indices from  bitmask
			uint32_t w = islocs[iw];
			while (w) {
				uint32_t bitm = (w - 1) ^ w;
				int bitind = BITIND(bitm);
				if (g_debug)
					print_board(s, bitind + ind0);
				int rc = (*addmoves)(s, bitind + ind0, shash, next);
				if (rc)
					return rc;
				w &= ~(bitm);
			}
		}
	}
	level_complete(next);
	return 0;
}

// General BFS search routine (works for both forward and backwards according to dir)
// shash , curr should be initialized  properly.
int search(statehash_t *shash, level_t *curr, level_t *next, int dir, int verbose) {
	int nmoves = 0;
	while (curr->n) {
		nmoves++;
		if (g_debug || (verbose && g_verbose > 0))
			printf("LEVEL: %d has %d states total hstates till now %d\n", nmoves, curr->nstates, shash->n);
		int rc = level_adv(curr, next, shash, dir);
		if (rc) {
			solver_assert(rc < 0);
			return rc; // This should only be a failure (-1)
		}
		level_t *pcurr = curr;
		curr = next;
		next = pcurr;
	}
	return 0;
}

int getlocs(uint32_t *mask, short *locs) {
	int i, ind0 = 0, nloc = 0;
	for (i = 0; i < g_mask_isz; i++, ind0 += 32) {
		uint32_t w = mask[i];
		while (w) {
			uint32_t bitm = (w - 1) ^ w;
			int bitind = BITIND(bitm);
			locs[nloc++] = ind0 + bitind;
			w &= ~(bitm);
		}
	}
	return nloc;
}

void make_state(short *bits, int nbits, uint32_t *locs) {
	if (g_smalln) {
		memset(locs, 0, sizeof(locs_t));
		memcpy(locs, bits, g_n << 1);
	} else {
		CLEARMASK(locs);
		int j;
		for (j = 0; j < g_n; j++) {
			int bit = bits[j];
			SETBIT(locs, bit);
		}
	}
}

void start_level(short *ilocs, int n, int dir, level_t *lev, statehash_t *hash) {
	int i, j;
	level_reset(lev);
	locmask_t locs;
	g_force_insert = 1;
	for (i = 0; i < n; i++, ilocs += (g_n + 1)) {
		make_state(ilocs, g_n, locs);
		hash_insert(hash, locs, ilocs[g_n], dir, lev);
	}
	g_force_insert = 0;
	level_complete(lev);
}

void statelocs(uint32_t *state, short *inds) {
	if (g_smalln) {
		memcpy(inds, state, g_n << 1);
	} else {
		getlocs(state, inds);
	}
}

void print_deadlock_board() {
	printf("DEBUG deadlock1 board:\n");
	int i, j;
	for (i = 0; i < g_board.height; i++) {
		for (j = 0; j < g_iboard_len[i]; j++) {
			if (g_iboard[i][j] < 0) {
				putchar(' ');
			} else {
				int deadlock = g_board.deadlock1[g_iboard[i][j]];
				printf("%1x", deadlock);
			}
		}
		putchar('\n');
	}
	putchar('\n');
}

int generate_deadlocks(level_t *curr, level_t *next) {
// Ugly. Use of global g_n to traverse a problem with only one box ...
	int pn = g_n;
	g_n = 1;
	int loc;
	static short inds[MAXBOARD];
	int i, j, dir;
	int orgdbg = g_debug;
	g_debug = 0;
	int ntargets = 0;

	for (i = 0; i < pn; i++) {
		int loc = g_board.target[i];
		for (dir = 0; dir < 4; dir++) {
			int n = g_board.neighbors[loc][dir];
			if (n < 0)
				continue;
			g_targets[2 * ntargets] = loc;
			g_targets[2 * ntargets + 1] = n;
			ntargets++;
			solver_assert(2 * ntargets <= MAXTARGETS);
		}
	}

// Try backward from one targets all possibilities until stuck, and then the uncovered states would be the deadlocking ones ...

	statehash_t *shash = hash_create(&g_state_mem, HASHBITS - 3, 1); // A smaller number of bits is sufficient for small search ...
	start_level(g_targets, ntargets, 1, curr, shash);
	search(shash, curr, next, 1, 0);
	memset(g_board.deadlock1, 0xf, g_size);
	for (i = 0; i < shash->size; i++) {
		int si = shash->hash[i];
		if (!si)
			continue;
		uint32_t *s = shash->area + si;
		statelocs(s, inds);
		int loc = inds[0];
		for (dir = 0; dir < 4; dir++) {
			int n = g_board.neighbors[loc][dir];
			if (n >= 0 && BIT(BITMASK(s), n))
				g_board.deadlock1[loc] &= ~(1 << dir); // mark this as not deadlock!
		}
	}
	if (g_verbose > 1)
		print_deadlock_board();
	g_dd2 = 0; // If original g_n <= 2 this will disable deadlock2 generation and checking.

// Try backward from pair targets all possibilities until stuck, and then the uncovered pair states would be the deadlocking ones ...
	if (pn > 2) {
		g_dd2 = g_n;
		memset(g_board.deadlock2, 0xff, g_size * 4 * DD2_MMASK * sizeof(uint32_t));
		g_n = 2;
		ntargets = 0;
		for (i = 0; i < pn - 1; i++) {
			int loc0 = g_board.target[i];
			for (j = i + 1; j < pn; j++) {
				int loc1 = g_board.target[j];
				for (dir = 0; dir < 8; dir++) {
					int n = g_board.neighbors[(dir >> 2) ? loc0 : loc1][dir & 3];
					if (n < 0 || n == loc0 || n == loc1)
						continue;
					g_targets[ntargets * 3] = loc0;
					g_targets[ntargets * 3 + 1] = loc1;
					g_targets[ntargets * 3 + 2] = n;
					ntargets++;
					solver_assert(3 * ntargets <= MAXTARGETS);
				}
			}
		}
		hash_clear(shash);
		start_level(g_targets, ntargets, 1, curr, shash);
		search(shash, curr, next, 1, 0);
		for (i = 0; i < shash->size; i++) {
			int si = shash->hash[i];
			if (!si)
				continue;
			uint32_t *s = shash->area + si;
			statelocs(s, inds);
			int loc0 = inds[0];
			int loc1 = inds[1];
			//printf("backstate %d %d\n",loc0,loc1);
			for (dir = 0; dir < 4; dir++) {
				int n0 = g_board.neighbors[loc0][dir];
				if (n0 >= 0 && BIT(BITMASK(s), n0)) { // mark this as not deadlock2!
					CLRBIT(g_board.deadlock2[(loc0 << 2) + dir], loc1);
				}
				int n1 = g_board.neighbors[loc1][dir];
				if (n1 >= 0 && BIT(BITMASK(s), n1)) { // mark this as not deadlock2!
					CLRBIT(g_board.deadlock2[(loc1 << 2) + dir], loc0);
				}
			}
		}
		for (dir = 0; dir < 4; dir++) {
			for (i = 0; i < g_size; i++) {
				CLRBIT(g_board.deadlock2[(i << 2) + dir], i);
			}
		}
	}
	g_n = pn;
	hash_free(shash);
	g_debug = orgdbg;
}

int do_solve2(int verbose, int first_push, int last_pull, short *startl, short *endl, int nends, level_t *currf, level_t *currb, level_t* next, statehash_t *shash, uint32_t *sol) {
	int nmoves = 0;
	int nfmoves = 0;
	int nbmoves = 0;
	int rc = 0, i;

	hash_clear(shash);
	if (first_push) {
// In cyclic mode old way we force one push at the start
		start_level(startl, 1, 0, next, shash);
		g_moveok = 0;
		level_adv(next, currf, shash, 0);
		g_moveok = 1;
		start_level(endl, nends, 1, currb, shash);
		nmoves = nfmoves = 1;
	} else if (last_pull) {
// To support cyclic mode in the saner way, we just force a pull at the end!
// This is nice as it does not break normal cyclic levels
// Regretably I was encoraged to "discover" this fact only AFTER the contest has ended ...
		start_level(endl, nends, 0, next, shash); // slight cheat to make this work, mark the dir his as 0 ..
		g_moveok = 0;
		level_adv(next, currb, shash, 1);
		g_moveok = 1;
// And then want to "forget" the final states as reachable from the start.
		uint32_t *initial = IND_TOSTATE(next->lstates[0]);
		memset(BITMASK(initial), 0, (g_lptr_ind - g_bitmask_ind) * 4);
		start_level(startl, 1, 0, currf, shash);
		nmoves = nbmoves = 1;
	} else {
		start_level(endl, nends, 1, currb, shash);
		start_level(startl, 1, 0, currf, shash);
	}

	while (currf->n && currb->n) {
		nmoves++;
		level_t *curr;
		int lev;
		int dir = currf->n > currb->n;
		if (currf->n == currb->n && nfmoves > nbmoves)
			dir = 1; // Thats a subtle case needed only for the recursive dosol mode to end properly
		if (!dir) {
			lev = ++nfmoves;
			curr = currf;
		} else {
			lev = ++nbmoves;
			curr = currb;
		}
		const char *dirs = !dir ? "forward" : "backward";
		if (((verbose & 1) && g_verbose > 0) || g_debug)
			printf("%s LEVEL: %d has %d states total hashstates til now %d\n", dirs, lev, curr->nstates, shash->n);
		rc = level_adv(curr, next, shash, dir);
		if (rc) {
			if (rc < 0)
				return rc; // Out of memory
			if (g_dosol) { // The recursive algorithm to actualy find and print the solution
						   // It  requires some more computation but almost no additional memory ...
				int ssz = g_hsize + 1;
				int ismain = !sol;
				if (ismain) { // The main (non recursive call)
					sol = (uint32_t *) MALLOC(ssz * sizeof(uint32_t) * (nmoves + 1));
					sol[0] = g_board.sloc;
					make_state(g_board.init_b, g_n, sol + 1);
				}
				int midmoves = nfmoves;
				solver_assert(midmoves > 0);
				short midl[MAXN + 1];
				statelocs(g_middle_state, midl);
				midl[g_n] = g_middle_sloc;
				if (g_debug) {
					printf("midmoves %d middle state is:\n", midmoves);
					print_board(g_middle_state, g_middle_sloc);
				}
				sol[midmoves * ssz] = g_middle_sloc;
				memcpy(sol + midmoves * ssz + 1, g_middle_state, g_hsize * 4);
				if ((verbose & 2) && g_verbose > 0)
					printf("DOSOL2 %p %p required %d moves  divided at %d required %d states\n", startl, endl, nmoves, midmoves, shash->n);
				if (midmoves > 1)
					do_solve2(verbose & ~1, first_push, 0, startl, midl, 1, currf, currb, next, shash, sol);
				if (nmoves - midmoves)
					do_solve2(verbose & ~1, 0, last_pull && nmoves - midmoves > 1, midl, endl, nends, currf, currb, next, shash, sol + midmoves * ssz);
				if (ismain) {
					printf("SOLUTION PRINTOUT: %d\n", nmoves);
					int i;
					if (g_dosol == 1) { // Print verbose solution of all board states
						for (i = 0; i < nmoves + 1; i++) {
							printf("%d:\n", i);
							print_board(sol + ssz * i + 1, sol[ssz * i]);
						}
					}
#ifdef ASCII_ANIMATE
					else if (g_dosol == 2) { // Print ascii animation of solution
						printf("\e[2J\e[?25l");
						for (i = 0; i < nmoves + 1; i++) {
							printf("\e[H%d\n", i);
							print_board(sol + ssz * i + 1, sol[ssz * i]);
							usleep(g_usleep);
						}
						printf("\e[?25h");
					}
#endif
					else { // Print compressed solution in LURD format
						char *sstring = MALLOC(nmoves + 1);
						char *dirs = "lrud";
						for (i = 0; i < nmoves; i++) {
							int ploc = sol[ssz * i];
							int cloc = sol[ssz * (i + 1)];
							int dir;
							for (dir = 0; dir < 4; dir++)
								if (g_board.neighbors[ploc][dir] == cloc)
									break;
							solver_assert(dir < 4);
							sstring[i] = dirs[dir];
							if (instate(sol + ssz * i + 1, cloc))
								sstring[i] = toupper(sstring[i]);
						}
						sstring[nmoves] = 0;
						printf("%s\n", sstring);
						free(sstring);
					}
					free(sol);
				}
			}
			return nmoves;
		}
		level_t *pcurr = curr;
		if (curr == currf) {
			currf = next;
		} else {
			currb = next;
		}
		next = pcurr;
	}
	return 0;
}

level_t g_level0, g_level1, g_level2;
// Two way solution routine works from both forward and backwards meeting in the middle
int solve2(int verbose) {
	level_t *currf = &g_level0;
	level_t *currb = &g_level1;
	level_t *next = &g_level2;
	generate_deadlocks(currf, next);
	statehash_t *shash = hash_create(&g_state_mem, HASHBITS, 1);
// Create all the possible final positions ...
	int i, dir, ntargets = 0;
	for (i = 0; i < g_n; i++) {
		int loc = g_board.target[i];
		for (dir = 0; dir < 4; dir++) {
			int n = g_board.neighbors[loc][dir];
			if (n >= 0 && getind((uint32_t *) (g_board.target), n) < 0) {
				memcpy(g_targets + ntargets * (g_n + 1), g_board.target, g_n * sizeof(short));
				g_targets[ntargets * (g_n + 1) + g_n] = n;
				ntargets++;
				solver_assert(ntargets * (g_n + 1) <= MAXTARGETS);
			}
		}
	}
	int rc = do_solve2(verbose, g_board.is_cyclic == 1, 1, g_board.init_b, g_targets, ntargets, currf, currb, next, shash, 0);
	hash_free(shash);
	return rc;
}

void mem_init() {
	alloc_init(&g_state_mem, MAXMEM, "states");
	level_init(&g_level0, LEVEL_MAXMEM, "level0");
	level_init(&g_level1, LEVEL_MAXMEM, "level1");
	level_init(&g_level2, LEVEL_MAXMEM, "level2");
}

int main(int ac, char **av) {
#ifndef HAS_CLZ
	int i;
	for(i=0;i<8;i++) bitind_tab[(1<<(i+1))-1]=i;
#endif
	tm_reset();
	mem_init();
	g_dosol = 0;
	if (ac <= 1) {
		printf("Needs at least an input file argument\n");
		exit(1);
	}
	FILE *f = fopen(av[1], "r");
	if (!f) {
		printf("could not open %s\n", av[1]);
		exit(1);
	}
	if (ac > 2 && !strcmp(av[2], "-a")) { // Calculate cyclic in Allstar format
		g_allstar = 1;
		av++, ac--;
	} else if (ac > 2) {
		if (!strcmp(av[2], "-s")) {
			g_dosol = 3; // Calc solution in LURD format
		} else if (!strcmp(av[2], "-sa")) {
			g_dosol = 2; // Ascii animate solution
			if (ac > 3) {
				g_usleep = atoi(av[3]) * 1000;
			}
		} else if (!strcmp(av[2], "-S")) {
			g_dosol = 1; // Calc solution and print all boards sequence
		} else {
			printf("Usage: soko input_file <-s/-S/-sa milisec_delay>\n");
		}
	}
	const char * errmsg = readin_f(f);
	if (errmsg) {
		printf("ERROR: %s\n", errmsg);
	} else {
		int n = solve2(0);
		if (n < 0) {
			printf("ERROR: Run out of memory ...\n");
		} else if (!n) {
			printf("ERROR: Unsolvable?\n");
		} else {
			printf("Shortest solution is %d moves\n", n);
		}
	}
	double time = tm_used();
	printf("TIME: %g\n", time);

	fflush(stdout);
	return EXIT_SUCCESS;
}
