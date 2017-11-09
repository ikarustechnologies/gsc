/*==================================================================
** NAME         :genreg.c
** TYPE         :main
** DESCRIPTION  :reads limits (ra,dec) of each region to generate 
**              :region indexes REGBIN, REGIND
** INPUT        :
** OUTPUT       :
** AUTHOR       :a.p.martinez
** DATE         :11/92
*=================================================================*/

#include <gsc.h>
#define scale 3600000L

static char *GSCDAT ;	/* Default, superseded by $GSCDAT */
static char *GSCBIN = (char *)0; /* Superseded by $GSCBIN */

static long ncmp = 0;

static char ob=0;
static char oc=0;
static char od=0;

static char usage[] = "\
Usage: genreg [-b] [-d] [-c]\n\
       -b : creates files $GSCBIN/regions.bin  and $GSCBIN/regions.ind\n\
	    from $GSCDAT/regions.dat or $GSCDAT/regions.ord\n\
       -c : creates file $GSCDAT/regions.ord\n\
       -d : creates file $GSCDAT/regions.dat\
" ;

typedef struct { unsigned short nr , dec0, dec1 ; } REGDEC ;

#ifdef __MSDOS__
static int fchmod(fno, mod) { return(0) ; }
#endif

static int compar(a,b)
  REGDEC *a, *b;
{
	ncmp++;
  	if (a->dec0 > b->dec0) return(1) ;
  	if (a->dec0 < b->dec0) return(-1) ;
  	if (a->dec1 > b->dec1) return(1) ;
  	if (a->dec1 < b->dec1) return(-1) ;
  	return(0);
}	

static REGDEC regs[9600] ;

main(argc,argv)
  int argc; char **argv;
{
  double deci1,deci2; 
  int i, n;
  int fb,fp,fo,fi;
  char region[256], *s, *r;
  tr_regions areg;
  REGDEC *preg;
  HEADER header;

	if (s = getenv("GSCDAT")) GSCDAT = s;
	if (s = getenv("GSCBIN")) GSCBIN = s;
	else { 
	    strcpy(region, GSCDAT); strcat(region, "/bin");
	    GSCBIN = strdup(region);
    	}
	
	while(--argc>0) {
	    ++argv; 
	    if (**argv == '-') switch(argv[0][1]) {
	        case 'b': ob=1; continue ;
	        case 'c': oc=1; continue ;
	        case 'd': od=1; continue ;
	    }
	    fprintf(stderr, "%s\n", usage);
	    exit(1);
	}

	if ((ob|oc|od)==0) {
	    fprintf(stderr, "%s\n", usage);
	    exit(0);
	}

   
    if (oc || od) {		/* Create the Character Version */

	strcpy(region, GSCDAT); strcat(region, "/regions.ord");
	fprintf(stderr,"....Prepare file: %s ", region);
	fo = open(region, O_BINARY|O_RDWR|O_CREAT|O_TRUNC);
	if (fo < 0 ) { perror(""); exit(1); }
	fchmod(fo, 0644);

	fprintf(stderr,"\n....Reading regions:");

	for(n=0, preg=regs; n<9537; n++, preg++) {
		header.region = n+1;
		header.dmin = n*180./9538. - 90.;
		find_reg(region,header.region);
		fp = open(region,O_BINARY);
		if(fp > 0) s=get_header(fp,&header), close(fp);
		/* else  { perror(region); exit(1) ; } */

		areg.nr  = header.region;
		areg.dec1 = (header.dmin+90.0)*scale;
		preg->nr   = header.region;
		preg->dec0 = (areg.dec1) >> 16;
		preg->dec1 = (areg.dec1) & 0xffff;
		if((areg.nr%100) == 0) fprintf(stderr," %04d",areg.nr);
	}
	
	fprintf(stderr, " %04d.\n", n);

	fprintf(stderr, "----qsort(%p,%d,%d,..)", regs,n,sizeof(REGDEC));
	qsort(regs,n,sizeof(REGDEC),compar);
	fprintf(stderr, " %ld comparisons.\n", ncmp);

	for(i=0, preg=regs, r=(char *)regs; i<n; i++, preg++, r+=strlen(r)) {
	    sprintf(r, "%04d ", preg->nr) ;
	}
	write(fo, regs, r-((char *)regs));
	close(fo);
    }

    if (od) {		/* Create the .dat  Version */

	strcpy(region, GSCDAT); strcat(region, "/regions.dat");
	fprintf(stderr,"....Prepare file: %s ", region);
	fo = open(region, O_WRONLY|O_CREAT|O_TRUNC);
	if (fo < 0 ) { perror(""); exit(1); }
	fchmod(fo, 0644);

	fprintf(stderr, "\nWriting out %s: ", region);

	for(r=(char *)regs, i=0; *r; r += 5, i++) {
	    memset(&header, 0, sizeof(header));
	    header.region = atoi(r);
	    header.dmin = header.region*180./9538. - 90.;
	    find_reg(region,header.region);
	    fp = open(region,O_BINARY);
	    if(fp < 0) { perror(region); exit(1); }
	    else { s=get_header(fp,&header); close(fp); }
	    write(fo, s, strlen(s));
	    if((i%100)==0) fputc('.', stderr);
	}
	close(fo);
	fprintf(stderr, "\n==== %d regions.\n", i);
    }


    if (ob) {	

	fprintf(stderr,"....Prepare files '%s'[.ind]", region);
	strcpy(region, GSCBIN); strcat(region, "/regions.bin");
	fb = open(region, O_BINARY|O_WRONLY|O_CREAT|O_TRUNC);
	if (fb < 0 ) { fprintf(stderr, "\n"); perror(region); exit(1); }
	fchmod(fb, 0644);

	strcpy(region, GSCBIN); strcat(region, "/regions.ind");
	fi = open(region, O_BINARY|O_WRONLY|O_CREAT|O_TRUNC);
	if (fi < 0 ) { fprintf(stderr, "\n"); perror(region); exit(1); }
	fchmod(fi, 0644);

	od = 1;
	strcpy(region, GSCDAT); strcat(region, "/regions.dat");
	fp = open(region, O_BINARY);
	if (fp < 0 ) { 
	    od = 0;
	    strcpy(region, GSCDAT); strcat(region, "/regions.ord");
	    fp = open(region, O_BINARY);
	    if (fp < 0) { fprintf(stderr, "\n"); perror(region); exit(1); }
	    ncmp = read(fp, regs, sizeof(regs));
	    if (ncmp < 0) ncmp &= 0xffff;
	    r = (char *)regs;
	    r[ncmp] = 0;
	    close(fp);
	}
	fprintf(stderr, "\n             from '%s'", region);

	fprintf(stderr, "\nWriting out..:");

	ncmp = 0;
	for(r=(char *)regs, i=0; i<9537; r += 5, i++) {
	    memset(&header, 0, sizeof(header));
	    header.region = atoi(r);
	    header.dmin = header.region*180./9538. - 90.;
	    if (od) s = get_header(fp,&header);
	    else {
	    	find_reg(region,header.region);
	    	fp = open(region,O_BINARY);
	    	if(fp < 0) perror(region);
	    	else { s=get_header(fp,&header); close(fp); }
	    }
	    areg.nr = header.region;
	    areg.nobj = header.nobj;
	    areg.alf1 = header.amin*scale;
	    areg.alf2 = header.amax*scale;
	    areg.dec1 = (header.dmin+90.0)*scale;
	    areg.dec2 = (header.dmax+90.0)*scale;
	    if(areg.alf1 < 0 || areg.alf2 > 360*scale || 
		areg.dec1 <0 || areg.dec2 >180*scale)
		fprintf(stderr, "\n....Header: %s\n",s);
	    if ((i%100) == 0) write(fi, &(areg.dec1), sizeof(INT)),
	    	fprintf(stderr, " %04d", i);
	    ncmp += write(fb,&areg,sizeof(areg));
    	}
    	fprintf(stderr, " %04d\n====Total of %d regions, %ld bytes\n", 
		i, i, ncmp);
	
	if (od) close(fp);
     	close (fb);
     	close (fi);
    }
    exit(0);
}
