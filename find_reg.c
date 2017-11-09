/*==================================================================
** NAME         :find_reg.c
** TYPE         :int
** DESCRIPTION  :find region-path according to reg-number
** INPUT        :
** OUTPUT       :
** AUTHOR       :a.p.martinez
** DATE         :10/92;
** LIB          :
*=================================================================*/

#include <stdio.h>
#include <stdlib.h>
#define ITEMS(a)	sizeof(a)/sizeof(a[0])

static char *GSCDAT ;	/* Default, superseded by $GSCDAT */

static int range[24] = {1,594,1178,1729,2259,2781,3246,3652,4014,4294,4492,
	4615,4663,5260,5838,6412,6989,7523,8022,8464,8840,9134,9346,9490};
static char *subdir[] = {"N0000","N0730","N1500","N2230","N3000","N3730",
	"N4500","N5230","N6000","N6730","N7500","N8230","S0000",
	"S0730","S1500","S2230","S3000","S3730","S4500","S5230",
	"S6000","S6730","S7500","S8230"};

int find_reg (region,nreg)
  char *region;
  int nreg;
{
  char *path;
  int i;

	path = getenv("GSCDAT");
	if (!path) path = GSCDAT ;

	for (i = ITEMS(range); --i >= 0; )
	    if (nreg >= range[i]) break;

	sprintf(region,"%s/%s/%04d.GSC",
	    path, (i>=0 ? subdir[i] : "-----"), nreg);
	return(1);
}
