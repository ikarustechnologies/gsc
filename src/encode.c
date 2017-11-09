/*==================================================================
** NAME         :encode.c  Version 2 (GSC 1.1)
** TYPE         :main
** DESCRIPTION  :encodes a GSC region in 12 char/entry
**              :plus a header with info on coding
** INPUT        :
** OUTPUT       :
** AUTHOR       :a.p.martinez
** DATE         :11/92
** Revised GSC 1.2 F. Ochsenbein
*=================================================================*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define LREC 46
#define MAXPL 15

static char plates[5*MAXPL];
static int nplates = 0;
typedef int (*SORT_FCT)(/* const void *, const void * */) ;

int band[] = { 0,1,6,8,10,11,12,13,14,18, 4,3,16, -1 };  /* -1 = Terminator */

typedef struct {        /* Record Contents */
        long id;
        double ra,dec;
        float dp, mag, dm;
        char mb[2];
        char cl, plat[4], mul;
	} REC;

int compar(a,b)
	REC *a, *b;
{
	int ia,ib;

	ia=a->dec*100000.0;
	ib=b->dec*100000.0;
	return(ia - ib);
}	

static int addplate(pl)
	char pl[4];
{
	char *p;
	
	for(p=plates; *p; p += 5)
		if(strncmp(p,pl,4)==0) return(0);
	strncpy(p,pl,4);
	nplates += 1;
	return(nplates);
}

main(argc,argv)
	int argc; char **argv;
{
	REC *gsc, *table;
	char *fits,*rec,*pl;
	int ll,lrec,nrec,i,npl,nregion,mode=0;
        double amin=360.0,amax=0.0,dmin,dmax,dx;

	int count=0,error=0;
	int hlen,nbl,scale_ra,scale_dec,scale_pos,scale_mag,vers,magoff;
	char *header,ww[5];
	int ba,da,dd,dp,mag,dm,mb,cl,pla,mul;
	unsigned char c[12];

/* ------ mode = 0 : read, sort by dec, print (header+table)
   ------ mode = 1 : read, print (table)
   ------ default : mode = 0
*/

	if(argc > 1) mode = atoi(argv[1]);
/*
   ---- reads FITS Table ---------------------------------------------
*/
	fits = (char *)malloc(2880);
	*fits = '\0';
	while(strncmp("XTENSION=",fits,9) != 0)
		read(0,fits,2880);
	rec = fits;
	while(strncmp("NAXIS1",rec,6) != 0) rec += 80;
	lrec = atoi(rec+10);
	rec += 80;
	while(strncmp("NAXIS2",rec,6) != 0) rec += 80;
	nrec = atoi(rec+10);
	rec += 80;
	while(strncmp("EXTNAME",rec,7) != 0) rec += 80;
	nregion = atoi(rec+47);
	read(0,fits,2880);

/* ---- reads GSC records ------------------------------------ */

	table = (REC *)malloc(sizeof(REC)*nrec);
	for(i=0,gsc=table; i<nrec; i++,gsc++)
	{
		read(0,fits,lrec);
		*(fits+lrec) = '\0';
		gsc->mul = *(fits+44);
		strncpy(gsc->plat,fits+40,4);
		gsc->cl = *(fits+39);
		gsc->mb[0] = *(fits+37);
		gsc->mb[1] = *(fits+38);
		fits[37] = '\0';
		gsc->dm = atof(fits+33);
		fits[33] = '\0';
		gsc->mag = atof(fits+28);
		fits[28] = '\0';
		gsc->dp = atof(fits+23);
		fits[23] = '\0';
		gsc->dec = atof(fits+14);
		fits[14] = '\0';
		gsc->ra = atof(fits+5);
/* ---- find ra-min ra-max ----------------------------------------- */
                if(i>0 && gsc->ra>(amax+60.)) gsc->ra -= 360.;
                if(i>0 && gsc->ra<(amin-60.)) gsc->ra += 360.;
                if(gsc->ra < amin) amin = gsc->ra;
                if(gsc->ra > amax) amax = gsc->ra;
		fits[5] = '\0';
		gsc->id = atoi(fits);
/* ---- find list of different plates ------------------------------ */
		addplate(gsc->plat);
	}
	npl = nplates;

	if(mode == 0)
	{
/* ---- sort by dec and find dec-min dec-max ----------------------- */

	qsort(table,nrec,sizeof(REC),(SORT_FCT)compar);
	dmin = table->dec;
	gsc = table+nrec-1;
	dmax = gsc->dec;
	}
/*
	printf("%05d %d %d\n",nregion,nrec,npl);
	for(i=0,pl=plates;i<npl;i++,pl +=5) {
		printf("%s\n",pl); 
	}
        printf("%.5f %.5f %.5f %.5f\n",amin,amax,dmin,dmax);
	
	for(i=0;i<nrec;i++) {
		puts(table+(i*(lrec+1)));
	}
*/

/* ------header information (ASCII)--------------------------------

	 1) size of header;
	 2) encoding version;
	 3) region no.					
	 4) number of records;
	 5) offset r.a.(deg);
	 6) ra - max;
	 7) offset dec.(deg);
	 8) dec - max;
	 9) offset mag.
	10) scale_ra;
	11) scale_dec;
	12) scale_pos;
        13) scale_mag;
        14) no. of plates;
        15-nn) plate list;	

	Encoding Version #2 : scaling factors

	r.a.		 100000
	dec.		 100000
	err.position	     10
	magnitude	    100
	err. magnit.	    100

   ------printing header --------------------------------------------*/
	
	header = (char *)malloc(200);
	vers = 2;
	magoff = 0;
	scale_ra = 100000;
	scale_dec = 100000;
	scale_pos = 10;
	scale_mag = 100;
	sprintf(header,"%d %05d %d %.5f %.5f %.5f %.5f %d %d %d %d %d %d",
		vers,nregion,nrec,amin,amax,dmin,dmax,magoff,scale_ra,
		scale_dec,scale_pos,scale_mag,npl);
	hlen = strlen(header)+npl*5 +4;
	nbl = (4- (hlen%4))%4;
	printf("%3d %s",hlen+nbl,header);
	for(i=0,pl=plates;i<npl;i++,pl += 5) printf(" %s",pl);
	for(i=0;i<nbl;i++) printf(" ");	
	fflush(stdout);

	for(count=0,gsc=table;count<nrec;gsc++,count++)
	{

/* ------ scaling and offsetting data --------------------------------- */

	if(gsc->id >= (1<<14)) error++,
 	   fprintf(stderr, "****error region %04d.%d id=%d\n",
		nregion,count,gsc->id);

	dx = gsc->ra - amin;
	if (dx >= 360.) dx -= 360.;
	da = (dx*(double)scale_ra)+0.5;
	if ((da < 0) || (da > 4194303)) error++,
 	   fprintf(stderr, "****error region %04d.%d da=%d\n",nregion,count,da);

	dd = ((gsc->dec - dmin )*(double)scale_dec)+0.5;
	if ((dd < 0) || (dd > 524287)) error++,
 	   fprintf(stderr, "****error region %04d.%d dd=%d\n",nregion,count,dd);

	dp = gsc->dp*scale_pos +0.5;
	if(dp > 511) 
 	   fprintf(stderr, "++++warning region %04d.%d dp=%d (511)\n",
	   nregion, count,dp), dp = 511;

	mag = (gsc->mag-magoff)*scale_mag +0.5;
	if ((mag < 0) || (mag >= (1<<11))) error++,
	  fprintf(stderr, "****error region %04d.%d mag=%d\n",nregion,count,mag);

	dm = gsc->dm*scale_mag +0.5;
	if(dm >= (1<<7)) error++,
 	   fprintf(stderr, "****error region %04d.%d dm=%d\n",nregion,count,dm);

	strncpy(ww,gsc->mb,2); ww[2] = '\0';
	mb = atoi(ww);
	for(i=0; (band[i] >= 0) && (band[i] != mb);i++)  ;
	ba = i ;
	if (band[i] < 0)
	    fprintf(stderr, "****Unknown band value: %d\n", mb) ;

	ww[0] = gsc->cl;
	ww[1] = '\0';
	cl = atoi(ww);

	for(i=0,pl=plates;i<npl;i++,pl +=5) 
		if(strncmp(gsc->plat,pl,4)==0) pla=i;

	mul = 0;
	if(gsc->mul == 'T') mul = 1;

/* ------ Encoding Version #2 : length of scaled data -----------------

	Data		bits

	spare		 1
	ID		14
	scaled RA	22
	scaled DEC	19
	pos.err.	 9
	magnitude	11
	mag.error	 7
	mag.band	 4      (Vers.1: 3)
	class.		 3	(Vers.1: 1)
	plate code	 4
	multip.code	 1	F=0,T=1
	
   ------ coding a record in c[12] --------------------------------- */

	c[0] = gsc->id >> 7;
	c[1] = (gsc->id << 1) | (da  >> 21);
	c[2] = da >> 13;
	c[3] = da >> 5;
	c[4] = (da << 3) | ((dd >> 16) & 7);
	c[5] = dd >> 8;
	c[6] = dd;
	c[7] = dp >> 1;
	c[8] = (dp << 7) | (dm & (255 >> 1));
	c[9] = mag >> 3;	
	c[10] = (mag << 5) | (ba << 1) | mul;
	c[11] = pla | (cl << 4);

/* --------------------------------------------------------------------- */	
	write(1,c,12);
	}

	exit(error);
}
