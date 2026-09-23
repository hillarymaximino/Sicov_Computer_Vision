# SICOV

### Sistema Inteligente de Controle Semafórico Adaptativo Baseado em Visão Computacional

Projeto de estudo e aprendizado em **Visão Computacional**, desenvolvido para o laboratório do **Assert — Campina Grande/PB**, com foco na aplicação prática de técnicas de aquisição de vídeo, detecção de objetos, processamento de imagens e controle semafórico adaptativo.

---

## Sobre o projeto

O **SICOV** — Sistema Inteligente de Controle Semafórico Adaptativo Baseado em Visão Computacional — tem como proposta estudar e desenvolver uma arquitetura capaz de observar o fluxo de veículos em diferentes aproximações de um cruzamento e utilizar essas informações para auxiliar na definição dos tempos das fases semafóricas.

O projeto utiliza câmeras para aquisição das imagens, um modelo da família **YOLO** para detecção e classificação dos veículos e uma camada de processamento responsável por estimar a ocupação das regiões monitoradas.

A estimativa produzida pela visão computacional é então enviada para uma camada de **controle determinístico**, responsável pela tomada de decisão sobre as fases do semáforo.

> **A inteligência artificial não comanda diretamente o semáforo.**
>
> A IA fornece uma estimativa da demanda observada, enquanto o controlador determina os tempos das fases respeitando os limites e regras de segurança definidos para o sistema.

---

## Objetivo

O principal objetivo do projeto é estudar e implementar uma malha de processamento composta por:

```text
Câmeras
   │
   ▼
Aquisição de vídeo
   │
   ▼
Detecção de veículos
   │
   ▼
Aplicação das ROIs
   │
   ▼
Contagem e classificação
   │
   ▼
Filtragem temporal
   │
   ▼
Estimativa de ocupação
   │
   ▼
Controlador semafórico
   │
   ▼
Tempos das fases
   │
   ▼
Dashboard
```

A proposta é desenvolver cada etapa de forma modular, permitindo que os componentes sejam estudados, testados e posteriormente integrados.

---

## Equipe

### Alunos participantes

* **Hillary Maximino**
* **Guilherme Gonçalves**
* **Leticia Vitória**

### Orientação

**Professor Orientador:**
Fagner de Araujo Pereira

**Aluno Orientador:**
Luiz Neto

### Local

**Laboratório do Assert — Campina Grande, Paraíba, Brasil**

---

# Escopo do projeto

O desenvolvimento está organizado em três grandes desafios técnicos:

### 1. Aquisição de vídeo

Responsável por estabelecer a comunicação com as câmeras e disponibilizar os frames em um formato comum para as demais camadas do sistema.

O projeto prevê três câmeras:

| Câmera   | Modelo                  | Comunicação       | Característica |
| -------- | ----------------------- | ----------------- | -------------- |
| Câmera 1 | Daheng MER2-302-37GM-P  | GigE Vision / PoE | Monocromática  |
| Câmera 2 | Daheng MER2-302-37GM-P  | GigE Vision / PoE | Monocromática  |
| Câmera 3 | Intelbras VIP 3430 B IA | RTSP / ONVIF      | Colorida / IR  |

As câmeras Daheng possuem resolução de **2048 × 1536 @ 37 FPS**, enquanto a câmera Intelbras possui resolução de **4 MP**.

A camada de aquisição deve abstrair as diferenças entre os protocolos, entregando os frames de forma padronizada para o módulo de visão.

O processamento previsto é de aproximadamente **10 FPS por câmera**, não sendo necessário processar todos os frames produzidos pelos dispositivos.

---

### 2. Detecção e contagem por ROI

A segunda etapa concentra os componentes relacionados à visão computacional.

O sistema deverá utilizar um modelo da família **YOLO** para detectar os veículos presentes nas imagens.

As classes consideradas pelo projeto são:

* Automóvel
* Motocicleta
* Ônibus
* Caminhão

Cada aproximação deverá possuir uma **Região de Interesse (ROI)** própria.

A contagem deverá considerar somente os veículos localizados dentro da ROI correspondente.

```text
Frame da câmera
       │
       ▼
     YOLO
       │
       ▼
Detecções
       │
       ▼
      ROI
       │
       ▼
Veículos válidos
       │
       ├── Automóveis
       ├── Motocicletas
       ├── Ônibus
       └── Caminhões
       │
       ▼
Contagem
       │
       ▼
Filtragem temporal
       │
       ▼
Estimativa de ocupação
```

A filtragem temporal será utilizada para reduzir oscilações nas detecções e evitar que uma falha isolada provoque alterações indevidas na estimativa utilizada pelo controlador.

---

### 3. Controle semafórico e interface

A terceira etapa recebe os dados de alto nível produzidos pela camada de visão.

Entre as camadas de visão e controle não deve ser transmitido o vídeo bruto. A comunicação deve utilizar informações como:

```text
Aproximação 1
├── Automóveis: X
├── Motocicletas: X
├── Ônibus: X
├── Caminhões: X
└── Ocupação: X

Aproximação 2
├── Automóveis: X
├── Motocicletas: X
├── Ônibus: X
├── Caminhões: X
└── Ocupação: X

Aproximação 3
├── Automóveis: X
├── Motocicletas: X
├── Ônibus: X
├── Caminhões: X
└── Ocupação: X
```

O controlador deverá utilizar essas estimativas para determinar as fases seguintes, respeitando:

* tempo mínimo de verde;
* tempo máximo de verde;
* período de amarelo;
* vermelho de segurança;
* bloqueio de movimentos conflitantes.

O sistema também deverá possuir uma interface para visualização do estado atual dos semáforos e da ocupação das aproximações.

---

# Tecnologias

A stack prevista para o desenvolvimento inclui:

### Backend e controle

* **C++17**
* **CMake**
* Multithreading nativo
* Algoritmo semafórico determinístico

### Visão computacional e IA

* **OpenCV**
* **NVIDIA CUDA**
* **YOLO11**
* ONNX
* Filtragem temporal
* Regiões de Interesse (ROI)

### Aquisição

* **GigE Vision**
* GenICam
* Daheng SDK
* RTSP
* ONVIF
* H.264
* OpenCV / GStreamer

### Interface

* Dashboard para visualização:

  * estado dos semáforos;
  * ocupação das aproximações;
  * informações produzidas pelo sistema.

A tecnologia definitiva do frontend será definida durante o desenvolvimento.

---

# Arquitetura

O projeto busca manter uma arquitetura modular composta principalmente por três camadas:

```text
┌──────────────────────────────────────────────┐
│                 AQUISIÇÃO                    │
│                                              │
│  Daheng 1 ─┐                                 │
│  Daheng 2 ─┼──► Frames padronizados           │
│  Intelbras ┘                                 │
└──────────────────────┬───────────────────────┘
                       │
                       ▼
┌──────────────────────────────────────────────┐
│              VISÃO COMPUTACIONAL             │
│                                              │
│  YOLO → Detecção → ROI → Contagem            │
│                    → Filtragem → Ocupação    │
└──────────────────────┬───────────────────────┘
                       │
              Dados de alto nível
                       │
                       ▼
┌──────────────────────────────────────────────┐
│                CONTROLE                      │
│                                              │
│  Ocupação → Algoritmo → Fases semafóricas    │
│                                              │
│  + limites mínimos/máximos                   │
│  + amarelo                                   │
│  + vermelho de segurança                     │
│  + conflitos                                 │
└──────────────────────┬───────────────────────┘
                       │
                       ▼
┌──────────────────────────────────────────────┐
│                 DASHBOARD                    │
│                                              │
│  Estado dos semáforos + ocupação das vias    │
└──────────────────────────────────────────────┘
```

Essa separação permite que cada módulo seja desenvolvido e testado individualmente antes da integração completa.

---

# Infraestrutura

A primeira etapa do projeto envolve a preparação da bancada de execução.

Entre os componentes previstos estão:

* Máquina de processamento;
* GPU;
* Drivers;
* NVIDIA CUDA;
* Ambiente de desenvolvimento C++;
* Rede para comunicação com as câmeras;
* Switch ou injetor PoE;
* Lentes C-mount para as câmeras industriais.

Também é considerada como possível evolução futura a migração para uma plataforma **Edge AI** com SoC ARM, decodificador de vídeo em hardware e NPU dedicada.

Essa migração, entretanto, é tratada como uma evolução do projeto e não como requisito da etapa inicial.

---

# Organização do desenvolvimento

O backlog do projeto está organizado em seis sprints de 15 dias.

| Sprint | Tema                            |
| ------ | ------------------------------- |
| S1     | Infraestrutura de bancada       |
| S2     | Aquisição de vídeo              |
| S3     | Detecção e contagem por ROI     |
| S4     | Controle semafórico e interface |
| S5     | Integração e demonstração       |
| S6     | Documentação e governança       |

### Sprint 1 — Infraestrutura

Preparação da máquina, GPU, CUDA, ambiente C++, rede, PoE e testes iniciais das câmeras.

### Sprint 2 — Aquisição

Implementação da comunicação com as duas câmeras Daheng via GigE Vision e com a câmera Intelbras via RTSP/ONVIF.

### Sprint 3 — Visão computacional

Integração do YOLO, detecção dos veículos, definição das ROIs, classificação, contagem e filtragem temporal.

### Sprint 4 — Controle e interface

Implementação do controlador semafórico e desenvolvimento do dashboard.

### Sprint 5 — Integração

Integração das três câmeras, visão computacional, estimativa de ocupação, controlador e dashboard.

### Sprint 6 — Documentação

Registro das decisões técnicas, parâmetros, configurações, calibrações e resultados obtidos durante o desenvolvimento.

---

# Requisitos principais

## Requisitos funcionais

* Aquisição de vídeo das três câmeras.
* Padronização dos frames.
* Processamento próximo de 10 FPS por câmera.
* Detecção de veículos utilizando YOLO.
* Classificação em automóvel, motocicleta, ônibus e caminhão.
* Definição de uma ROI para cada aproximação.
* Contagem dos veículos dentro das ROIs.
* Aplicação de filtragem temporal.
* Estimativa de ocupação.
* Comunicação entre visão e controle utilizando dados de alto nível.
* Controle adaptativo das fases do semáforo.
* Respeito aos tempos mínimo e máximo.
* Implementação de amarelo e vermelho de segurança.
* Bloqueio de movimentos conflitantes.
* Dashboard para acompanhamento do sistema.
* Testes individuais dos principais módulos.
* Testes integrados com as três câmeras.

---

# Requisitos não funcionais

O projeto também considera os seguintes requisitos:

### Desempenho

Manter processamento próximo de **10 FPS por câmera**, sem necessidade de processar todos os frames produzidos pelas câmeras.

### Estabilidade

O sistema deve permanecer funcional durante a execução integrada, permitindo identificar falhas transitórias.

### Registro

Eventos relevantes e erros devem ser registrados para facilitar acompanhamento e depuração.

### Modularidade

As camadas de aquisição, visão e controle devem permanecer separadas.

### Segurança

As regras de segurança do controlador devem ser preservadas independentemente das oscilações ou alterações nas estimativas provenientes da visão computacional.

### Testabilidade

Os principais módulos devem possuir mecanismos para testes individuais.

### Evolução

A arquitetura deve permitir futuras alterações de câmeras, modelos de detecção ou plataforma de execução sem necessidade de reconstrução completa do sistema.

---

# Decisões em estudo

Algumas decisões fazem parte do próprio processo de aprendizado do projeto e serão definidas e documentadas pela equipe durante o desenvolvimento.

Entre elas:

* Como definir e calibrar cada ROI;
* Quantas medições utilizar na filtragem temporal;
* Como combinar as medições consecutivas;
* Como transformar a ocupação em tempo de verde;
* Quais serão os limites mínimo e máximo;
* Como lidar com a diferença entre imagens monocromáticas e coloridas;
* Utilizar modelo pré-treinado ou realizar fine-tuning;
* Qual formato utilizar na comunicação entre visão e controle;
* Qual tecnologia será utilizada no dashboard.

Essas decisões deverão ser registradas em `docs/decisoes-tecnicas.md`.

---

# Estrutura do projeto

```text
SICOV/
│
├── src/
│   ├── acquisition/       # Aquisição das câmeras
│   ├── vision/            # Detecção, ROI e contagem
│   ├── control/           # Controle semafórico
│   ├── interface/         # Dashboard
│   └── common/             # Componentes compartilhados
│
├── tests/                  # Testes unitários e integração
│
├── config/                 # Configurações do sistema
│   ├── cameras/
│   ├── roi/
│   └── controller.yaml
│
├── models/                 # Modelos de detecção
│
├── scripts/                # Scripts auxiliares
│
├── data/                   # Dados e amostras
│
├── logs/                   # Logs de execução
│
├── docs/                   # Documentação
│
├── assets/                 # Diagramas e imagens
│
├── CMakeLists.txt
├── .gitignore
├── LICENSE
└── README.md
```

---

# Fluxo de dados

A comunicação principal do SICOV segue o fluxo:

```text
Câmera
  │
  ▼
Aquisição
  │
  ▼
Frame padronizado
  │
  ▼
YOLO
  │
  ▼
Detecções
  │
  ▼
ROI
  │
  ▼
Contagem por classe
  │
  ▼
Filtragem temporal
  │
  ▼
Ocupação da aproximação
  │
  ▼
Controlador
  │
  ▼
Fase do semáforo
  │
  ▼
Dashboard
```

A camada de controle recebe somente informações necessárias à tomada de decisão, mantendo a independência em relação ao vídeo bruto.

---

# Classes de veículos

O sistema considera inicialmente quatro categorias:

| Classe       | Descrição   |
| ------------ | ----------- |
| `car`        | Automóvel   |
| `motorcycle` | Motocicleta |
| `bus`        | Ônibus      |
| `truck`      | Caminhão    |

A contagem total de uma aproximação é obtida a partir dos veículos válidos identificados dentro da respectiva ROI.

```text
N = carros + motos + ônibus + caminhões
```

---

# Testes

Os testes deverão ser organizados por camada.

```text
tests/
├── acquisition/
│   ├── test_daheng.cpp
│   ├── test_rtsp.cpp
│   └── test_frame.cpp
│
├── vision/
│   ├── test_detector.cpp
│   ├── test_roi.cpp
│   ├── test_counter.cpp
│   └── test_filter.cpp
│
├── control/
│   ├── test_phases.cpp
│   └── test_controller.cpp
│
└── integration/
    └── test_pipeline.cpp
```

A intenção é permitir que aquisição, detecção/contagem, controle e interface sejam validados individualmente antes dos testes integrados.

---

# Documentação

As decisões e resultados do projeto deverão ser registrados em:

```text
docs/
├── arquitetura.md
├── requisitos.md
├── decisoes-tecnicas.md
├── configuracao-bancada.md
├── workshops.md
└── backlog.md
```

A documentação deverá contemplar especialmente:

* configuração das câmeras;
* definição e calibração das ROIs;
* parâmetros da filtragem temporal;
* parâmetros do controlador;
* limites dos tempos semafóricos;
* tratamento das diferentes câmeras;
* formato de comunicação entre as camadas;
* decisões tomadas pela equipe;
* resultados dos testes.

---

# Entregáveis

Ao longo do desenvolvimento, estão previstos os seguintes entregáveis:

* Máquina de bancada montada e funcional;
* Ambiente de inferência configurado;
* Código da camada de aquisição;
* Código da camada de detecção e contagem;
* Código do controlador semafórico;
* Dashboard;
* Integração das três câmeras;
* Demonstração ao vivo;
* Documentação das decisões técnicas;
* Workshops periódicos para acompanhamento do desenvolvimento.

---

# Status

> **Em desenvolvimento**

O projeto está sendo desenvolvido como uma atividade de estudo e aprendizado, permitindo que a equipe explore progressivamente os conceitos de aquisição de imagens, visão computacional, inteligência artificial, processamento de dados e sistemas de controle.

---

# Equipe

**Hillary Maximino**
**Guilherme Gonçalves**
**Leticia Vitória**

**Professor Orientador:** Fagner de Araujo Pereira
**Aluno Orientador:** Luiz Neto

**Assert — Laboratório de Campina Grande/PB**

---

## Licença

Este repositório está destinado ao estudo, aprendizado e desenvolvimento do projeto SICOV.

A definição da licença de distribuição do código deverá ser realizada pela equipe responsável pelo projeto.
