#include <stdint.h>
#include <sys/epoll.h>
#include <unistd.h>

int32_t setup_epoll(int32_t listen_fd) {
    int32_t epoll_fd = epoll_create1(0);
    if (epoll_fd == -1) return -1;

    struct epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = listen_fd;

    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, listen_fd, &ev) == -1) {
        close(epoll_fd);
        return -1;
    }
    return epoll_fd;
}
