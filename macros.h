
#define BNS_ARRAY_COUNT(x) ((int)(sizeof(x)/sizeof((x)[0])))

#define BNS_UNUSED(x) (void(x))


#define BNS_MIN(a,b) ((a) < (b) ? (a) : (b))
#define BNS_MAX(a,b) ((a) > (b) ? (a) : (b))

#define BNS_ABS(x) (((x) < 0) ? -(x) : (x))
