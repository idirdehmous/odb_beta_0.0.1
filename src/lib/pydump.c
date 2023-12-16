
/* odbdump_main.c */

#include <ctype.h>
#include <math.h>

#include "odbdump.h"


extern int ec_is_little_endian();
extern double util_walltime_();


PRIVATE char *lld_dotify(ll_t n) 
     /* See ifsaux/support/drhook.c for a little variation of this beast [lld_commie] */
{ 
  const char dot= '.' ;
  char      *sd = NULL;
  char      *pd = NULL;
  char      s[100]    ;
  char      *p        ;
  int len, ndots      ;
  sprintf(s,"%lld",n) ;
  len = STRLEN(s)     ;
  ndots = (len-1)/3;
  if (ndots > 0) {
    int lensd = len + ndots + 1;
    ALLOC(sd, lensd);
    pd = sd + len + ndots;
    *pd-- = '\0';
    p = s + len - 1;
    len = 0;
    while (p-s >= 0) {
      *pd-- = *p--;
      ++len;
      if (p-s >= 0 && len%3 == 0) *pd-- = dot;
    }
  }
  else {
    sd = STRDUP(s);
  }
  return sd;
}

#define MEGA ((double)1048576.0)

//int   pyodb(int argc, char *argv[])
int pyodb (char *database , char *sql_query , char *d )
{
  
  int i_am_little = ec_is_little_endian();
  //char *database  = NULL;
  //char *sql_query = NULL;
  char *poolmask  = NULL;
  char *varvalue  = NULL;
  char *outfile   = NULL;
  char *queryfile = NULL;
  char *delim     = NULL;
  Bool print_newline = true;
  Bool print_mdi  = true; /* by default prints "NULL", not value of the NULL */
  Bool print_title= true;
  Bool raw_binary = false;
  Bool stat_only  = false;
  Bool packed     = false;

  const char dummydb[] = "$ODB_SYSDBPATH/DUMMY";
  int maxlines = -1;
  char *dbl_fmt   = NULL;
  void *h         = NULL;
  int maxcols     = 0;
  ll_t Nbytes     = 0;
  int rc          = 0;
  int errflg      = 0;
  int c ;
  double wlast;
  FILE *fp        = stdout;
  extern int optind;
 


  //if (sql_query)  FREE (sql_query) ; 
  //if (database) FREE(database);
  if (maxlines == 0) return rc;

  dbl_fmt = STRDUP("%.10g");
  if (stat_only) { print_newline = true; print_title = true; raw_binary = false; }
  if (!delim) delim = STRDUP(" ");


  if (outfile) fp = fopen(outfile, "w");

  if (print_title) {
    fprintf(fp, "# ---------------------------------------------------------------\n");
    fprintf(fp, "# Database    : %s\n", database ? database : ".");
    fprintf(fp, "# SQL-queries : %s\n", sql_query);
    fprintf(fp, "# Data endianess : %s-endian\n", i_am_little ? "little" : "big");
    fprintf(fp, "# ---------------------------------------------------------------\n");
    fflush(fp);
  }


  wlast = util_walltime_();
  h = odbdump_open(database, sql_query, queryfile, poolmask, varvalue, &maxcols);

  
  if (packed && h && maxcols > 0) {
    maxcols *= 2; /* ca. worst case scenario when packed */
  }


  if (h && maxcols > 0) {
    int new_dataset = 0;
    colinfo_t *ci = NULL;
    int nci = 0;
    double *d = NULL; // DATA VALUES 
    int nd;    
    int query_num = 0;
    ll_t nrows = 0;
    ll_t nrtot = 0;
    int ncols = 0;
    int (*nextrow)(void *, void *, int, int *) = 
      packed ? odbdump_nextrow_packed : odbdump_nextrow;
    int dlen = packed ? maxcols * sizeof(*d) : maxcols;

    // MY VARIABLES 
    int date , hour ; 
    int IntLen  ; 
    int FlLen   ; 
    IntLen = 5  ; 
    FlLen  = 20 ; 
    char IntStr[IntLen]  ; 
    char FlStr[FlLen]   ;
    // FOR DATE TIME 
    char DtStr [8] ; 
    char TmStr [6] ; 

    char result[15];
    ALLOCX(d, maxcols);

    while ( (nd = nextrow(h, d, dlen, &new_dataset)) > 0) {
      int i;

      if (new_dataset) {
	/* New query ? */
	ci = odbdump_destroy_colinfo(ci, nci);
	ci = odbdump_create_colinfo(h, &nci);
      }

      if (stat_only) goto next_please;

      if (!packed) nd = MIN(nd, nci);
      if (raw_binary) {
	if (packed) {
	  fwrite(d, sizeof(char), nd, fp);
	}
	else {
	  fwrite(d, sizeof(*d), nd, fp);
	}
      }
      else {
	char *separ = " ";
	for (i=0; i<nd; i++) {
          
	  colinfo_t *pci = &ci[i];
	  fprintf(fp,"%s",separ);
	  if (print_mdi && pci->dtnum != DATATYPE_STRING && ABS(d[i]) == mdi) {
	    fprintf(fp,"NULL");
	  }
	  else {
	    switch (pci->dtnum) {

	    case DATATYPE_STRING:
	      {
		int js;
		char cc[sizeof(double)+1];
		char *scc = cc;
		union {
		  char s[sizeof(double)];
		  double d;
		} u;
		u.d = d[i];
		
		for (js=0; js<sizeof(double); js++) {
		  char c = u.s[js];
		  *scc++ = isprint(c) ? c : ' '; /* unprintables as blanks */
		} /* for (js=0; js<sizeof(double); js++) */
		*scc = '\0';
		//fprintf(fp,"\"%s\"",cc);
		sprintf(result, "%s", cc) ;
		printf ("%s,\n" , result) ;
	      }

	      break;
	    case DATATYPE_YYYYMMDD:
	      date =  (int)d[i]  ;  
	      //printf("%8.8d," , date ) ; 
	      sprintf(DtStr, "%d\n", date ) ;

	      break;
	    case DATATYPE_HHMMSS:
              hour = (int)d[i] ;
	      //printf( "%6.6d,\n" , hour ) ;   
	      sprintf(TmStr, "%d", hour ) ;
	      printf(TmStr, "%d\n", hour) ; 
	      break;
	    case DATATYPE_INT4:
	      //printf("%d,",  (int) d[i]) ;
              sprintf(IntStr, "%d", (int)d[i]) ;
	      printf ( "%s\n",  IntStr ) ;
	      break;
	    default:
	      //printf( "%lf,", (double)  d[i]) ; 
              sprintf(FlStr, "%lf", (double)d[i]) ;
	      printf ( "%s\n",  FlStr ) ;
	      break;
	      printf("%s , %s \n" ,  DtStr, TmStr );
	    } /* switch (pci->dtnum) */
	  }
	  separ = delim;
	} /* for (i=0; i<nd; i++) */

	//fprintf(fp, print_newline ? "\n" : "\e[0K\r"); /* The "\e[0K" hassle clears up to the EOL */
	//if (!print_newline) fflush(fp);
 
      } 

    next_please:
      ++nrows;
      if (maxlines > 0 && ++nrtot >= maxlines) break; /* while (...) */
    } /* while (...) */

    ci = odbdump_destroy_colinfo(ci, nci);
    rc = odbdump_close(h);
    FREEX(d);

    fflush(fp);
  } /* if (h && maxcols > 0) ... */
  else {
    rc = -1;
  }

  if (fp) {
    if (rc != 0) fprintf(fp, "# return code = %d : please look at the file 'odbdump.stderr'\n",rc);

    fclose(fp);
  }
  printf("%d" ,  rc) ; 
  return rc;
}

