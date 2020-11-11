#pragma once

#define min(X, Y) (((X) < (Y)) ? (X) : (Y))
#define max(a,b) (((a)>(b))?(a):(b))

#define CONST_ARR_LEN(arr) sizeof(arr) / sizeof(arr[0])