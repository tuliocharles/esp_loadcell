## Componente `esp_loadcell` para ESP-IDF

## Introdução 
O `esp_loadcell` é um componente desenvolvido para o framework **ESP-IDF** (Espressif IoT Development Framework) projetado para a leitura e gerenciamento de conversores ADC de alta precisão dedicados a células de carga, com suporte aos CIs:

- **[HX711](https://www.alldatasheet.com/datasheet-pdf/pdf/1132222/AVIA/HX711.html)**
- **[ADS1232](https://www.alldatasheet.com/datasheet-pdf/pdf/618977/TI1/ADS1232.html)**
---

## Cabeçalho `esp_loadcell.h`

### Enums

#### `loadcell_adc_type_t`
Identifica o CI conversor utilizado na aplicação.

```c
typedef enum {
    ADS1232 = 0,
    HX711
} loadcell_adc_type_t;
```

| Enum | Valor | Descrição |
| :--- | :---: | :--- |
| `ADS1232` | `0` | Seleciona o conversor ADS1232. |
| `HX711` | `1` | Seleciona o conversor HX711. |

---

### Configs

#### `esp_loadcell_config_t`
Estrutura contendo a configuração de pinos GPIO e parâmetros funcionais necessários para inicialização.

```c
typedef struct
{
    gpio_num_t   dout;
    gpio_num_t   pd_sck;
    gpio_num_t   speed_io;
    gpio_num_t   gain0_io;
    gpio_num_t   gain1_io;
    gpio_num_t   pdwn_io;
    gpio_num_t   a0_io;
    uint8_t      speed;
    loadcell_    adc_type_t type;
    uint32_t     gain;
} esp_loadcell_config_t;
```

**Campos:**
- `dout`     (`gpio_num_t`): Pino de dados brutos vindo do conversor (*Data Out*).
- `pd_sck`   (`gpio_num_t`): Pino de entrada de clock e controle de Power Down.
- `speed_io` (`gpio_num_t`): Pino de seleção de velocidade no ADS1232 (`1` = 80Hz, `0` = 10Hz). *(Não utilizado no HX711)*.
- `gain0_io` (`gpio_num_t`): Pino Bit 0 de seleção de ganho do ADS1232. *(Não utilizado no HX711)*.
- `gain1_io` (`gpio_num_t`): Pino Bit 1 de seleção de ganho do ADS1232. *(Não utilizado no HX711)*.
- `pdwn_io`  (`gpio_num_t`): Pino de desativação/Power Down do ADS1232. *(Não utilizado no HX711)*.
- `a0_io`    (`gpio_num_t`): Pino de seleção de canal no ADS1232 (`1` = Canal 2, `0` = Canal 1). *(Não utilizado no HX711)*.
- `speed`    (`uint8_t`): Definição de taxa de amostragem inicial.
- `type`     (`loadcell_adc_type_t`): Tipo do CI conversor (`ADS1232` ou `HX711`).
- `gain`     (`uint32_t`): Quantidade de pulsos extras de clock para configuração de ganho/canal no HX711.

---

### Handles

#### `esp_loadcell_handle_t`
Ponteiro para a estrutura interna de controle do periférico.

```c
typedef struct esp_loadcell_t *esp_loadcell_handle_t;
```

---

### Funções

#### `init_esp_loadcell`

Inicializa a instância do driver, aloca memória, configura os pinos GPIO e registra a interrupção ISR no pino `DOUT`.

```c
esp_err_t init_esp_loadcell(esp_loadcell_config_t *config, esp_loadcell_handle_t *handle);
```

- **Parâmetros:**
  - `config` (`esp_loadcell_config_t`): Ponteiro para a estrutura de configuração `esp_loadcell_config_t`.
  - `handle` (`esp_loadcell_handle_t`): Ponteiro onde será retornado o handle da instância criada.
- **Retornos:**
  - `ESP_OK`: Inicialização bem-sucedida.
  - `ESP_ERR_INVALID_ARG`: Ponteiro nulo passado em `config` ou `handle`.

---

#### `esp_loadcell_read`

Habilita a interrupção no pino `DOUT`, aguarda o sinal de pronto via semáforo (timeout de 500ms) e executa a leitura serial dos 24 bits de dados do conversor.

```c
esp_err_t esp_loadcell_read(esp_loadcell_handle_t handle, int32_t *data_read);
```

- **Parâmetros:**
  - `handle` (`esp_loadcell_handle_t`): Handle da instância iniciada da célula de carga.
  - `data_read` (`int32_t`): Ponteiro onde o valor inteiro com sinal de 32 bits lido será armazenado.
- **Retornos:**
  - `ESP_OK`: Leitura realizada com sucesso.
  - `ESP_FAIL` ou `-ESP_FAIL`: Falha no timeout do semáforo ou erro na leitura.

---

#### `esp_loadcell_set_speed`

Altera o estado do pino de velocidade (`speed_io`) para modificar a taxa de conversão do ADS1232.

```c
esp_err_t esp_loadcell_set_speed(esp_loadcell_handle_t handle, uint8_t speed);
```

- **Parâmetros:**
  - `handle` (`esp_loadcell_handle_t`): Handle da instância da célula de carga.
  - `speed` (`speed`): Estado lógico (`1` para alta velocidade / 80Hz, `0` para baixa velocidade / 10Hz).
- **Retornos:**
  - `ESP_OK`: Alteração realizada com sucesso.
  - `ESP_ERR_INVALID_ARG`: Handle fornecido é nulo (`NULL`).

---

## Exemplos

### Exemplo de Aplicação Básica com HX711: [esp_loadcell_example.c](https://github.com/christopherandev/esp_loadcell/blob/main/examples/esp_loadcell_example/main/esp_loadcell_example.c)
