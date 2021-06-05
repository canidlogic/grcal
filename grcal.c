/*
 * grcal.c
 * 
 * Implementation of grcal.h
 * 
 * See the header for further information.
 */

#include "grcal.h"
#include <stdlib.h>

/*
 * Constants
 * =========
 */

/*
 * The day index of 1582-10-15 on a proleptic Gregorian calendar where
 * day zero is 1200-03-01.
 * 
 * Internal calculations use day offsets from 1200-03-01 because the
 * computation is much easier that way.
 */
#define DAY_OFFSET INT32_C(139750)

/*
 * The day offset of the first Monday in the Gregorian calendar.
 * 
 * Day offset zero is 1582-10-15, which is a Friday, so this constant
 * should have a value of 3.
 */
#define FIRST_MONDAY 3

/*
 * The number of days in a week.
 */
#define WEEK_LENGTH 7

/*
 * The number of months in a year.
 */
#define MONTH_COUNT 12

/*
 * The number of months that March-based years are offset from standard
 * Gregorian years.
 */
#define MONTH_OFFSET 2

/*
 * The number of days in a long, 31-day month.
 */
#define LONG_MONTH_LENGTH 31

/*
 * The number of days in a short, 30-day month.
 */
#define SHORT_MONTH_LENGTH 30

/*
 * The number of days in a month with a leap day in leap year.
 */
#define LEAP_MONTH_LENGTH 29

/*
 * The number of days in a month with a leap day in a non-leap year.
 */
#define NONLEAP_MONTH_LENGTH 28

/*
 * The number of days in an aligned quad century (400 years).
 */
#define QC_DAYS INT32_C(146097)

/*
 * The number of days in an aligned century.
 */
#define C_DAYS INT32_C(36524)

/*
 * The number of days in an aligned quad year (4 years).
 */
#define Q_DAYS 1461

/*
 * The number of days in a year, not including the leap day at the end
 * of a quad year.
 */
#define Y_DAYS 365

/*
 * The number of days in a leap year.
 */
#define Y_LEAP_DAYS 366

/*
 * The number of centuries in a quad century.
 */
#define QC_C_COUNT 4

/*
 * The number of quad years in a century.
 */
#define C_Q_COUNT 25

/*
 * The number of years in a quad year.
 */
#define Q_Y_COUNT 4

/*
 * The number of years in a quad century.
 */
#define QC_YEARS 400

/*
 * The number of years in a century.
 */
#define C_YEARS 100

/*
 * The number of years in a quad year.
 */
#define Q_YEARS 4

/*
 * The year in which day offset zero happened.
 * 
 * This is *after* adjustment by DAY_OFFSET.
 */
#define BASE_YEAR 1200

/*
 * The last year supported in the Gregorian calendar.
 */
#define MAX_YEAR 9999

/*
 * The pattern of March-based month lengths, expressed as a string.
 * 
 * This string has twelve characters, corresponding to the twelve
 * months.  However, the first character corresponds to the *third*
 * month, which causes the variable-length second month to be the last
 * month in the pattern.
 * 
 * Pattern digits that are "+" indicate 31-day months, pattern digits
 * that are "-" indicate 30-day months, and pattern digits that are "*"
 * indicate variable-length months.
 */
static const char *m_pattern = "+-+-++-+-++*";

/*
 * Local functions
 * ===============
 */

/* Function prototypes */
static int isLeapYear(int y);
static int monthLength(int i);

/*
 * Determine whether the given (January-based) year is a leap year
 * according to Gregorian calendar rules.
 * 
 * The year must be at least one or a fault occurs.
 * 
 * Parameters:
 * 
 *   y - the year to check
 * 
 * Return:
 * 
 *   non-zero if leap year, zero if not
 */
static int isLeapYear(int y) {
  int leapyear = 0;
  
  /* Check parameter */
  if (y < 1) {
    abort();
  }
  
  /* Years divisible by 400 are leap years */
  if (y % 400 == 0) {
    leapyear = 1;
  }
  
  /* Years divisible by 4 but not by 100 are leap years */
  if ((y % 4 == 0) && (y % 100 != 0)) {
    leapyear = 1;
  }
  
  /* Return result */
  return leapyear;
}

/*
 * Return the length of the month corresponding to the provided
 * March-based month index.
 * 
 * The value i has zero for the third month of the Gregorian year,
 * matching the indexing system used for m_pattern.  A fault occurs if
 * it is out of range.
 * 
 * The return value is either the number of days the month always has,
 * or zero if the month has variable length.
 * 
 * Parameters:
 * 
 *   i - the zero-indexed, March-based month offset
 * 
 * Return:
 * 
 *   the number of days in this month, or zero if the month is variable
 *   length
 */
static int monthLength(int i) {
  
  char c = 0;
  int result = 0;
  
  /* Check parameter */
  if ((i < 0) || (i >= MONTH_COUNT)) {
    abort();
  }
  
  /* Get the requested pattern character */
  c = m_pattern[i];
  
  /* Interpret the pattern character */
  if (c == '+') {
    result = LONG_MONTH_LENGTH;
  
  } else if (c == '-') {
    result = SHORT_MONTH_LENGTH;
    
  } else if (c == '*') {
    result = 0;
    
  } else {
    abort();
  }
  
  /* Return result */
  return result;
}

/*
 * Public function implementations
 * ===============================
 * 
 * See the header for specifications.
 */

/*
 * grcal_offsetToDate function.
 */
void grcal_offsetToDate(
    int32_t   offs,
    int     * pYear,
    int     * pMonth,
    int     * pDayOfMonth) {
  
  int qc = 0;
  int c = 0;
  int q = 0;
  int y = 0;
  int d = 0;
  
  int year = 0;
  int month = 0;
  int day = 0;
  
  int ml = 0;
  
  /* Check parameter */
  if ((offs < 0) || (offs > GRCAL_DAY_MAX)) {
    abort();
  }
  
  /* Adjust offset so it uses 1200-03-01 as day zero */
  offs += DAY_OFFSET;
  
  /* Compute number of quad centuries, centuries, quad years, and
   * years, and adjust offs for remainder of days */
  qc   = (int) (offs / QC_DAYS);
  offs =        offs % QC_DAYS ;
  
  c    = (int) (offs / C_DAYS);
  offs =        offs % C_DAYS ;
  
  q    = (int) (offs / Q_DAYS);
  offs =        offs % Q_DAYS ;
  
  y    = (int) (offs / Y_DAYS);
  offs =        offs % Y_DAYS ;
  
  d    = (int)  offs;
  
  /* Special case:  c might be four, but only if the day offset refers
   * to the leap day that is added at the end of each quad century; in
   * this special case, adjust c, q, y, and d so that they refer to
   * the leap day at the end of the quad century */
  if (c == QC_C_COUNT) {
    c = QC_C_COUNT - 1;
    q = C_Q_COUNT - 1;
    y = Q_Y_COUNT - 1;
    d = Y_LEAP_DAYS - 1;
  }
  
  /* Special case:  y might be four, but only if the day offset refers
   * to the leap day that is added at the end of a quad year; in this
   * special case, adjust y and d so that they refer to the leap day
   * at the end of a quad year */
  if (y == Q_Y_COUNT) {
    y = Q_Y_COUNT - 1;
    d = Y_LEAP_DAYS - 1;
  }
  
  /* Compute the (March-based) year */
  year =  (qc * QC_YEARS) + (c * C_YEARS) +
          (q  * Q_YEARS)  +  y            + BASE_YEAR;
  
  /* Compute the (March-based) month offset */
  month = 0;
  while (d > 0) {
    /* Get the length in days of the current month offset */
    ml = monthLength(month);
    
    /* If this is a variable-length month or this month contains the
     * remaining day offset, break */
    if ((ml == 0) || (d < ml)) {
      break;
    }
    
    /* Otherwise, increase the month count and decrease the
     * remaining days */
    month++;
    d -= ml;
  }
  
  /* Compute the (final, one-based) day of the month */
  day = d + 1;
  
  /* Apply the month offset to the month to get the (zero-based, mod
   * 12) standard month offset */
  month += MONTH_OFFSET;
  
  /* If adjusted month went past the end of the year, subtract twelve
   * from the month and add one to the year to get the (zero-based)
   * standard month offset */
  if (month >= MONTH_COUNT) {
    month -= MONTH_COUNT;
    year++;
  }
  
  /* Add one to the month offset to get the standard, one-based month
   * number */
  month++;
  
  /* Return any computed results that were requested */
  if (pYear != NULL) {
    *pYear = year;
  }
  
  if (pMonth != NULL) {
    *pMonth = month;
  }
  
  if (pDayOfMonth != NULL) {
    *pDayOfMonth = day;
  }
}

/*
 * grcal_dateToOffset function.
 */
int grcal_dateToOffset(
    int32_t * pOffs,
    int       year,
    int       month,
    int       dayofmonth) {
  
  int result = 1;
  int month_len = 0;
  
  int32_t qc = 0;
  int32_t c = 0;
  int32_t q = 0;
  int32_t y = 0;
  
  int32_t offs = 0;
  int x = 0;
  
  /* Fail if the year is BASE_YEAR or less, or if month or dayofmonth
   * are less than one */
  if ((year <= BASE_YEAR) || (month < 1) || (dayofmonth < 1)) {
    result = 0;
  }
  
  /* Fail if the year is greater than MAX_YEAR or if month is greater
   * than MONTH_COUNT -- the range of everything but the upper bound
   * of dayofmonth has now been checked */
  if (result) {
    if ((year > MAX_YEAR) || (month > MONTH_COUNT)) {
      result = 0;
    }
  }
  
  /* Convert to zero-based day offset within month */
  if (result) {
    dayofmonth--;
  }
  
  /* Convert to March-based year offsets by making month zero-based,
   * then subtracting two months, and decrementing the year and adding
   * twelve to the month if it goes below zero */
  if (result) {
    month = month - 1 - MONTH_OFFSET;
    if (month < 0) {
      year--;
      month += MONTH_COUNT;
    }
  }
  
  /* Check the day offset against the length of the month, taking leap
   * years into account */
  if (result) {
    /* Get the length of the month */
    month_len = monthLength(month);
    
    /* If month is variable length, use leap year to determine month
     * length -- increment year because March-based years have their
     * last (leap) month in the next year according to the
     * January-based years used for leap year calculations */
    if (month_len == 0) {
      if (isLeapYear(year + 1)) {
        month_len = LEAP_MONTH_LENGTH;
      } else {
        month_len = NONLEAP_MONTH_LENGTH;
      }
    }
    
    /* Verify upper bound of day offset within month */
    if (dayofmonth >= month_len) {
      result = 0;
    }
  }
  
  /* We've now converted to March-based offsets and checked the range
   * of the input parameters -- begin by decreasing the year by
   * BASE_YEAR to make it relative to the base year */
  if (result) {
    year -= BASE_YEAR;
  }
  
  /* Get the number of quad centuries, centuries, and quad years in
   * the year offset, leaving year as the remainder years */
  if (result) {
    qc   = (int32_t) (year / QC_YEARS);
    year =            year % QC_YEARS ;
    
    c    = (int32_t) (year / C_YEARS);
    year =            year % C_YEARS ;
    
    q    = (int32_t) (year / Q_YEARS);
    y    = (int32_t) (year % Q_YEARS);
  }
  
  /* Calculate the number of days to the start of the year, using
   * 1200-03-01 as day zero */
  if (result) {
    offs = (qc * QC_DAYS) + (c * C_DAYS) + 
           (q  * Q_DAYS ) + (y * Y_DAYS);
  }
  
  /* Get to the start of the month by adding month lengths together;
   * the variable length month won't figure in here because that is
   * the last month in March-based years and we are only getting to
   * the beginning of months, not passing them */
  if (result) {
    for(x = 0; x < month; x++) {
      offs += (int32_t) (monthLength(x));
    }
  }
  
  /* Finally, add the day-in-month offset in */
  if (result) {
    offs += (int32_t) (dayofmonth);
  }
  
  /* We can now convert to a day offset with day zero as 1582-10-15 */
  if (result) {
    offs -= DAY_OFFSET;
  }
  
  /* Fail if offset is outside the allowable range */
  if (result) {
    if ((offs < 0) || (offs > GRCAL_DAY_MAX)) {
      result = 0;
    }
  }
  
  /* Write the computed offset, if it was requested */
  if (result) {
    if (pOffs != NULL) {
      *pOffs = offs;
    }
  }
  
  /* Return status */
  return result;
}

/*
 * grcal_weekday function.
 */
int grcal_weekday(int32_t offs) {
  
  /* Check parameter */
  if ((offs < 0) || (offs > GRCAL_DAY_MAX)) {
    abort();
  }
  
  /* If the day offset is before the first Monday of the Gregorian
   * calendar, advance it by a week */
  if (offs < FIRST_MONDAY) {
    offs += WEEK_LENGTH;
  }
  
  /* Compute the result */
  return ((int) ((offs - FIRST_MONDAY) % WEEK_LENGTH)) + 1;
}
