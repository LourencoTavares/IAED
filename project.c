/* iaed25 - ist1114698 - project */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_BYTES 51    /**< Tamanho máximo para nomes de vacina */
#define MAX_DIGITS_LOTE 21    /**< Tamanho máximo para o identificador do lote */
#define MAX_LOTES 1000     /**< Número máximo de lotes */
#define BUFMAX 65536    /**< Tamanho máximo do buffer de entrada */

/**
 * @brief Estrutura que representa uma data
 */
typedef struct {
    int dia, mes, ano;      /**< Ano, Mes, Ano */
} Data;

/**
 * @brief Estrutura que representa um lote de vacina
 */
typedef struct {
    char nome_vacina[MAX_BYTES];
    char lote[MAX_DIGITS_LOTE];
    Data data;
    unsigned int doses_disponiveis;
    unsigned int doses_aplicadas;
} Lote_Vacina;

/**
 * @brief Estrutura que representa uma inoculação
 */
typedef struct {
    char *nome_utente;
    Lote_Vacina *lote_utilizado;
    Data data_inoculacao;
} Inoculacao;

/**
 * @brief Estrutura que contém toda a informação do sistema
 */
typedef struct {
    int numero_lotes;
    Lote_Vacina info[MAX_LOTES];
    Data data_atual;
    Inoculacao *inoculacoes;
    int num_inoculacoes;
    struct MensagensErro *erro;
} Sys;

/**
 * @brief Estrutura que armazena as mensagens de erro
 */
typedef struct MensagensErro {
    const char *msg_demasiadas_vacinas;
    const char *msg_lote_duplicado;
    const char *msg_lote_invalido;
    const char *msg_nome_invalido;
    const char *msg_data_invalida;
    const char *msg_quantidade_invalida;
    const char *msg_vacina_inexistente;
    const char *msg_esgotado;
    const char *msg_ja_vacinado;
    const char *msg_lote_inexistente;
    const char *msg_utente_inexistente;
    const char *msg_sem_memoria;
} MensagensErro;

/**
 * @brief Remove os espaços em branco iniciais de uma string
 * @param s String a processar
 * @return Ponteiro para o início da string sem espaços
 */
static char *poupaMemória(char *s) {
    while (*s && isspace((unsigned char)*s))
        s++;
    return s;
}

/**
 * @brief Cria e inicializa as mensagens de erro
 * @param usa_pt Se 1, usa mensagens em português, caso contrário, em inglês
 * @return Ponteiro para a estrutura MensagensErro
 */
MensagensErro *criaMensagemErro(int usa_pt) {
    MensagensErro *erro = malloc(sizeof(MensagensErro));
    if (erro == NULL) {
        puts("sem memória");
        exit(1);
    }
    if (usa_pt) {
        erro->msg_demasiadas_vacinas = "demasiadas vacinas";
        erro->msg_lote_duplicado = "número de lote duplicado";
        erro->msg_lote_invalido = "lote inválido";
        erro->msg_nome_invalido = "nome inválido";
        erro->msg_data_invalida = "data inválida";
        erro->msg_quantidade_invalida = "quantidade inválida";
        erro->msg_vacina_inexistente = "vacina inexistente";
        erro->msg_esgotado = "esgotado";
        erro->msg_ja_vacinado = "já vacinado";
        erro->msg_lote_inexistente = "lote inexistente";
        erro->msg_utente_inexistente = "utente inexistente";
        erro->msg_sem_memoria = "sem memória";
    } else {
        erro->msg_demasiadas_vacinas = "too many vaccines";
        erro->msg_lote_duplicado = "duplicate batch number";
        erro->msg_lote_invalido = "invalid batch";
        erro->msg_nome_invalido = "invalid name";
        erro->msg_data_invalida = "invalid date";
        erro->msg_quantidade_invalida = "invalid quantity";
        erro->msg_vacina_inexistente = "no such vaccine";
        erro->msg_esgotado = "no stock";
        erro->msg_ja_vacinado = "already vaccinated";
        erro->msg_lote_inexistente = "no such batch";
        erro->msg_utente_inexistente = "no such user";
        erro->msg_sem_memoria = "No memory";
    }
    return erro;
}

/**
 * @brief Lê os dados de entrada para criar um lote
 * @param entrada_dados Linha de entrada com os dados
 * @param lote Buffer para o identificador do lote
 * @param dia Ponteiro para o dia
 * @param mes Ponteiro para o mês
 * @param ano Ponteiro para o ano
 * @param dose Ponteiro para a quantidade de doses
 * @param nome_vacina Buffer para o nome da vacina
 * @param sys Ponteiro para a estrutura do sistema
 * @return 1 se os dados forem lidos corretamente, 0 caso contrário
 */
static int leEntrada(const char *entrada_dados, char *lote, int *dia, int *mes, int *ano, unsigned int *dose, char *nome_vacina, Sys *sys) {
    char lote_torario[BUFMAX];
    int n = sscanf(entrada_dados, "c %s %d-%d-%d %u %s", lote_torario, dia, mes, ano, dose, nome_vacina);
    if (n != 6) {
        puts(sys->erro->msg_lote_invalido);
        return 0;
    }
    if (strlen(lote_torario) > MAX_DIGITS_LOTE - 1) {
        puts(sys->erro->msg_lote_invalido);
        return 0;
    }
    strcpy(lote, lote_torario);
    return 1;
}

/**
 * @brief Valida o identificador do lote
 * @param sys Ponteiro para o sistema
 * @param lote Identificador do lote
 * @return 1 se o lote é válido, 0 caso contrário
 */
static int validaLote(Sys *sys, const char *lote) {
    for (int i = 0; lote[i] != '\0'; i++) {
        if (!(isdigit((unsigned char)lote[i]) || (lote[i] >= 'A' && lote[i] <= 'F'))) {
            puts(sys->erro->msg_lote_invalido);
            return 0;
        }
    }
    return 1;
}

/**
 * @brief Verifica se é possível adicionar um novo lote
 * @param sys Ponteiro para o sistema
 * @return 1 se ainda há espaço para mais lotes, 0 caso contrário
 */
static int procuraMaxLote(Sys *sys) {
    if (sys->numero_lotes >= MAX_LOTES) {
        puts(sys->erro->msg_demasiadas_vacinas);
        return 0;
    }
    return 1;
}

/**
 * @brief Verifica se já existe um lote com o mesmo identificador
 * @param sys Ponteiro para o sistema
 * @param lote Identificador do lote
 * @return 1 se não houver duplicado, 0 caso exista
 */
static int procuraLoteDuplicado(Sys *sys, const char *lote) {
    for (int i = 0; i < sys->numero_lotes; i++) {
        if (strcmp(sys->info[i].lote, lote) == 0) {
            puts(sys->erro->msg_lote_duplicado);
            return 0;
        }
    }
    return 1;
}

/**
 * @brief Valida o nome da vacina
 * @param nome_vacina Nome da vacina
 * @param sys Ponteiro para o sistema
 * @return 1 se o nome for válido, 0 caso contrário
 */
static int validaNome(const char *nome_vacina, Sys *sys) {
    if (strlen(nome_vacina) > MAX_BYTES - 1) {
        puts(sys->erro->msg_nome_invalido);
        return 0;
    }
    if (strchr(nome_vacina, '_') != NULL) {
        puts("invalid character in vaccine name");
        return 0;
    }
    for (int i = 0; nome_vacina[i] != '\0'; i++) {
        if (nome_vacina[i] == ' ' || nome_vacina[i] == '\t' || nome_vacina[i] == '\n') {
            puts(sys->erro->msg_nome_invalido);
            return 0;
        }
    }
    return 1;
}


/**
 * @brief Valida a data informada
 * @param dia Dia
 * @param mes Mês
 * @param ano Ano
 * @param data_atual Data atual do sistema
 * @param sys Ponteiro para o sistema
 * @return 1 se a data for válida, 0 caso contrário
 */
static int validaData(int dia, int mes, int ano, Data data_atual, Sys *sys) {
    int dia_min = 1, mes_min = 1, ano_min = 2025;
    if (ano < ano_min || (ano == ano_min && mes < mes_min) || (ano == ano_min && mes == mes_min && dia < dia_min)) {
        puts(sys->erro->msg_data_invalida);
        return 0;
    }
    int max_dia;
    switch (mes) {
        case 2: {
            if ((ano % 400 == 0) || (ano % 4 == 0 && ano % 100 != 0))
                max_dia = 29;
            else
                max_dia = 28;
            break;
        }
        case 4: case 6: case 9: case 11: {
            max_dia = 30;
            break;
        }
        default: {
            max_dia = 31;
            break;
        }
    }
    if (mes < 1 || mes > 12 || dia < 1 || dia > max_dia) {
        puts(sys->erro->msg_data_invalida);
        return 0;
    }
    if (ano < data_atual.ano || (ano == data_atual.ano && mes < data_atual.mes) || (ano == data_atual.ano && mes == data_atual.mes && dia < data_atual.dia)) {
        puts(sys->erro->msg_data_invalida);
        return 0;
    }
    return 1;
}

/**
 * @brief Valida a quantidade de doses
 * @param dose Quantidade de doses
 * @param sys Ponteiro para o sistema
 * @return 1 se a dose for válida, 0 caso contrário
 */
static int validaDose(unsigned int dose, Sys *sys) {
    if (dose == 0) {
        puts(sys->erro->msg_quantidade_invalida);
        return 0;
    }
    return 1;
}

/**
 * @brief Verifica se o utente está registado no sistema
 * @param sys Ponteiro para o sistema
 * @param nomeUtente Nome do utente
 * @return 1 se o utente existe, 0 caso contrário
 */
static int verificaUtente(Sys *sys, const char *nomeUtente) {
    for (int i = 0; i < sys->num_inoculacoes; i++) {
        if (strcmp(sys->inoculacoes[i].nome_utente, nomeUtente) == 0)
            return 1;
    }
    printf("%s: %s\n", nomeUtente, sys->erro->msg_utente_inexistente);
    return 0;
}

/**
 * @brief Valida a data para filtros de inoculação
 * @param sys Ponteiro para o sistema
 * @param dia Dia
 * @param mes Mês
 * @param ano Ano
 * @return 1 se a data for válida, 0 caso contrário
 */
static int filtroValidaData(Sys *sys, int dia, int mes, int ano) {
    int maxDia = (mes == 2) ? (((ano % 400 == 0) || (ano % 4 == 0 && ano % 100 != 0)) ? 29 : 28)
                    : ((mes == 4 || mes == 6 || mes == 9 || mes == 11) ? 30 : 31);
    if (mes < 1 || mes > 12 || dia < 1 || dia > maxDia ||
        (ano > sys->data_atual.ano ||
        (ano == sys->data_atual.ano && mes > sys->data_atual.mes) ||
        (ano == sys->data_atual.ano && mes == sys->data_atual.mes && dia > sys->data_atual.dia))) {
        puts(sys->erro->msg_data_invalida);
        return 0;
    }
    return 1;
}

/**
 * @brief Verifica se o lote informado existe para filtros
 * @param sys Ponteiro para o sistema
 * @param idLote Identificador do lote
 * @return 1 se o lote existir, 0 caso contrário
 */
static int filtroValidaLote(Sys *sys, const char *idLote) {
    for (int i = 0; i < sys->numero_lotes; i++) {
        if (sys->info[i].nome_vacina[0] && strcmp(sys->info[i].lote, idLote) == 0)
            return 1;
    }
    printf("%s: %s\n", idLote, sys->erro->msg_lote_inexistente);
    return 0;
}

/**
 * @brief Insere um novo lote no sistema
 * @param sys Ponteiro para o sistema
 * @param lote Identificador do lote
 * @param nome_vacina Nome da vacina
 * @param dia Dia da validade
 * @param mes Mês da validade
 * @param ano Ano da validade
 * @param dose Quantidade de doses
 */
static void insereLote(Sys *sys, const char *lote, const char *nome_vacina, int dia, int mes, int ano, unsigned int dose) {
    strcpy(sys->info[sys->numero_lotes].lote, lote);
    strcpy(sys->info[sys->numero_lotes].nome_vacina, nome_vacina);
    sys->info[sys->numero_lotes].data.dia = dia;
    sys->info[sys->numero_lotes].data.mes = mes;
    sys->info[sys->numero_lotes].data.ano = ano;
    sys->info[sys->numero_lotes].doses_disponiveis = dose;
    sys->info[sys->numero_lotes].doses_aplicadas = 0;
    sys->numero_lotes++;
    printf("%s\n", lote);
}

/**
 * @brief Processa a criação de um lote a partir dos dados de entrada
 * @param sys Ponteiro para o sistema
 * @param entrada_dados Linha de entrada
 */
static void cria_lote(Sys *sys, char *entrada_dados) {
    char lote[MAX_DIGITS_LOTE];
    char nome_vacina[MAX_BYTES];
    int dia, mes, ano;
    unsigned int dose;
    if (!leEntrada(entrada_dados, lote, &dia, &mes, &ano, &dose, nome_vacina, sys))
        return;
    if (!validaLote(sys, lote))
        return;
    if (!procuraMaxLote(sys))
        return;
    if (!procuraLoteDuplicado(sys, lote))
        return;
    if (!validaNome(nome_vacina, sys))
        return;
    if (!validaData(dia, mes, ano, sys->data_atual, sys))
        return;
    if (!validaDose(dose, sys))
        return;
    insereLote(sys, lote, nome_vacina, dia, mes, ano, dose);
}

/**
 * @brief Recolhe os lotes de vacina que correspondem a um nome
 * @param sys Ponteiro para o sistema
 * @param nome Nome da vacina (ou NULL para todos)
 * @param vetor Vetor para armazenar os lotes encontrados
 * @param max_size Tamanho máximo do vetor
 * @return Número de lotes coletados
 */
static int coletaLotes(Sys *sys, const char *nome, Lote_Vacina *vetor[], int max_size) {
    int total = 0;
    for (int i = 0; i < sys->numero_lotes; i++) {
        if (sys->info[i].nome_vacina[0] != '\0' &&
            (nome == NULL || strcmp(sys->info[i].nome_vacina, nome) == 0)) {
            vetor[total++] = &sys->info[i];
            if (total == max_size)
                break;
        }
    }
    return total;
}

/**
 * @brief Compara dois lotes com base na data e no identificador
 * @param lote1 Ponteiro para o primeiro lote
 * @param lote2 Ponteiro para o segundo lote
 * @return Valor negativo se lote1 precede lote2, zero se forem iguais, ou valor positivo caso contrário
 */
static int comparaLotes(Lote_Vacina *lote1, Lote_Vacina *lote2) {
    if (lote1->data.ano != lote2->data.ano)
        return lote1->data.ano - lote2->data.ano;
    if (lote1->data.mes != lote2->data.mes)
        return lote1->data.mes - lote2->data.mes;
    if (lote1->data.dia != lote2->data.dia)
        return lote1->data.dia - lote2->data.dia;
    return strcmp(lote1->lote, lote2->lote);
}

/**
 * @brief Ordena um vetor de lotes de vacina
 * @param vetor Vetor de ponteiros para os lotes
 * @param n Número de lotes
 */
static void ordenaVacinas(Lote_Vacina *vetor[], int n) {
    int i, j;
    for (i = 0; i < n - 1; i++) {
        for (j = 0; j < n - i - 1; j++) {
            if (comparaLotes(vetor[j], vetor[j+1]) > 0) {
                Lote_Vacina *t = vetor[j];
                vetor[j] = vetor[j+1];
                vetor[j+1] = t;
            }
        }
    }
}

/**
 * @brief Imprime a informação dos lotes
 * @param lotes Vetor de ponteiros para os lotes
 * @param total Número total de lotes
 */
static void imprimeLotes(Lote_Vacina *lotes[], int total) {
    for (int i = 0; i < total; i++) {
        printf("%s %s %02d-%02d-%d %u %u\n",
                lotes[i]->nome_vacina,
                lotes[i]->lote,
                lotes[i]->data.dia,
                lotes[i]->data.mes,
                lotes[i]->data.ano,
                lotes[i]->doses_disponiveis,
                lotes[i]->doses_aplicadas);
    }
}

/**
 * @brief Processa e imprime os lotes de uma vacina específica
 * @param sys Ponteiro para o sistema
 * @param nome Nome da vacina
 */
static void processaVacina(Sys *sys, const char *nome) {
    Lote_Vacina *vetor[MAX_LOTES];
    int total = coletaLotes(sys, nome, vetor, MAX_LOTES);
    if (total == 0) {
        printf("%s: %s\n", nome, sys->erro->msg_vacina_inexistente);
        return;
    }
    ordenaVacinas(vetor, total);
    imprimeLotes(vetor, total);
}

/**
 * @brief Lista os lotes de vacina, filtrando por nome se necessário
 * @param sys Ponteiro para o sistema
 * @param entrada_dados Linha de entrada
 */
static void lista_vacinas(Sys *sys, char *entrada_dados) {
    char *p = entrada_dados + 1;
    p = poupaMemória(p);
    if (*p == '\0') {
        Lote_Vacina *vetor[MAX_LOTES];
        int total = coletaLotes(sys, NULL, vetor, MAX_LOTES);
        ordenaVacinas(vetor, total);
        imprimeLotes(vetor, total);
    } else {
        char *nomeVacina = strtok(p, " \t\n");
        while (nomeVacina != NULL) {
            processaVacina(sys, nomeVacina);
            nomeVacina = strtok(NULL, " \t\n");
        }
    }
}

/**
 * @brief Extrai os dados de inoculação da linha de entrada
 * @param sys Ponteiro para o sistema
 * @param entrada_dados Linha de entrada
 * @param utente Buffer para o nome do utente
 * @param vacina Buffer para o nome da vacina
 * @return 1 se os dados forem extraídos com sucesso, 0 caso contrário
 */
static int extraiDados(Sys *sys, char *entrada_dados, char *utente, char *vacina) {
    char *p = entrada_dados + 1;
    p = poupaMemória(p);
    if (*p == '"') {
        p++;
        char *inicio = p;
        char *fim = strchr(p, '"');
        if (fim == NULL) {
            puts(sys->erro->msg_utente_inexistente);
            return 0;
        }
        size_t len = fim - inicio;
        if (len >= BUFMAX)
            len = BUFMAX - 1;
        strncpy(utente, inicio, len);
        utente[len] = '\0';
        p = fim + 1;
    } else {
        if (sscanf(p, "%s", utente) != 1) {
            puts(sys->erro->msg_utente_inexistente);
            return 0;
        }
        while (*p && !isspace((unsigned char)*p))
            p++;
    }
    p = poupaMemória(p);
    if (*p == '\0') {
        puts(sys->erro->msg_utente_inexistente);
        return 0;
    }
    if (sscanf(p, "%s", vacina) != 1) {
        puts(sys->erro->msg_utente_inexistente);
        return 0;
    }
    return 1;
}

/**
 * @brief Verifica se uma inoculação já foi registada para o utente na data atual
 * @param sys Ponteiro para o sistema
 * @param nomeUtente Nome do utente
 * @param nomeVacina Nome da vacina
 * @return 1 se a inoculação já existe, 0 caso contrário
 */
static int inoculacaoRegistada(Sys *sys, const char *nomeUtente, const char *nomeVacina) {
    for (int i = 0; i < sys->num_inoculacoes; i++) {
        Inoculacao inv = sys->inoculacoes[i];
        if (strcmp(inv.nome_utente, nomeUtente) == 0 &&
            strcmp(inv.lote_utilizado->nome_vacina, nomeVacina) == 0 &&
            inv.data_inoculacao.dia == sys->data_atual.dia &&
            inv.data_inoculacao.mes == sys->data_atual.mes &&
            inv.data_inoculacao.ano == sys->data_atual.ano)
            return 1;
    }
    return 0;
}

/**
 * @brief Seleciona o lote disponível mais adequado para a vacina
 * @param sys Ponteiro para o sistema
 * @param nomeVacina Nome da vacina
 * @return Ponteiro para o lote selecionado ou NULL se nenhum estiver disponível
 */
static Lote_Vacina *selecionaLoteDisponivel(Sys *sys, const char *nomeVacina) {
    Lote_Vacina *loteSelecionado = NULL;
    for (int i = 0; i < sys->numero_lotes; i++) {
        Lote_Vacina *lote = &sys->info[i];
        if (strcmp(lote->nome_vacina, nomeVacina) == 0) {
            int dataValida = 1;
            if (lote->data.ano < sys->data_atual.ano)
                dataValida = 0;
            else if (lote->data.ano == sys->data_atual.ano) {
                if (lote->data.mes < sys->data_atual.mes)
                    dataValida = 0;
                else if (lote->data.mes == sys->data_atual.mes && lote->data.dia < sys->data_atual.dia)
                    dataValida = 0;
            }
            if (dataValida && lote->doses_disponiveis > 0) {
                if (loteSelecionado == NULL || comparaLotes(lote, loteSelecionado) < 0)
                    loteSelecionado = lote;
            }
        }
    }
    return loteSelecionado;
}

/**
 * @brief Regista uma nova inoculação no sistema
 * @param sys Ponteiro para o sistema
 * @param nomeUtente Nome do utente
 * @param loteSelecionado Lote selecionado para a inoculação
 * @return 1 se o registo passou, 0 caso contrário
 */
static int registaNovaInoculacao(Sys *sys, const char *nomeUtente, Lote_Vacina *loteSelecionado) {
    loteSelecionado->doses_disponiveis--;
    loteSelecionado->doses_aplicadas++;
    Inoculacao *t = realloc(sys->inoculacoes, (sys->num_inoculacoes + 1) * sizeof(Inoculacao));
    if (t == NULL) {
        puts(sys->erro->msg_sem_memoria);
        return 0;
    }
    sys->inoculacoes = t;
    sys->inoculacoes[sys->num_inoculacoes].nome_utente = malloc(strlen(nomeUtente) + 1);
    if (sys->inoculacoes[sys->num_inoculacoes].nome_utente == NULL) {
        puts(sys->erro->msg_sem_memoria);
        return 0;
    }
    strcpy(sys->inoculacoes[sys->num_inoculacoes].nome_utente, nomeUtente);
    sys->inoculacoes[sys->num_inoculacoes].lote_utilizado = loteSelecionado;
    sys->inoculacoes[sys->num_inoculacoes].data_inoculacao = sys->data_atual;
    sys->num_inoculacoes++;
    return 1;
}

/**
 * @brief Processa a aplicação de vacina para um utente
 * @param sys Ponteiro para o sistema
 * @param entrada_dados Linha de entrada
 */
static void aplica_vacina(Sys *sys, char *entrada_dados) {
    char nomeUtente[BUFMAX], nomeVacina[BUFMAX];
    if (!extraiDados(sys, entrada_dados, nomeUtente, nomeVacina))
        return;
    if (inoculacaoRegistada(sys, nomeUtente, nomeVacina)) {
        puts(sys->erro->msg_ja_vacinado);
        return;
    }
    Lote_Vacina *loteSelecionado = selecionaLoteDisponivel(sys, nomeVacina);
    if (loteSelecionado == NULL) {
        puts(sys->erro->msg_esgotado);
        return;
    }
    if (!registaNovaInoculacao(sys, nomeUtente, loteSelecionado)) {
        return;
    }
    printf("%s\n", loteSelecionado->lote);
}

/**
 * @brief Processa a saída de um lote de vacina
 * @param sys Ponteiro para o sistema
 * @param entrada_dados Linha de entrada
 */
static void retira_vacina(Sys *sys, char *entrada_dados) {
    char lote[BUFMAX];
    char *p = entrada_dados + 1;
    p = poupaMemória(p);
    if (sscanf(p, "%s", lote) != 1){
        return;
    }
    int encontrou_lote = 0;
    for (int i = 0; i < sys->numero_lotes; i++) {
        if (sys->info[i].nome_vacina[0] != '\0' && strcmp(sys->info[i].lote, lote) == 0) {
            encontrou_lote = 1;
            printf("%u\n", sys->info[i].doses_aplicadas);
            if (sys->info[i].doses_aplicadas == 0) {
                sys->info[i].nome_vacina[0] = '\0';
            }
            else
                sys->info[i].doses_disponiveis = 0;
            break;
        }
    }
    if (!encontrou_lote)
        printf("%s: %s\n", lote, sys->erro->msg_lote_inexistente);
}

/**
 * @brief Extrai os argumentos para apagar inoculações
 * @param entrada_dados Linha de entrada
 * @param nomeUtente Buffer para o nome do utente
 * @param dia Ponteiro para o dia
 * @param mes Ponteiro para o mês
 * @param ano Ponteiro para o ano
 * @param dataFiltro Ponteiro para o flag de filtro de data
 * @param idLote Buffer para o identificador do lote
 * @param loteFiltro Ponteiro para o flag de filtro de lote
 * @return 1 se os argumentos forem extraídos com sucesso, 0 caso contrário
 */
static int extraiArgumentos(char *entrada_dados, char *nomeUtente, int *dia, int *mes, int *ano, int *dataFiltro, char *idLote, int *loteFiltro) {
    char *j = entrada_dados + 1;
    j = poupaMemória(j);
    if (*j == '"') {
        j++;
        char *fimAspas = strchr(j, '"');
        size_t tamanhoNome = fimAspas ? (size_t)(fimAspas - j) : strlen(j);
        if (tamanhoNome >= BUFMAX)
            tamanhoNome = BUFMAX - 1;
        for (size_t i = 0; i < tamanhoNome; i++)
            nomeUtente[i] = j[i];
        nomeUtente[tamanhoNome] = '\0';
        if (fimAspas)
            j = fimAspas + 1;
    } else {
        if (sscanf(j, "%s", nomeUtente) != 1)
            return 0;
    }
    while (*j && !isspace((unsigned char)*j))
        j++;
    j = poupaMemória(j);
    *dataFiltro = 0;
    if (*j) {
        if (sscanf(j, "%d-%d-%d", dia, mes, ano) == 3) {
            *dataFiltro = 1;
            while (*j && !isspace((unsigned char)*j))
                j++;
        }
    }
    j = poupaMemória(j);
    *loteFiltro = 0;
    if (*j) {
        if (sscanf(j, "%s", idLote) == 1)
            *loteFiltro = 1;
    }
    return 1;
}

/**
 * @brief Remove as inoculações de um utente conforme os filtros
 * @param sys Ponteiro para o sistema
 * @param nomeUtente Nome do utente
 * @param dataFiltro Flag para filtro de data
 * @param dia Dia para filtro
 * @param mes Mês para filtro
 * @param ano Ano para filtro
 * @param loteFiltro Flag para filtro de lote
 * @param idLote Identificador do lote para filtro
 * @return Número de inoculações removidas
 */
static int removeInoculacoes(Sys *sys, const char *nomeUtente, int dataFiltro, int dia, int mes, int ano, int loteFiltro, const char *idLote) {
    int totalApagadas = 0;
    for (int i = 0; i < sys->num_inoculacoes; ) {
        int removeRegistro = (strcmp(sys->inoculacoes[i].nome_utente, nomeUtente) == 0);
        if (removeRegistro && dataFiltro) {
            removeRegistro = (sys->inoculacoes[i].data_inoculacao.dia == dia &&
                                sys->inoculacoes[i].data_inoculacao.mes == mes &&
                                sys->inoculacoes[i].data_inoculacao.ano == ano);
        }
        if (removeRegistro && loteFiltro) {
            removeRegistro = (strcmp(sys->inoculacoes[i].lote_utilizado->lote, idLote) == 0);
        }
        if (removeRegistro) {
            free(sys->inoculacoes[i].nome_utente);
            for (int j = i; j < sys->num_inoculacoes - 1; j++)
                sys->inoculacoes[j] = sys->inoculacoes[j + 1];
            sys->num_inoculacoes--;
            totalApagadas++;
        } else {
            i++;
        }
    }
    return totalApagadas;
}

/**
 * @brief Apaga as inoculações de um utente
 * @param sys Ponteiro para o sistema
 * @param entrada_dados Linha de entrada
 */
static void apaga_aplicacoes(Sys *sys, char *entrada_dados) {
    char nomeUtente[BUFMAX], idLote[BUFMAX];
    int dia = 0, mes = 0, ano = 0, dataFiltro = 0, loteFiltro = 0;
    if (!extraiArgumentos(entrada_dados, nomeUtente, &dia, &mes, &ano, &dataFiltro, idLote, &loteFiltro))
        return;
    if (dataFiltro && !filtroValidaData(sys, dia, mes, ano))
        return;
    if (loteFiltro && !filtroValidaLote(sys, idLote))
        return;
    if (!verificaUtente(sys, nomeUtente))
        return;
    int totalApagadas = removeInoculacoes(sys, nomeUtente, dataFiltro, dia, mes, ano, loteFiltro, idLote);
    printf("%d\n", totalApagadas);
}

/**
 * @brief Extrai o nome do utente da entrada
 * @param entrada_dados Linha de entrada
 * @param utente Buffer para o nome do utente
 * @return 1 se o nome for extraído com sucesso, 0 caso contrário
 */
static int obtemNome(char *entrada_dados, char utente[BUFMAX]) {
    char *i = entrada_dados + 1;
    i = poupaMemória(i);
    if (*i == '\0')
        return 0;
    if (*i == '"') {
        i++;
        char *fim = strchr(i, '"');
        size_t len = fim ? (size_t)(fim - i) : strlen(i);
        if (len >= BUFMAX)
            len = BUFMAX - 1;
        strncpy(utente, i, len);
        utente[len] = '\0';
    } else {
        if (sscanf(i, "%s", utente) != 1)
            utente[0] = '\0';
    }
    return 1;
}

/**
 * @brief Imprime as inoculações registadas, com opção de filtrar por utente
 * @param sys Ponteiro para o sistema
 * @param filtra Flag que indica se deve filtrar por utente
 * @param utente Nome do utente para filtrar
 */
static void imprimeInoculacoes(Sys *sys, int filtra, const char utente[BUFMAX]) {
    int contador = 0;
    for (int i = sys->num_inoculacoes - 1; i >= 0; i--) {
        if (filtra && strcmp(sys->inoculacoes[i].nome_utente, utente) != 0)
            continue;
        printf("%s %s %02d-%02d-%d\n",
               sys->inoculacoes[i].nome_utente,
               sys->inoculacoes[i].lote_utilizado->lote,
               sys->inoculacoes[i].data_inoculacao.dia,
               sys->inoculacoes[i].data_inoculacao.mes,
               sys->inoculacoes[i].data_inoculacao.ano);
        contador++;
    }
    if (filtra && contador == 0)
        printf("%s: %s\n", utente, sys->erro->msg_utente_inexistente);
}

/**
 * @brief Lista as inoculações registadas, filtrando por utente se indicado
 * @param sys Ponteiro para o sistema
 * @param entrada_dados Linha de entrada
 */
static void lista_inoculacoes(Sys *sys, char *entrada_dados) {
    char utente[BUFMAX];
    int filtra = obtemNome(entrada_dados, utente);
    imprimeInoculacoes(sys, filtra, utente);
}

/**
 * @brief Lê a data de entrada
 * @param entrada_dados Linha de entrada
 * @param dia Ponteiro para o dia
 * @param mes Ponteiro para o mês
 * @param ano Ponteiro para o ano
 * @return 1 se a data for lida corretamente, 0 se os dados estiverem incorretos, -1 se não houver dados
 */
static int leDataEntrada(char *entrada_dados, int *dia, int *mes, int *ano) {
    char *i = entrada_dados + 1;
    i = poupaMemória(i);
    if (*i == '\0')
        return -1;
    if (sscanf(i, "%d-%d-%d", dia, mes, ano) != 3)
        return 0;
    return 1;
}

/**
 * @brief Avança a data atual do sistema
 * @param sys Ponteiro para o sistema
 * @param entrada_dados Linha de entrada
 */
static void avanca_to(Sys *sys, char *entrada_dados) {
    int dia, mes, ano;
    int resultado = leDataEntrada(entrada_dados, &dia, &mes, &ano);
    if (resultado == -1) {
        printf("%02d-%02d-%d\n", sys->data_atual.dia, sys->data_atual.mes, sys->data_atual.ano);
        return;
    }
    if (resultado == 0 || !validaData(dia, mes, ano, sys->data_atual, sys))
        return;
    sys->data_atual.dia = dia;
    sys->data_atual.mes = mes;
    sys->data_atual.ano = ano;
    printf("%02d-%02d-%d\n", sys->data_atual.dia, sys->data_atual.mes, sys->data_atual.ano);
}

/**
 * @brief Liberta a memória alocada para as inoculações
 * @param sys Ponteiro para o sistema
 */
static void libertaInoculacoes(Sys *sys) {
    for (int i = 0; i < sys->num_inoculacoes; i++) {
        free(sys->inoculacoes[i].nome_utente);
    }
    free(sys->inoculacoes);
}


static void altera_Nome(Sys *sys, char *entrada_dados) {
    char nomeAntigo[BUFMAX], nomeNovo[BUFMAX];
    char *p = entrada_dados + 1;
    p = poupaMemória(p);
    if (sscanf(p, "%s %s", nomeAntigo, nomeNovo) != 2)
        return;
    
    int antigo = 0;
    int novo = 0;
    for (int i = 0; i < sys->num_inoculacoes; i++) {
        if (strcmp(sys->inoculacoes[i].nome_utente, nomeAntigo) == 0)
            antigo = 1;
        if (strcmp(sys->inoculacoes[i].nome_utente, nomeNovo) == 0)
            novo = 1;
    }
    
    if (!antigo) {
        printf("%s: no such user\n", nomeAntigo);
        return;
    }
    
    if (novo) {
        printf("%s: user already exists\n", nomeNovo);
        return;
    }

    for (int i = 0; i < sys->num_inoculacoes; i++) {
        if (strcmp(sys->inoculacoes[i].nome_utente, nomeAntigo) == 0) {
            char *t = malloc(strlen(nomeNovo) + 1);
            if (t == NULL) {
                puts(sys->erro->msg_sem_memoria);
                return;
            }
            strcpy(t, nomeNovo);
            free(sys->inoculacoes[i].nome_utente);
            sys->inoculacoes[i].nome_utente = t;
        }
    }
    printf("%s\n", nomeNovo);
}


/**
 * @brief Função principal
 * @return 0 sempre que o programa terminar corretamente
 */
int main(int i, char *j[]) {
    int usa_pt = 0;
    if (i == 2 && strcmp(j[1], "pt") == 0)
        usa_pt = 1;
    
    Sys sys;
    sys.data_atual.dia = 1;
    sys.data_atual.mes = 1;
    sys.data_atual.ano = 2025;
    sys.numero_lotes = 0;
    sys.num_inoculacoes = 0;
    sys.inoculacoes = NULL;
    sys.erro = criaMensagemErro(usa_pt);
    
    char buf[BUFMAX];
    while (fgets(buf, BUFMAX, stdin)) {
        switch (buf[0]) {
            case 'q': libertaInoculacoes(&sys); free(sys.erro); return 0;
            case 'c': cria_lote(&sys, buf); break;
            case 'l': lista_vacinas(&sys, buf); break;
            case 'a': aplica_vacina(&sys, buf); break;
            case 'r': retira_vacina(&sys, buf); break;
            case 'd': apaga_aplicacoes(&sys, buf); break;
            case 'u': lista_inoculacoes(&sys, buf); break;
            case 't': avanca_to(&sys, buf); break;
            case 'm': altera_Nome(&sys, buf); break;
        }
    }
    libertaInoculacoes(&sys);
    free(sys.erro);
    return 0;
}