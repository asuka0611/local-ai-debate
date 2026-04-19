#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "debate.h"

static void json_escape(const char *src, char *dst, int dst_size) {
    int j = 0;

    for (int i = 0; src[i] != '\0' && j < dst_size - 1; i++) {
        char c = src[i];

        if (c == '"' || c == '\\') {
            if (j < dst_size - 2) {
                dst[j++] = '\\';
                dst[j++] = c;
            }
        } else if (c == '\n') {
            if (j < dst_size - 2) {
                dst[j++] = '\\';
                dst[j++] = 'n';
            }
        } else if (c == '\r') {
            /* skip */
        } else {
            dst[j++] = c;
        }
    }

    dst[j] = '\0';
}

static int extract_content(const char *json, char *out, int out_size) {
    const char *key = "\"content\":\"";
    char *start = strstr((char *)json, key);

    if (!start) {
        snprintf(out, out_size, "parse error: %s", json);
        return -1;
    }

    start += strlen(key);

    int i = 0;
    while (*start && i < out_size - 1) {
        if (*start == '"' && start[-1] != '\\') {
            break;
        }

        if (*start == '\\' && start[1] == 'n') {
            out[i++] = '\n';
            start += 2;
            continue;
        }

        if (*start == '\\' && start[1] == '"') {
            out[i++] = '"';
            start += 2;
            continue;
        }

        if (*start == '\\' && start[1] == '\\') {
            out[i++] = '\\';
            start += 2;
            continue;
        }

        out[i++] = *start;
        start++;
    }

    out[i] = '\0';
    return 0;
}

static void build_system_prompt(const char *role, char *out, int out_size) {
    if (strstr(role, "ChatGPT")) {
        snprintf(out, out_size,
            "あなたは ChatGPT 推進役 です。"
            "テーマの価値や可能性を前向きに示してください。"
            "論理的で整理された話し方をしてください。");
    } else if (strstr(role, "Claude")) {
        snprintf(out, out_size,
            "あなたは Claude 批判役 です。"
            "前提の甘さ、条件不足、リスク、論理の穴を丁寧に指摘してください。");
    } else if (strstr(role, "Gemini")) {
        snprintf(out, out_size,
            "あなたは Gemini 整理役 です。"
            "両者の主張を比較し、論点を整理し、合意点と対立点を見つけてください。");
    } else if (strstr(role, "Grok")) {
        snprintf(out, out_size,
            "あなたは Grok 現実役 です。"
            "実行可能性、コスト、再現性、時間制約の観点で率直に評価してください。");
    } else {
        snprintf(out, out_size,
            "あなたは議論に参加するAIです。");
    }
}

static const char *turn_instruction(int turn) {
    switch (turn) {
        case 1:
            return "初期主張を述べてください。立場を明確にして結論を先に言ってください。";
        case 2:
            return "直前の他者の主張に反論してください。相手の名前を1回出し、弱点を1つ指摘してください。";
        case 3:
            return "さらに深掘りして反論してください。論理の穴か前提の弱さを指摘してください。";
        case 4:
            return "ここまでの議論を踏まえ、自分の主張を修正するか維持するかを明言してください。";
        case 5:
            return "新しい観点を1つ追加してください。今まで出ていない比較軸や条件を入れてください。";
        case 6:
            return "他の参加者の主張のうち、一部だけ同意できる点を1つ示し、そのうえで残る反対点を述べてください。";
        case 7:
            return "抽象論ではなく、具体的な場面や使い方を1つ挙げてください。";
        case 8:
            return "ここまでの流れを踏まえ、自分の立場を再整理してください。最重要論点を1つに絞ってください。";
        case 9:
            return "収束方向に進めてください。合意点を1つ、残る対立点を1つ示してください。";
        case 10:
            return "最終コメントを述べてください。結論を短くまとめ、本質を1文で示してください。";
        default:
            return "テーマについて発言してください。";
    }
}

static void build_history(DebateSession *session, char *out, int out_size) {
    out[0] = '\0';

    for (int i = 0; i < session->agent_count; i++) {
        Agent *a = &session->agents[i];

        if (strlen(a->last_message) == 0) continue;

        strncat(out, a->name, out_size - strlen(out) - 1);
        strncat(out, " (", out_size - strlen(out) - 1);
        strncat(out, a->role, out_size - strlen(out) - 1);
        strncat(out, "): ", out_size - strlen(out) - 1);
        strncat(out, a->last_message, out_size - strlen(out) - 1);
        strncat(out, "\n", out_size - strlen(out) - 1);
    }

    if (strlen(out) == 0) {
        strncat(out, "まだ議論履歴はありません。\n", out_size - strlen(out) - 1);
    }
}

static void build_user_prompt(
    DebateSession *session,
    Agent *agent,
    int turn,
    char *out,
    int out_size
) {
    char history[2500];
    build_history(session, history, sizeof(history));

    snprintf(out, out_size,
        "テーマ: %s\n"
        "あなたの名前: %s\n"
        "あなたの役割: %s\n"
        "現在のターン: %d\n"
        "これまでの議論:\n%s\n"
        "今回の指示: %s\n"
        "制約:\n"
        "- 日本語で返す\n"
        "- 50〜90文字程度\n"
        "- 1発言だけ返す\n"
        "- 具体的に返す\n"
        "- 同じ表現を繰り返しすぎない\n"
        "- 相手の名前を使える場面では使う\n",
        session->theme,
        agent->name,
        agent->role,
        turn,
        history,
        turn_instruction(turn)
    );
}

int generate_agent_message(
    DebateSession *session,
    Agent *agent,
    int turn,
    char *out,
    int out_size
) {
    char system_prompt[1024];
    char user_prompt[4096];
    char system_escaped[2048];
    char user_escaped[8192];
    char raw_json[MAX_MSG];

    build_system_prompt(agent->role, system_prompt, sizeof(system_prompt));
    build_user_prompt(session, agent, turn, user_prompt, sizeof(user_prompt));

    json_escape(system_prompt, system_escaped, sizeof(system_escaped));
    json_escape(user_prompt, user_escaped, sizeof(user_escaped));

    if (ollama_generate("qwen3.5:4b", system_escaped, user_escaped, raw_json, sizeof(raw_json)) != 0) {
        snprintf(out, out_size, "Ollama request failed: %s", raw_json);
        return -1;
    }

    if (extract_content(raw_json, out, out_size) != 0) {
        snprintf(out, out_size, "Failed to parse Ollama response");
        return -1;
    }

    return 0;
}