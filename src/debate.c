#include <stdio.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
static void sleep_ms(int ms) { Sleep(ms); }
#else
#include <unistd.h>
static void sleep_ms(int ms) { usleep(ms * 1000); }
#endif

#include "debate.h"

static void print_line(char ch, int count) {
    for (int i = 0; i < count; i++) {
        putchar(ch);
    }
    putchar('\n');
    fflush(stdout);
}

void run_debate(DebateSession *session, int turns) {
    printf("\n");
    fflush(stdout);

    print_line('=', 50);
    printf("議論開始\n");
    fflush(stdout);

    print_line('=', 50);
    printf("テーマ: %s\n\n", session->theme);
    fflush(stdout);

    for (int i = 0; i < session->agent_count; i++) {
        session->agents[i].last_message[0] = '\0';
    }

    for (int t = 1; t <= turns; t++) {
        print_line('-', 50);
        printf("ターン %d\n", t);
        fflush(stdout);
        print_line('-', 50);

        for (int i = 0; i < session->agent_count; i++) {
            Agent *agent = &session->agents[i];
            char response[1024];

            printf("[%s | %s]\n", agent->name, agent->role);
            fflush(stdout);

            if (generate_agent_message(session, agent, t, response, sizeof(response)) != 0) {
                printf("[%s]: %s\n\n", agent->name, response);
                fflush(stdout);

                strncpy(agent->last_message, response, sizeof(agent->last_message) - 1);
                agent->last_message[sizeof(agent->last_message) - 1] = '\0';
                continue;
            }

            printf("[%s]: %s\n\n", agent->name, response);
            fflush(stdout);

            strncpy(agent->last_message, response, sizeof(agent->last_message) - 1);
            agent->last_message[sizeof(agent->last_message) - 1] = '\0';
        }
    }

    print_line('=', 50);
    printf("議論終了\n");
    fflush(stdout);
    print_line('=', 50);
}