#include <pthread.h>
#include <sys/mman.h>  // necessary for mlockall
#include <sys/time.h>
#include <cstring>
#include <stdexcept>
#include <string>
#include "p3_util.h"

// Changed to global variable for runtime control
bool set_cpu = false;

void LockMemory() {
    int ret = mlockall(MCL_CURRENT | MCL_FUTURE);
    if (ret) {
        throw std::runtime_error{std::string("mlockall failed: ") + std::strerror(errno)};
    }
}

void setCPU(int cpu_id = 1) {
    if (!set_cpu) return;
    cpu_set_t cpu_set;
    CPU_ZERO(&cpu_set);
    CPU_SET(cpu_id, &cpu_set);
    
    int result = pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpu_set);
    if (result != 0) {
        printf("Error setting CPU affinity\n");
    }
}

class ThreadRT {
    int priority_;
    int policy_;
    pthread_t thread_;
    struct timespec start_time_, end_time_;

    static void* RunThreadRT(void* data) {
        if (set_cpu) {
            setCPU(1);  // All RT threads run on CPU 1 when set_cpu is true
        }
        printf("[RT thread #%lu] running on CPU #%d\n", pthread_self(), sched_getcpu());
        ThreadRT* thread = static_cast<ThreadRT*>(data);
        thread->Run();
        return NULL;
    }

public:
    int app_id_;
    ThreadRT(int app_id, int priority, int policy) : app_id_(app_id), priority_(priority), policy_(policy) {}

    void Start() {
        pthread_attr_t attr;
        pthread_attr_init(&attr);
        pthread_attr_setschedpolicy(&attr, policy_);
        
        struct sched_param param;
        param.sched_priority = priority_;
        pthread_attr_setschedparam(&attr, &param);
        pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
        
        clock_gettime(CLOCK_MONOTONIC, &start_time_);
        pthread_create(&thread_, &attr, &ThreadRT::RunThreadRT, this);
        pthread_attr_destroy(&attr);
    }

    void Join() {
        pthread_join(thread_, NULL);
        clock_gettime(CLOCK_MONOTONIC, &end_time_);
        double elapsed = (end_time_.tv_sec - start_time_.tv_sec) +
                        (end_time_.tv_nsec - start_time_.tv_nsec) / 1e9;
        printf("[RT thread #%lu] App #%d Ends (Elapsed: %.3f seconds)\n", 
               thread_, app_id_, elapsed);
    }

    virtual void Run() = 0;
};

class ThreadNRT {
    pthread_t thread_;
    struct timespec start_time_, end_time_;

    static void* RunThreadNRT(void* data) {
        if (set_cpu) {
            setCPU(1);  // All threads run on CPU 1 when set_cpu is true
        }
        printf("[NRT thread #%lu] running on CPU #%d\n", pthread_self(), sched_getcpu());
        ThreadNRT* thread = static_cast<ThreadNRT*>(data);
        thread->Run();
        return NULL;
    }

public:
    int app_id_;
    ThreadNRT(int app_id) : app_id_(app_id) {}

    void Start() {
        clock_gettime(CLOCK_MONOTONIC, &start_time_);
        pthread_create(&thread_, NULL, &ThreadNRT::RunThreadNRT, this);
    }

    void Join() {
        pthread_join(thread_, NULL);
        clock_gettime(CLOCK_MONOTONIC, &end_time_);
        double elapsed = (end_time_.tv_sec - start_time_.tv_sec) +
                        (end_time_.tv_nsec - start_time_.tv_nsec) / 1e9;
        printf("[NRT thread #%lu] App #%d Ends (Elapsed: %.3f seconds)\n", 
               thread_, app_id_, elapsed);
    }

    virtual void Run() = 0;
};

class AppTypeX : public ThreadRT {
public:
    AppTypeX(int app_id, int priority, int policy) : ThreadRT(app_id, priority, policy) {}
    
    void Run() {
        struct timespec workload_start, workload_end;
        clock_gettime(CLOCK_MONOTONIC, &workload_start);
        
        printf("Running App #%d...\n", app_id_);
        // Use CannyP3 for first RT app, BusyCal for others
        if (app_id_ == 1) {
            CannyP3();
        } else {
            BusyCal();
        }
        
        clock_gettime(CLOCK_MONOTONIC, &workload_end);
        double workload_time = (workload_end.tv_sec - workload_start.tv_sec) +
                              (workload_end.tv_nsec - workload_start.tv_nsec) / 1e9;
        printf("[RT thread #%lu] App #%d Workload time: %.3f seconds\n", 
               pthread_self(), app_id_, workload_time);
    }
};

class AppTypeY : public ThreadNRT {
public:
    AppTypeY(int app_id) : ThreadNRT(app_id) {}
    
    void Run() {
        struct timespec workload_start, workload_end;
        clock_gettime(CLOCK_MONOTONIC, &workload_start);
        
        printf("Running App #%d...\n", app_id_);
        BusyCal();
        
        clock_gettime(CLOCK_MONOTONIC, &workload_end);
        double workload_time = (workload_end.tv_sec - workload_start.tv_sec) +
                              (workload_end.tv_nsec - workload_start.tv_nsec) / 1e9;
        printf("[NRT thread #%lu] App #%d Workload time: %.3f seconds\n", 
               pthread_self(), app_id_, workload_time);
    }
};

int main(int argc, char** argv) {
    int exp_id = 0;
    if (argc < 2) {
        fprintf(stderr, "WARNING: default exp_id=0\n");
    } else {
        exp_id = atoi(argv[1]);
    }
    
    LockMemory();
    
    // Set CPU affinity based on experiment
    set_cpu = (exp_id == 1 || exp_id == 3 || exp_id == 4);
    
    switch(exp_id) {
        case 0: {  // Default case
            AppTypeX app1(1, 80, SCHED_FIFO);
            AppTypeY app2(2);
            app1.Start();
            app2.Start();
            app1.Join();
            app2.Join();
            break;
        }
        case 1: {  // One CannyP3 RT + Two NRT on CPU 1
            AppTypeX app1(1, 80, SCHED_FIFO);  // CannyP3 RT
            AppTypeY app2(2);                   // NRT
            AppTypeY app3(3);                   // NRT
            app1.Start();
            app2.Start();
            app3.Start();
            app1.Join();
            app2.Join();
            app3.Join();
            break;
        }
        case 2: {  // Same as case 1 but free CPU
            set_cpu = false;
            AppTypeX app1(1, 80, SCHED_FIFO);
            AppTypeY app2(2);
            AppTypeY app3(3);
            app1.Start();
            app2.Start();
            app3.Start();
            app1.Join();
            app2.Join();
            app3.Join();
            break;
        }
        case 3: {  // Two RT (SCHED_FIFO) + one NRT on CPU 1 
            AppTypeX app1(1, 80, SCHED_FIFO);
            AppTypeX app2(2, 80, SCHED_FIFO);
            AppTypeY app3(3);
            app1.Start();
            app2.Start();
            app3.Start();
            app1.Join();
            app2.Join();
            app3.Join();
            break;
        }
        case 4: {  // Two RT (SCHED_RR) + one NRT on CPU 1
            AppTypeX app1(1, 80, SCHED_RR);
            AppTypeX app2(2, 80, SCHED_RR);
            AppTypeY app3(3);
            app1.Start();
            app2.Start();
            app3.Start(); 
            app1.Join();
            app2.Join();
            app3.Join();
            break;
        }
        case 5: { // Same as case 3 but free CPU
            set_cpu = false;
            AppTypeX app1(1, 80, SCHED_FIFO);
            AppTypeX app2(2, 80, SCHED_FIFO);  
            AppTypeY app3(3);
            app1.Start();
            app2.Start();
            app3.Start();
            app1.Join();
            app2.Join();
            app3.Join();
            break;
        }
        default:
            printf("ERROR: exp_id NOT FOUND\n");
    }
    
    return 0;
}
