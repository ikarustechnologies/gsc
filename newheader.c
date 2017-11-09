/*==================================================================
** NAME         :encode.c  Version 2 (GSC 1.1)
** TYPE         :main
** DESCRIPTION  :encodes a GSC region in 12 char/entry
**              :plus a header with info on coding
** INPUT        :
** OUTPUT       :
** AUTHOR       :a.p.martinez
** DATE         :11/92
*=================================================================*/

#include <gsc.h>

main(argc,argv)
	int argc; char **argv;
{
	HEADER h;
	char *head,*s,*pl,*el,*elist,*bsearch();
	int size,lrec,nrec,i,j;
	int fr,ft,nbl,hlen,notf;
	unsigned char *table;
	char *ple,*pe,plate[6];


	if(argc > 1) {
		if((fr = open(argv[1],0))<0)
		{ printf("can't open %s\n",argv[1]);
		  exit(0);
		}
		}
	else { printf("give input file as argument\n");
		exit(0);
		}
   
	if((ft=open("/usr1/GSC/tab/plates.epoch",0)) <0)
		{ printf("can't open plates.epoch\n");
                  exit(0);
                }
        size = lseek(ft,0,2);
	ple = (char *)malloc(size);
        lseek(ft,0,0);
        read(ft,ple,size);
	close(ft);

        get_header(fr,&h);
        size = lseek(fr,0,2)-h.len;
        table = (unsigned char *)malloc(size);
        lseek(fr,h.len,0);
        read(fr,table,size);
	close(fr);
/* ---- ------------------------------------ */

	elist=(char *)malloc(100);
	s = (char *)malloc(15);
	for(i=0,pl=h.list+1,el=elist;i<h.npl;i++,pl +=5,el +=9) 
	{
	strncpy(plate,pl,4); plate[4]='\0';
/*
printf("plate %d <%s>\n",i,plate);
*/
	for(j=0,pe=ple,notf=1;j<1519;j++,pe +=14)
	{
	  if(strncmp(plate,pe,4)==0){
		strncpy(s,pe,13); s[13]='\0'; notf=0;}
	if(notf==0)break;
	}
	if(notf == 1)
	{ printf("plate %s not found\n",plate);
	  exit(0);
	}
/*
printf("%s\n",s);
*/
	strncpy(el,s+5,8);

	}

/* ------ new header information (ASCII)-----------------------------

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
        15-nn) plate list, epoch-list;	

   ------printing header --------------------------------------------*/
	
	head = (char *)malloc(300);
  sprintf(head,"%d %05d %d %.5f %.5f %.5f %.5f %.0f %.0f %.0f %.0f %.0f %d",
		h.vers,h.region,h.nobj,h.amin,h.amax,h.dmin,h.dmax,
		h.magoff,h.scale_ra,
		h.scale_dec,h.scale_pos,h.scale_mag,h.npl);
	hlen = strlen(head)+h.npl*5+h.npl*9 +4;
	nbl = (4- (hlen%4))%4;
	printf("%3d %s",hlen+nbl,head);
	for(i=0,pl=h.list;i<h.npl;i++,pl += 5){
		strncpy(plate,pl,5); plate[5]='\0';
		printf("%s",plate); }
	for(i=0,pl=elist;i<h.npl;i++,pl += 9){
		strncpy(s,pl,8); s[8]='\0';
		printf(" %s",s); }
	for(i=0;i<nbl;i++) printf(" ");	
	fflush(stdout);

/* --------------------------------------------------------------------- */	
if(argc > 2 && atoi(argv[2]) == 0) {printf("\n"); exit(0);}
	write(1,table,size);
}
