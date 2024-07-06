/**
 * @file utils.h
 *
 * @brief This file contains the utility macros
 *
 * This file contains the utility macros
 *
 * @author Ritam Singal (ritamsingal)
 * @author Harshil Patel (harshil-patel11)
 * @bug No known bugs
 */

#ifndef UTILS_H
#define UTILS_H

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define CEIL(x) ((x) >= 0 ? ((int)(x) + ((x) != (int)(x))) : (int)(x))
#define FLOOR(x) ((x) >= 0 ? (int)(x) : ((int)(x) - ((x) != (int)(x))))

#endif