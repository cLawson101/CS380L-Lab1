#include <iostream>

#include <algorithm>
#include <cstring>
#include <stdint.h>
#include <malloc.h>

// for mmap:
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>

// for core consolidation
#include <sched.h>

// for timing information
#include <sys/resource.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdio.h>

#include <chrono>

#define IO_SIZE 4096

using namespace std;

void shuffle(uint64_t *array, size_t n)
{
  if (n > 1) {
    size_t i;
    for (i = 0; i < n - 1; i++) {
      size_t j = i + rand() / (RAND_MAX / (n - i) + 1);
      uint64_t t = array[j];
      array[j] = array[i];
      array[i] = t;
    }
  }
}

void do_file_io(int fd, char *buf,
      uint64_t *offset_array, size_t n, int opt_read){
	int ret = 0;

	for(int i = 0; i < n; i++){
		ret = lseek(fd, offset_array[i], SEEK_SET);
		// printf("About to seek: %ld in file %d\n", offset_array[i], fd);
		if(ret == -1) {
			perror("lseek");
			exit(-1);
		}
		
		if(opt_read){
			// printf("About to read %d bytes from file %d\n", IO_SIZE, fd);
			// printf("Buff address: %lx\n", (uint64_t) buf);
			ret = read(fd, buf, IO_SIZE);
		}
		else{
			// printf("About to write %d bytes to file %d\n", IO_SIZE, fd);
			ret = write(fd, buf, IO_SIZE);
		}
		if(ret == -1){
			perror("read/write");
			exit(-1);
		}
	}
}


int main(){
        // Core consolidation
        cpu_set_t my_set;
        CPU_ZERO(&my_set);
        CPU_SET(1, &my_set);
        sched_setaffinity(0, sizeof(cpu_set_t), &my_set);

        int fd = open("file-1g", O_DIRECT | O_RDWR);
        if (fd == -1){
                perror("open error");
                exit(255);
        }

	// printf("Opened file: %d\n", fd);

	uint64_t offset_array [1 << 18];
	
	offset_array[0] = 0;

	for(int i = 1; i < 1<<18; ++i){
		offset_array[i] = offset_array[i-1] + IO_SIZE;
	}

        // Now going to shuffle the data

        shuffle(offset_array, 1<<18);

        using std::chrono::high_resolution_clock;
        using std::chrono::duration_cast;
        using std::chrono::duration;
        using std::chrono::milliseconds;
	
	char* buf = (char*) aligned_alloc(4096, 1 << 30);
	// printf("Buffer allocated: %lx\n", (uint64_t) buf);

	for(int i = 0; i < IO_SIZE - 1; ++i){
		buf[i] = 'a' + i % 26;
	}
	buf[IO_SIZE-1] = '\0';
        auto t1 = high_resolution_clock::now();

        do_file_io(fd, buf, offset_array, 1 << 18, 0);
	

        // msync(addr, 1<<30, MS_SYNC);

        auto t2 = high_resolution_clock::now();

        auto ms_int = duration_cast<milliseconds>(t2 - t1);
        duration<double, std::milli> ms_double = t2 - t1;

        // cout << "Completed Task" << endl;

        printf("Total time(s): %f\n", ms_double.count() / 1000);

        // munmap(addr, 1<<30);

        return 0;
}
