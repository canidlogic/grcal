#ifndef GRCAL_H_INCLUDED
#define GRCAL_H_INCLUDED

/*
 * grcal.h
 * =======
 * 
 * Provides functions for working with Gregorian calendar dates.  This
 * only provides the core Gregorian functions of moving between counts
 * of days and year-month-day dates, as well as determining the day of
 * the week.
 * 
 * Day offsets can be in range [0, GRCAL_DAY_MAX].  The day offset zero
 * corresponds to 1582-10-15, which is the first day on the Gregorian
 * calendar.  Any dates prior to this would have used the Julian
 * calendar, and there is a discontinuity immediately prior to the day
 * zero date of 1582-10-15, such that the previous day was actually
 * 1582-10-04 in the Julian calendar.  This is why day offsets prior to
 * zero are not supported by this library.
 * 
 * The implementation code may make reference to "proleptic" dates in
 * the Gregorian calendar prior to 1582-10-15.  These dates are computed
 * as if the Gregorian calendar were already in effect prior to the day
 * zero 1582-10-15, but they will not match historical dates because the
 * Julian calendar was used previously.  Proleptic dates are only used
 * during computations; clients are never able to use proleptic dates in
 * the public interface.
 */

#include <stddef.h>
#include <stdint.h>

/*
 * The maximum valid Gregorian day offset.
 * 
 * This corresponds to 9999-12-31, after which the four-digit year
 * format will overflow.
 */
#define GRCAL_DAY_MAX INT32_C(3074323)

/*
 * The day offset of the Unix epoch (January 1, 1970).
 * 
 * This is not actually used within the grcal library, but it is
 * provided for convenience of clients that are working with Unix
 * timestamps.
 */
#define GRCAL_DAY_UNIX INT32_C(141427)

/*
 * Convert a Gregorian day offset into the year, month, and day of
 * month.
 * 
 * The month and day of month are one-indexed, so the first month is one
 * and the first day of the month is one.
 * 
 * The provided Gregorian day offset must be in range zero up to and
 * including GRCAL_DAY_MAX or a fault occurs.
 * 
 * The Gregorian day offset system is defined such that day zero
 * corresponds to 1582-10-15.
 * 
 * You may pass NULL for any return values that you do not require.  The
 * calculation will still be the same.  However, the return values are
 * undefined if more than one passed pointer points to the same integer.
 * 
 * Parameters:
 * 
 *   offs - the Gregorian day offset to convert
 * 
 *   pYear - pointer to the variable to receive the Gregorian year, or
 *   NULL
 * 
 *   pMonth - pointer to the variable to receive the Gregorian month, or
 *   NULL
 * 
 *   pDayOfMonth - pointer to the variable to receive the Gregorian day
 *   of the month, or NULL
 */
void grcal_offsetToDate(
    int32_t   offs,
    int     * pYear,
    int     * pMonth,
    int     * pDayOfMonth);

/*
 * Convert a Gregorian date into a Gregorian day offset.
 * 
 * The month and day are one-indexed, so the first month is one and the
 * first day of the month is one.
 * 
 * The Gregorian day offset will be written to *pOffs if the function is
 * successful.  If NULL, the value is not written.  This can be useful
 * for merely checking whether a year-month-day combination is valid.
 * 
 * If the provided year-month-day combination is not valid in the
 * Gregorian calendar, the function will fail and zero will be returned.
 * 
 * Parameters:
 * 
 *   pOffs - pointer to the variable to receive the converted Gregorian
 *   day offset, or NULL to just check whether the year-month-day combo
 *   is valid
 * 
 *   year - the Gregorian year
 * 
 *   month - the month of the year
 * 
 *   dayofmonth - the day of the month
 * 
 * Return:
 * 
 *   non-zero if successful, zero if provided year-month-day combination
 *   is not valid
 */
int grcal_dateToOffset(
    int32_t * pOffs,
    int       year,
    int       month,
    int       dayofmonth);

/*
 * Convert a Gregorian day offset into a weekday.
 * 
 * The provided Gregorian day offset must be in range zero up to and
 * including GRCAL_DAY_MAX or a fault occurs.
 * 
 * The Gregorian day offset system is defined such that day zero
 * corresponds to 1582-10-15.
 * 
 * The return value is the day within the 7-day week that the given day
 * offset occurs on.  The following are all possible return values:
 * 
 *   1 - Monday
 *   2 - Tuesday
 *   3 - Wednesday
 *   4 - Thursday
 *   5 - Friday
 *   6 - Saturday
 *   7 - Sunday
 * 
 * Parameters:
 * 
 *   offs - the Gregorian day offset
 * 
 * Return:
 * 
 *   the one-indexed weekday of that day, where one is Monday
 */
int grcal_weekday(int32_t offs);

#endif
