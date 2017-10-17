#include <stdio.h>
#include <string.h>

#include "message.h"

#define LANGCOUNT   (sizeof(langs)/sizeof(langs[0]))

static const char* langs[] = { "pt_br", "en_us" };
static int lang = 0;

static const char* messages[LANGCOUNT][1000] = {
    {
        "Novo Jogo",
        "Continuar",
        "Ranking",
        "Idioma: %s",
        "Editor de Níveis",
        "Sair",
        "Melhores tempos",
        "Nível %d",
        "Resumir",
        "Recomeçar",
        "Tempo: %02u:%02u",
        "Nível Concluído!",
        "Fim de Jogo!",
        "Novo Nível",
        "Carregar Nível",
        "Jogar Nível",
        "Salvar Nível",
        "Cancelar",
        "Voltar",
        "Título",
        "Autor",
        "Nível salvo com sucesso!"
    }, {
        "New Game",
        "Continue",
        "Ranking",
        "Language: %s",
        "Level Editor",
        "Exit",
        "Best times",
        "Level %d",
        "Resume",
        "Restart",
        "Time: %02u:%02u",
        "Level Complete!",
        "Game Over!",
        "New Level",
        "Load Level",
        "Play Level",
        "Save Level",
        "Cancel",
        "Go Back",
        "Title",
        "Author",
        "Level successfully saved!"
    }
};
static const char* nullmsg = "NULL";

int setMessageLanguage(int newlang) {
    lang = (newlang < 0) ? 0 : newlang % LANGCOUNT;
}

const char* getMessage(int msg) {
    if (msg < 0 || msg >= 1000) return nullmsg;
    return messages[lang][msg];
}

int getLangCount() {
    return LANGCOUNT;
}

const char* getLangString() {
    return langs[lang];
}
