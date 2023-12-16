
/* odbdump_main.c */

#include <ctype.h>

#include "odbdump.h"

#define USAGE \
 "Usage: odbdump.x [-h] [-r] [-N] [-T] [-m <n>] [-% dbl_fmt] [-V '$var1=value1; $var2=value2']\n" \
 "                 [-g] [-p poolmask] [-s] [-b] [-D 'delimiter'] [-P]\n" \
 "                 {-q 'sql_query'|-v queryfile.sql}\n" \
 "                 [-o output_file] [-d] [-i] database_path\n" \
 " The option '-i' can be ignored, if database_path is the last argument\n" \
 "   If database_path is not given, then the current directory is tried for the database_path\n" \
 " For data query either one of the options -q or -v must be supplied. Last occurence of these two prevails\n" \
 " The option '-s' prints only timing statistics i.e. fetches but does not dump the data itself\n" \
 " The option '-b' dumps data out in a raw binary (as is/no conversions) manner\n" \
 " The option '-p' can be supplied multiple times\n" \
 " The option '-d' selects DUMMY-database in $ODB_SYSDBPATH/DUMMY. Good for calculator mode\n" \
 " The option '-N' switches OFF write of NULLs; values are written instead\n" \
 " The option '-T' switches OFF write of title (i.e. column info)\n" \
 " The option '-r' prints 'carriage-return' at end of line instead of 'newline'\n" \
 " The option '-m <n>' prints up to <n>-lines only (in total)\n" \
 " The option '-%% dbl_fmt' prints doubles & Formulas in format '%%dbl_fmt'. Default : dbl_fmt = '%%.14g'\n" \
 " The option '-g' turns on ODB internal debugging (output goes to odbdump.stderr though)\n" \
 " The option  -D 'delimiter' provides delimited between columns. Default is space ' '\n" \
 " The option '-P' delivers output, row-by-row, in ODB-packed form. Implies -b, too\n"

#define FLAGS "bdD:ghi:m:No:p:Pq:rsTv:V:%:"

extern int ec_is_little_endian();
extern double util_walltime_();

PRIVATE char *lld_dotify(ll_t n) 
     /* See ifsaux/support/drhook.c for a little variation of this beast [lld_commie] */
{ 
  const char dot = '.';
  char *sd = NULL;
  char *pd = NULL;
  char s[100];
  char *p;
  int len, ndots;
  sprintf(s,"%lld",n);
  len = STRLEN(s);
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

#define PRINT_TIMING(txt, nrows, ncols) \
if (print_title && print_newline) { \
  double wthis = util_walltime_(); \
  double wdelta = wthis - wlast; \
  if (txt) fprintf(fp, "# %s in %.3f secs\n", txt, wdelta);	\
  else if (nrows > 0 && ncols > 0) { \
    double rows_per_sec = nrows/wdelta; \
    ll_t nbytes = packed ? Nbytes : nrows * ((ll_t) ncols) * ((ll_t) sizeof(double)); \
    double MBytes_per_sec = (nbytes/wdelta)/MEGA; \
    char *dotified = lld_dotify(nbytes); \
    fprintf(fp, "# Total %lld row%s, %d col%s, %s %sbytes in %.3f secs : %.0f rows/s, %.0f MB/s\n", \
            (long long int)nrows, (nrows != 1) ? "s" : "",		\
            ncols, (ncols != 1) ? "s" : "", \
            dotified, packed ? "packed-" : "", \
            wdelta, rows_per_sec, MBytes_per_sec); \
    FREE(dotified); \
  } \
  if (txt || (nrows > 0 && ncols > 0)) wlast = wthis; \
}

int main(int argc, char *argv[])
{
  int i_am_little = ec_is_little_endian();
  char *database = NULL;
  char *sql_query = NULL;
  char *poolmask = NULL;
  char *varvalue = NULL;
  char *outfile = NULL;
  char *queryfile = NULL;
  char *delim = NULL;
  Bool print_newline = true;
  Bool print_mdi = true; /* by default prints "NULL", not value of the NULL */
  Bool print_title = true;
  Bool debug_on = false;
  Bool raw_binary = false;
  Bool stat_only = false;
  Bool packed = false;
  const char dummydb[] = "$ODB_SYSDBPATH/DUMMY";
  int maxlines = -1;
  char *dbl_fmt = NULL;
  void *h = NULL;
  int maxcols = 0;
  ll_t Nbytes = 0;
  int rc = 0;
  int errflg = 0;
  int c;
  double wlast;
  FILE *fp = stdout;
  extern int optind;

  while ((c = getopt(argc, argv, FLAGS)) != -1) {
    switch (c) {
    case 'b':
      raw_binary = true;
      break;
    case 'D':
      if (delim) FREE(delim);
      delim = STRDUP(optarg);
      break;
    case 'g':
      debug_on = true;
      break;
    case 'd':
    case 'i':
      if (database) FREE(database);
      database = STRDUP((c == 'd') ? dummydb : optarg);
      break;
    case 'm':
      maxlines = atoi(optarg);
      break;
    case 'N':
      print_mdi = false;
      break;
    case 'o':
      if (outfile) FREE(outfile);
      outfile = STRDUP(optarg);
      break;
    case 'p':
      if (poolmask) { /* Append more ; remember to add the comma (',') between !! */
	int len = STRLEN(poolmask) + 1 + STRLEN(optarg) + 1;
	char *p;
	ALLOC(p, len);
	snprintf(p, len, "%s,%s", poolmask, optarg);
	FREE(poolmask);
	poolmask = p;
      }
      else {
	poolmask = STRDUP(optarg);
      }
      break;
    case 'P':
      packed = true;
      raw_binary = true;
      break;
    case 'q':
      if (queryfile) FREE(queryfile);
      if (sql_query) FREE(sql_query);
      sql_query = STRDUP(optarg);
      break;
    case 'r':
      print_newline = false;
      break;
    case 's':
      stat_only = true;
      break;
    case 'T':
      print_title = false;
      break;
    case 'v':
      if (sql_query) FREE(sql_query);
      if (queryfile) FREE(queryfile);
      queryfile = STRDUP(optarg);
      break;
    case 'V':
      if (varvalue) FREE(varvalue);
      {
	int len = STRLEN(optarg) + 1;
	char *p = optarg;
	char *pv;
	Bool last_was_comma = false;
	ALLOC(varvalue, len);
	pv = varvalue;
	while (*p) {
	  int cp = *p++;
	  if (isspace(cp) || !isprint(cp)) {
	    continue;
	  }
	  else if (cp == ';' || cp == ',') {
	    if (!last_was_comma) {
	      *pv++ = ',';
	      last_was_comma = true;
	    }
	  }
	  else {
	    *pv++ = cp;
	    last_was_comma = false;
	  }
	} /* while (*p) */
	*pv = '\0';
      }
      break;
    case '%':
      if (dbl_fmt) FREE(dbl_fmt);
      {
	int len = STRLEN(optarg) + 2;
	ALLOC(dbl_fmt, len);
	snprintf(dbl_fmt, len, "%%%s", optarg);
      }
      break;
    case 'h': /* help !! */
    default:
      ++errflg;
      break;
    }
  } /* while ((c = getopt(argc, argv, FLAGS)) != -1) */

  if (argc - 1 == optind) {
    if (database) FREE(database);
    database = STRDUP(argv[argc-1]);
  }
  else if (argc > optind) {
    ++errflg;
  }

  if (!sql_query && !queryfile) ++errflg;

  if (errflg > 0) {
    fprintf(stderr,"%s",USAGE);
    return errflg;
  }

  if (maxlines == 0) return rc;

  if (!dbl_fmt) dbl_fmt = STRDUP("%.14g");
  if (debug_on) (void) ODBc_debug_fp(stderr);
  if (stat_only) { print_newline = true; print_title = true; raw_binary = false; }
  if (raw_binary) print_title = false;
  if (!delim) delim = STRDUP(" ");

  if (outfile) fp = fopen(outfile, "w");

  if (print_title) {
    fprintf(fp, "# ---------------------------------------------------------------\n");
    fprintf(fp, "# Database    : %s\n", database ? database : ".");
    fprintf(fp, "# SQL-queries : %s\n", sql_query);
    if (poolmask) fprintf(fp, "# Poolmask : %s\n", poolmask);
    if (varvalue) fprintf(fp, "# Varvalue : %s\n", varvalue);
    fprintf(fp, "# Data endianess : %s-endian\n", i_am_little ? "little" : "big");
    fprintf(fp, "# ---------------------------------------------------------------\n");
    fflush(fp);
  }

  wlast = util_walltime_();

  h = odbdump_open(database, sql_query, queryfile, poolmask, varvalue, &maxcols);

  PRINT_TIMING("Database open", 0, 0);
  
  if (packed && h && maxcols > 0) {
    maxcols *= 2; /* ca. worst case scenario when packed */
  }

  if (h && maxcols > 0) {
    int new_dataset = 0;
    colinfo_t *ci = NULL;
    int nci = 0;
    double *d = NULL;
    int nd;
    int query_num = 0;
    ll_t nrows = 0;
    ll_t nrtot = 0;
    int ncols = 0;
    int (*nextrow)(void *, void *, int, int *) = 
      packed ? odbdump_nextrow_packed : odbdump_nextrow;
    int dlen = packed ? maxcols * sizeof(*d) : maxcols;

    ALLOCX(d, maxcols);

    while ( (nd = nextrow(h, d, dlen, &new_dataset)) > 0) {
      int i;

      if (new_dataset) {
	/* New query ? */
	ci = odbdump_destroy_colinfo(ci, nci);
	ci = odbdump_create_colinfo(h, &nci);
	if (print_title) {
	  PRINT_TIMING(NULL, nrows, ncols);
	  fprintf(fp, "#[%d]",++query_num);
	  if (stat_only) fprintf(fp, " ncols=%d", nci);
	  if (!stat_only) {
	    char *separ = " ";
	    for (i=0; i<nci; i++) {
	      colinfo_t *pci = &ci[i];
	      fprintf(fp,"%s%s:%s",
		      separ,
		      pci->type_name,
		      pci->nickname ? pci->nickname : pci->name);
	      separ = delim;
	    }
	  }
	  fprintf(fp, "\n");
	  fflush(fp);
	}
	new_dataset = 0;
	nrows = 0;
	ncols = nci;
	Nbytes = 0;
      }

      if (packed) Nbytes += nd;

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
		fprintf(fp,"\"%s\"",cc);
	      }
	      break;
	    case DATATYPE_YYYYMMDD:
	      fprintf(fp, "%8.8d", (int)d[i]);
	      break;
	    case DATATYPE_HHMMSS:
	      fprintf(fp, "%6.6d", (int)d[i]);
	      break;
	    case DATATYPE_INT4:
	      fprintf(fp, "%d", (int)d[i]);
	      break;
	    default:
	      fprintf(fp, dbl_fmt, d[i]);
	      break;
	    } /* switch (pci->dtnum) */
	  }
	  separ = delim;
	} /* for (i=0; i<nd; i++) */

	fprintf(fp, print_newline ? "\n" : "\e[0K\r"); /* The "\e[0K" hassle clears up to the EOL */
	if (!print_newline) fflush(fp);
      } /* if (raw_binary) ... else ... */

    next_please:
      ++nrows;
      if (maxlines > 0 && ++nrtot >= maxlines) break; /* while (...) */
    } /* while (...) */

    PRINT_TIMING(NULL, nrows, ncols);
    /* if (!print_newline) fprintf(fp, "\n"); */

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
  return rc;
}
