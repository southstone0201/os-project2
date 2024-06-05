#include <linux/syscalls.h>
#include <linux/kernel.h>
#include <linux/linkage.h>
#include <linux/sched.h>
#include <linux/slab.h>
#define IDLE 0
#define MAX 100

pid_t now = IDLE;
int now_job = -1;
int timeslice = 10;

typedef struct job_t {
    pid_t pid;
    int job;
    int priority; // 우선순위 필드 추가
} job_t;

typedef struct queue
{
	int first;
	int last;
	job_t jobs[MAX];
}queue_t;

queue_t wq = {0, 0};


job_t ku_pop(void){
	job_t tmp = wq.jobs[wq.first+1];
	wq.first = (wq.first + 1) % MAX;
	return tmp;
}

bool ku_is_new(pid_t newpid){
	int i = 0;
	for ( i = (wq.first+1); i != (wq.last+1)%MAX; i = (i + 1) % MAX){
		if ((wq.jobs[i]).pid == newpid) return false;
		}
	return true;
	
}

void ku_push(job_t n_job){
	wq.last = (wq.last + 1) % MAX;
	wq.jobs[wq.last] = n_job;
}

bool ku_is_empty(void){
	if (wq.first == wq.last) return true;
	return false;
}

void ku_cominsert(job_t new){
	int i = 0;
	job_t tmp;

	ku_push(new);

	for (i = (wq.first) % MAX; i != (wq.last) % MAX; i = (i + 1) % MAX){
		if ((wq.jobs[i]).job > (wq.jobs[(i + 1) % MAX]).job) {
			tmp = wq.jobs[i];
			wq.jobs[i] = wq.jobs[(i + 1) % MAX];
			wq.jobs[(i + 1) % MAX] = tmp;
		}
	}
}

void insert_job_by_priority(job_t new_job) {
    int i;
	int j;
    for (i = wq.first+1; i < wq.last+1; i++) {
        if (wq.jobs[i].priority > new_job.priority) {
            break;
        }
    }
    for (j = wq.last; j >= i; j--) {
        wq.jobs[j + 1] = wq.jobs[j];
    }
    wq.jobs[i] = new_job;
    wq.last++;
}



	

SYSCALL_DEFINE2(kucpu_fcfs, char*, name, int, job){
	job_t newjob = {current -> pid, job, 0};
	
	if (now == IDLE) now = newjob.pid;
	if (now == newjob.pid){
		if (job == 0){
			printk("Process Finished: %s\n", name);
			if (ku_is_empty()) now = IDLE;
			else now = (ku_pop()).pid;
		}
		else {
			printk("Working: %s\n", name);
		}
		return 0;
	}
	else{
		if(ku_is_new(newjob.pid)){
			ku_push(newjob);
		} 
		printk("Working Denied:%s\n", name);
	}
	return 1;
}

SYSCALL_DEFINE3(kucpu_priority, char*, name, int, job, int, priority) {
    job_t new_job = {current->pid, job, priority}; \
    if (now == IDLE) {
		now = new_job.pid;
    }else if (now == new_job.pid) {
		if(new_job.priority <= wq.jobs[wq.first+1].priority){
        if (job == 0) {
            printk("Process Finish: %s\n", name);
            if (ku_is_empty())
                now = IDLE;
            else
                now = ku_pop().pid;
				printk("poped\n %s", name);
        } else {
            printk("Working: %s\n", name);
        }
        return 0;
		}
		else{
			insert_job_by_priority(new_job);
			printk("inserted because of priority%s\n", name);
			now = ku_pop().pid;
			return 0;
		}
    }else {
		if (ku_is_new(new_job.pid)) {
            insert_job_by_priority(new_job); 
			printk("inserted because of new job%s\n", name);

		}
        
        printk("Working Denied: %s\n", name);
    }
    return 1;
}

SYSCALL_DEFINE2(kucpu_srtf, char*, name, int, job) { 
	job_t new_job = {current->pid, job,0};
	if (now == IDLE) {
		now = new_job.pid;
	}
	if (now == new_job.pid) {
		if (job == 0) {
			printk("Process Finish: %s\n", name);
			if (ku_is_empty())
				now = IDLE;
			else
				now = (ku_pop()).pid;
		} 
		else {
			printk("Working: %s\n", name);
		}
		now_job = job - 1; 
		return 0;
	}
	else {
		if (job < now_job) { 
			now = new_job.pid;
			printk("Working: %s\n", name);
			now_job = job - 1;
            return 0;
		}
		else if (ku_is_new(new_job.pid)) {
            ku_cominsert(new_job);
		    printk("Working Denied: %s\n", name);
        }
	    return 1;
    }
}

SYSCALL_DEFINE2(kucpu_rr, char*, name, int, job) { 
	job_t new_job = {current->pid, job, 0};
	if (now == IDLE) {
		now = new_job.pid;
	}
	if (now == new_job.pid) {
		if (job == 0) {
			printk("Process Finish: %s\n", name);
			timeslice = 10; 
			if (ku_is_empty())
				now = IDLE;
			else
				now = (ku_pop()).pid;
			return 0;
		}
		if (timeslice == 0) {
			printk("----> Turn Over: %s\n", name);
			ku_push(new_job);
			if (ku_is_empty()) now = IDLE;
			else now = (ku_pop()).pid;
			timeslice = 10; 
			return 1;
		}
		printk("Working: %s\n", name);
		timeslice--; 
		return 0;
	}
	else {
		if (ku_is_new(new_job.pid)) ku_push(new_job);
	}
	return 1;
}