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
**		Adapted Tubo-C: F. Ochsenbein, April 1996
**		-fits option (regenerate FITS files) Aug. 2002
*=================================================================*/

#include <gsc.h>

static char usage[] = "\
Usage: decode [-help] [-fits] gsc_bin_file [gsc_bin_file...]\n\
       Several files may be decoded\n\
";
static char ofits;
/* Parts marked with # are ignored in FITS output */
static char fits_record[] = "\
#####00001#127.10780#+07.31447#  0.2#14.95#0.40# 1#0#03R6#F\
" ;


static char fits_SIMPLE[] = "\
SIMPLE  =                    T / Standard FITS format                           \
BITPIX  =                    8 / Character Information                          \
NAXIS   =                    0 / No image data array present                    \
EXTEND  =                    T / Extension exists                               \
FILETYPE= 'GSC_REGION'         / Indicates file type                            \
ORIGIN  = 'ST ScI  '           / Site which issued tape                         \
DATE    = '2002-08-27'         / Date of issue                                  \
                                                                                \
COMMENT                          THE GUIDE STAR CATALOG Version GSC-ACT         \
COMMENT                                                                         \
COMMENT              An all-sky astrometric and photometric catalog             \
COMMENT              prepared for the operation of the Hubble Space             \
COMMENT                                Telescope.                               \
COMMENT                                                                         \
COMMENT              Copyright,   1992, Association of Universities             \
COMMENT                      for Research in Astronomy, Inc.                    \
COMMENT                                                                         \
COMMENT The GSC-ACT is a recalibration of GSC1.1 using the ACT                  \
COMMENT (Astrographic Catalog/Tycho, Urban et al. 1997, catalog I/246) performed\
COMMENT by the Project Pluto (http://www.projectpluto.com/gsc_act.htm)          \
COMMENT Plate RMS values are given at http://www.projectpluto.com/results.txt   \
COMMENT with most plates coming in at under 0.3 arcseconds.                     \
COMMENT                                                                         \
COMMENT The original Guide Star Catalog (GSC) was prepared by the               \
COMMENT Space Telescope Science Institute (ST ScI),                             \
COMMENT 3700 San Martin Drive,  Baltimore,  MD 21218,  USA.                     \
COMMENT ST ScI  is  operated  by the Association of Universities for Research in\
COMMENT Astronomy, Inc. (AURA), under contract with the National Aeronautics and\
COMMENT Space Administration (NASA).                                            \
                                                                                \
END                                                                             \
                                                                                \
                                                                                \
                                                                                \
                                                                                \
                                                                                \
"; 

static char fits_HEADER[] = "\
XTENSION= 'TABLE   '           / Table Extension                                \
BITPIX  =                    8 / Character Information                          \
NAXIS   =                    2 / Two-dimensional table                          \
NAXIS1  =                   45 / Number of characters per line                  \
NAXIS2  =                %5d / Number of rows                                 \
PCOUNT  =                    0 / No random parameters                           \
GCOUNT  =                    1 / Only one group                                 \
TFIELDS =                   10 / Ten fields per row                             \
                                                                                \
EXTNAME = 'GSC_REGION_%05d'   / GSC Region No. %05d                           \
EXTVER  =                    1 / Integer Version Number                         \
                                                                                \
TTYPE1  = 'GSC_ID  '           / ID within Region                               \
TBCOL1  =                    1 / Start in column 1                              \
TFORM1  = 'I5      '           / Integer, 5 character field (I5.5 Style)        \
                                                                                \
TTYPE2  = 'RA_DEG  '           / Right Ascension - Decimal Degrees (0 to 360)   \
TBCOL2  =                    6 / Start in column 6                              \
TFORM2  = 'F9.5    '           / Floating, 9 character field                    \
                                                                                \
TTYPE3  = 'DEC_DEG '           / Declination - Decimal Degrees (-90 to +90)     \
TBCOL3  =                   15 / Start in column 15                             \
TFORM3  = 'F9.5    '           / Floating, 9 character field                    \
                                                                                \
TTYPE4  = 'POS_ERR '           / Position Error in Arc Seconds                  \
TBCOL4  =                   24 / Start in column 24                             \
TFORM4  = 'F5.1    '           / Floating, 5 character field                    \
                                                                                \
TTYPE5  = 'MAG     '           / Magnitude                                      \
TBCOL5  =                   29 / Start in column 29                             \
TFORM5  = 'F5.2    '           / Floating, 5 character field                    \
                                                                                \
TTYPE6  = 'MAG_ERR '           / Magnitude error                                \
TBCOL6  =                   34 / Start in column 34                             \
TFORM6  = 'F4.2    '           / Floating, 4 character field                    \
                                                                                \
TTYPE7  = 'MAG_BAND'           / Magnitude Band                                 \
TBCOL7  =                   38 / Start in column 38                             \
TFORM7  = 'I2      '           / Integer, 2 character field (I2.2 Style)        \
                                                                                \
TTYPE8  = 'CLASS   '           / Classification                                 \
TBCOL8  =                   40 / Start in column 40                             \
TFORM8  = 'I1      '           / Integer, 1 character field                     \
                                                                                \
TTYPE9  = 'PLATE_ID'           / GSSS Internal Plate Number                     \
TBCOL9  =                   41 / Start in column 41                             \
TFORM9  = 'A4      '           / 4 character field                              \
                                                                                \
TTYPE10 = 'MULTIPLE'           / (T/F) Flag for additional entries              \
TBCOL10 =                   45 / Start in column 45                             \
TFORM10 = 'A1      '           / Logical flag, 1 character field                \
                                                                                \
END                                                                             \
                                                                                \
                                                                                \
                                                                                \
                                                                                \
                                                                                \
                                                                                \
                                                                                \
                                                                                \
                                                                                \
                                                                                \
                                                                                \
                                                                                \
                                                                                \
                                                                                \
                                                                                \
                                                                                \
                                                                                \
                                                                                \
                                                                                \
" ;



void main(argc,argv)
        int argc; char **argv;
{
	unsigned char c[12];
	char *ptr, *p;
	int n=0,fp=0, i, j;
	HEADER h;
	GSCREC r;

	while (--argc > 0) {
		++argv;
		if (**argv == '-') switch(argv[0][1]) {
		case 'h': case 'H': case '?':
			printf("%s", usage);
			exit(0);
		case 'f':	/* FITS output */
			ofits = 1;
			printf("%s", fits_SIMPLE) ;
			continue;
		default:
			fprintf(stderr, "****Bad option: %s\n", *argv);
			puts(usage);
			exit(1);
		}

		fp = open(*argv, O_BINARY);
		if(fp < 0) { perror(*argv); exit(1); }

		if ((ofits == 0) && (n > 0)) 
		    puts("");	/* Empty line between files */

		n++;

		/* Print the Header line of the GSC binary file	*/
		ptr = get_header(fp,&h);
		if (!ptr) {
			fprintf(stderr, "****Bad GSC binary file: %s\n", *argv);
			exit(1);
		}

		/* Write out the Header */
		if (ofits) {	/* Field number is 3rd word */
		    for (p=ptr; isgraph(*p); p++) ; while (isspace(*p)) p++;
		    while (isgraph(*p)) p++; while (isspace(*p)) p++;
		    i = atoi(p) ;
		    /* The next word is the number of rows */
		    while (isgraph(*p)) p++; while (isspace(*p)) p++;
		    printf(fits_HEADER, atoi(p), i, i) ;
		}
		else puts(ptr);

		/* Display the decoded contents of the GSC binary file */
		for (i=0; read(fp,c,12) == 12; i++) {
			decode_c(c,&h,&r),
			ptr = gsc2a(-1,0,&r);
			if (ofits) for (j=0; fits_record[j]; j++) {
			    if (fits_record[j] != '#') putchar(ptr[j]) ;
			}
			else puts(ptr) ;
			/* prtgsc(-1,0,&r); */
		}
		/* For FITS, terminate the 2880-byte record */
		if (ofits) while (i&63) printf("%45s", ""), i++ ;
	}

	/* No file specified ==> It's an error ! */
	if (n == 0) {
	    fprintf(stderr, "****Missing argument(s)\n%s", usage);
	    exit(1) ;
	}

	exit(0);
}
