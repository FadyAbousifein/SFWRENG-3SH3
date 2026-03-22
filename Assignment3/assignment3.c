/*
 * Name: Fady Abousifein
 * Student Number: 400506836
 * Course: SFWRENG 3SH3 - Operating Systems
 * Assignment: Assignment 3 - Memory Management (Paging Simulator)
 * Term: Winter 2026
 *
 * Compilation:
 *   gcc assignment3.c -o assignment3
 *
 * Execution:
 *   ./assignment3
 */

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

// Global Definitions (Paging)
#define OFFSET_BITS 8
#define OFFSET_MASK ((1 << OFFSET_BITS) - 1)
#define PAGE_SIZE (1 << OFFSET_BITS)              // 256 bytes
#define NUM_PAGES ((1 << 16) / PAGE_SIZE)        // 256 pages
#define PAGES ((1 << 15) / PAGE_SIZE)            // 128 frames

// Global Definitions (Constants)
#define BUFFER_SIZE 10
#define TLB_SIZE 16
#define BACKING_STORE_SIZE (1 << 16)

// Global Memory Structures
signed char* backingStorePtr;
signed char physicalMemory[PAGES * PAGE_SIZE];

int pageTable[NUM_PAGES];
int frameTable[PAGES];
int oldFrame = 0; 

// TLB structure
typedef struct {
    int pageNum[TLB_SIZE];
    int frameNum[TLB_SIZE];
    int curr; 
} TLBentry;

// Function prototypes
int addPage(int pageNum, TLBentry *TLB);
int search_TLB(int page, TLBentry *TLB);
void TLB_Add(int page, int frame, TLBentry *TLB);
void TLB_Update(int page, int frame, int oldPage, TLBentry *TLB);

int main(void) {

    int totalAddresses = 0;
    int pageFaults = 0;
    int TLBHits = 0;

    // Open the Backing Store
    int mmapfile_fd = open("BACKING_STORE.bin", O_RDONLY);
    if (mmapfile_fd == -1) {
        printf("Error opening BACKING_STORE.bin\n");
        return -1;
    }

    backingStorePtr = mmap(0, BACKING_STORE_SIZE, PROT_READ, MAP_PRIVATE, mmapfile_fd, 0);
    if (backingStorePtr == MAP_FAILED) {
        printf("mmap failed\n");
        return -1;
    }
    
    // Initialize all the memory structures 
    memset(physicalMemory, -1, sizeof(physicalMemory));
    memset(pageTable, -1, sizeof(pageTable));
    memset(frameTable, -1, sizeof(frameTable));

    // Open File
    FILE *fptr = fopen("addresses.txt", "r");
    if (fptr == NULL) {
        printf("Error opening addresses.txt\n");
        return -1;
    }

    // Initialize TLB
    TLBentry *TLB = malloc(sizeof(TLBentry));
    TLB->curr = 0;
    memset(TLB->pageNum, -1, sizeof(TLB->pageNum));
    memset(TLB->frameNum, -1, sizeof(TLB->frameNum));

    char addressBuffer[BUFFER_SIZE];

    // Iterate through file 
    while (fgets(addressBuffer, sizeof(addressBuffer), fptr)) {

        int address = atoi(addressBuffer);

        int offset = address & OFFSET_MASK;
        int page = address >> OFFSET_BITS;
        int frame;

        // Check TLB first
        int index = search_TLB(page, TLB);

        if (index != -1) {
            frame = TLB->frameNum[index];
            TLBHits++;
        } else {
            frame = pageTable[page];

            // Page fault
            if (frame == -1) {
                frame = addPage(page, TLB);
                pageFaults++;
            } else {
                TLB_Add(page, frame, TLB);
            }
        }

        int physical_address = frame * PAGE_SIZE + offset;
        int value = physicalMemory[physical_address];

        printf("Logical address: %d Physical address: %d Value: %d\n",
               address, physical_address, value);

        totalAddresses++;
    }

    // Print statistics
    printf("\nTotal addresses = %d\n", totalAddresses);
    printf("Page faults = %d\n", pageFaults);
    printf("TLB hits = %d\n", TLBHits);

    // Cleanup
    free(TLB);
    fclose(fptr);
    munmap(backingStorePtr, BACKING_STORE_SIZE);
    close(mmapfile_fd);

    return 0;
}

// Function to add page (handles page fault and FIFO replacement) 
int addPage(int pageNum, TLBentry *TLB) {

    int oldPage = frameTable[oldFrame];

    // Invalidate old page
    if (oldPage != -1) {
        pageTable[oldPage] = -1;
    }

    // Load new page into frame
    frameTable[oldFrame] = pageNum;
    pageTable[pageNum] = oldFrame;

    memcpy(&physicalMemory[oldFrame * PAGE_SIZE],
           backingStorePtr + (pageNum * PAGE_SIZE),
           PAGE_SIZE);

    // FIFO circular update
    oldFrame = (oldFrame + 1) % PAGES;

    // Update TLB
    TLB_Update(pageNum, pageTable[pageNum], oldPage, TLB);

    return pageTable[pageNum];
}

// Function searches through TLB for page
int search_TLB(int page, TLBentry *TLB) {
    for (int i = 0; i < TLB_SIZE; i++) {
        if (TLB->pageNum[i] == page)
            return i;
    }
    return -1;
}

// Function to add page and frame to TLB
void TLB_Add(int page, int frame, TLBentry *TLB) {
    TLB->pageNum[TLB->curr] = page;
    TLB->frameNum[TLB->curr] = frame;
    TLB->curr = (TLB->curr + 1) % TLB_SIZE;
}

// Function to update TLB when a page has been replaced
void TLB_Update(int page, int frame, int oldPage, TLBentry *TLB) {

    int i = search_TLB(oldPage, TLB);

    if (i  == -1) {
        TLB_Add(page, frame, TLB);
        return;
    }

    TLB->pageNum[i] = page;
    TLB->frameNum[i] = frame;
}
