/*++++++++++++++
.IDENTIFICATION epoch.c
.LANGUAGE       C
.AUTHOR         Francois Ochsenbein [CDS]
.ENVIRONMENT    
.KEYWORDS       
.VERSION  1.0   23-Apr-1996
.COMMENTS       Add epoch in GSC files
---------------*/

#include <gsc.h>

static char *epochs;

char *find_plate(plate) char *plate ;
{
  char *p;
    for (p=epochs; p; p = strchr(p, '\n')) {
	if (*p == '\n') p++;
	if(strncmp(p,plate,4) == 0) return(p);
    }
    return("\n");
}

main(argc, argv) int argc; char **argv;
{
  long size;
  char *s, *p, *ph, *ep;
  HEADER h;
  int f, i, j;
  char buf[BUFSIZ];

    if (argc != 3) {
	fprintf(stderr, "Usage: epoch plates.epoch_file file.GSC\n");
	exit(1);
    }

    /* Load the "plates.epoch" file */
    f = open(argv[1], O_BINARY);
    if (f <= 0) { perror(argv[1]); exit(1); }
    size = lseek(f, 0L, 2); lseek(f, 0L, 0);
    epochs = malloc(size+1) ;
    read(f, epochs, size); epochs[size] = 0;
    close(f);

    argv++, argc-- ;
    f = open(argv[1], O_BINARY);
    if (f <= 0) { perror(argv[1]); exit(1); }

    ph = get_header(f, &h);
    ep = h.list + 5*(h.npl);

    for (p=h.list+1, i=h.npl; --i >= 0; p += 5) {
	*(ep++) = ' ';
	if (p[0] == '*') {	/* Special plate * 56 instead of +056 */
	    p[0] = '+' ;
	    if (p[1] == ' ') p[1] = '0' ;
	    if (p[2] == ' ') p[2] = '0' ;
	}
	s = find_plate(p);
	if (!isgraph(*s)) {
	    fprintf(stderr, "++++Plate without epoch: "); fflush(stderr);
	    write(2, p, 4); fprintf(stderr, "\n");
	    for (j=8; --j >= 0; ) *(ep++) = '.' ;
	}
	else for (j=8, s+=5; --j >= 0; ) *(ep++) = *(s++) ;
    }

    /* Round up the length of header */
    *(ep++) = ' ';
    while ((ep-ph)&3) *(ep++) = ' ';
    *ep = 0;
    ep[-1] = '\n' ;
    sprintf(ph, "%3d", (ep-ph)); ph[3] = ' ';
    write(1, ph, (ep-ph));
    /* fprintf(stderr, "%s", ph); */

    while(i=read(f, buf, sizeof(buf))) {
	if (i < 0) { perror(argv[1]); exit(1); }
	if (i != write(1, buf, i)) { perror("stdout"); exit(1); }
    }

    exit(0);

}

