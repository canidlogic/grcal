/*
 * grcal_query.c
 * =============
 * 
 * Use the grcal library routines to convert between Gregorian dates and
 * day offsets.
 * 
 * Syntax
 * ------
 * 
 *   grcal_query [offset]
 *   grcal_query [year] [month] [day]
 * 
 * Operation
 * ---------
 * 
 * The one-argument invocation takes a day offset and reports the year,
 * month, day, and weekday that it corresponds to on the Gregorian
 * calendar.  The day offset of zero is 1582-10-15.
 * 
 * The three-argument invocation takes a year, month, day in the
 * Gregorian calendar and reports the day offset.
 * 
 * Successful results are reported to standard output.  Errors are
 * reported to standard error.
 * 
 * Compilation
 * -----------
 * 
 * Must be built with the grcal library.  Sample invocation for gcc:
 * 
 *   gcc -o grcal_query grcal_query.c grcal.c
 */

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "grcal.h"

/*
 * Constants
 * =========
 */

/*
 * An array of strings containing three-letter names for the days of the
 * week, starting with Monday.
 */
static char *DAY_NAMES[7] = {
  "Mon",
  "Tue",
  "Wed",
  "Thu",
  "Fri",
  "Sat",
  "Sun"
};

/*
 * Local functions
 * ===============
 */

/* Prototypes */
static int parseInt(const char *pstr, int32_t *pv);

/*
 * Parse the given string as a signed integer.
 * 
 * pstr is the string to parse.
 * 
 * pv points to the integer value to use to return the parsed numeric
 * value if the function is successful.
 * 
 * In two's complement, this function will not successfully parse the
 * least negative value.
 * 
 * Parameters:
 * 
 *   pstr - the string to parse
 * 
 *   pv - pointer to the return numeric value
 * 
 * Return:
 * 
 *   non-zero if successful, zero if failure
 */
static int parseInt(const char *pstr, int32_t *pv) {
  
  int negflag = 0;
  int32_t result = 0;
  int status = 1;
  int32_t d = 0;
  
  /* Check parameters */
  if ((pstr == NULL) || (pv == NULL)) {
    abort();
  }
  
  /* If first character is a sign character, set negflag appropriately
   * and skip it */
  if (*pstr == '+') {
    negflag = 0;
    pstr++;
  } else if (*pstr == '-') {
    negflag = 1;
    pstr++;
  } else {
    negflag = 0;
  }
  
  /* Make sure we have at least one digit */
  if (*pstr == 0) {
    status = 0;
  }
  
  /* Parse all digits */
  if (status) {
    for( ; *pstr != 0; pstr++) {
    
      /* Make sure in range of digits */
      if ((*pstr < '0') || (*pstr > '9')) {
        status = 0;
      }
    
      /* Get numeric value of digit */
      if (status) {
        d = (int32_t) (*pstr - '0');
      }
      
      /* Multiply result by 10, watching for overflow */
      if (status) {
        if (result <= INT32_MAX / 10) {
          result = result * 10;
        } else {
          status = 0; /* overflow */
        }
      }
      
      /* Add in digit value, watching for overflow */
      if (status) {
        if (result <= INT32_MAX - d) {
          result = result + d;
        } else {
          status = 0; /* overflow */
        }
      }
    
      /* Leave loop if error */
      if (!status) {
        break;
      }
    }
  }
  
  /* Invert result if negative mode */
  if (status && negflag) {
    result = -(result);
  }
  
  /* Write result if successful */
  if (status) {
    *pv = result;
  }
  
  /* Return status */
  return status;
}

/*
 * Program entrypoint
 * ==================
 */

int main(int argc, char *argv[]) {
  
  int status = 1;
  int i = 0;
  const char *pModule = NULL;
  
  int32_t in_offs = 0;
  int32_t in_year = 0;
  int32_t in_month = 0;
  int32_t in_day = 0;
  
  int32_t o_offs = 0;
  int o_year = 0;
  int o_month = 0;
  int o_day = 0;
  int o_wkday = 0;
  
  /* Get module name */
  if ((argc > 0) && (argv != NULL)) {
    pModule = argv[0];
  }
  if (pModule == NULL) {
    pModule = "grcal_query";
  }
  
  /* Check that arguments are present */
  if ((argc > 0) && (argv == NULL)) {
    abort();
  }
  for(i = 0; i < argc; i++) {
    if (argv[i] == NULL) {
      abort();
    }
  }
  
  /* Must have either one additional parameter or three */
  if ((argc != 2) && (argc != 4)) {
    fprintf(stderr, "%s: Wrong number of parameters!\n", pModule);
    status = 0;
  }
  
  /* Determine course of action based on number of arguments */
  if (status && (argc == 2)) {
    /* Convert day offset to Gregorian date -- parse the parameter */
    if (!parseInt(argv[1], &in_offs)) {
      fprintf(stderr, "%s: Could not parse parameter!\n", pModule);
      status = 0;
    }
    
    /* Check range of adjusted day offset */
    if (status) {
      if ((in_offs < 0) || (in_offs > GRCAL_DAY_MAX)) {
        fprintf(stderr, "%s: Day offset out of range!\n", pModule);
        status = 0;
      }
    }
    
    /* Convert offset to date and weekday */
    if (status) {
      grcal_offsetToDate(in_offs, &o_year, &o_month, &o_day);
      o_wkday = grcal_weekday(in_offs);
    }
    
    /* Print the result */
    if (status) {
      printf("%04d-%02d-%02d %s\n",
              o_year, o_month, o_day,
              DAY_NAMES[o_wkday - 1]);
    }
    
  } else if (status && (argc == 4)) {
    /* Convert Gregorian date to day offset -- parse parameters */
    if (status) {
      if (!parseInt(argv[1], &in_year)) {
        fprintf(stderr, "%s: Could not parse year!\n", pModule);
        status = 0;
      }
    }
    
    if (status) {
      if (!parseInt(argv[2], &in_month)) {
        fprintf(stderr, "%s: Could not parse month!\n", pModule);
        status = 0;
      }
    }
    
    if (status) {
      if (!parseInt(argv[3], &in_day)) {
        fprintf(stderr, "%s: Could not parse day!\n", pModule);
        status = 0;
      }
    }
    
    /* Check basic ranges of each argument to prevent integer casting
     * problems */
    if (status) {
      if ((in_year < 0) || (in_year > 9999)) {
        fprintf(stderr, "%s: Year is out of range!\n", pModule);
        status = 0;
      }
    }
    
    if (status) {
      if ((in_month < 1) || (in_month > 12)) {
        fprintf(stderr, "%s: Month is out of range!\n", pModule);
        status = 0;
      }
    }
    
    if (status) {
      if ((in_day < 1) || (in_day > 31)) {
        fprintf(stderr, "%s: Day is out of range!\n", pModule);
        status = 0;
      }
    }
    
    /* Attempt to convert date to offset */
    if (status) {
      if (!grcal_dateToOffset(
            &o_offs,
            (int) in_year,
            (int) in_month,
            (int) in_day)) {
        fprintf(stderr, "%s: Date is not valid!\n", pModule);
        status = 0;
      }
    }
    
    /* Print the result */
    if (status) {
      printf("%ld\n", (long) o_offs);
    }
    
  } else if (status) {
    /* Shouldn't happen because we checked argument count earlier */
    abort();
  }
  
  /* Invert status and return */
  if (status) {
    status = 0;
  } else {
    status = 1;
  }
  return status;
}
