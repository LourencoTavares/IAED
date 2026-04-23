# Sistema de Gestão de Vacinas (IAED)

Projeto desenvolvido no âmbito da unidade curricular de **Introdução aos Algoritmos e Estruturas de Dados (2024/2025)**.

## Descrição

Este projeto consiste na implementação de um sistema de gestão de vacinas e inoculações, permitindo registar lotes de vacinas, aplicar doses a utentes e gerir o histórico de aplicações. 

A interação com o programa é feita através de comandos no terminal, seguindo um formato específico.

---

## Objetivo

Desenvolver um programa em linguagem C que:
- Gere lotes de vacinas (nome, lote, validade, doses)
- Registe inoculações em utentes
- Permita consulta e remoção de dados
- Simule a passagem do tempo
- Respeite restrições de memória e estrutura do enunciado

---

## Funcionalidades Implementadas

- Criação e gestão de lotes de vacinas  
- Aplicação de doses a utentes  
- Listagem de vacinas e aplicações  
- Remoção de lotes e registos  
- Gestão de datas e validade  
- Sistema de comandos via terminal  

---

## Comandos principais

- `c` → criar lote de vacina  
- `l` → listar vacinas  
- `a` → aplicar vacina  
- `r` → remover lote  
- `d` → apagar registos  
- `u` → listar aplicações  
- `t` → avançar tempo  
- `q` → sair  

---

## Estrutura do Projeto

- `project.c` → código principal  
- `enunciado.md` → descrição do projeto  

---

## Como compilar e depois executar

``gcc -O3 -Wall -Wextra -Werror -Wno-unused-result -o proj *.c`` para compilar e de seguida, ``./proj`` para executar.

---

## Exemplo de utilização

``
- ``c A0C0 31-7-2025 210 malaria``
- ``l``
- ``a "Joao Miguel" malaria``
- ``u``
- ``q``
