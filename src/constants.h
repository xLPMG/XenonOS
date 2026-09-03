#ifndef CONSTANTS_H
#define CONSTANTS_H

#define XENON_OS_VERSION "0.1"

// MARK: Terminal

#define VGA_WIDTH 80
#define VGA_HEIGHT 25

// MARK: Memory

#define PAGE_SIZE 4096
#define MAX_MEMORY (4 * 1024 * 1024 * 1024ULL) // = 4 GB (is the max anyway)

// #frames = MAX_MEMORY / PAGE_SIZE
// each bit in the bitmap represents a frame (4 KB)
// 1 = used, 0 = free
// 32 bits per uint32_t
#define BITMAP_SIZE (MAX_MEMORY / PAGE_SIZE / 32)

// Total number of physical frames the bitmap can track
#define FRAME_COUNT (BITMAP_SIZE * 32)

#define PAGE_DIRECTORY_ENTRIES 1024
#define PAGE_TABLE_ENTRIES 1024
#define RECURSIVE_INDEX 1023
#define RECURSIVE_TABLES_BASE 0xFFC00000u

#endif // CONSTANTS_H