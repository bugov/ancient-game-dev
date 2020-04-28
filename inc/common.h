#ifndef _COMMON_H_
#define _COMMON_H_

#define CELL_WIDTH 96
#define CELL_HEIGHT 96
#define ICON_SIZE 48
#define FPS 20
#define SECOND 1000

#ifdef DEBUG
#define dp(...) do { fprintf( stderr, __VA_ARGS__ ); } while (0)
#else
#define dp(...) do { } while (0)
#endif

#define min(a, b) ((a) < (b) ? (a) : (b))
#define max(a, b) ((a) > (b) ? (a) : (b))

#endif // _COMMON_H_

