/*==================================================================
** NAME         :gsc_dec.c     ** Version 2 (GSC 1.1) **
** TYPE         :main
** DESCRIPTION  :decode GSC records encoded as c[12]
**              :
** INPUT        :encoded region, stdin or command line
**              :
** OUTPUT       :decoded region, stdout
**              :
** AUTHOR       :a.p.martinez
** DATE         :09/92
*=================================================================*/
#include <gsc.h>

main(argc,argv)
        int argc; char **argv;
{
	unsigned char c[12];
	int n,nreg,fp=0;
	char region[81];
	HEADER h;
	GSCREC r;

	if(argc == 2) {
		nreg = atoi(*++argv);
		find_reg(region,nreg);
		fp = open(region,0);
		if(fp < 0) {
			printf("+++ unable to open file %s\n",argv[1]);
			exit(-1);
			}
	}

/* ------ reading header of coded region --------------------------*/

	puts(get_header(fp,&h));

	while((n=read(fp,c,12)) == 12)
	{
/* ------ decoding (version #2) -----------------------------------*/

	puts(decode_c(c,&h,&r));
	}
}
