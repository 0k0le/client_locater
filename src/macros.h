#ifndef __MACROS_H
#define __MACROS_H

// Useful macros

// Cross C/C++
#define UNUSED(x) ((void)(x))
#define ELEMENTSINARRAY(array) (sizeof(array)/sizeof(array[0]))

#if !defined(__cplusplus)

#define START ({
#define END })

#else

#define START {
#define END }

#endif //!defined(__cplusplus)

// ERROR HANDLER DEFINE
#define ERRCQ(exitcode, format, ...) START \
    fprintf(stderr, "%s() @ L%d " format "\nstrerror --> %s\n", \
	 __FUNCTION__, __LINE__, ##__VA_ARGS__, strerror(errno)); \
    fflush(stdout); \
    fflush(stderr); \
    exit(exitcode); \
END
#define ERRQ(format, ...) ERRCQ(1, format, ##__VA_ARGS__);

// ASSERT DEFINE
#ifdef _DEBUG
#define ASSERT(expr) \
    if(!(expr)) { \
        fprintf(stderr, "Failed @ LINE %d in FUNC %s() with expression " #expr "\n", __LINE__, __FUNCTION__); \
        fflush(stderr); \
        assert(expr); \
    } \
    
#else
#define ASSERT(expr) assert(expr);
#endif // _DEBUG

// USAGE DEFINE
#define USAGE(arg0, format, ...) START \
    fprintf(stderr, "%s " format "\n", arg0, ##__VA_ARGS__); \
    exit(1); \
END

#ifdef _DEBUG
#define DEBUG(format, ...) \
	printf("DEBUG: " format "\n", ##__VA_ARGS__);

#else
#define DEBUG(format, ...)
#endif

#endif

