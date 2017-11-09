/*==================================================================
** NAME         :tod.c
** TYPE         :void
** DESCRIPTION  :transforms a [signed]sexagesimal string to decimal
**              :accepts free input format
** INPUT        :char *string
** OUTPUT       :double *decimal
** AUTHOR       :apm
** DATE         :06/91; 09/91
*=================================================================*/

void tod(string,deci)
	double *deci;
	char *string;
{
	int sign;
	double dd,s=0.,h=0.,m=0.; 
	char minus='-' , *ps;

	if(*string == '-' || *string == '+')
	{
	  ps=string;
	  while(*(++ps) == ' ');
	  if(ps-1 != string)
	  {  *(ps-1) = *string;
	     *string = ' ';  }
	}
	sign=1;
	if(strchr(string,minus))sign= -1;
	sscanf(string,"%lf%lf%lf",&h,&m,&s);
	dd=(s/60.+ m)/60.;
	if(sign < 0) *deci= h-dd;
	else         *deci= h+dd; 
	return;
}
