//
//  ViewController.m
//  LookupDeakLock
//
//  Created by styf on 2021/12/1.
//

#import "ViewController.h"
#import "ThreadMonitor.h"
#import <os/lock.h>
#import <pthread/pthread.h>
#import <mach/mach_init.h>
@interface ViewController ()
/// <#name#>
@property (nonatomic, assign) BOOL checkThreadCancel;
/// <#name#>
@property (nonatomic, strong) NSThread *manyWorkThread;
/// <#name#>
@property (nonatomic, strong) NSThread *holdLockAThread;
/// <#name#>
@property (nonatomic, strong) NSLock *lockA;
/// <#name#>
@property (nonatomic, strong) NSThread *holdLockBThread;
/// <#name#>
//@property (nonatomic, strong) NSLock *lockB;
@property (nonatomic, assign) pthread_rwlock_t lockB;
/// <#name#>
@property (nonatomic, strong) NSThread *holdLockCThread;
/// <#name#>
//@property (nonatomic, strong) NSLock *lockC;

@property (nonatomic, assign) os_unfair_lock lockC;

/// <#name#>
@property (nonatomic, strong) NSThread *holdlockSemaphoreThread;
/// <#name#>
@property (nonatomic, strong) dispatch_semaphore_t lockSemaphore;

@property (nonatomic, strong) dispatch_queue_t current_queue;

@end

@implementation ViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    
    _lockA = [[NSLock alloc]init];
    _lockA.name = @"I am LockA";
    
//    _lockB = [[NSLock alloc]init];
//    _lockB.name = @"I am LockB";
  pthread_rwlock_init(&_lockB, NULL);
    
//  _lockC = OS_UNFAIR_LOCK_INIT;
//    _lockC.name = @"I am LockC";
    
    _lockSemaphore = dispatch_semaphore_create(1);
    
    __weak typeof(self) weakSelf = self;
    dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
        while (!weakSelf.checkThreadCancel) {
            [ThreadMonitor checkThreads];
            sleep(3);
        }
    });
    
//    [self testDoManyWork];
    
    [self testWaitNSLock];
    
  
  _current_queue = dispatch_queue_create("gcd dead lock", DISPATCH_QUEUE_SERIAL);
  
  dispatch_async(_current_queue, ^{
    NSLog(@"QThread want lockA");
    [self.lockA lock];
    NSLog(@"QThread hold lockA success");
  });
  
    
    
//    _holdlockSemaphoreThread = [[NSThread alloc]initWithTarget:self selector:@selector(holdlockSemaphore) object:nil];
//    [_holdlockSemaphoreThread setName:@"I hold lockSemaphore!"];
//    [_holdlockSemaphoreThread start];
}

- (void)touchesBegan:(NSSet<UITouch *> *)touches withEvent:(UIEvent *)event {
    
}

- (void)testWaitNSLock {
    _holdLockAThread = [[NSThread alloc]initWithTarget:self selector:@selector(holdLockA) object:nil];
    [_holdLockAThread setName:@"I hold LockA!"];
    [_holdLockAThread start];
    
    _holdLockBThread = [[NSThread alloc]initWithTarget:self selector:@selector(holdLockB) object:nil];
    [_holdLockBThread setName:@"I hold LockB!"];
    [_holdLockBThread start];
    
    _holdLockCThread = [[NSThread alloc]initWithTarget:self selector:@selector(holdLockC) object:nil];
    [_holdLockCThread setName:@"I hold LockC!"];
    [_holdLockCThread start];
}

- (void)testDoManyWork {
    _manyWorkThread = [[NSThread alloc]initWithTarget:self selector:@selector(doManyWork) object:nil];
    [_manyWorkThread setName:@"I am busy!"];
    [_manyWorkThread start];
}

- (void)holdlockSemaphore {
    dispatch_semaphore_wait(_lockSemaphore, DISPATCH_TIME_FOREVER);
    NSLog(@"SThread hold lockSemaphore success");
    sleep(2);
    
    NSLog(@"SThread want lockA");
    [_lockA lock];
    NSLog(@"SThread hold lockA success");
}

- (void)holdLockA {
    [_lockA lock];
    
    NSLog(@"AThread hold lockA success");
    sleep(2);
    
    NSLog(@"AThread want lockB");
  pthread_rwlock_rdlock(&_lockB);
//    [_lockB lock];
    NSLog(@"AThread hold lockB success");
}

- (void)holdLockB {
//    [_lockB lock];
  pthread_rwlock_wrlock(&_lockB);
    
    NSLog(@"BThread hold lockB success");
    sleep(2);
    
    NSLog(@"BThread want lockC");
//    [_lockC lock];
  os_unfair_lock_lock(&_lockC);
  NSLog(@"BThread hold lockC success");
}

- (void)holdLockC {
//  os_unfair_lock unfairlock = OS_UNFAIR_LOCK_INIT;
//  os_unfair_lock_lock(&unfairlock);
//  pthread_t pthread = pthread_from_mach_thread_np((mach_port_t)unfairlock._os_unfair_lock_opaque);
//  uint64_t threadid;
//  pthread_threadid_np(pthread, &threadid);
//
//  pthread_t pthreadself = pthread_self();
//  mach_port_t machthreadself = mach_thread_self();
//  unsigned int owner_id = *(unsigned int *)((char *)&unfairlock);
  _lockC = OS_UNFAIR_LOCK_INIT;
  os_unfair_lock_lock(&_lockC);
//    [_lockC lock];
    
    NSLog(@"CThread hold lockC success");
    sleep(2);
    
//    NSLog(@"CThread want lockA");
//    [_lockA lock];
//    NSLog(@"CThread hold lockA success");
  
  NSLog(@"CThread want Q");
  dispatch_sync(self.current_queue, ^{
    NSLog(@"CThread hold Q success");
  });
  
//    NSLog(@"CThread want lockS");
//  dispatch_semaphore_wait(_lockSemaphore, DISPATCH_TIME_FOREVER);
//  NSLog(@"CThread hold lockS success");
}

- (void)doManyWork {
    NSLog(@"doManyWork start");
    int a = 0;
    for (int i = 0; i < 10000000; i++) {
        a = i;
        for (int j = 0; j < 10000000; j++) {
            a--;
        }
    }
    NSLog(@"doManyWork end");
}
@end
