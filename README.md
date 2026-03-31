
# Sistema de Otimização e Monitoramento de Recursos Hídricos via IoT

Este projeto consiste em um ecossistema de hardware e software desenvolvido para o monitoramento em tempo real de níveis hídricos em reservatórios urbanos. A solução integra sensores IoT, processamento local em linguagem C e notificações automatizadas via WhatsApp.

##  Contexto Acadêmico
* **Instituição**: Universidade Federal do Maranhão (UFMA).
* **Departamento**: Centro de Ciências Exatas e Tecnologia - Engenharia da Computação.
* **Disciplina**: Sistemas Distribuídos.
* **Professor Orientador**: Luiz Henrique Neves Rodrigues.

##  Equipe
* **Emanuel Lopes Silva** (20250071189)
* **José Nunes de Sousa Neto** (20250071231)
* **Josuel Pinheiro Barros Júnior** (20250071240)
* **Louise Reis Mendes** (20250013588)
* **Raianny Cristina Ferreira da Silva** (20250013621)

---

##  Arquitetura do Sistema
O sistema é dividido em quatro camadas fundamentais para garantir a escalabilidade e a robustez dos dados:

| Camada | Responsabilidade | Entrega Principal |
| :--- | :--- | :--- |
| **Aquisição** | Sensores (Ultrassônico/Pressão) e ESP32 | Circuito montado e funcional. |
| **Processamento Local** | Firmware em C (filtragem de ruído/debounce) | Código documentado e tratamento de erros. |
| **Integração** | Middleware municipal e formatação JSON | Endpoint de recepção e banco de dados. |
| **Notificação** | Evolution API + Bot local via Docker | Instância Docker e lógica de alertas. |

---

##  Objetivos do Projeto
* **Precisão de Leitura**: Realizar leituras de nível via microcontrolador ESP32.
* **Eficiência de Firmware**: Implementar algoritmos em C para manipulação de vetores e cálculos estatísticos.
* **Comunicação Distribuída**: Transmissão de dados via protocolo HTTP/MQTT em formato JSON.
* **Infraestrutura Moderna**: Orquestração de serviços (Middleware e Evolution API) utilizando **Docker Compose**.

---

##  Cronograma Macro
O projeto segue um ciclo de vida de 14 semanas dentro do calendário acadêmico:

1. **Iniciação (1 sem)**: Requisitos e aprovação do TAP.
2. **Bibliografia (1 sem)**: Pesquisa em sensores e programação IoT.
3. **Planejamento (2 sem)**: Desenho da arquitetura e escolha de componentes.
4. **Execução (5 sem)**: Codificação em C e configuração da Evolution API.
5. **Testes (3 sem)**: Validação de métricas e envio de alertas.
6. **Encerramento (2 sem)**: Documentação técnica e relatório final.

---

##  Riscos e Plano de Contingência
* **Oxidação de Hardware**: Uso de sensores com proteção IP67.
* **Perda de Dados**: Implementação de buffer local no firmware em C para evitar falhas por instabilidade de rede.
* **Falha de Notificação**: Em caso de problemas com o WhatsApp, o sistema prevê a integração com **Telegram** ou um **Frontend simplificado**.

##  Conformidade Legal
O projeto está em total conformidade com a **Lei Geral de Proteção de Dados (LGPD)**, garantindo o tratamento seguro de números de telefone e logs de mensagens disparados pelo Bot.

---
**Responsável pela Edição atual:** Emanuel Lopes Silva
**Data de Atualização:** 30/03/2026
