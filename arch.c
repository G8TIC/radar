/*
 * arch.c -- CPU/Machine architectures and features
 */
 
#include "arch.h"



/*
 * arch_type() - Get architecture we were compiled for ...
 */
enum arch arch_type(void)
{
        #if defined(__x86_64__) || defined(_M_X64)
                return ARCH_X86_64;
        #elif defined(i386) || defined(__i386__) || defined(__i386) || defined(_M_IX86)
                return ARCH_X86_32;
        #elif defined(__aarch64__) || defined(_M_ARM64) || defined(__ARM_64BIT_STATE)
                return ARCH_ARM_64;
        #elif defined(__ARM_32BIT_STATE)                
                return ARCH_ARM_32;
        #elif defined(mips) || defined(__mips__) || defined(__mips)
                return ARCH_MIPS;
        #elif defined(__powerpc) || defined(__powerpc__) || defined(__powerpc64__) || defined(__POWERPC__) || defined(__ppc__) || defined(__PPC__) || defined(_ARCH_PPC)
                return ARCH_PPC;
        #elif defined(__PPC64__) || defined(__ppc64__) || defined(_ARCH_PPC64)
                return ARCH_PPC_64;
        #elif defined(__sparc__) || defined(__sparc)
                return ARCH_SPARC;
        #else
                return ARCH_UNKNOWN;
        #endif
        
        return ARCH_UNKNOWN;
}


/*
 * arch_name() - Return printable string for architecture
 */
char * arch_name(enum arch arch)
{
        switch(arch) {
                case ARCH_X86_32: 	return "x86_32"; break;
                case ARCH_X86_64:	return "x86_64"; break;
                case ARCH_ARM_32:	return "ARM_32"; break;
                case ARCH_ARM_64:	return "ARM_64"; break;
                case ARCH_MIPS:		return "MIPS"; break;
                case ARCH_PPC:		return "PPC"; break;
                case ARCH_PPC_64:	return "PPC_64"; break;
                case ARCH_SPARC:	return "SPARC"; break;
                default:
                case ARCH_UNKNOWN:	return "Unknown"; break;
        }
}


/*
 * arch_arm_number() - return ARM architecture number
 */
int arch_arm_number(void)
{
        static int arm;
        
#if defined(__ARM_ARCH)
        arm = __ARM_ARCH;
#else
        arm = 0;
#endif
        
        return arm;
}



