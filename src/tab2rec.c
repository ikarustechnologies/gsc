/*==================================================================
** NAME         :tab2rec.c
** TYPE         :int 
** DESCRIPTION  :transforms a GSC record into a GSCREC structure
** INPUT        :pointer to record
** OUTPUT       :GSC structure
** AUTHOR       :a.p.martinez
** DATE         :10/92
*=================================================================*/
#include <gsc.h>

int tab2rec(tab,rec)
	char *tab;
	GSCREC *rec;
{
	int i;
	char s[90];

	strncpy(s,tab,5);
	s[5] = '\0';
	rec->reg = atoi(s);
	strncpy(s,tab+5,5);
	s[5] = '\0';
	rec->id = atoi(s);
	for(i=1;i<13;i++) {
	  strncpy(s,tab+fbeg[i],flen[i]);
	  s[flen[i]] = '\0';
	  switch (i) {
		case 1 :
		rec->ra = atof(s); break;
		case 2 :
		rec->dec = atof(s); break;
		case 3 :
		rec->poserr = atof(s); break;
		case 4 :
		rec->m = atof(s); break;
		case 5 :
		rec->merr = atof(s); break;
		case 6 :
		rec->mb = atoi(s); break;
		case 7 :
		rec->cl = atoi(s); break;
		case 8 :
		strncpy(rec->plate,s,4);
		rec->plate[4] = '\0'; break;
		case 9 :
		rec->mu = s[0]; break;
		case 10 :
		rec->dist = atof(s); break;
		case 11 :
		rec->posang = atof(s); break;
		case 12 :
		rec->epoch = atof(s);
		}
	}
	return(0);
}
