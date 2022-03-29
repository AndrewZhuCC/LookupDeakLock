//
//  TWMDeadLockCheckerTypes.h
//  LookupDeakLock
//
//  Created by 朱安智 on 2022/3/29.
//

#ifndef TWMDeadLockCheckerTypes_h
#define TWMDeadLockCheckerTypes_h

#import <os/lock.h>

#pragma mark - pthread_mutex

typedef os_unfair_lock _pthread_lock;

struct pthread_mutex_options_s {
    uint32_t
        protocol:2,
        type:2,
        pshared:2,
        policy:3,
        hold:2,
        misalign:1,
        notify:1,
        mutex:1,
        ulock:1,
        unused:1,
        lock_count:16;
};

typedef struct _pthread_mutex_ulock_s {
    uint32_t uval;
} *_pthread_mutex_ulock_t;

struct pthread_mutex_s {
    long sig;
    _pthread_lock lock;
    union {
        uint32_t value;
        struct pthread_mutex_options_s options;
    } mtxopts;
    int16_t prioceiling;
    int16_t priority;
#if defined(__LP64__)
    uint32_t _pad;
#endif
    union {
        struct {
            uint32_t m_tid[2]; // thread id of thread that has mutex locked
            uint32_t m_seq[2]; // mutex sequence id
            uint32_t m_mis[2]; // for misaligned locks m_tid/m_seq will span into here
        } psynch;
        struct _pthread_mutex_ulock_s ulock;
    };
#if defined(__LP64__)
    uint32_t _reserved[4];
#else
    uint32_t _reserved[1];
#endif
};

struct pthread_rwlock_s {
  long sig;
  _pthread_lock lock;
  uint32_t
    unused:29,
    misalign:1,
    pshared:2;
  uint32_t rw_flags;
#if defined(__LP64__)
  uint32_t _pad;
#endif
  uint32_t rw_tid[2]; // thread id of thread that has exclusive (write) lock
  uint32_t rw_seq[4]; // rw sequence id (at 128-bit aligned boundary)
  uint32_t rw_mis[4]; // for misaligned locks rw_seq will span into here
#if defined(__LP64__)
  uint32_t _reserved[34];
#else
  uint32_t _reserved[18];
#endif
};

#pragma mark - GCD

#define DISPATCH_CONCAT(x,y) DISPATCH_CONCAT1(x,y)
#define DISPATCH_CONCAT1(x,y) x ## y

#define DISPATCH_COUNT_ARGS(...) DISPATCH_COUNT_ARGS1(, ## __VA_ARGS__, \
    _8, _7, _6, _5, _4, _3, _2, _1, _0)
#define DISPATCH_COUNT_ARGS1(z, a, b, c, d, e, f, g, h, cnt, ...) cnt

#if BYTE_ORDER == LITTLE_ENDIAN
#define DISPATCH_STRUCT_LE_2(a, b)        struct { a; b; }
#define DISPATCH_STRUCT_LE_3(a, b, c)     struct { a; b; c; }
#define DISPATCH_STRUCT_LE_4(a, b, c, d)  struct { a; b; c; d; }
#else
#define DISPATCH_STRUCT_LE_2(a, b)        struct { b; a; }
#define DISPATCH_STRUCT_LE_3(a, b, c)     struct { c; b; a; }
#define DISPATCH_STRUCT_LE_4(a, b, c, d)  struct { d; c; b; a; }
#endif
#if __has_feature(c_startic_assert)
#define DISPATCH_UNION_ASSERT(alias, st) \
    _Static_assert(sizeof(struct { alias; }) == sizeof(st), "bogus union");
#else
#define DISPATCH_UNION_ASSERT(alias, st)
#endif
#define DISPATCH_UNION_LE(alias, ...) \
    DISPATCH_UNION_ASSERT(alias, DISPATCH_CONCAT(DISPATCH_STRUCT_LE, \
        DISPATCH_COUNT_ARGS(__VA_ARGS__))(__VA_ARGS__)) \
    union { alias; DISPATCH_CONCAT(DISPATCH_STRUCT_LE, \
        DISPATCH_COUNT_ARGS(__VA_ARGS__))(__VA_ARGS__); }

#define DISPATCH_ATOMIC64_ALIGN  __attribute__((aligned(8)))

#define _OS_OBJECT_HEADER(isa, ref_cnt, xref_cnt) \
        isa; /* must be pointer-sized and use __ptrauth_objc_isa_pointer */ \
        int volatile ref_cnt; \
        int volatile xref_cnt

#define _OS_OBJECT_CLASS_HEADER() \
    void *_os_obj_objc_class_t[5]

#define OS_OBJECT_STRUCT_HEADER(x) \
  _OS_OBJECT_HEADER(\
  const struct x##_vtable_s *__ptrauth_objc_isa_pointer do_vtable, \
  do_ref_cnt, \
  do_xref_cnt)

#define _DISPATCH_OBJECT_HEADER(x) \
  struct _os_object_s _as_os_obj[0]; \
  OS_OBJECT_STRUCT_HEADER(dispatch_##x); \
  struct dispatch_##x##_s *volatile do_next; \
  struct dispatch_queue_s *do_targetq; \
  void *do_ctxt; \
  union { \
    dispatch_function_t do_finalizer; \
    void *do_introspection_ctxt; \
  }

#define DISPATCH_OBJECT_HEADER(x) \
  struct dispatch_object_s _as_do[0]; \
  _DISPATCH_OBJECT_HEADER(x)

#define _DISPATCH_QUEUE_CLASS_HEADER(x, __pointer_sized_field__) \
  DISPATCH_OBJECT_HEADER(x); \
  __pointer_sized_field__; \
  DISPATCH_UNION_LE(uint64_t volatile dq_state, \
      dispatch_lock dq_state_lock, \
      uint32_t dq_state_bits \
  )

#define DISPATCH_QUEUE_CLASS_HEADER(x, __pointer_sized_field__) \
  _DISPATCH_QUEUE_CLASS_HEADER(x, __pointer_sized_field__)

typedef uint32_t dispatch_lock;

typedef struct _os_object_vtable_s {
  _OS_OBJECT_CLASS_HEADER();
} _os_object_vtable_s;

typedef struct _os_object_s {
  _OS_OBJECT_HEADER(
  const _os_object_vtable_s *__ptrauth_objc_isa_pointer os_obj_isa,
  os_obj_ref_cnt,
  os_obj_xref_cnt);
} _os_object_s;

struct dispatch_object_s {
  _DISPATCH_OBJECT_HEADER(object);
};

struct dispatch_queue_s {
  DISPATCH_QUEUE_CLASS_HEADER(queue, void *__dq_opaque1);
  /* 32bit hole on LP64 */
} DISPATCH_ATOMIC64_ALIGN;

#endif /* TWMDeadLockCheckerTypes_h */
