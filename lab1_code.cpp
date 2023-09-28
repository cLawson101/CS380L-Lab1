#include <iostream>

#include <algorithm>
#include <cstring>
#include <stdint.h>

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


int main(){
	// Core consolidation
	cpu_set_t my_set;
	CPU_ZERO(&my_set);
	CPU_SET(1, &my_set);
	sched_setaffinity(0, sizeof(cpu_set_t), &my_set);

	int fd = open("file-1g", O_RDWR);
	if (fd == -1){
		perror("open error");
		exit(255);
	}

	uint64_t* addr = (uint64_t*) mmap(NULL, 1 << 30, PROT_WRITE | PROT_READ, MAP_SHARED, fd, 0u);
	
	if(addr == (void*)-1){
		printf("Found an error\n");
		perror("mmap error");
		exit(255);
	}
	
	printf("Address returned by mmap: %p\n", addr);

	uint64_t all_pages [1<<18];

	uint64_t cur_address = reinterpret_cast<std::uintptr_t>(addr);
	
	printf("Starting Address: %10lx\n", cur_address);
	printf("Increment Size  : %10lx\n", (uint64_t)1 << 12);

	for(int i = 0; i < 1 << 18; ++i){
		all_pages[i] = cur_address + (uint64_t)(i * (1 << 12));
		/*
		if(i < 100){
			printf("all_pages[%3d]: 0x%10lx\n", i, all_pages[i]);
		}
		*/
		
	}

	// Now going to shuffle the data
	
	shuffle(all_pages, 1<<18);
	
	/*
	for(int i = 0; i < 100; ++i){
		printf("all_pages[%3d]: 0x%10lx\n", i, all_pages[i]);
	}
	*/
	
	cout << "About to write to page" << endl;

	using std::chrono::high_resolution_clock;
	using std::chrono::duration_cast;
	using std::chrono::duration;
	using std::chrono::milliseconds;

	auto t1 = high_resolution_clock::now();

	for(int i = 0; i < 1 << 18; ++i){
		uint64_t* cur_page = (uint64_t*)all_pages[i];
		// printf("About to write to page %10p\n", cur_page);
		*cur_page = 0x0abcdef;
	}

	msync(addr, 1<<30, MS_SYNC);

	auto t2 = high_resolution_clock::now();

	auto ms_int = duration_cast<milliseconds>(t2 - t1);
	duration<double, std::milli> ms_double = t2 - t1;

	cout << "Completed Task" << endl;
	
	printf("Total time(s): %f\n", ms_double.count() / 1000);

	munmap(addr, 1<<30);

	return 0;
}
