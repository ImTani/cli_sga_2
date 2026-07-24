#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>

#define N 3

// the child installs this handler for SIGTERM. instead of just being killed by
// the default action, the child catches the signal and exits cleanly itself.
void on_term(int sig) {
    _exit(0);
}

int main() {
    pid_t pids[N];

    for (int i = 0; i < N; i++) {
        pid_t pid = fork();     // fork splits this into two processes
        if (pid < 0) {
            perror("fork");
            exit(1);
        }
        if (pid == 0) {
            signal(SIGTERM, on_term);       // child handles SIGTERM itself
            printf("child %d (pid %d) started\n", i, getpid());
            if (i == N - 1) {
                while (1) sleep(1);         // this one hangs so the parent has to step in
            } else {
                sleep(2);
                printf("child %d (pid %d) done\n", i, getpid());
            }
            exit(0);
        }
        pids[i] = pid;      // in the parent, fork gives back the child pid, save it
    }

    // monitor the children instead of just assuming which one is stuck.
    // waitpid with WNOHANG doesn't block, so i poll each second and mark who finished.
    int done[N] = {0};
    int alive = N;
    for (int t = 0; t < 4 && alive > 0; t++) {
        sleep(1);
        for (int i = 0; i < N; i++) {
            if (!done[i]) {
                int status;
                if (waitpid(pids[i], &status, WNOHANG) == pids[i]) {
                    printf("parent: child pid %d finished on its own\n", pids[i]);
                    done[i] = 1;
                    alive--;
                }
            }
        }
    }

    // anything still alive after the grace period counts as unresponsive, so kill it
    for (int i = 0; i < N; i++) {
        if (!done[i]) {
            printf("parent: child pid %d is unresponsive, sending SIGTERM\n", pids[i]);
            kill(pids[i], SIGTERM);
            int status;
            waitpid(pids[i], &status, 0);       // reap it so it doesn't become a zombie
            if (WIFEXITED(status))
                printf("parent: child pid %d caught the signal and exited cleanly\n", pids[i]);
        }
    }

    printf("parent: all children handled, no zombies left\n");
    return 0;
}
