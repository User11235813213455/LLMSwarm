#pragma once

/**
 * @brief Convenience function which returns a counter which counts milliseconds; No assumptions shall be made about the initial value
 * @return Millisecond counter with an undefined start
 */
unsigned int getMillis();

/**
 * @brief Returns the time difference between to millisecond timestamps while handling up to one overflow; The order of the timestamps is important because the overflow calulcation depends on it
 * @param pTimestampOld The "older" timestamp
 * @param pTimestampNew The "newer" timestamp
 * @return Time difference in milliseconds between the two timestamps
 */
unsigned int getTimedif(unsigned int pTimestampOld, unsigned int pTimestampNew);