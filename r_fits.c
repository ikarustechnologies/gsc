/*==================================================================
** NAME         :r_region.c
** TYPE         :main
** DESCRIPTION  :transform a REGION of the GSC
**              :from table-fits format to ASCII table sorted by dec
** INPUT        :table (fits) region
** OUTPUT       :table region (48+1 chars/rec)
** AUTHOR       :a.p.martinez
** DATE         :9/92
** LIB          :
*=================================================================*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

int collapse(tab,nrec,lrec,res)

/*----- collapse sorted tab(nrec x lrec) into unique lines ->res -----*/

        char *tab ,*res;
	int nrec,lrec;
{
	char *p,*t,*prev,*line;
        int i=0,j=0;

	prev = (char *)malloc(lrec+1);
	line = (char *)malloc(lrec+1);
	p = res;
	t = tab;
        *prev = '\0';
        while(i < nrec)
        {
	strcpy(line,t);
	t += lrec;

           if(strcmp(line,prev) != 0)
           {
		strcpy(p,line);
		p += lrec;
		j++;
		strcpy(prev,line);
           }
	i++;
        }
	free(prev); free(line);
	return(j);
}

int compar1(a,b)
	char *a, *b;
{
	if(strncmp(a+14,b+14,9)>0) return(1);
	if(strncmp(a+14,b+14,9)<0) return(-1);
	return(0);
}	

int compar2(a,b)
	char *a, *b;
{
	if(strcmp(a,b) > 0) return(1);
	if(strcmp(a,b) < 0) return(-1);
	return(0);
}	

main(argc,argv)
	int argc; char **argv;
{
	char str[10],*fits,*rec,*plate,*list, *table;
	int ll,lrec,nrec,i,npl,nregion,mode=0;
	double ra,amin=360.0,amax=0.0,dmin,dmax;
	FILE *fp;

/* ------ mode = 0 : read, sort by dec, print (header+table)
   ------ mode = 1 : read, print (table)
   ------ default : mode = 0
*/

	if(argc > 1) mode = atoi(argv[1]);
	fits = (char *)malloc(2880);
	*fits = '\0';
	while(strncmp("XTENSION=",fits,9) != 0)
	{
	read(0,fits,2880);
	}	
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
/*
	printf("lrec %d, nrec %d plate %05d\n",lrec,nrec,nregion);
*/
	plate = (char *)malloc(nrec*5);
	table = (char *)malloc(nrec*lrec);
	read(0,table,nrec*lrec);
	
/* ---- generate list of plates and find ra-min ra-max ------------- */

	for(i=0;i<nrec;i++) {
		strncpy(plate+i*5,table+(i*lrec)+40,4);
		*(plate+i*5+4) = '\0';
		strncpy(str,table+(i*lrec)+5,9);
		str[9] = '\0';
		ra = atof(str);
		if(ra>((amin+amax)/2. +60.)) ra -= 360.;
		if(ra<((amin+amax)/2. -60.)) ra += 360.;
		if(ra < amin) amin = ra;
		if(ra > amax) amax = ra;
	}

	if(mode == 0)
	{
/* ---- sort by dec and find dec-min dec-max ----------------------- */

	qsort(table,nrec,lrec,compar1);
	strncpy(str,table+14,9); str[9] = '\0';
	dmin = atof(str);
	strncpy(str,table+(nrec-1)*lrec+14,9); str[9] = '\0';
	dmax = atof(str);

/* ---- sort plates and retain uniques ----------------------------- */
	ll = 5;
	qsort(plate,nrec,ll,compar2);
	list = (char *)malloc(100);
	npl = collapse(plate,nrec,ll,list);

	printf("%05d\n%4d\n",nregion,npl);
	for(i=0;i<npl;i++) {
		printf("%s\n",list); 
		list += ll; 
	}
	printf("%.5f %.5f %.5f %.5f\n",amin,amax,dmin,dmax);
	}
	for(i=0;i<nrec;i++) {
		strncpy(fits,table+(i*lrec),lrec);
		*(fits+lrec) = '\0';
		puts(fits);
	}
  exit(0);
}
