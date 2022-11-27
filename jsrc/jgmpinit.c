/* Copyright (c) 1990-2022, Jsoftware Inc.                                 */
/* Licensed use only.                                                      */
/*                                                                         */
/* j init libgmp support                                                   */

#define JGMPINIT    // tell jgmp.h to declare storage rather than externs
#ifdef _WIN32
 #include <windows.h>
#else
 #include <dlfcn.h>  // -ldl
#endif
#include "j.h"      // includes jgmp.h

///////////////////////////////////////////////////////////////
// Constants
// see jgmp.h for some notes on type X
//
#if C_LE
 #if SY_64
  #define Xrh 1,0,FHRHISGMP,0,0
 #else
  #define Xrh 1,0,FHRHISGMP
 #endif
#else
 #if SY_64
  #define Xrh 0,0,FHRHISGMP,0,1
 #else
  #define Xrh  FHRHISGMP,0,1
 #endif
#endif

#define XFIXED0(nam, typ,val) \
 struct AD __attribute__((aligned(CACHELINESIZE))) B##nam= \
 {AKXR(0),typ,0,typ,ACPERMANENT,1,Xrh,{(I)val}}; \
 X nam= (X)&B##nam

/* like struct AD but a data element */
struct BDV1 {I k;I f;I m;I t;I c;I n;
#if C_LE
 RANKT r;UC filler;US h;
#if SY_64
 US origin;S lock;
#endif
#else
 #if SY_64
  S lock;US origin;
 #endif
 US h;UC filler;RANKT r;
#endif
 I s[1];UI d;};
 
#define XFIXED1(nam, typ,sgn,val) \
 struct BDV1 __attribute__((aligned(CACHELINESIZE))) B##nam= \
 {XHSZ,typ,0,typ,ACPERMANENT,1,Xrh,sgn,(UI)val}; \
 X nam= (X)&B##nam

XFIXED1(X_1,LIT,-1,1);  // _1x (not an array)
XFIXED0(X0, LIT, 0);    //  0x (not an array)
XFIXED1(X1, LIT, 1,1);  //  1x (not an array)
XFIXED1(X2, LIT, 1,2);  //  2x (not an array

mpz_t mpX1= {1,1,(mp_limb_t*)&BX1.s[1]};

// one-off special case relying on X/A type pun, among other things
// XFIXED0(AX1,XNUM,(X)&BX1);// ''$1x

#undef Xrh
#undef XFIXED0
#undef XFIXED1

#define RATIO(n,d) {(X)&BX##n, (X)&BX##d}

Q Q__ = RATIO(_1, 0);  //  _1r0 (not an array)
Q Q_1 = RATIO(_1, 1);  //  _1r1 (not an array)
Q Q0  = RATIO( 0, 1);  //   0r1 (not an array)
Q Q1r2= RATIO( 1, 2);  //   1r2 (not an array)
Q Q1  = RATIO( 1, 1);  //   1r1 (not an array)
Q Q_  = RATIO( 1, 0);  //   1r0 (not an array)

#undef RATIO

////////////////////////////////////////////////////////////////
// memory management and glue for new libgmp numbers
// See jgmp.h for additional notes
//
// we use raw malloc/free here because libgmp
// does not know which thread we're running in. 
// and we finish initialization if/when this reaches J

// see: https://gmplib.org/manual/Custom-Allocation 
#ifndef IMPORTGMPLIB
static void (*jmp_set_memory_functions) (
 void *(*alloc_func_ptr) (size_t),
 void *(*realloc_func_ptr) (void *, size_t, size_t),
 void (*free_func_ptr) (void *, size_t)
);
#endif

long long gmpmallocs;

#if MEMAUDIT&0x40
#define GUARDSIZE  (CACHELINESIZE)
#define GUARDsSIZE (2*CACHELINESIZE)
C GUARDBLOCK[GUARDSIZE];
#else
#define GUARDSIZE  0
#define GUARDsSIZE 0
#endif

/*static*/void*jmalloc4gmp(size_t size){
 size_t avxsize= (size+7)&(-8); // libgmp's use of __strlen_avx2 segfaults when libgmp asks for size=2
 C*m= malloc(avxsize+XHSZ+GUARDsSIZE);
 if(!m)SEGFAULT;
 if(!m)R0; // assert(z);// FIXME (but can't without replacing libgmp)
#if MEMAUDIT&0x40
 memcpy(m, GUARDBLOCK, GUARDSIZE);
 memcpy(m+size+XHSZ+GUARDSIZE, GUARDBLOCK, GUARDSIZE);
#endif
 X z= (X)(m+GUARDSIZE); // J header starts here
 AK(z)= XHSZ;           // always rank 1
 AFLAG(z)= 0;           // should be a safe value
 AT(z)= LIT;            // matches significance of AN(z), simplifies 3!:1
 AC(z)= 1;              // always 1 ref in libgmp
 AN(z)= size;           // track amount of requested memory
 AR(z)= 1;              // always rank 1
 AFHRH(z)= FHRHISGMP;   // mark as a GMP psuedo-array
 /*
 fprintf(stderr,"jmalloc4gmp (%lli) z: %llx (size: %zx), ", ++gmpmallocs, (UI)z, size);
 fprintf(stderr,"AK(z): %llx (%lli), ", AK(z), AK(z));
 fprintf(stderr,"AFLAG(z): %llx (%lli), ", AFLAG(z), AFLAG(z));
 fprintf(stderr,"AT(z): %llx (%lli), ", AT(z), AT(z));
 fprintf(stderr,"AC(z): %llx (%lli), ", AC(z), AC(z));
 fprintf(stderr,"AN(z): %llx (%lli), ", AN(z), AN(z));
 fprintf(stderr,"AR(z): %hhx (%hhu), ", AR(z), AR(z));
 fprintf(stderr,"AFHRH(z): %hx (%hi)\n", AFHRH(z), AFHRH(z));
 */
#if MEMAUDIT&8
 static I lfsr= 1;
 DO(size/SZI, lfsr= (lfsr<<1) ^ (lfsr<0 ?0x1b :0); if (i!=2&&i!=6)((I*)(XHSZ+(C*)z))[i]= lfsr;);
#endif
 R CAV1(z);
}

static void*jrealloc4gmp(void*ptr, size_t old, size_t new){
 size_t avxnew= (new+7)&(-8); // libgmp's use of __strlen_avx2 segfaults when libgmp asks for size=2
 X x= (X)(((C*)ptr)-XHSZ);
 C*m= (C*)x-GUARDSIZE;
#if MEMAUDIT&0x40
 if (memcmp(m, GUARDBLOCK, GUARDSIZE)) SEGFAULT;
 if (memcmp(m+old+XHSZ+GUARDSIZE, GUARDBLOCK, GUARDSIZE)) SEGFAULT;
#endif
 // assert(1==AC(x));   // must not have been exposed to J
 // assert(FHRHISGMP==AFHRH(x))
 m= realloc(m, avxnew+XHSZ+GUARDsSIZE);
 if(!m)SEGFAULT;
 if(!m)R0; // assert(m);// FIXME (but can't without replacing libgmp)
#if MEMAUDIT&0x40
 memcpy(m+new+XHSZ+GUARDSIZE, GUARDBLOCK, GUARDSIZE);
#endif
 X z= (X)(m+GUARDSIZE); // J header starts here
 AN(z)= new;            // track allocated memory
 /*
 fprintf(stderr,"jrealloc4gmp z: %llx (size: %zx -> %zx), ", (UI)z, old, new);
 fprintf(stderr,"AK(z): %llx (%lli), ", AK(z), AK(z));
 fprintf(stderr,"AFLAG(z): %llx (%lli), ", AFLAG(z), AFLAG(z));
 fprintf(stderr,"AT(z): %llx (%lli), ", AT(z), AT(z));
 fprintf(stderr,"AC(z): %llx (%lli), ", AC(z), AC(z));
 fprintf(stderr,"AN(z): %llx (%lli), ", AN(z), AN(z));
 fprintf(stderr,"AR(z): %hhx (%hhu), ", AR(z), AR(z));
 fprintf(stderr,"AFHRH(z): %hx (%hi)\n", AFHRH(z), AFHRH(z));
 */
#if MEMAUDIT&8
 static I lfsr= 1;
 if (new > old) {
  DO((new-old)/SZI, lfsr= (lfsr<<1)^(lfsr<0 ?0x1b :0); if (i!=2&&i!=6)((I*)(old+XHSZ+(C*)z))[i]= lfsr;);
 }
#endif
 R CAV1(z);
}

/*static*/ void jfree4gmp(void*ptr, size_t n){
 // assert(1==AC(x));
 // assert(FHRHISGMP=AT(x))
 X x= UNvoidAV1(ptr);
 if (ACPERMANENT&AC(x)) SEGFAULT;
 C* m= (C*)x-GUARDSIZE;
#if MEMAUDIT&0x40
 if (memcmp(m, GUARDBLOCK, GUARDSIZE)) SEGFAULT;
 if (memcmp(m+n+XHSZ+GUARDSIZE, GUARDBLOCK, GUARDSIZE)) SEGFAULT;
#endif
 /*
 fprintf(stderr,"\tjfree4gmp (%lli) x: %llx, ", gmpmallocs--, (UI)x);
 fprintf(stderr,"AK(x): %llx (%lli), ", AK(x), AK(x));
 fprintf(stderr,"AFLAG(x): %llx (%lli), ", AFLAG(x), AFLAG(x));
 fprintf(stderr,"AT(x): %llx (%lli), ", AT(x), AT(x));
 fprintf(stderr,"AC(x): %llx (%lli), ", AC(x), AC(x));
 fprintf(stderr,"AN(x): %llx (%lli), ", AN(x), AN(x));
 fprintf(stderr,"AR(x): %hhx (%hhu), ", AR(x), AR(x));
 fprintf(stderr,"AFHRH(x): %hx (%hi)\n", AFHRH(x), AFHRH(x));
 */
#if MEMAUDIT&0x4
 AK(x)= AFLAG(x)= AM(x)= AT(x)= AC(x)= AN(x)= 0xdeadbeef;
#endif
 free(m);
}

// dehydrate fresh (mpz_t) as J (X)
X jtXmpzcommon(J jt, mpz_t mpz) {
 if(unlikely(!mpz->_mp_size)) R X0; // mpz is new but may not have memory
 X x=UNvoidAV1(mpz->_mp_d);         // we gave libgmp AV1(x) for this block of memory
#if MEMAUDIT&1
 if(FHRHISGMP!=AFHRH(x)) SEGFAULT;
 if(1!=AC(x)) SEGFAULT;             // should be pristine from libgmp
 if(ACISPERM(AC(x))) SEGFAULT;      // should never happen
#endif
 XSGN(x)= mpz->_mp_size;            // size of number and its sign
 I n= AN(x);                        // length of memory allocated for number
 I sz= XHSZ+n;                      // bytes allocated
#if SY_64
 x->origin= THREADID(jt);           // track thread which created this array
#endif
 jt->bytes+= sz;                    // summarize the size of the new space
 jt->malloctotal+= sz;              // ditto
 jt->mfreegenallo+= sz;             // ditto
 R x;                               // CAUTION: J must issue death warrant 
}

// dehydrate fresh (mpq_t) as J (Q)
Q jtQmpq(J jt, mpq_t mpq) {
 Q q; A*pushp;
 q.n= Xmpzcommon(&mpq->_mp_num);   // dehydrate numerator
 q.d= Xmpzcommon(&mpq->_mp_den);   // dehydrate denominator
 if (tpushnoret(q.n)) {            // numerator is transient? (and, thus, non-zero)
  if (unlikely(!pushp)) {          // numerator death warrant valid?
   frgmp(q.n); frgmp(q.d); R Q0;   // discard transients and fail
  } else {                         // numerator handled
   if (tpushnoret(q.d)) {          // denominator is positive?
    if (unlikely(!pushp)) {        // denominator death warrant valid?
     *AZAPLOC(q.n)= 0;             // no: retract numerator warrant
     frgmp(q.n); frgmp(q.d); R Q0; // clean up and fail
    }                              // both warrants posted
   } else {                        // denominator is 0?
    R (0<XSGN(q.n)) ?Q_ :Q__;      // libgmp doesn't do infinity but J sometimes does
   }
  }
 } else {                          // numerator is 0
  frgmp(q.d);                      // denominator is something else? result is 0
  R Q0;
 }
 R q;                              // death warrant has been issued
}

///////////////////////////////////////////////////////////////
// link with libgmp
#ifdef _WIN32
 #define LIBEXT ".dll"
#define LIBGMPNAME "mpir" LIBEXT
#else
 #ifdef __APPLE__
  #define LIBEXT ".dylib"
 #else
  #ifdef ANDROID
   #define LIBEXT ".so"
  #else
   #define LIBEXT ".so.10"    /* symlink of .so only available when devel package installed */
  #endif
 #endif
#define LIBGMPNAME "libgmp" LIBEXT
#endif

static void*libgmp;
#ifdef IMPORTGMPLIB
B nogmp(){R 0;}
#else
B nogmp(){R!libgmp;}
#endif

#ifdef _WIN32
void dldiag(){}
#define jgmpfn(fn) j##fn= GetProcAddress(libgmp,"__g"#fn); if(!(j##fn)){fprintf(stderr,"%s\n","error loading "#fn);};
#else
void dldiag(){char*s=dlerror();if(s)fprintf(stderr,"%s\n",s);}
#define jgmpfn(fn) j##fn= dlsym(libgmp,"__g"#fn); dldiag();
#endif

// referenced gmp routines are declared twice:
// once here for dynamic linking and 
// also in jgmp.h for global name declaration
void jgmpinit() {
 if (SZI != sizeof (mp_limb_t)) SEGFAULT; // verify a fundamental assumption
#if MEMAUDIT&0x40
 ((UIL*)GUARDBLOCK)[0]= (UIL)0x1abe11ed1abe11edLL;
 memcpy(GUARDBLOCK+sizeof (UIL), GUARDBLOCK, GUARDSIZE-sizeof (UIL));
#endif
#ifndef IMPORTGMPLIB
 if (jmpz_init) return; // link with libgmp only once
#endif
#ifndef IMPORTGMPLIB
#ifdef _WIN32
 libgmp= LoadLibraryA(LIBGMPNAME);
 if (!libgmp) {fprintf(stderr,"%s\n","error loading gmp library");R;}
#else
 libgmp= dlopen(LIBGMPNAME, RTLD_LAZY); 
 if (!libgmp) {dldiag();R;}
#endif
#endif
#ifdef IMPORTGMPLIB
 mp_set_memory_functions(jmalloc4gmp, jrealloc4gmp, jfree4gmp);
#else
 jgmpfn(mp_set_memory_functions);
 jmp_set_memory_functions(jmalloc4gmp, jrealloc4gmp, jfree4gmp);
 jgmpfn(mpq_add);          // https://gmplib.org/manual/Rational-Arithmetic
 // DO NOT USE //          jgmpfn(mpq_canonicalize); // https://gmplib.org/manual/Rational-Number-Functions
 jgmpfn(mpq_clear);        // https://gmplib.org/manual/Initializing-Rationals
 jgmpfn(mpq_cmp);          // https://gmplib.org/manual/Comparing-Rationals
#ifndef RASPI
 jgmpfn(mpq_cmp_z);        // https://gmplib.org/manual/Comparing-Rationals
#endif
 jgmpfn(mpq_div);          // https://gmplib.org/manual/Rational-Arithmetic
 jgmpfn(mpq_get_d);        // https://gmplib.org/manual/Rational-Conversions
 jgmpfn(mpq_get_str);      // https://gmplib.org/manual/Rational-Conversions
 jgmpfn(mpq_init);         // https://gmplib.org/manual/Initializing-Rationals
 jgmpfn(mpq_mul);          // https://gmplib.org/manual/Rational-Arithmetic
 jgmpfn(mpq_out_str);      // https://gmplib.org/manual/I_002fO-of-Rationals
 jgmpfn(mpq_set);          // https://gmplib.org/manual/Initializing-Rationals
 jgmpfn(mpq_sub);          // https://gmplib.org/manual/Rational-Arithmetic
 jgmpfn(mpz_abs);          // https://gmplib.org/manual/Integer-Arithmetic
 jgmpfn(mpz_add);          // https://gmplib.org/manual/Integer-Arithmetic
 jgmpfn(mpz_add_ui);       // https://gmplib.org/manual/Integer-Arithmetic
 jgmpfn(mpz_bin_ui);       // https://gmplib.org/manual/Number-Theoretic-Functions
 jgmpfn(mpz_cdiv_q);       // https://gmplib.org/manual/Integer-Division
 jgmpfn(mpz_fdiv_q);       // https://gmplib.org/manual/Integer-Division
 jgmpfn(mpz_clear);        // https://gmplib.org/manual/Initializing-Integers
 jgmpfn(mpz_cmp);          // https://gmplib.org/manual/Integer-Comparisons
 jgmpfn(mpz_cmpabs_ui);    // https://gmplib.org/manual/Integer-Comparisons
 jgmpfn(mpz_cmp_si);       // https://gmplib.org/manual/Integer-Comparisons
 jgmpfn(mpz_fac_ui);       // https://gmplib.org/manual/Number-Theoretic-Functions
 jgmpfn(mpz_fdiv_q);       // https://gmplib.org/manual/Integer-Division
 jgmpfn(mpz_fdiv_qr);      // https://gmplib.org/manual/Integer-Division
 jgmpfn(mpz_fdiv_qr_ui);   // https://gmplib.org/manual/Integer-Division
 jgmpfn(mpz_fdiv_r);       // https://gmplib.org/manual/Integer-Division
 jgmpfn(mpz_gcd);          // https://gmplib.org/manual/Number-Theoretic-Functions
 jgmpfn(mpz_get_d);        // https://gmplib.org/manual/Converting-Integers
 jgmpfn(mpz_get_d_2exp);   // https://gmplib.org/manual/Converting-Integers
 jgmpfn(mpz_get_str);      // https://gmplib.org/manual/Converting-Integers
 jgmpfn(mpz_get_si);       // https://gmplib.org/manual/Converting-Integers
 jgmpfn(mpz_init);         // https://gmplib.org/manual/Initializing-Integers
 jgmpfn(mpz_init_set_d);   // https://gmplib.org/manual/Simultaneous-Integer-Init-_0026-Assign
 jgmpfn(mpz_init_set_str); // https://gmplib.org/manual/Simultaneous-Integer-Init-_0026-Assign
 jgmpfn(mpz_init_set_si);  // https://gmplib.org/manual/Simultaneous-Integer-Init-_0026-Assign
 jgmpfn(mpz_lcm);          // https://gmplib.org/manual/Number-Theoretic-Functions
 jgmpfn(mpz_mul);          // https://gmplib.org/manual/Integer-Arithmetic
 jgmpfn(mpz_neg);          // https://gmplib.org/manual/Integer-Arithmetic
 jgmpfn(mpz_out_str);      // https://gmplib.org/manual/I_002fO-of-Integers
 jgmpfn(mpz_powm);         // https://gmplib.org/manual/Integer-Exponentiation
 jgmpfn(mpz_pow_ui);       // https://gmplib.org/manual/Integer-Exponentiation
 jgmpfn(mpz_probab_prime_p);//https://gmplib.org/manual/Number-Theoretic-Functions
 jgmpfn(mpz_ui_pow_ui);    // https://gmplib.org/manual/Integer-Exponentiation
 jgmpfn(mpz_root);         // https://gmplib.org/manual/Integer-Roots
 jgmpfn(mpz_set);          // https://gmplib.org/manual/Assigning-Integers
 jgmpfn(mpz_set_si);       // https://gmplib.org/manual/Assigning-Integers
 jgmpfn(mpz_sizeinbase);   // https://gmplib.org/manual/Miscellaneous-Integer-Functions
 jgmpfn(mpz_sub);          // https://gmplib.org/manual/Integer-Arithmetic
#endif
}
