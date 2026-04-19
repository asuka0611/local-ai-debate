#include <stdio.h>
#include <string.h>
#include "debate.h"

int main(void) {
    DebateSession session;

    printf("テーマを入力: ");
    fgets(session.theme, sizeof(session.theme), stdin);
    session.theme[strcspn(session.theme, "\n")] = '\0';

    session.agent_count = 4;

    strcpy(session.agents[0].name, "GPT");
    strcpy(session.agents[0].role, "ChatGPT 推進役");

    strcpy(session.agents[1].name, "Claude");
    strcpy(session.agents[1].role, "Claude 批判役");

    strcpy(session.agents[2].name, "Gemini");
    strcpy(session.agents[2].role, "Gemini 整理役");

    strcpy(session.agents[3].name, "Grok");
    strcpy(session.agents[3].role, "Grok 現実役");

    run_debate(&session, 10);

    return 0;
}