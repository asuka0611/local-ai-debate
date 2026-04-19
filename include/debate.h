#ifndef DEBATE_H
#define DEBATE_H

#define MAX_AGENTS 4
#define MAX_MSG 4096

typedef struct {
    char name[32];
    char role[128];
    char last_message[1024];
} Agent;

typedef struct {
    char theme[256];
    Agent agents[MAX_AGENTS];
    int agent_count;
} DebateSession;

/* Ollama 呼び出し */
int ollama_generate(
    const char *model,
    const char *system_prompt,
    const char *user_prompt,
    char *out,
    int out_size
);

/* 1エージェント分の発言生成 */
int generate_agent_message(
    DebateSession *session,
    Agent *agent,
    int turn,
    char *out,
    int out_size
);

/* 議論実行 */
void run_debate(DebateSession *session, int turns);

#endif