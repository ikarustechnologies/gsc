/*==================================================================
.NAME         :gsc-server.c
.TYPE         :main
.DESCRIPTION  :search by cone(coordinates) in GSC 
             :finds regions of the GSC in file gsc_reg.bin
             :finds objects in selected regions

.AUTHOR       :apm, maa
.DATE         :9/92;10/92; 04/95
*=================================================================*/

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <syslog.h>

#include "gsc.h"

#define PHIMIN   0.0
#define PHIMAX  10.0
#define MAGMIN   0.0
#define MAGMAX  25.0

#define DEFSORT    5
#define DSS_PATH   "/home/ac3d/gsc"

char *http;

main(argc,argv)
    int argc; char **argv;
{
    char **arg, *decode_c(), *get_header();
    int to_d(), compar();
    double dispos();
    FILE *fin;
    char *s,*bl=" ",region[80];
    int narg,sum=0,j,k,l,i=0,kk,p,stat,ft,x1,x2,z1,z2,xx1,xx2,zz1,zz2;
    int scale=3600000,alpha,hscale,hscad2,xr1,xr2;
    int np,fz,f2,fr,cn,nrec,recl,indsize,size; 
    int da,dd,da1,da2,dd1,dd2,nout=0;
    double rar,der,a1,a2,d1,d2,a,phi,phi1=PHIMIN,phi2=PHIMAX;
    double pig=M_PI,radian=180./M_PI,da0,dd0,dist;
    double cosdec,ra0,f,phir=150.0;
    float magmin=MAGMIN, magmax=MAGMAX;
    
    tr_regions rec[NREC];
    int ind2[1000];
    HEADER header;
    GSCREC gscrec;
    unsigned char *table, *c;
    static char *gsctab;
    static char hdr[]="gsc-id      ra   (2000)   dec  pos-e  mag mag-e  b\
 c   pl mu     d'  pa\n";
    static char hdr10[]="gsc-id      ra   (2000)   dec  pos-e  mag mag-e  b\
 c   epoch  mu     d'  pa\n";
    static char hdr1[]="gsc-id        ra   (2000)   dec    pos-e  mag mag-e  b\
 c   pl mu     d'  pa\n";
    static char hdr11[]="gsc-id        ra   (2000)   dec    pos-e  mag mag-e\
  b c   epoch  mu     d'  pa\n";
    static char hdr2[]="gsc-id      ra   (2000)   dec    mag mu     d'  pa\n";
    static char hdr3[]="gsc-id        ra   (2000)   dec      mag mu\
     d'  pa\n";
    
    char path[256], binpath[256], indpath[256], line[256];
    struct sockaddr_in name;
    int length = sizeof(name);
    char *inet_ntoa(), *getenv(), *netaddr = NULL;
    

    /* check here if we are called from an HTTP deamon */
    if( (http=getenv("QUERY_STRING")) || (http=getenv("PATH_INFO")) );
    
    if(http){
      printf("Content-Type: text/plain\n\n");
      fflush(stdout);
      strcpy(path, DSS_PATH);
    } else {
	 if(argc == 1){
	      printf("Usage: %s <gsc-path>\n", argv[0]);
	      exit(0);  
	 }
	 strcpy(path, argv[1]);
    }

    /* ------ identify ourselves to the system log deamon */
    openlog("GSC",(LOG_PID | LOG_CONS), LOG_USER);
 
    if(argc == 2){
    /* ------ get name of peer and log it in the syslog  */
	 if(getpeername(0, (struct sockaddr *)&name, &length) >= 0)
	   netaddr = inet_ntoa(name.sin_addr);
    }
    
    /* ------ set up configuration ------------------------------------- */
    
    sprintf(binpath, "%s/tab/gsc_regions.bin", path);
    sprintf(indpath, "%s/tab/gsc_regions.ind", path);
    
    /* ------ open region list and read region index ------------------- */
    
    fz=open(binpath,0);  
    if(fz < 1) {
	 sprintf(line, "** gsc-server: Can't open %s\n", binpath); 
	 perror(line);
	 syslog(LOG_ERR, "Can't open %s\n", binpath); 
	 exit(1); }
    
    f2=open(indpath,0);  
    if(f2 < 1) {
	 sprintf(line, "** gsc-server: Can't open %s\n", indpath);
	 perror(line);
	 syslog(LOG_ERR, "Can't open %s\n", indpath); 
	 exit(1); }
    indsize = lseek(f2,0,2);
    lseek(f2,0,0);
    if( (cn=read(f2,ind2,indsize)) < indsize){
	 sprintf(line, "** gsc-server: Corrupted index file\n");
	 perror(line);
	 syslog(LOG_ERR, "Corrupted index file %s\n", indpath); 
	 exit(1); }
    close(f2);
    
    /* ------ controls and settings ----------------------------------- */
#if 0
static int opt[11] = { 0,0,0,0,0,0,0,0,0,0,0 };
       /* options:     0 1 2 3 4 5 6 7 8 9 10		  */
       /* options:     r c f d m s p n h l e              */
#endif

#define radius	opt[0]
#define center	opt[1]
#define filein	opt[2]
#define mfiltr	opt[3]
#define magf	opt[4]
#define sortf	opt[5]
#define format	opt[6]
#define linout	opt[7]
#define hdrout	opt[8]
#define maxsor	opt[9]
#define epochf	opt[10]

    sortf  = DEFSORT;
    format = 3;
    hdrout = 1;
    linout = maxsor = MAXNSORT;
    gsctab = (char *)malloc(maxsor*81);
    c = (unsigned char *)malloc(12);

/* ------ loop reading input line  ------------------------------------*/

    if(http){
      strncpy(line, http, sizeof(line)-1);
      l = 1;
    } else {
      l = read(0,line,sizeof(line));
      line[--l] = '\0'; /* strip out the newline */
    }
    while(l > 0){
	if(!scan(line, &da0, &dd0, &phi1, &phi2, &magmin, &magmax)
	   && !http) continue;

	if(sortf) fsort  = sortf-1;
	if(hdrout == 1) {
	     switch(format){
		case 0:
		  if(epochf==0) write(1,hdr,strlen(hdr));
		  else write(1,hdr10,strlen(hdr10));
		  break;
		case 1:
		  if(epochf==0) write(1,hdr1,strlen(hdr1));
		  else write(1,hdr11,strlen(hdr11));
		  break;
		case 2: 
		  write(1,hdr2,strlen(hdr2));
		  break;
		case 3: 
		  write(1,hdr3,strlen(hdr3));
		  break;
	     }
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
	if(fabs(dd0)+phi == 90.0) {
	  x1 = alpha - 90.0*scale;
	  x2 = alpha + 90.0*scale;
	} else if(fabs(dd0)+phi < 90.0) {
	  a=asin(sin(phi/radian)/cosdec)*radian*scale;
	  x1 = alpha - a;
	  x2 = alpha + a;
	} else {
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

	zz1 = z1-phir/60.*scale;
        j = indsize/4 - 1;
	for(i=0; i < j ;i++) {
	  if(ind2[i] >= zz1) break;
        }
	p = i*100*sizeof(tr_regions);
	lseek(fz,p,0);

	zz1 = -hscale;
	while(zz1 <= z2){
	  if( (cn=read(fz,rec,sizeof(rec))) < 1) break;
	  nrec = cn/sizeof(tr_regions);
	  for(i=0;i<nrec;i++){
	     zz1 =rec[i].dec1;
	     zz2 =rec[i].dec2;
	     if(zz1 < z2 && zz2 > z1){
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

		if(xx1 < x2 && xx2 > x1) {
		     a1 = (double)x1/scale;
		     a2 = (double)x2/scale;
		     if (x1 < 0 && rec[i].alf2 > hscad2) {
			  a1 += 360.0;
			  a2 += 360.0;
		     }

/* ------ open and decode selected region -------------------------- */

		     find_reg(region,rec[i].nr,path);
		     fr = open(region,0);
		     if (fr < 0) { 
		       fprintf(stderr, 
			  "** gsc-server: Can't find region %s (%d)\n", 
			       region, rec[i].nr);
		       syslog(LOG_ERR, "Can't find region %s (%d)\n", 
			       region, rec[i].nr);
		       exit(1);
		     }
		     get_header (fr,&header);
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

		     for(k=0;k<np;k++){
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

			if(da>da1 && da<da2 && dd>dd1 && dd<dd2){
			   s = decode_c(c,&header,&gscrec);
	       
			   rar = gscrec.ra;
			   der = gscrec.dec;
			   gscrec.posang=dispos(&ra0,&dd0,&rar,&der,&dist);
			   gscrec.dist = dist;
			   if(dist < phi2 && dist >= phi1 &&
			      gscrec.m >= magmin && gscrec.m <= magmax) {
			       if(!mfiltr || gscrec.mu == 'F') {
				    if(sortf == 0) {
					 prtgsc(format,epochf,gscrec);
					 nout++;
					 if(nout >= linout) goto done;
				    } else {
					 strcpy(gsctab+(nout*81),s);
					 sprintf(gsctab+(nout*81+59),
						 "; %6.2f %3.0f %8.3f\0",
						 dist,gscrec.posang,
						 gscrec.epoch);
					 nout++;
					 if(nout > maxsor) goto done;
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
      done:
	if(sortf != 0) {
	     qsort(gsctab,nout,81,compar);
	     if(nout > linout) nout = linout;
	     for(k=0; k<nout ;k++) {
		  if(format==0) 
		    write(1,gsctab+(k*81),strlen(gsctab+(k*81))); 
		  else {
		    tab2rec(gsctab+(k*81),&gscrec);
		    prtgsc(format,epochf,gscrec);
		  }
	     }
	}
	write(1, "[EOD]\n", 6);
	nout = 0;
	if(http){
	  l = 0;
	} else {
	  l = read(0,line,sizeof(line));
	  line[--l] = '\0'; /* strip out the newline */
	}
    }
    if(netaddr) syslog(LOG_INFO, "Served %s\n", netaddr);
    close(fz);
}


int scan(line,ra,dec,r1,r2,m1,m2)
char *line;
double *ra, *dec, *r1, *r2;
float *m1, *m2;
{
	char *pp, *cc, *qq, *s;
	int rh, rm, ddd, dm, n, ii, jj;
	double rs, ds, x, y;

	*m1 = MAGMIN;
	*m2 = MAGMAX;
	*r1 = PHIMIN;
	*r2 = PHIMAX;
	format = 3;
	sortf  = DEFSORT;
	center = mfiltr = 0;
	linout = maxsor;

	rs = ds = 0.;
	rh = rm = ddd = dm = ii = jj = 0;
	pp = strtok(line, ";/");
	while(pp && *pp){
	     while(*pp && *pp == ' ') pp++;
	     cc = pp;
	     while(*pp && isalpha(*pp)) pp++;
	     switch(tolower(*cc)){
		case 'f':
		  /* format [0-3] */
		  while(*pp && !isdigit(*pp)) pp++;
		  n = atoi(pp);
		  format = (n == 9 || n == 8 || (n >= 0 && n <= 3))? n : 3;
		  break;
		case 'n':
		  /* linout  */
		  while(*pp && !isdigit(*pp)) pp++;
		  n = atoi(pp);
		  linout = (n > 0 && n <= maxsor)? n : maxsor;
		  break;
		case 's':
		  /* sort field [1-12]|{r|d|m|-} */
		  while(*pp && !isalnum(*pp)) pp++;
		  switch(*pp){
		     case 'i': sortf = 1; break;
		     case 'r': sortf = 2; break;
		     case 'd': sortf = 3; break;
		     case 'm': sortf = 5; break;
		     case 'R': sortf = 11; break;
		     default:
		       if(isdigit(*pp)) {
			    n = atoi(pp);
			    sortf = (n >= 0 && n <= 12)? n : 11;
		       } else {
			    fprintf(stderr,
                              "Wrong sort field >%s<. Type ? for help.\n", pp);
			    return(0);
		       }
		  }
		  break;
		case 'r':
		  /* radius [1] 2 in arcmin */
		  while(*pp && !isalnum(*pp)) pp++;
		  if((s=strchr(pp, ','))) *s = ' ';
		  x = y = 0.;
		  ii = jj = 0;
		  sscanf(pp, "%lf %lf", &x, &y);
		  sscanf(pp, "%d %d", &ii, &jj);
		  if( ii > 0 && (int)x != ii ) x = (double)ii;
		  if( jj > 0 && (int)y != jj ) y = (double)jj;
		  if( y == 0. ){ 
		       *r2 = (x == 0.)? PHIMAX : x; 
		       *r1 = 0.;
		  } else {
		       *r1 = x;
		       *r2 = y;
		  }
		  radius = 1;
		  break;
		  
		case 'q': exit(0);

		case '*': 
		  /* filter multiple stars out */
		  mfiltr = 1;
		  break;

		case 'm':
		  /* limit mag 1 [2] */ 
		  while(*pp && !isalnum(*pp)) pp++;
		  if((s=strchr(pp, ','))) *s = ' ';
		  sscanf(pp, "%f %f", m1, m2);
		  sscanf(pp, "%d %d", &ii, &jj);
		  if( ii > 0 )
		    *m1 = ( (int)*m1 == ii)? *m1 : (float)ii;
		  if( jj > 0 )
		    *m2 = ( (int)*m2 == jj)? *m2 : (float)jj;
		  magf = 1;
		  break;
		       
		default:
		  if(isdigit(*cc) 
		     || *cc == '-' || *cc == '+' || *cc == 'c'){
		       /* center coordinates: 
			  hh mm ss.s +/-dd mm ss.s or
			  h.hhhh +/-d.dddd
			  */
		       for(qq=pp; *qq && *qq != '+' && *qq != '-'; qq++);
		       *dec = atof(qq);
		       for(s=qq; *s ; s++)
			 if(!isdigit(*s) && *s != '.') *s = ' ';
		       sscanf(qq, "%d %d %lf", &ddd, &dm, &ds);
		       x = (double)abs(ddd) + dm/60. + ds/3600.;
		       y = (*dec < 0.)? -*dec : *dec;
		       if( y < x ){
			    *dec = x;
			    *dec *= (ddd < 0)? -1. : 1. ;
		       }
		       *qq = '\0';
		       *ra = atof(pp);
		       for(s=pp; *s ; s++)
			 if(!isdigit(*s) && *s != '.') *s = ' ';
		       sscanf(pp, "%d %d %lf", &rh, &rm, &rs);
		       x = (double)rh + rm/60. + rs/3600.;
		       if( *ra < x ) *ra  = x;
		       center = 1;

		  } else if(*cc > ' ') { 
		       help();
		       if(*cc != '?' && tolower(*cc) != 'h' && !http)
			 fprintf(stderr, "** Unknown command: %s\n", cc);
		       return(0); 
		  }
		  break;
	     }
	     pp = strtok((char *)0, ";/");
	}
	if(!center){
	     if(!http) fprintf(stderr, 
		 "** You must supply a coordinate center. Type ? for help.\n");
	     return(0);
	}
/* 
	printf("Query: %g %g %g %g %g %g\n", *ra,*dec,*r1,*r2,*m1,*m2);
	fflush(stdout);
*/
	return(1);
}

int compar(a,b)
	char *a, *b;
{
if(strncmp(a+fbeg[fsort],b+fbeg[fsort],flen[fsort]) > 0) return(order);
if(strncmp(a+fbeg[fsort],b+fbeg[fsort],flen[fsort]) < 0) return(-order);
	return(0);
}

#define ITEMS(a)	sizeof(a)/sizeof(a[0])

static int range[24] = {1,594,1178,1729,2259,2781,3246,3652,4014,4294,4492,
	4615,4663,5260,5838,6412,6989,7523,8022,8464,8840,9134,9346,9490};
static char *subdir[] = {"N0000","N0730","N1500","N2230","N3000","N3730",
	"N4500","N5230","N6000","N6730","N7500","N8230","S0000",
	"S0730","S1500","S2230","S3000","S3730","S4500","S5230",
	"S6000","S6730","S7500","S8230"};


int find_reg (region,nreg,path)
	char *region, *path;
	int nreg;
{
	int i;

	for (i = ITEMS(range); --i >= 0; )
		if (nreg >= range[i]) break;

	sprintf(region,"%s/%s/%04d.GSC",path,subdir[i],nreg);
	return(1);
}

help()
{
     static char helptxt[] = 
"Searching the Guide Star Catalogue (1.1) by coordinates.\n\
Enter a sequence of commands separated with slashes (/) all in one line.\n\
\n\
command    parameters      format/units\n\
\n\
center     ra +|-dec       free format\n\
radius     r1[,r2]         arcmin (default: 0,10')\n\
mag        mag1[,mag2]     limits in magnitude (mag1<mag2)\n\
format     0               full list, ra in h.hhh, dec in d.ddd\n\
           1               ra in hh mm ss, dec in dd mm ss\n\
           2               summary of GSC record\n\
           3               as 2, but ra/dec in hh mm ss dd mm ss (default)\n\
sort       ra              sorted by RA\n\
	   dec		   sorted by DEC\n\
           mag		   sorted by magnitude (default)\n\
           Rad		   sorted by distance to center\n\
           0		   no sorting\n\
nout       no. of lines    Number of lines in output (Max = default = 1000).\n\
*                          Single star filter; leaves multiple out\n\
\n\
Commands can abbreviated to first letter.\n\
Distance from center (arcmin) and position angle (degrees East of North)\n\
are always provided at the end of each record.\n";

	write(1,helptxt,sizeof(helptxt));
}

int prtgsc(fmt,iepoch,gscrec)
	int fmt,iepoch;
	GSCREC gscrec;
{
char *form2 = "%05d%05d %9.5f %9.5f %5.2f %c; %6.2f %3.0f\n";
char *form3 = "%05d%05d%s %s %5.2f %c; %6.2f %3.0f\n";
char *form9 = "GSC%05d%05d|%s|%s|%5.2f;\n";
char *form8 = "GSC%05d%05d %9.5f %9.5f %4.1f %5.2f %4.2f %c %7.2f %3.0f;\n";
char *form0 = "%05d%05d %9.5f %9.5f%6.1f%6.2f%5.2f%3d%2d%5s %c;%7.2f%4.0f\n";
char *form10= "%05d%05d %9.5f %9.5f%6.1f%6.2f%5.2f%3d%2d%9.3f %c;%7.2f%4.0f\n";
char *form1 = "%05d%05d%s %s%6.1f%6.2f%5.2f%3d%2d%5s %c;%7.2f%4.0f\n";
char *form11 = "%05d%05d%s %s%6.1f%6.2f%5.2f%3d%2d%9.3f %c;%7.2f%4.0f\n";
char sa1[20],sd1[20], buff[1024];
double rar,der;
void dtos();

	rar = gscrec.ra;
	der = gscrec.dec;
	switch(fmt){
	case 9: /* condensed format, used by the ESO-NTT TCS */
		rar /= 15.0;
		dtos(&rar,sa1,2);
		dtos(&der,sd1,1);
		sprintf(buff, form9, gscrec.reg,gscrec.id,sa1+1,sd1,gscrec.m);
		write(1, buff, strlen(buff));
		break;
	case 8: /* condensed format, used by the GSC clients */
		sprintf(buff,form8,gscrec.reg,gscrec.id,gscrec.ra,gscrec.dec,
			gscrec.poserr,gscrec.m,gscrec.merr,gscrec.mu,
			gscrec.dist,gscrec.posang);
		write(1, buff, strlen(buff));
		break;
	case 3: /* summary, RA/DEC in hhmmss/ddmmss */
		rar /= 15.0;
		dtos(&rar,sa1,2); sa1[0]=' ';
		dtos(&der,sd1,1);
		sprintf(buff, form3, gscrec.reg,gscrec.id,sa1,sd1,
			gscrec.m,gscrec.mu,gscrec.dist,gscrec.posang);
		write(1, buff, strlen(buff));
		break;
	case 2: /* summary, RA/DEC in decimal hrs/deg */
		sprintf(buff, form2, gscrec.reg,gscrec.id,
			gscrec.ra,gscrec.dec,
			gscrec.m,gscrec.mu,gscrec.dist,gscrec.posang);
		write(1, buff, strlen(buff));
		break;
	case 1: /* full, RA/DEC in hhmmss/ddmmss */
		rar /= 15.0;
		dtos(&rar,sa1,2); sa1[0]=' ';
		dtos(&der,sd1,1);
		if(iepoch == 0) {
		   sprintf(buff,form1,gscrec.reg,gscrec.id,sa1,sd1,
			gscrec.poserr,gscrec.m,gscrec.merr,gscrec.mb,gscrec.cl,
			gscrec.plate,gscrec.mu,gscrec.dist,gscrec.posang);
		} else {
		   sprintf(buff,form11,gscrec.reg,gscrec.id,sa1,sd1,
			gscrec.poserr,gscrec.m,gscrec.merr,gscrec.mb,gscrec.cl,
			gscrec.epoch,gscrec.mu,gscrec.dist,gscrec.posang);
	        }
		write(1, buff, strlen(buff));
		break;
	default: /* full, RA/DEC in decimal hrs/deg */
		if(iepoch == 0)
		  sprintf(buff,form0,gscrec.reg,gscrec.id,gscrec.ra,gscrec.dec,
			gscrec.poserr,gscrec.m,gscrec.merr,gscrec.mb,gscrec.cl,
			gscrec.plate,gscrec.mu,gscrec.dist,gscrec.posang);
		else
		 sprintf(buff,form10,gscrec.reg,gscrec.id,gscrec.ra,gscrec.dec,
			gscrec.poserr,gscrec.m,gscrec.merr,gscrec.mb,gscrec.cl,
			gscrec.epoch,gscrec.mu,gscrec.dist,gscrec.posang);
	   
		write(1, buff, strlen(buff));
		break;
	}
	return(0);
}



