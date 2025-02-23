#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <math.h>

#include "storage_mgr.h"

// Global file handler for managing disk operations
FILE *diskHandler = NULL;

// Initializes the storage manager by setting the disk handler to NULL.
void initStorageManager(void) {
    // Initialize the disk handler to NULL at startup
    diskHandler = NULL;
}

// Creates a new page file with the specified name and initializes it with an empty page.
RC createPageFile(char *fileName) {
    // Try to create a new file with read/write permissions
    diskHandler = fopen(fileName, "wb+");
    
    if (!diskHandler) {
        return RC_FILE_NOT_FOUND;
    }
    
    // Allocate memory for initial page
    char *initialPage = malloc(PAGE_SIZE);
    if (!initialPage) {
        fclose(diskHandler);
        return RC_WRITE_FAILED;
    }
    
    // Initialize page with zeros
    memset(initialPage, 0, PAGE_SIZE);
    
    // Write the initial empty page
    size_t writeResult = fwrite(initialPage, 1, PAGE_SIZE, diskHandler);
    free(initialPage);
    
    if (writeResult != PAGE_SIZE) {
        fclose(diskHandler);
        return RC_WRITE_FAILED;
    }
    
    fclose(diskHandler);
    return RC_OK;
}

// Opens an existing page file and initializes the file handle with its statistics.
RC openPageFile(char *fileName, SM_FileHandle *fileHandle) {
    struct stat fileStats;
    
    // Attempt to open existing file
    diskHandler = fopen(fileName, "rb+");
    if (!diskHandler) {
        return RC_FILE_NOT_FOUND;
    }
    
    // Get file statistics
    if (fstat(fileno(diskHandler), &fileStats) < 0) {
        fclose(diskHandler);
        return RC_ERROR;
    }
    
    // Initialize file handle
    fileHandle->fileName = fileName;
    fileHandle->totalNumPages = fileStats.st_size / PAGE_SIZE;
    fileHandle->curPagePos = 0;
    
    fclose(diskHandler);
    return RC_OK;
}

// Closes the page file associated with the given file handle.
RC closePageFile(SM_FileHandle *fileHandle) {
    if (!fileHandle) {
        return RC_FILE_HANDLE_NOT_INIT;
    }
    
    if (diskHandler) {
        diskHandler = NULL;
    }
    
    return RC_OK;
}

// Destroys the specified page file if it exists.
RC destroyPageFile(char *fileName) {
    // Verify file exists before attempting deletion
    if (access(fileName, F_OK) != -1) {
        if (remove(fileName) == 0) {
            return RC_OK;
        }
    }
    return RC_FILE_NOT_FOUND;
}

// Reads a block of data from the specified page number into the provided memory page.
RC readBlock(int pageNum, SM_FileHandle *fileHandle, SM_PageHandle memPage) {
    // Validate inputs
    if (!fileHandle || !memPage) {
        return RC_FILE_HANDLE_NOT_INIT;
    }
    
    if (pageNum < 0 || pageNum >= fileHandle->totalNumPages) {
        return RC_READ_NON_EXISTING_PAGE;
    }
    
    // Open file for reading
    diskHandler = fopen(fileHandle->fileName, "rb");
    if (!diskHandler) {
        return RC_FILE_NOT_FOUND;
    }
    
    // Calculate page offset
    long offset = pageNum * PAGE_SIZE;
    
    // Seek to correct position
    if (fseek(diskHandler, offset, SEEK_SET) != 0) {
        fclose(diskHandler);
        return RC_READ_NON_EXISTING_PAGE;
    }
    
    // Read page content
    size_t bytesRead = fread(memPage, 1, PAGE_SIZE, diskHandler);
    
    if (bytesRead != PAGE_SIZE) {
        fclose(diskHandler);
        return RC_ERROR;
    }
    
    // Update current position
    fileHandle->curPagePos = offset + PAGE_SIZE;
    
    fclose(diskHandler);
    return RC_OK;
}

// Returns the current position of the block in the file handle.
int getBlockPos(SM_FileHandle *fileHandle) {
    return fileHandle ? fileHandle->curPagePos : -1;
}

// Reads the first block of the page file into the provided memory page.
RC readFirstBlock(SM_FileHandle *fileHandle, SM_PageHandle memPage) {
    if (!fileHandle || !memPage) {
        return RC_FILE_HANDLE_NOT_INIT;
    }
    
    diskHandler = fopen(fileHandle->fileName, "rb");
    if (!diskHandler) {
        return RC_FILE_NOT_FOUND;
    }
    
    // Read first page
    size_t bytesRead = fread(memPage, 1, PAGE_SIZE, diskHandler);
    
    if (bytesRead != PAGE_SIZE) {
        fclose(diskHandler);
        return RC_ERROR;
    }
    
    fileHandle->curPagePos = PAGE_SIZE;
    
    fclose(diskHandler);
    return RC_OK;
}

// Reads the previous block relative to the current position in the file handle.
RC readPreviousBlock(SM_FileHandle *fileHandle, SM_PageHandle memPage) {
    if (!fileHandle || !memPage) {
        return RC_FILE_HANDLE_NOT_INIT;
    }
    
    int currentPage = fileHandle->curPagePos / PAGE_SIZE;
    if (currentPage <= 1) {
        return RC_READ_NON_EXISTING_PAGE;
    }
    
    return readBlock(currentPage - 2, fileHandle, memPage);
}

// Reads the current block into the provided memory page.
RC readCurrentBlock(SM_FileHandle *fileHandle, SM_PageHandle memPage) {
    if (!fileHandle || !memPage) {
        return RC_FILE_HANDLE_NOT_INIT;
    }
    
    int currentPage = fileHandle->curPagePos / PAGE_SIZE;
    return readBlock(currentPage - 1, fileHandle, memPage);
}

// Reads the next block relative to the current position in the file handle.
RC readNextBlock(SM_FileHandle *fileHandle, SM_PageHandle memPage) {
    if (!fileHandle || !memPage) {
        return RC_FILE_HANDLE_NOT_INIT;
    }
    
    int currentPage = fileHandle->curPagePos / PAGE_SIZE;
    if (currentPage >= fileHandle->totalNumPages) {
        return RC_READ_NON_EXISTING_PAGE;
    }
    
    return readBlock(currentPage, fileHandle, memPage);
}

// Reads the last block of the page file into the provided memory page.
RC readLastBlock(SM_FileHandle *fileHandle, SM_PageHandle memPage) {
    if (!fileHandle || !memPage) {
        return RC_FILE_HANDLE_NOT_INIT;
    }
    
    return readBlock(fileHandle->totalNumPages - 1, fileHandle, memPage);
}

// Writes a block of data to the specified page number from the provided memory page.
RC writeBlock(int pageNum, SM_FileHandle *fileHandle, SM_PageHandle memPage) {
    if (!fileHandle || !memPage) {
        return RC_FILE_HANDLE_NOT_INIT;
    }
    
    if (pageNum < 0) {
        return RC_WRITE_FAILED;
    }
    
    // Ensure capacity if writing beyond current size
    if (pageNum >= fileHandle->totalNumPages) {
        RC result = ensureCapacity(pageNum + 1, fileHandle);
        if (result != RC_OK) {
            return result;
        }
    }
    
    diskHandler = fopen(fileHandle->fileName, "rb+");
    if (!diskHandler) {
        return RC_FILE_NOT_FOUND;
    }
    
    // Seek to target page position
    long offset = pageNum * PAGE_SIZE;
    if (fseek(diskHandler, offset, SEEK_SET) != 0) {
        fclose(diskHandler);
        return RC_WRITE_FAILED;
    }
    
    // Write page content
    size_t bytesWritten = fwrite(memPage, 1, PAGE_SIZE, diskHandler);
    
    if (bytesWritten != PAGE_SIZE) {
        fclose(diskHandler);
        return RC_WRITE_FAILED;
    }
    
    fileHandle->curPagePos = offset + PAGE_SIZE;
    
    fclose(diskHandler);
    return RC_OK;
}

// Writes the current block's data back to the page file.
RC writeCurrentBlock(SM_FileHandle *fileHandle, SM_PageHandle memPage) {
    if (!fileHandle || !memPage) {
        return RC_FILE_HANDLE_NOT_INIT;
    }
    
    int currentPage = fileHandle->curPagePos / PAGE_SIZE;
    return writeBlock(currentPage - 1, fileHandle, memPage);
}

// Appends an empty block to the end of the page file.
RC appendEmptyBlock(SM_FileHandle *fileHandle) {
    if (!fileHandle) {
        return RC_FILE_HANDLE_NOT_INIT;
    }
    
    diskHandler = fopen(fileHandle->fileName, "ab+");
    if (!diskHandler) {
        return RC_FILE_NOT_FOUND;
    }
    
    // Create empty block
    char *emptyBlock = malloc(PAGE_SIZE);
    if (!emptyBlock) {
        fclose(diskHandler);
        return RC_WRITE_FAILED;
    }
    
    memset(emptyBlock, 0, PAGE_SIZE);
    
    // Write empty block
    size_t bytesWritten = fwrite(emptyBlock, 1, PAGE_SIZE, diskHandler);
    free(emptyBlock);
    
    if (bytesWritten != PAGE_SIZE) {
        fclose(diskHandler);
        return RC_WRITE_FAILED;
    }
    
    fileHandle->totalNumPages++;
    fileHandle->curPagePos = fileHandle->totalNumPages * PAGE_SIZE;
    
    fclose(diskHandler);
    return RC_OK;
}

// Ensures that the buffer pool has enough capacity for the specified number of pages.
RC ensureCapacity(int numberOfPages, SM_FileHandle *fileHandle) {
    if (!fileHandle) {
        return RC_FILE_HANDLE_NOT_INIT;
    }
    
    while (fileHandle->totalNumPages < numberOfPages) {
        RC result = appendEmptyBlock(fileHandle);
        if (result != RC_OK) {
            return result;
        }
    }
    
    return RC_OK;
}