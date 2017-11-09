/*==================================================================
** NAME         :gsc.c
** TYPE         :main
** DESCRIPTION  :search by cone(coordinates) in GSC 
**              :finds regions of the GSC in file gsc_reg.bin
**              :finds objects in selected regions
** INPUT        :RA(deci.h), Dec(dec.d), radius1 [,radius2] (arcmin)
**              :
** OUTPUT       :list of GSC objects
**              :
** AUTHOR       :apm
** DATE         :9/92;10/92;
** LIB          :pm.a; use cca to compile
*=================================================================*/

#include <gsc.h>


int compar(a,b)
	char *a, *b;
{
if(strncmp(a+fbeg[fsort],b+fbeg[fsort],flen[fsort]) > 0) return(order);
if(strncmp(a+fbeg[fsort],b+fbeg[fsort],flen[fsort]) < 0) return(-order);
	return(0);
}

main(argc,argv)
	int argc; char **argv;
{
	char **arg, *getenv(), *decode_c(), *get_header();
	int to_d();
	double dispos();
        FILE *fin;
	char line[82],*s,*bl=" ",region[80];
	int narg,sum=0,j,k,i=0,ii,p,stat,ft,x1,x2,z1,z2,xx1,xx2,zz1,zz2;
	int scale=3600000,alpha,hscale,hscad2,xr1,xr2;
	int np,fz,f2,fr,cc,nrec,recl,size; 
	int da,dd,da1,da2,dd1,dd2,nout=0;
	double rar,der,a1,a2,d1,d2,a,phi,phi1=0.0,phi2=10.0;
	double pig=M_PI,radian=180./M_PI,da0,dd0,dist;
	double cosdec,ra0,f,phir=150.0;
        float magmin=0.0, magmax=25.0;

	tr_regions rec[NREC];
	int ind2[1000];
	HEADER header;
	GSCREC gscrec;
	unsigned char *table, *c;
	static char *gsctab;
	char *entete="GSC-id      ra   (2000)   dec  pos-e  mag mag-e  b\
 c   pl mu     d'  pa";
	char *entete10="GSC-id      ra   (2000)   dec  pos-e  mag mag-e  b\
 c   epoch  mu     d'  pa";
	char *entete1="GSC-id        ra   (2000)   dec    pos-e  mag mag-e  b\
 c   pl mu     d'  pa";
	char *entete11="GSC-id        ra   (2000)   dec    pos-e  mag mag-e  b\
 c   epoch  mu     d'  pa";
	char *entete2="GSC-id      ra   (2000)   dec    mag      d'  pa";
	char *entete3="GSC-id        ra   (2000)   dec      mag      d'  pa";

	if(argc == 1)  
	{
	ft = open(getenv("GSCDOC"),0);
	size=lseek(ft,0,2);
	s=(char *)malloc(size);
	lseek(ft,0,0);
	read(ft,s,size);
	write(1,s,size);
	free(s);
	close(ft);
	exit(0);  
	}

/* ------ parse command-line options ------------------------------------*/

        line[0] = '\0';
	arg = argv;
	narg = argc;
        while(--narg) {
          arg++;
          if(strcmp("-r",*arg)==0 && narg > 1) {
                for(j=1;j<narg;j++) {
                strcat(line,arg[j]);
                strcat(line,bl);
                }
/* printf("case-r: <%s>\n",line); */
	  if(line[0] != '-') 
	  {
          for(j=0;j<strlen(line);j++) {
                if(isalpha(line[j])) line[j-1]='\0';
                if(line[j]== '-' || line[j]== ':' || line[j]== '/' ||
			line[j]== ',') line[j]=' '; }
          i=sscanf(line,"%lf%lf",&phi1,&phi2);
          if(i==1) {phi2=phi1; phi1=0.0;}
	  opt[0] = 1;
/* printf("phi= %.1f %.1f\n",phi1,phi2); */
	  }
/* insert next 3 lines to override default radius
	  else {
		printf("radius not provided\n");
		exit(1); }
*/
          line[0] = '\0';
          }

          else if(strcmp("-c",*arg)==0 && narg > 1) {
                for(j=1;j<narg;j++) {
                strcat(line,arg[j]);
                strcat(line,bl);
                }
/* printf("case-c: <%s>\n",line); */
	  if(line[0] != '-') 
	  {
          for(j=0;j<strlen(line);j++) {
                if(isalpha(line[j])) line[j-1]='\0'; }
	  if(to_d(line,&da0,&dd0) <0) {
		printf("format error in %s\n",line);
		exit(1);  }
/* printf("%s = da0,dd0=%.3f %.3f, phi= %.2f\n",line,da0,dd0,phi2);	 */
          line[0] = '\0';
	  opt[1] = 1;
	  }
	  else {
		printf("coordinates not provided\n");
		exit(1); }
          }

          else if(strcmp("-f",*arg)==0 && narg > 1) {
                s = *++arg;
                if(*s != '-') {
                        if(!(fin=fopen(s,"r"))) {
                        printf("unable to open file %s\n",s);
                        exit(1); }
			opt[2] = 1;  }
                arg--;
          }

          else if(strcmp("-d",*arg)==0 && narg > 1) {
                s = *++arg;
		if(*s != '-') {
			config = s;
			opt[3] = 1; }
                arg--;
          }

          else if(strcmp("-m",*arg)==0 && narg > 2) {
                opt[4] = 1;
                magmin = atof(*++arg);
                magmax = atof(*++arg);
                arg -= 2;
          }

          else if(strcmp("-s",*arg)==0) {
	  	opt[5] = 11;
		if(narg >1) {
			opt[5]=atoi(*++arg);
			arg--; }
		if(opt[5]==0)opt[5]=11;
          }

          else if(strcmp("-p",*arg)==0 && narg >1) {
		opt[6] = atoi(*++arg);
		arg--;
	  }

          else if(strcmp("-n",*arg)==0 && narg >1) {
		opt[7] = atoi(*++arg);
		arg--;
	  }

          else if(strcmp("-h",*arg)==0) {
	  	opt[8] = 1;
          }

          else if(strcmp("-l",*arg)==0 && narg >1) {
		opt[9] = atoi(*++arg);
		arg--;
	  }

          else if(strcmp("-e",*arg)==0) {
	  	opt[10] = 1;
          }
        }

/* ------ input according to old, no-option format --------------- */

	for(j=0;j<sizeof(opt)/4;j++) sum += opt[j];
	if(argc >= 3 && sum == 0)
	{
	da0=atof(argv[1]);
	dd0=atof(argv[2]);
	opt[1] = 1;
	if(argc>=4) { 
		phi2 = atof(argv[3]);
		opt[0] = 1;
		}
	if(argc==5) { 
		phi1 = phi2;
		phi2 = atof(argv[4]);
		}
	}
	else if(argc <= 3 && sum == 0) {
		phi2 = atof(argv[1]);
		opt[0] = 1;
		if(argc == 3) {
			phi1 = phi2;
			phi2 = atof(argv[2]);
			}
	}

/* ------ open region list and read region index ------------------- */

	fz=open(getenv("REGBIN"),0);  
	if(fz < 1) {
		printf("+++bad gsc_regions file+++\n");
		exit(1); }

	f2=open(getenv("REGIND"),0);  
	if(f2 < 1) {
		printf("+++bad index file+++\n");
		exit(1); }
	size = lseek(f2,0,2);
	lseek(f2,0,0);
	cc=read(f2,ind2,size);
	close(f2);

/* ------ controls and settings ----------------------------------- */

	if((opt[1] & opt[2])== 1) {
		printf("decide yourself: -c or -f ?\n");
		exit(1);  }
	if(opt[5] < 0) {
		opt[5]= -opt[5]; 
		order= -1;}
	if(opt[5] > 12) {
		printf("only fields from 1 to 12 available for sorting\n");
		exit(1); }
	if(opt[5]==8 && opt[10]==1)opt[5]=13;
	if(opt[7] < 0) opt[7] = 0;
	if(opt[7]>0 && opt[5]==0) opt[5]=11;
	if(opt[9] == 0 && opt[5] != 0)opt[9]=MAXNSORT;
	if(opt[7]>opt[9]) opt[7]=opt[9];
	if(opt[7] == 0 && opt[5] != 0)opt[7]=opt[9];
	fsort = opt[5]-1;
	gsctab = (char *)malloc(opt[9]*81);
/* insert next 3 lines to override default radius 
	if(opt[0]==0) {
		printf("radius not provided\n");
		exit(1); }
*/
	if(opt[1]==0) {
		if(opt[2]==1) {
		fin = fopen(input,"r");
		fgets(line,80,fin); }
		else gets(line);
	}
	c = (unsigned char *)malloc(12);
	
/* ------ start operations ----------------------------------------- */

	do
	{
	if(opt[1]==0) {
		if(to_d(line,&da0,&dd0) <0) {
		printf("error format in %s\n",line);
		exit(1);  }
		}
	if(opt[8] == 1) {
		if(opt[6]==0) {
			if(opt[10]==0) puts(entete);
			else puts(entete10); }
		if(opt[6]==1) {
			if(opt[10]==0) puts(entete1);
			else puts(entete11); }
		if(opt[6]==2) puts(entete2);
		if(opt[6]==3) puts(entete3);
	}

	alpha = da0*15.*scale;
	cosdec = cos(dd0/radian);
	ra0 = da0*15.0;

/* ------ find r.a. - dec box ------------------------------- */
	phi = phi2/60.;
	d1 = dd0-phi;
	d2 = dd0+phi;
	if(d1 < -90.0) d1= -90.0;
	if(d2 >  90.0) d2= 90.0;
	z1 = (d1+90.)*scale;
	z2 = (d2+90.)*scale;

	hscale = 360.0*scale;
	hscad2 = 180.0*scale;
	if(fabs(dd0)+phi == 90.000000)
	{
	  x1 = alpha - 90.0*scale;
	  x2 = alpha + 90.0*scale;
	}
	else if(fabs(dd0)+phi < 90.000000)
	{
	  a=asin(sin(phi/radian)/cosdec)*radian*scale;
	  x1 = alpha - a;
	  x2 = alpha + a;
	}
	else 
	{
	  x1 = 0;
	  x2 = hscale;
	}
	stat = 0;
	if(x2 > hscale) {
		x2 -= hscale;
		x1 -= hscale;
		stat = 1;  }
	if(x1 < 0) { stat = 1;  }
	xr1 = x1;
	xr2 = x2;
/* ------ searching regions in index ---------------------------- */

	for(i=0;i<size/4;i++) {
		if(ind2[i] < z1-phir/60.*scale)ii=i;
		else break;
		}
	if((i= --ii) < 0)i=0;
	p = i*100*sizeof(tr_regions);
	lseek(fz,p,0);
	zz1 = -hscale;

	while(zz1 <= z2)
	{
	cc=read(fz,rec,sizeof(rec));
	if(cc < 1) { 
		break;  }
	nrec = cc/sizeof(tr_regions);
	for(i=0;i<nrec;i++)
	{
	  zz1 =rec[i].dec1;
	  zz2 =rec[i].dec2;
	  if(zz1 < z2 && zz2 > z1)
	  {
	    xx1 = rec[i].alf1;
	    xx2 = rec[i].alf2;

	    x1 = xr1;
	    x2 = xr2;
	    if(xx2 > hscale || (x1<0 && xx2>hscad2)) { 
			xx1 -= hscale;
			xx2 -= hscale;
			if(stat == 0) {
			x1 -= hscale;
			x2 -= hscale; }
	    		} 

	    if(xx1 < x2 && xx2 > x1)
	      {
		a1 = (double)x1/scale;
		a2 = (double)x2/scale;
		if (x1 < 0 && rec[i].alf2 > hscad2) {
		a1 += 360.0;
		a2 += 360.0;
		}

/* ------ open and decode selected region -------------------------- */

	find_reg(region,rec[i].nr);
	fr = open(region,0);
	if (fr < 0) { 
	    fprintf(stderr, "Can't find region %s (%d)\n", region, rec[i].nr);
	    exit(1);
	}
	get_header(fr,&header);
	size = lseek(fr,0,2)-header.len;
	np = size/12;                         /* version 1 */
	table = (unsigned char *)malloc(size);
	lseek(fr,header.len,0);
	read(fr,table,size);
	close(fr);

	da1 = (a1-header.amin)*header.scale_ra+0.5;
	da2 = (a2-header.amin)*header.scale_ra+0.5;
	dd1 = (d1-header.dmin)*header.scale_dec+0.5;
	dd2 = (d2-header.dmin)*header.scale_dec+0.5;

	for(k=0;k<np;k++)
	{
	c = table+(k*12);                     /* version 1 */

	da = ((c[1]&1)<< 8) | c[2];
	da <<= 8;
	da |= c[3];
	da <<= 8;
	da |= c[4];
	da >>= 3;

	dd = c[4] & 7;
	dd <<= 8;
	dd |= c[5];
	dd <<= 8;
	dd |= c[6];

	if(da>da1 && da<da2 && dd>dd1 && dd<dd2)
		{
		s = decode_c(c,&header,&gscrec);

		rar = gscrec.ra;
		der = gscrec.dec;
		gscrec.posang=dispos(&ra0,&dd0,&rar,&der,&dist);
		gscrec.dist = dist;
		if(dist < phi2 && dist >= phi1)
	       {
                  if(gscrec.m >= magmin && gscrec.m <= magmax)
             {
		if(opt[5] == 0) prtgsc(opt[6],opt[10],gscrec);
		else {
		strcpy(gsctab+(nout*81),s);
		sprintf(gsctab+(nout*81+59),"; %6.2f %3.0f %8.3f\0",
			dist,gscrec.posang,gscrec.epoch);
		nout++;
		if(nout > opt[9]) {
		 printf("+++ max number of sorted lines (%d) excedeed\n",
			opt[9]);
		 printf("+++ retry with option -l no-of-line\n");
		 exit(0); }
		}
	      }
	       }
		}
	}
	free(table);
	      
	    }
	  }
	  if(zz1 > z2) break;
	}
	}
	if(opt[5] != 0) {
		qsort(gsctab,nout,81,compar);
		if(opt[7] < nout) nout = opt[7];
		for(k=0;k<nout;k++) {
		if(opt[6]==0) puts(gsctab+(k*81)); 
		else {
			tab2rec(gsctab+(k*81),&gscrec);
			prtgsc(opt[6],opt[10],gscrec);
			}
		}
	}
	if(opt[1]==1) break;
	if(opt[2]==1) s=fgets(line,80,fin);
	else s=gets(line);
	if(s) printf("\n");
	nout = 0;
	}
	while(s);

	close(fz);
}
