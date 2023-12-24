/*
 * arch.h -- CPU archeticture and features
 */

#ifndef _ARCH_H
#define _ARCH_H

/*
 * list of CPU architectures
 */
enum arch {
        ARCH_UNKNOWN,
        ARCH_X86_32,
        ARCH_X86_64,
        ARCH_ARM_32,
        ARCH_ARM_64,
        ARCH_MIPS,
        ARCH_PPC,
        ARCH_PPC_64,
        ARCH_SPARC
};

/*
 * external functions
 */
enum arch arch_type(void);
char * arch_name(enum arch);
int arch_arm_number(void);

#endif
