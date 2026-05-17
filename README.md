# SFX Core - API Gateway de Alta Performance & Ecossistema Multilinguagem

Bem-vindo ao **SFX Core**, a infraestrutura de backend assíncrona projetada para o projeto integrado de São Francisco Xavier. Este projeto não é apenas mais um servidor web comum. O coração dele foi construído em **C puro (ISO C11)** rodando diretamente sobre o Kernel do Linux através do motor assíncrono `epoll`. O resultado? Uma arquitetura capaz de aguentar mais de 4.000 usuários simultâneos em uma única thread, consumindo o mínimo de memória RAM.

> 🚀 **O Grande Diferencial:** O API Gateway funciona como um **"Cano Cego" (Transparent Proxy)**. Ele não quer saber de lógica de negócios. Ele apenas recebe requisições do front-end e repassa os bytes brutos (JSON) para microsserviços locais. **Isso significa que você pode construir a sua funcionalidade na linguagem que você mais dominar!**

---

## 🏗️ Arquitetura do Projeto

O projeto utiliza **Vertical Slice Architecture (VSA)** combinada com orientação a microsserviços via **IPC (Inter-Process Communication)** utilizando Unix Domain Sockets.

*   **O Core (Gateway em C):** Gerencia a rede, valida rotas através de uma Whitelist, entrega arquivos estáticos (HTML/CSS/JS) com performance máxima usando a System Call `sendfile` (Zero-Copy no Kernel do Linux) e roteia tráfego dinâmico.
*   **As Slices (`application/`):** Cada pasta dentro de `application` representa uma funcionalidade de negócio isolada (ex: `local_news`). Ela contém seu próprio front-end, seu banco de dados (PostgreSQL) e o seu próprio executável/serviço independente.

---

## 🤝 Como Contribuir (Escolha sua Linguagem!)

Você quer criar uma funcionalidade para o portal de São Francisco Xavier? Excelente! Você **não** precisa mexer no código em C do Gateway. Você só precisa criar um microsserviço que fique escutando em um arquivo de socket (ex: `/tmp/meu_servico.sock`) e processar o JSON que o Gateway vai te enviar.

Veja como a sua linguagem favorita conversa com o nosso Gateway:

*   **C# (.NET 8/10):** Você pode usar o Kestrel configurando-o para ouvir em um Unix Socket em vez de uma porta TCP (`options.ListenUnixSocket("/tmp/meu_servico.sock")`).
*   **Python:** Use a biblioteca nativa `socket` com a família `AF_UNIX`.
*   **Node.js / Go / Rust:** Todos possuem suporte nativo na biblioteca padrão para abrir servidores usando Unix Sockets.

### Passo a Passo para Criar sua Feature:

1.  Crie uma pasta para a sua feature dentro de `application/seu_recurso/`.
2.  Desenvolva o seu front-end (HTML/CSS/JS) e salve lá dentro.
3.  Crie o seu microsserviço (na linguagem que quiser) para rodar em background ouvindo o socket `/tmp/seu_recurso.sock`.
4.  Abra o arquivo `routes.txt` na raiz do projeto e registre os seus caminhos:

```text
GET  /seu_recurso     = ./application/seu_recurso/index.html
GET  /recurso.js      = ./application/seu_recurso/index.js
POST /api/seu_recurso = /tmp/seu_recurso.sock
```

Pronto! O Gateway vai entregar o seu HTML e, quando o front-end disparar um POST, os dados vão cair direto no seu microsserviço.

---

## 🛠️ Como Rodar o Core Localmente (No Host Linux)

Como o projeto utiliza recursos nativos do Kernel Linux (epoll), ele deve ser executado nativamente em ambiente Linux (ou via WSL2).

### Pré-requisitos (Exemplo para Arch/EndeavourOS):

```bash
sudo pacman -S gcc make postgresql
```

### 1. Clonar e Compilar o Gateway:

O projeto possui um gerenciamento de build incremental automatizado via Makefile.

```bash
# Compilar o API Gateway
make

# Limpar arquivos de build antigos
make clean
```

### 2. Inicializar o Banco de Dados Local (PostgreSQL):

Caso sua slice utilize persistência (como a `local_news`), certifique-se de que o serviço do Postgres está ativo e a base de dados criada:

```bash
sudo systemctl enable --now postgresql
psql -d sfx_db -U seu_usuario -f application/local_news/local_news.sql
```

### 3. Executar a Infraestrutura:

Abra o microsserviço da slice em uma aba do terminal e o Gateway na outra:

```bash
# Executar o microsserviço de notícias
gcc application/local_news/local_news.c -o local_news_service -lpq
./local_news_service

# Executar o API Gateway (Em outra aba)
./sfx_server
```

Agora basta abrir o seu navegador em `http://localhost:8080/local_news` e ver o ecossistema modular funcionar integrado!

---

## 🛡️ Padrões de Qualidade Aplicados

Para garantir que o código em C seja estável e imune a falhas críticas comuns de segurança, o desenvolvimento do Core segue as diretrizes do SEI CERT C Coding Standard e boas práticas do MISRA C, garantindo tipagem estrita (`stdint.h`), tratamento obsessivo de retornos de funções e blindagem contra estouro de buffers de memória (Buffer Overflow).
