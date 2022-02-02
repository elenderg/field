

# include "dextern"

	/* variables used locally */

	/* lookahead computations */


int tbitset;  /* size of lookahead sets */
struct looksets lkst [ LSETSIZE ];
int nlset = 0; /* next lookahead set index */
int nolook = 0; /* flag to suppress lookahead computations */
struct looksets clset;  /* temporary storage for lookahead computations */

/* working set computations */

struct wset wsets[ WSETSIZE ];
struct wset *cwp;

	/* state information */

int nstate = 0;		/* number of states */
struct item *pstate[NSTATES+2];	/* pointers to the descriptions of the states */
int tystate[NSTATES];	/* contains type information about the states */
int indgo[NSTATES];		/* index to the stored goto table */
int tstates[ NTERMS ]; /* states generated by terminal gotos */
int ntstates[ NNONTERM ]; /* states generated by nonterminal gotos */
int mstates[ NSTATES ]; /* chain of overflows of term/nonterm generation lists  */

	/* storage for the actions in the parser */

int amem[ACTSIZE];	/* action table storage */
int *memp = amem;	/* next free action table position */

	/* other storage areas */

int temp1[TEMPSIZE]; /* temporary storage, indexed by terms + ntokens or states */
int lineno= 1; /* current input line number */
int fatfl = 1;  	/* if on, error is fatal */
int nerrors = 0;	/* number of errors */

	/* storage for information about the nonterminals */

int **pres[NNONTERM+2];  /* vector of pointers to productions yielding each nonterminal */
struct looksets *pfirst[NNONTERM+2];  /* vector of pointers to first sets for each nonterminal */
int pempty[NNONTERM+1];  /* vector of nonterminals nontrivially deriving e */




/*
 *************************** 
 * main: 
 * 
 */
 
int main (int argc, char *argv[] ){

    /* initialize and read productions */
	setup (argc,argv); 

	tbitset = NWORDS(ntokens);

	/* make table of which productions yield a given nonterminal */
	cpres();

	/* make a table of which nonterminals can match the empty string */
	cempty();


	/* make a table of firsts of nonterminals */
	cpfir();

	/* generate the states */
	stagen();

	/* write the states and the tables */
	output();


	go2out();
	
	hideprod();
	
	summary();
	
	callopt();
	
	others();

	exit (0);
}


others(){ /* put out other arrays, copy the parsers */
	register c, i, j;

	finput = fopen( PARSER, "r" );
	if( finput == NULL ) error( "cannot find parser %s", PARSER );

	warray( "yyr1", levprd, nprod );

	aryfil( temp1, nprod, 0 );
	PLOOP(1,i)temp1[i] = prdptr[i+1]-prdptr[i]-2;
	warray( "yyr2", temp1, nprod );

	aryfil( temp1, nstate, -1000 );
	TLOOP(i){
		for( j=tstates[i]; j!=0; j=mstates[j] ){
			temp1[j] = tokset[i].value;
			}
		}
	NTLOOP(i){
		for( j=ntstates[i]; j!=0; j=mstates[j] ){
			temp1[j] = -i;
			}
		}
	warray( "yychk", temp1, nstate );

	warray( "yydef", defact, nstate );

	/* copy parser text */

	while( (c=getc(finput) ) != EOF ){
		if( c == '$' ){
			if( (c=getc(finput)) != 'A' ) putc( '$', ftable );
			else { /* copy actions */
				faction = fopen( ACTNAME, "r" );
				if( faction == NULL ) error( "cannot reopen action tempfile" );
				while( (c=getc(faction) ) != EOF ) putc( c, ftable );
				fclose(faction);
				ZAPFILE(ACTNAME);
				c = getc(finput);
				}
			}
		putc( c, ftable );
		}

	fclose( ftable );
	}

char *chcopy( p, q )  char *p, *q; {
	/* copies string q into p, returning next free char ptr */
	while( *p = *q++ ) ++p;
	return( p );
	}

# define ISIZE 400
char *writem(pp) int *pp; { /* creates output string for item pointed to by pp */
	int i,*p;
	static char sarr[ISIZE];
	char *q;

	for( p=pp; *p>0 ; ++p ) ;
	p = prdptr[-*p];
	q = chcopy( sarr, nontrst[*p-NTBASE].name );
	q = chcopy( q, " : " );

	for(;;){
		*q++ = ++p==pp ? '_' : ' ';
		*q = '\0';
		if((i = *p) <= 0) break;
		q = chcopy( q, symnam(i) );
		if( q> &sarr[ISIZE-30] ) error( "item too big" );
		}

	if( (i = *pp) < 0 ){ /* an item calling for a reduction */
		q = chcopy( q, "    (" );
		sprintf( q, "%d)", -i );
		}

	return( sarr );
	}

char *symnam(i){ /* return a pointer to the name of symbol i */
	char *cp;

	cp = (i>=NTBASE) ? nontrst[i-NTBASE].name : tokset[i].name ;
	if( *cp == ' ' ) ++cp;
	return( cp );
	}

struct wset *zzcwp = wsets;
int zzgoent = 0;
int zzgobest = 0;
int zzacent = 0;
int zzexcp = 0;
int zzclose = 0;
int zzsrconf = 0;
int * zzmemsz = mem0;
int zzrrconf = 0;

summary(){ /* output the summary on the tty */

	if( foutput!=NULL ){
		fprintf( foutput, "\n%d/%d terminals, %d/%d nonterminals\n", ntokens, NTERMS,
			    nnonter, NNONTERM );
		fprintf( foutput, "%d/%d grammar rules, %d/%d states\n", nprod, NPROD, nstate, NSTATES );
		fprintf( foutput, "%d shift/reduce, %d reduce/reduce conflicts reported\n", zzsrconf, zzrrconf );
		fprintf( foutput, "%d/%d working sets used\n", zzcwp-wsets,  WSETSIZE );
		fprintf( foutput, "memory: states,etc. %d/%d, parser %d/%d\n", zzmemsz-mem0, MEMSIZE,
			    memp-amem, ACTSIZE );
		fprintf( foutput, "%d/%d distinct lookahead sets\n", nlset, LSETSIZE );
		fprintf( foutput, "%d extra closures\n", zzclose - 2*nstate );
		fprintf( foutput, "%d shift entries, %d exceptions\n", zzacent, zzexcp );
		fprintf( foutput, "%d goto entries\n", zzgoent );
		fprintf( foutput, "%d entries saved by goto default\n", zzgobest );
		}
	if( zzsrconf!=0 || zzrrconf!=0 ){
		  fprintf( stdout,"\nconflicts: ");
		  if( zzsrconf )fprintf( stdout, "%d shift/reduce" , zzsrconf );
		  if( zzsrconf && zzrrconf )fprintf( stdout, ", " );
		  if( zzrrconf )fprintf( stdout, "%d reduce/reduce" , zzrrconf );
		  fprintf( stdout, "\n" );
		  }

	fclose( ftemp );
	if( fdefine != NULL ) fclose( fdefine );
	}

/* VARARGS1 */
error(s,a1) char *s; { /* write out error comment */
	
	++nerrors;
	fprintf( stderr, "\n fatal error: ");
	fprintf( stderr, s,a1);
	fprintf( stderr, ", line %d\n", lineno );
	if( !fatfl ) return;
	summary();
	exit(1);
	}

aryfil( v, n, c ) int *v,n,c; { /* set elements 0 through n-1 to c */
	int i;
	for( i=0; i<n; ++i ) v[i] = c;
	}

setunion( a, b ) register *a, *b; {
	/* set a to the union of a and b */
	/* return 1 if b is not a subset of a, 0 otherwise */
	register i, x, sub;

	sub = 0;
	SETLOOP(i){
		*a = (x = *a)|*b++;
		if( *a++ != x ) sub = 1;
		}
	return( sub );
	}

prlook( p ) struct looksets *p;{
	register j, *pp;
	pp = p->lset;
	if( pp == 0 ) fprintf( foutput, "\tNULL");
	else {
		fprintf( foutput, " { " );
		TLOOP(j) {
			if( BIT(pp,j) ) fprintf( foutput,  "%s ", symnam(j) );
			}
		fprintf( foutput,  "}" );
		}
	}

cpres(){ /* compute an array with the beginnings of  productions yielding given nonterminals
	The array pres points to these lists */
	/* the array pyield has the lists: the total size is only NPROD+1 */
	register **pmem;
	register c, j, i;
	static int * pyield[NPROD];

	pmem = pyield;

	NTLOOP(i){
		c = i+NTBASE;
		pres[i] = pmem;
		fatfl = 0;  /* make undefined  symbols  nonfatal */
		PLOOP(0,j){
			if (*prdptr[j] == c) *pmem++ =  prdptr[j]+1;
			}
		if(pres[i] == pmem){
			error("nonterminal %s not defined!", nontrst[i].name);
			}
		}
	pres[i] = pmem;
	fatfl = 1;
	if( nerrors ){
		summary();
		exit(1);
		}
	if( pmem != &pyield[nprod] ) error( "internal Yacc error: pyield %d", pmem-&pyield[nprod] );
	}

int indebug = 0;
cpfir() {
	/* compute an array with the first of nonterminals */
	register *p, **s, i, **t, ch, changes;

	zzcwp = &wsets[nnonter];
	NTLOOP(i){
		aryfil( wsets[i].ws.lset, tbitset, 0 );
		t = pres[i+1];
		for( s=pres[i]; s<t; ++s ){ /* initially fill the sets */
			for( p = *s; (ch = *p) > 0 ; ++p ) {
				if( ch < NTBASE ) {
					SETBIT( wsets[i].ws.lset, ch );
					break;
					}
				else if( !pempty[ch-NTBASE] ) break;
				}
			}
		}

	/* now, reflect transitivity */

	changes = 1;
	while( changes ){
		changes = 0;
		NTLOOP(i){
			t = pres[i+1];
			for( s=pres[i]; s<t; ++s ){
				for( p = *s; ( ch = (*p-NTBASE) ) >= 0; ++p ) {
					changes |= setunion( wsets[i].ws.lset, wsets[ch].ws.lset );
					if( !pempty[ch] ) break;
					}
				}
			}
		}

	NTLOOP(i) pfirst[i] = flset( &wsets[i].ws );
	if( !indebug ) return;
	if( (foutput!=NULL) ){
		NTLOOP(i) {
			fprintf( foutput,  "\n%s: ", nontrst[i].name );
			prlook( pfirst[i] );
			fprintf( foutput,  " %d\n", pempty[i] );
			}
		}
	}

state(c){ /* sorts last state,and sees if it equals earlier ones. returns state number */
	int size1,size2;
	register i;
	struct item *p1, *p2, *k, *l, *q1, *q2;
	p1 = pstate[nstate];
	p2 = pstate[nstate+1];
	if(p1==p2) return(0); /* null state */
	/* sort the items */
	for(k=p2-1;k>p1;k--) {	/* make k the biggest */
		for(l=k-1;l>=p1;--l)if( l->pitem > k->pitem ){
			int *s;
			struct looksets *ss;
			s = k->pitem;
			k->pitem = l->pitem;
			l->pitem = s;
			ss = k->look;
			k->look = l->look;
			l->look = ss;
			}
		}
	size1 = p2 - p1; /* size of state */

	for( i= (c>=NTBASE)?ntstates[c-NTBASE]:tstates[c]; i != 0; i = mstates[i] ) {
		/* get ith state */
		q1 = pstate[i];
		q2 = pstate[i+1];
		size2 = q2 - q1;
		if (size1 != size2) continue;
		k=p1;
		for(l=q1;l<q2;l++) {
			if( l->pitem != k->pitem ) break;
			++k;
			}
		if (l != q2) continue;
		/* found it */
		pstate[nstate+1] = pstate[nstate]; /* delete last state */
		/* fix up lookaheads */
		if( nolook ) return(i);
		for( l=q1,k=p1; l<q2; ++l,++k ){
			int s;
			SETLOOP(s) clset.lset[s] = l->look->lset[s];
			if( setunion( clset.lset, k->look->lset ) ) {
				tystate[i] = MUSTDO;
				/* register the new set */
				l->look = flset( &clset );
				}
			}
		return (i);
		}
	/* state is new */
	if( nolook ) error( "yacc state/nolook error" );
	pstate[nstate+2] = p2;
	if(nstate+1 >= NSTATES) error("too many states" );
	if( c >= NTBASE ){
		mstates[ nstate ] = ntstates[ c-NTBASE ];
		ntstates[ c-NTBASE ] = nstate;
		}
	else {
		mstates[ nstate ] = tstates[ c ];
		tstates[ c ] = nstate;
		}
	tystate[nstate]=MUSTDO;
	return(nstate++);
	}

int pidebug = 0; /* debugging flag for putitem */
putitem( ptr, lptr )  int *ptr;  struct looksets *lptr; {
	register struct item *j;

	if( pidebug && (foutput!=NULL) ) {
		fprintf( foutput, "putitem(%s), state %d\n", writem(ptr), nstate );
		}
	j = pstate[nstate+1];
	j->pitem = ptr;
	if( !nolook ) j->look = flset( lptr );
	pstate[nstate+1] = ++j;
	if( (int *)j > zzmemsz ){
		zzmemsz = (int *)j;
		if( zzmemsz >=  &mem0[MEMSIZE] ) error( "out of state space" );
		}
	}

cempty(){ /* mark nonterminals which derive the empty string */
	/* also, look for nonterminals which don't derive any token strings */

# define EMPTY 1
# define WHOKNOWS 0
# define OK 1

	register i, *p;

	/* first, use the array pempty to detect productions that can never be reduced */
	/* set pempty to WHONOWS */
	aryfil( pempty, nnonter+1, WHOKNOWS );

	/* now, look at productions, marking nonterminals which derive something */

	more:
	PLOOP(0,i){
		if( pempty[ *prdptr[i] - NTBASE ] ) continue;
		for( p=prdptr[i]+1; *p>=0; ++p ){
			if( *p>=NTBASE && pempty[ *p-NTBASE ] == WHOKNOWS ) break;
			}
		if( *p < 0 ){ /* production can be derived */
			pempty[ *prdptr[i]-NTBASE ] = OK;
			goto more;
			}
		}

	/* now, look at the nonterminals, to see if they are all OK */

	NTLOOP(i){
		/* the added production rises or falls as the start symbol ... */
		if( i == 0 ) continue;
		if( pempty[ i ] != OK ) {
			fatfl = 0;
			error( "nonterminal %s never derives any token string", nontrst[i].name );
			}
		}

	if( nerrors ){
		summary();
		exit(1);
		}

	/* now, compute the pempty array, to see which nonterminals derive the empty string */

	/* set pempty to WHOKNOWS */

	aryfil( pempty, nnonter+1, WHOKNOWS );

	/* loop as long as we keep finding empty nonterminals */

again:
	PLOOP(1,i){
		if( pempty[ *prdptr[i]-NTBASE ]==WHOKNOWS ){ /* not known to be empty */
			for( p=prdptr[i]+1; *p>=NTBASE && pempty[*p-NTBASE]==EMPTY ; ++p ) ;
			if( *p < 0 ){ /* we have a nontrivially empty nonterminal */
				pempty[*prdptr[i]-NTBASE] = EMPTY;
				goto again; /* got one ... try for another */
				}
			}
		}

	}

int gsdebug = 0;
stagen(){ /* generate the states */

	int i, j;
	register c;
	register struct wset *p, *q;

	/* initialize */

	nstate = 0;
	/* THIS IS FUNNY from the standpoint of portability */
	/* it represents the magic moment when the mem0 array, which has
	/* been holding the productions, starts to hold item pointers, of a
	/* different type... */
	/* someday, alloc should be used to allocate all this stuff... for now, we
	/* accept that if pointers don't fit in integers, there is a problem... */

	pstate[0] = pstate[1] = (struct item *)mem;
	aryfil( clset.lset, tbitset, 0 );
	putitem( prdptr[0]+1, &clset );
	tystate[0] = MUSTDO;
	nstate = 1;
	pstate[2] = pstate[1];

	aryfil( amem, ACTSIZE, 0 );

	/* now, the main state generation loop */

	more:
	SLOOP(i){
		if( tystate[i] != MUSTDO ) continue;
		tystate[i] = DONE;
		aryfil( temp1, nnonter+1, 0 );
		/* take state i, close it, and do gotos */
		closure(i);
		WSLOOP(wsets,p){ /* generate goto's */
			if( p->flag ) continue;
			p->flag = 1;
			c = *(p->pitem);
			if( c <= 1 ) {
				if( pstate[i+1]-pstate[i] <= p-wsets ) tystate[i] = MUSTLOOKAHEAD;
				continue;
				}
			/* do a goto on c */
			WSLOOP(p,q){
				if( c == *(q->pitem) ){ /* this item contributes to the goto */
					putitem( q->pitem + 1, &q->ws );
					q->flag = 1;
					}
				}
			if( c < NTBASE ) {
				state(c);  /* register new state */
				}
			else {
				temp1[c-NTBASE] = state(c);
				}
			}
		if( gsdebug && (foutput!=NULL) ){
			fprintf( foutput,  "%d: ", i );
			NTLOOP(j) {
				if( temp1[j] ) fprintf( foutput,  "%s %d, ", nontrst[j].name, temp1[j] );
				}
			fprintf( foutput, "\n");
			}
		indgo[i] = apack( &temp1[1], nnonter-1 ) - 1;
		goto more; /* we have done one goto; do some more */
		}
	/* no more to do... stop */
	}

int cldebug = 0; /* debugging flag for closure */
closure(i){ /* generate the closure of state i */

	int c, ch, work, k;
	register struct wset *u, *v;
	int *pi;
	int **s, **t;
	struct item *q;
	register struct item *p;

	++zzclose;

	/* first, copy kernel of state i to wsets */

	cwp = wsets;
	ITMLOOP(i,p,q){
		cwp->pitem = p->pitem;
		cwp->flag = 1;    /* this item must get closed */
		SETLOOP(k) cwp->ws.lset[k] = p->look->lset[k];
		WSBUMP(cwp);
		}

	/* now, go through the loop, closing each item */

	work = 1;
	while( work ){
		work = 0;
		WSLOOP(wsets,u){
	
			if( u->flag == 0 ) continue;
			c = *(u->pitem);  /* dot is before c */
	
			if( c < NTBASE ){
				u->flag = 0;
				continue;  /* only interesting case is where . is before nonterminal */
				}
	
			/* compute the lookahead */
			aryfil( clset.lset, tbitset, 0 );

			/* find items involving c */

			WSLOOP(u,v){
				if( v->flag == 1 && *(pi=v->pitem) == c ){
					v->flag = 0;
					if( nolook ) continue;
					while( (ch= *++pi)>0 ){
						if( ch < NTBASE ){ /* terminal symbol */
							SETBIT( clset.lset, ch );
							break;
							}
						/* nonterminal symbol */
						setunion( clset.lset, pfirst[ch-NTBASE]->lset );
						if( !pempty[ch-NTBASE] ) break;
						}
					if( ch<=0 ) setunion( clset.lset, v->ws.lset );
					}
				}
	
			/*  now loop over productions derived from c */
	
			c -= NTBASE; /* c is now nonterminal number */
	
			t = pres[c+1];
			for( s=pres[c]; s<t; ++s ){
				/* put these items into the closure */
				WSLOOP(wsets,v){ /* is the item there */
					if( v->pitem == *s ){ /* yes, it is there */
						if( nolook ) goto nexts;
						if( setunion( v->ws.lset, clset.lset ) ) v->flag = work = 1;
						goto nexts;
						}
					}
	
				/*  not there; make a new entry */
				if( cwp-wsets+1 >= WSETSIZE ) error( "working set overflow" );
				cwp->pitem = *s;
				cwp->flag = 1;
				if( !nolook ){
					work = 1;
					SETLOOP(k) cwp->ws.lset[k] = clset.lset[k];
					}
				WSBUMP(cwp);
	
			nexts: ;
				}
	
			}
		}

	/* have computed closure; flags are reset; return */

	if( cwp > zzcwp ) zzcwp = cwp;
	if( cldebug && (foutput!=NULL) ){
		fprintf( foutput, "\nState %d, nolook = %d\n", i, nolook );
		WSLOOP(wsets,u){
			if( u->flag ) fprintf( foutput, "flag set!\n");
			u->flag = 0;
			fprintf( foutput, "\t%s", writem(u->pitem));
			prlook( &u->ws );
			fprintf( foutput,  "\n" );
			}
		}
	}

struct looksets *flset( p )   struct looksets *p; {
	/* decide if the lookahead set pointed to by p is known */
	/* return pointer to a perminent location for the set */

	register struct looksets *q;
	int j, *w;
	register *u, *v;

	for( q = &lkst[nlset]; q-- > lkst; ){
		u = p->lset;
		v = q->lset;
		w = & v[tbitset];
		while( v<w) if( *u++ != *v++ ) goto more;
		/* we have matched */
		return( q );
		more: ;
		}
	/* add a new one */
	q = &lkst[nlset++];
	if( nlset >= LSETSIZE )error("too many lookahead sets" );
	SETLOOP(j){
		q->lset[j] = p->lset[j];
		}
	return( q );
	}
