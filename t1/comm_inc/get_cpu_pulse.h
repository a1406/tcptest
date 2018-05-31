#ifndef get_cpu_pulse_h_
#define get_cpu_pulse_h_

inline uint64_t get_rand_val() 
{ 	
	uint32_t _edx=0;
 	uint32_t _eax=0;
#if defined(__x86_64__)	
	__asm__ __volatile__(
		".byte 0x0f, 0x31"
		:"=a"(_eax),"=d"(_edx)
	); 
#elif defined(__i386__)	
	__asm__ __volatile__(
		".byte 0x0f, 0x31"
		:"=a"(_eax),"=d"(_edx)
	); 
#endif	
	return (((uint64_t)_edx)<< 32 ) | _eax; 
}

#endif//get_cpu_pulse_h_

