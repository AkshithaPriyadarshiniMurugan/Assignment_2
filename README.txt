📜 EXECUTING THE SCRIPT

🚀 Follow these steps to compile and run the project:

1️⃣ Navigate to the Project Root Directory
   - Open a terminal and navigate to the assign2 directory.
   - Run `ls` to confirm you are in the correct directory.

2️⃣ Clean Previous Builds
   - Execute `make clean` to remove any previously compiled .o files.

3️⃣ Compile the Project
   - Run `make` to compile all project files, including `test_assign2_1.c`.

4️⃣ Run the First Test Case
   - Execute `make run_test1 > t1.log` to run `test_assign2_1.c` and store the output in `t1.log`.

5️⃣ Compile Custom Test File
   - Run `make test2` to compile `test_assign2_2.c`.

6️⃣ Run the Second Test Case
   - Execute `make run_test2 > t2.log` to run `test_assign2_2.c` and store the output in `t2.log`.

🛠 BUFFER POOL FUNCTIONS

The buffer pool functions manage pages stored in memory while maintaining synchronization with the disk-based page file.

🔹 **initBufferPool(...)**
   - Initializes a buffer pool in memory.
   - **Parameters:**
     - `numPages`: Number of page frames in the buffer.
     - `pageFileName`: Name of the cached page file.
     - `strategy`: Page replacement strategy (FIFO, LRU, LFU, CLOCK).
     - `stratData`: Additional parameters for replacement strategy.

🔹 **shutdownBufferPool(...)**
   - Destroys the buffer pool, freeing all allocated resources.
   - Calls `forceFlushPool(...)` before destruction.
   - Throws `RC_PINNED_PAGES_IN_BUFFER` if any pages are still in use.

🔹 **forceFlushPool(...)**
   - Writes all dirty pages (dirtyBit = 1) back to disk.
   - Ensures that pages not in use (fixCount = 0) are written to the page file.

📄 PAGE MANAGEMENT FUNCTIONS

These functions handle operations like pinning and unpinning pages, marking pages as dirty, and forcing writes to disk.

🔹 **pinPage(...)**
   - Pins a page from disk into the buffer pool.
   - Uses replacement strategies if space is unavailable.
   - Supports FIFO, LRU, LFU, and CLOCK strategies.

🔹 **unpinPage(...)**
   - Unpins a specific page by decrementing its fixCount.

🔹 **makeDirty(...)**
   - Marks a page frame as modified (dirtyBit = 1).

🔹 **forcePage(...)**
   - Writes the specified page back to disk and resets its dirtyBit.

📊 STATISTICS FUNCTIONS

These functions provide insights into buffer pool usage and performance metrics.

🔹 **getFrameContents(...)**
   - Returns an array of page numbers currently stored in the buffer.

🔹 **getDirtyFlags(...)**
   - Returns an array indicating which pages are dirty.

🔹 **getFixCounts(...)**
   - Returns an array of fixCount values for all pages in the buffer.

🔹 **getNumReadIO(...)**
   - Returns the total number of pages read from disk.

🔹 **getNumWriteIO(...)**
   - Returns the total number of pages written to disk.

🔄 PAGE REPLACEMENT ALGORITHMS

These algorithms determine which page is replaced when the buffer is full.

🔹 **FIFO (First-In-First-Out)**
   - Replaces the oldest page in the buffer.

🔹 **LFU (Least Frequently Used)**
   - Replaces the page accessed the least times.

🔹 **LRU (Least Recently Used)**
   - Replaces the page that hasn't been used for the longest time.

🔹 **CLOCK Algorithm**
   - Uses a circular queue to track page access and replace pages with hitNum = 0.

🧪 TEST CASES

📌 Additional test cases are implemented in `test_assign2_2.c`. These test cases evaluate:
   - LFU and CLOCK page replacement strategies.
   - Various buffer management functionalities.
