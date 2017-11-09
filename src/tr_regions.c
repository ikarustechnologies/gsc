/*==================================================================
** NAME         :tr_regions.c
** TYPE         :main
** DESCRIPTION  :transform the aux. table REGIONS of the GSC
**              :to decimal deg and to binary format
** INPUT        :table regions (48 char.s/rec)
**              :
** OUTPUT       :
**              :
** AUTHOR       :a.p.martinez
** DATE         :9/92
** LIB          :
*=================================================================*/
#include <stdio.h>
#include <string.h>
	typedef struct { int nr,ra1,ra2,dec1,dec2; } tr_regions ;
	typedef struct  { int addr,d1; } ind_reg;

void collapse(tr_reg, nrec)
        tr_regions *tr_reg ;
	int nrec;
{
	char *getenv();
	ind_reg ind2[200];
	int prev,field,dummy,line;
        int i=0,j=0,fi;
	double zz;

        prev = -2;
        while(i < nrec)
        {
	line = tr_reg->dec1;
	tr_reg++;

	field = line;

           if(field != prev)
           {
	      ind2[j].addr = i*sizeof(tr_regions);
	      ind2[j].d1 = line;
	      j++;
              prev = field;
           }
	i++;
        }
	fi = creat(getenv("REGIND"),0644);
	if(fi <= 0)  fi = open(getenv("REGIND"),1);
	write(fi,ind2,j*sizeof(ind_reg));
/*
	i=0;
	while(i <j) {
		zz = (ind2[i].d1/3600000.0) -90.0;
		printf("%4d %10d %.4f\n",i,ind2[i].d1,zz);
		i++;
		}
*/
       return;
}

int compar(a,b)
	tr_regions *a, *b;
{
	if(a->dec1 > b->dec1) return(1);
	if(a->dec1 < b->dec1) return(-1);
	return(0);
}	
main(argc,argv)
	int argc; char **argv;
{
	double deci1,deci2,z1; void tod(); 
	int k,scale=3600000, i=0, reg;
	char str[81], *gets();
	char a1[15],a2[15],d1[15],d2[15];
	tr_regions *tr_reg , *ind2;

	tr_reg = (tr_regions *)malloc(9600*sizeof(tr_regions));	
	while(gets(str))
	{
	sscanf(str,"%5d%13c%12c%9c%9c",&reg,a1,a2,d1,d2);

	tr_reg->nr = reg;
	tod(a1,&deci1);  
	tod(a2,&deci2);  
	if(deci2 < deci1) deci2 += 24.0;
	tr_reg->ra1 = deci1*15.0*scale;
	tr_reg->ra2 = deci2*15.0*scale;
	tod(d1,&deci1);  
	tod(d2,&deci2);  
	if(deci1 > deci2) {
		z1 = deci1;
		deci1 = deci2;
		deci2 = z1; }
	tr_reg->dec1 = (deci1+90.0)*scale;
	tr_reg->dec2 = (deci2+90.0)*scale;
/*
	printf("%d %d %d %d %d\n",reg,tr_reg->ra1,tr_reg->ra2,tr_reg->dec1,
		tr_reg->dec2);
*/
	tr_reg++;
	i++;
	}


	tr_reg -= i; 
	qsort(tr_reg,i,sizeof(tr_regions),compar);
	collapse(tr_reg,i);
	write(1,tr_reg,i*sizeof(tr_regions));

}
