/*==================================================================
** NAME         :gsc_enc.c  Version 2 (GSC 1.1)
** TYPE         :main
** DESCRIPTION  :encodes a GSC region in 12 char/entry
**              :plus a header with info on coding
** INPUT        :
**              :
** OUTPUT       :
**              :
** AUTHOR       :a.p.martinez
** DATE         :09/92
*=================================================================*/
#include <stdio.h>
#include <string.h>
#include <math.h>

#define LREC 46
int band[10] = { 0,1,6,8,10,11,12,13,14,18 };  

/*==================================================================
** NAME         :lookreg.c
** TYPE         :int
** DESCRIPTION  :binary search in regions file of GSC
**              :
**              :
** AUTHOR       :apm
** DATE         :09/92;
*=================================================================*/

int lookreg(fp,z)
	FILE *fp;
	char *z;
{
	int ls,cond,p0=0,p1,p,lrec=49;
	char *rec,*plat;

	rec = (char *)malloc((lrec+1));
	plat = (char *)malloc(6);
	fseek(fp,0,2);
	p1 = ftell(fp)/lrec;
	fseek(fp,0,0);
	fgets(rec,lrec,fp);
	strncpy(plat,rec,5);
	*(plat+5) = '\0';

	if(strcmp(plat,z) >= 0) return(p0);
	cond=0;
	while(cond == 0)
	{
	   p=(p1+p0)/2;
	   fseek(fp,p*lrec,0);
	   fgets(rec,lrec,fp);
	   strncpy(plat,rec,5);
	   *(plat+5) = '\0';
	   if(strcmp(plat,z) <= 0 && p-p0 < 10) cond=1;
/*
	   printf("%s %s %d\n",plat,z,p-p0);  
*/
	   if(strcmp(plat,z) > 0) p1=p;
	   else            p0=p;
	}
	free(rec);
	free(plat);
	return(p*lrec);
}

main(argc,argv)
	int argc; char **argv;
{
	int count;
	void tod(); char *getenv();
	int i,npl,k=0,offset,error;
	int hlen,nbl,scale_ra,scale_dec,scale_pos,scale_mag,vers,magoff;
	char *header, *p, *s, *rec, *reca, *reg, *plat, *plate;
	char cnpl[10],coo[13], *ww, *list;
	double amin,amax,dmin,dmax, dx;
	FILE *f2;
	int ba;
	int id,da,dd,dp,mag,dm,mb,cl,pl,mul;
	unsigned char c[12];

	plate = (char *)malloc(6);
	plat = (char *)malloc(6);
	reg = (char *)malloc(50);

/* ----- reading region no. and list of plates -----------  */

	gets(plate);
	gets(cnpl);
	npl = atoi(cnpl);
	list = (char *)malloc(npl*5);
	for(i=0;i<npl;i++) {
		gets(list+(5*i)); 
	}

/* ----- searching ra,dec limits of region -----------  */

	f2 = fopen(getenv("REGIONS"),"r");
	offset = lookreg(f2,plate);
	fseek(f2,offset,0);

	*plat = '\0';
	while(strncmp(plate,plat,5) != 0) {
		fgets(reg,50,f2);
		strncpy(plat,reg,5); 
		*(plat+5)='\0';
	}
	strncpy(coo,reg+6,12); coo[12]='\0';
	tod(coo,&amin);
	amin *= 15.0;
	strncpy(coo,reg+19,11); coo[11]='\0';
	tod(coo,&amax);
	strncpy(coo,reg+31,8); coo[8]='\0';
	tod(coo,&dmin);
	strncpy(coo,reg+40,8); coo[8]='\0';
	tod(coo,&dmax);
	if(dmin > dmax) dmin = dmax;
		
    /* fprintf(stderr, "%s%.5f %.5f\n",reg,amin,dmin);  */

/* ------header information (ASCII)--------------------------------

	1) size of header;
	2) region no.					
	3) encoding version;
	4) offset r.a.(deg);
	5) offset dec.(deg);
	6) offset mag.
	7) scale_ra;
	8) scale_dec;
	9) scale_pos;
       10) scale_mag;
       11) no. of plates;
       12-27) plate list;	

	Encoding Version #2 : scaling factors

	r.a.		 100000
	dec.		 100000
	err.position	     10
	magnitude	    100
	err. magnit.	    100

   ------printing header --------------------------------------------*/
	
	header = (char *)malloc(128);
	vers = 2;
	magoff = 0;
	scale_ra = 100000;
	scale_dec = 100000;
	scale_pos = 10;
	scale_mag = 100;
	sprintf(header,"%s %d %.5f %.5f %d %d %d %d %d %d",plate,vers,
		amin,dmin,magoff,scale_ra,scale_dec,scale_pos,scale_mag,npl);
	hlen = strlen(header)+npl*5 +4;
	nbl = (4- (hlen%4))%4;
	printf("%3d %s",hlen+nbl,header);
	for(i=0;i<npl;i++) printf(" %s",list+(i*5));
	for(i=0;i<nbl;i++) printf(" ");	
	fflush(stdout);
		
/* ------reading GSC-region --------------------------------------*/

	reca = (char *)malloc(LREC);
	ww = (char *)malloc(40);
	error = 0;

	count = 0;
	while(gets(reca))
	{
/* ------ scaling and offsetting data --------------------------------- */

	count++;
	strncpy(ww,reca,5); *(ww+5) = '\0';
	id = atoi(ww);
	if(id >= (1<<14)) error++,
 	   fprintf(stderr, "***** error region %s.%d id=%d\n",plate, count,id);

	strncpy(ww,reca+5,9); *(ww+9) = '\0';
	dx = atof(ww) - amin;
	if (dx >= 360.) dx -= 360.;
	da = (dx*(double)scale_ra)+0.5;
	if ((da < 0) || (da > 4194303)) error++,
 	   fprintf(stderr, "***** error region %s.%d da=%d\n",plate, count,da);

	strncpy(ww,reca+14,9); *(ww+9) = '\0';
	dd = ((atof(ww)- dmin )*(double)scale_dec)+0.5;
	if ((dd < 0) || (dd > 524287)) error++,
 	   fprintf(stderr, "***** error region %s.%d dd=%d\n",plate, count,dd);

	strncpy(ww,reca+23,5); *(ww+5) = '\0';
	dp = atof(ww)*scale_pos;
	if(dp > 511) 
 	   fprintf(stderr, "++++warning region %s.%d dp=%d (511)\n",
	   plate, count,dp), dp = 511;

	strncpy(ww,reca+28,5); *(ww+5) = '\0';
	mag = atof(ww)*scale_mag;
	if ((mag < 0) || (mag >= (1<<11))) error++,
	  fprintf(stderr, "***** error region %s.%d mag=%d\n",plate, count,mag);

	strncpy(ww,reca+33,4); *(ww+4) = '\0';
	dm = atof(ww)*scale_mag;
	if(dm >= (1<<7)) error++,
 	   fprintf(stderr, "***** error region %s.%d dm=%d\n",plate, count,dm);

	strncpy(ww,reca+37,2); *(ww+2) = '\0';
	mb = atoi(ww);
	for(i=0;i<10;i++)
		if(mb == band[i]) ba = i;

	strncpy(ww,reca+39,1); *(ww+1) = '\0';
	cl = atoi(ww);

	strncpy(plat,reca+40,4); *(plat+4) = '\0';
	for(i=0;i<npl;i++) 
		if(strncmp(plat,list+(5*i),4)==0) pl=i;

	strncpy(ww,reca+44,1); *(ww+1) = '\0';
	mul = 0;
	if(strncmp("T",ww,1) == 0) mul = 1;

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

	c[0] = id >> 7;
	c[1] = (id << 1) | (da  >> 21);
	c[2] = da >> 13;
	c[3] = da >> 5;
	c[4] = (da << 3) | ((dd >> 16) & 7);
	c[5] = dd >> 8;
	c[6] = dd;
	c[7] = dp >> 1;
	c[8] = (dp << 7) | (dm & (255 >> 1));
	c[9] = mag >> 3;	
	c[10] = (mag << 5) | (ba << 1) | mul;
	c[11] = pl | (cl << 4);

/* --------------------------------------------------------------------- */	

	write(1,c,12);

	}

	/* fprintf(stderr, ".... Errors: %d\n", error);*/
	exit(error);
	/*if(error == 1) 
 	fprintf(stderr, "***** error in coding region %s *****\n",plate); */
}

