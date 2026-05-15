# S.A.F.E. (Smart Automated Fire Extinguisher)

![STM32](https://img.shields.io/badge/STM32-F303-blue?style=for-the-badge&logo=stmicroelectronics)
![C](https://img.shields.io/badge/Language-C-00599C?style=for-the-badge&logo=c)
![Hardware](https://img.shields.io/badge/Hardware-Embedded-red?style=for-the-badge)

**S.A.F.E.** è un sistema embedded real-time progettato per rilevare autonomamente la presenza di fiamme, puntare una torretta motorizzata verso l'origine del fuoco e attivare una pompa ad acqua per l'estinzione. 

Sviluppato su architettura **ARM Cortex-M4 (STM32F303)**, il progetto si distingue per la sua logica software completamente non-bloccante e reattiva alle emergenze.

---

## Caratteristiche Principali

* **Modalità "Radar" di Pattugliamento:** In assenza di minacce, la torretta spazzola l'area coprendo un campo visivo di 120° (da 30° a 150°) calcolando dinamicamente il segnale PWM.
* **Targeting Istantaneo (Zero-Delay):** Grazie alla lettura ad alta frequenza dei sensori a 5 vie, il sistema interrompe il pattugliamento e punta il bersaglio in frazioni di secondo.
---

## Componenti Hardware

* **Scheda di Sviluppo:** STM32F3Discovery (Microcontrollore STM32F303VCT6 operante a 48 MHz).
* **Attuatore di Puntamento:** Servomotore SG90 (Pilotato via Timer Hardware a 50Hz).
* **Sensori Visivi:** Modulo Sensore di Fiamma IR a 5 canali (FOV ~120°).
* **Attuatore Estinguente:** Pompa d'acqua ad immersione pilotata tramite modulo Relè 5V.
* **Input Utente:** 2x Pulsanti tattili esterni (Breadboard) + 1x USER Button integrato (PA0).

---

## Architettura di Sistema (Pinout e Periferiche)

Il progetto è stato configurato tramite **STM32CubeMX** sfruttando le librerie **HAL**.

| Periferica | Pin / Risorsa | Configurazione | Funzione |
| :--- | :--- | :--- | :--- |
| **PWM Output** | `TIM2_CH1` | Prescaler: 47, ARR: 19999 | Generazione onda quadra a 50Hz (1MHz tick) per il servomotore |
| **Sensori Fiamma** | `GPIO_Input` (x5) | No Pull-up/down | Lettura digitale (0/1) per le 5 direzioni spaziali |
| **Relè Pompa** | `GPIO_Output` | Push-Pull | Attivazione circuito di potenza estinguente |
| **Pulsanti Manuali**| `PA1`, `PA2` | Internal Pull-Up | Input operatore (Active Low) per rotazione manuale |
| **Tasto USER** | `PA0` | External Pull-Down (Nativo)| Modalità Test/Reset (Active High) |
| **LED di Stato** | `PE8` | Push-Pull | Feedback visivo blu di sistema armato |

---

## Come eseguire il progetto

1. **Clona la repository:**
   ```bash
   git clone [https://github.com/Aontast/SAFE-STM32.git](https://github.com/Aontast/SAFE-STM32.git)
