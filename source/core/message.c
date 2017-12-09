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
        "Logout",
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
        "Confirmar",
        "Título",
        "Autor",
        "Nível salvo com sucesso!",
        "Níveis Online",
        "Baixar Nível",
        "Enviar Nível",
        "Meus Níveis",
        "Meu Perfil",
        "Nível baixado com sucesso!",
        "Falha ao baixar nível.",
        "Usuário",
        "Senha",
        "Sim",
        "Não",
        "Salvar alterações do nível?",
        "Deletar nível selecionado?",
        "NÃO TESTADO",
        "VÁLIDO",
        "NÃO TESTADO*",
        "VÁLIDO*"
    }, {
        "New Game",
        "Continue",
        "Ranking",
        "Language: %s",
        "Level Editor",
        "Exit",
        "Logout",
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
        "Confirm",
        "Title",
        "Author",
        "Level successfully saved!",
        "Online Levels",
        "Download Level",
        "Upload Level",
        "My Levels",
        "My Profile",
        "Level successfully downloaded!",
        "Failed to download level.",
        "Username",
        "Password",
        "Yes",
        "No",
        "Save changes?",
        "Delete selected level?",
        "UNTESTED",
        "VALID",
        "UNTESTED*",
        "VALID*"
    }
};
static const char* nullmsg = "NULL";

void setLanguage(int newlang) {
    lang = (newlang < 0) ? 0 : (newlang % LANGCOUNT);
}

int getLanguage() {
    return lang;
}

const char* getMessage(int msg) {
    if (msg < 0 || msg >= 1000) return nullmsg;
    return messages[lang][msg];
}

const char* getLangString() {
    return langs[lang];
}
