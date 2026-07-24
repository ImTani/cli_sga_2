#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

// keeping every record the same size so i can jump straight to record n later.
// that only works because they're all fixed length.
struct employee {
    int  id;
    char name[32];
    int  salary;
};

int main() {
    // O_RDWR = read+write, O_CREAT = make it if missing, O_TRUNC = empty it if it's there.
    // had to look the flags up. open gives back a file descriptor (a small int) i use after this.
    int fd = open("employees.dat", O_RDWR | O_CREAT | O_TRUNC, 0644);
    if (fd < 0) {
        perror("open");
        exit(1);
    }

    struct employee staff[3] = {
        {1, "alice", 50000},
        {2, "bob",   45000},
        {3, "carol", 60000},
    };
    // checking the write return, a short write would quietly corrupt a record and
    // the point was a secure utility.
    if (write(fd, staff, sizeof(staff)) != sizeof(staff)) {
        perror("write");
        exit(1);
    }

    // update just bob without rewriting the whole file, jump to his record first.
    // he's the 2nd employee but index 1, mixed that up at first:
    // lseek(fd, 2 * sizeof(struct employee), SEEK_SET);   // nope, that's carol
    struct employee bob = {2, "bob", 55000};
    lseek(fd, 1 * sizeof(struct employee), SEEK_SET);
    if (write(fd, &bob, sizeof(bob)) != sizeof(bob)) {
        perror("write");
        exit(1);
    }

    // grab carol (index 2) directly without reading the ones before her
    struct employee r;
    lseek(fd, 2 * sizeof(struct employee), SEEK_SET);
    if (read(fd, &r, sizeof(r)) != sizeof(r)) {
        perror("read");
        exit(1);
    }
    printf("fetched record 3 directly: id=%d name=%s salary=%d\n", r.id, r.name, r.salary);

    // read the whole thing back to check bob's update actually stuck
    lseek(fd, 0, SEEK_SET);
    printf("all records:\n");
    while (read(fd, &r, sizeof(r)) == sizeof(r)) {
        printf("  id=%d name=%s salary=%d\n", r.id, r.name, r.salary);
    }

    close(fd);
    return 0;
}
