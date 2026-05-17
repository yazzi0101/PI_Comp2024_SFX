#include <stdio.h>
#include <stdint.h>
#include <sys/resource.h>

int32_t configure_system_limits(void) {
    struct rlimit rl;

    /* Define o limite para 8192 file descriptors ativos */
    rl.rlim_cur = 8192;
    rl.rlim_max = 8192;

    if (setrlimit(RLIMIT_NOFILE, &rl) == -1) {
        perror("Erro critico de infraestrutura: Falha ao setar setrlimit");
        return -1;
    }
    return 0;
}
