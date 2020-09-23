#include <linux/i2c-dev.h>
#include <sys/ioctl.h>

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>

#include "BME280_driver/bme280.h"

#define NUMERO_VALORES_PARA_MEDIA 10

struct identifier
{
    /* Variável que contém o endereço do dispositivo */
    uint8_t enderecoDispositivo;

    /* Variável que contém o descritor do arquivo */
    int8_t fd;
};

int8_t user_i2c_read(uint8_t reg_addr, uint8_t *data, uint32_t len, void *intf_ptr);
int8_t user_i2c_write(uint8_t reg_addr, const uint8_t *data, uint32_t len, void *intf_ptr);
void user_delay_us(uint32_t period, void *intf_ptr);
int8_t stream_sensor_data_forced_mode(struct bme280_dev *dev);
int8_t stream_sensor_data_normal_mode(struct bme280_dev *dev);
void print_sensor_data(struct bme280_data *comp_data);

int main(int argc, char *argv[])
{
    struct bme280_dev dispositivo;
    struct identifier identificador;

    int8_t rslt = BME280_OK;

    char i2cInterface[] = "/dev/i2c-1";

    if ((identificador.fd = open(i2cInterface, O_RDWR)) < 0)
    {
        fprintf(stderr, "Erro ao abrir o barramento i2c %s\n", i2cInterface);
        exit(1);
    }

    /* ME280_I2C_ADDR_PRIM : Referente ao endereço 0x76 */
    identificador.enderecoDispositivo = BME280_I2C_ADDR_PRIM;

    if (ioctl(identificador.fd, I2C_SLAVE, identificador.enderecoDispositivo) < 0)
    {
        fprintf(stderr, "Erro ao acessar o barramento e/ou se comunicar com o dispositivo.\n");
        exit(1);
    }

    /* Interface I2C */
    dispositivo.intf = BME280_I2C_INTF;
    dispositivo.read = user_i2c_read;
    dispositivo.write = user_i2c_write;
    dispositivo.delay_us = user_delay_us;

    /* Ponteiro da interface */
    dispositivo.intf_ptr = &identificador;

    /* Inicializa a bme280 */
    rslt = bme280_init(&dispositivo);
    if (rslt != BME280_OK)
    {
        fprintf(stderr, "Erro ao inicializar o dispositivo (código: %+d).\n", rslt);
        exit(1);
    }

    rslt = stream_sensor_data_normal_mode(&dispositivo);
    if (rslt != BME280_OK)
    {
        fprintf(stderr, "Erro ao transmitir dados do sensor (código: %+d).\n", rslt);
        exit(1);
    }

    return 0;
}

int8_t user_i2c_read(uint8_t reg_addr, uint8_t *data, uint32_t len, void *intf_ptr)
{
    struct identifier identificador;

    identificador = *((struct identifier *)intf_ptr);

    write(identificador.fd, &reg_addr, 1);
    read(identificador.fd, data, len);

    return 0;
}

int8_t user_i2c_write(uint8_t reg_addr, const uint8_t *data, uint32_t len, void *intf_ptr)
{
    uint8_t *buf;
    struct identifier identificador;

    identificador = *((struct identifier *)intf_ptr);

    buf = malloc(len + 1);
    buf[0] = reg_addr;
    memcpy(buf + 1, data, len);
    if (write(identificador.fd, buf, len + 1) < (uint16_t)len)
    {
        return BME280_E_COMM_FAIL;
    }

    free(buf);

    return BME280_OK;
}

void user_delay_us(uint32_t period, void *intf_ptr)
{
    usleep(period);
}

void defineDataHoraAtual(char **dataHora)
{
    char cur_time[128];
    *dataHora = malloc(128);

    time_t t;
    struct tm *ptm;

    t = time(NULL);
    ptm = localtime(&t);

    strftime(cur_time, 128, "%d/%m/%Y %H:%M:%S", ptm);

    strcpy(*dataHora, cur_time);
}

void salva_dados(char nome_do_arquivo[], struct bme280_data *comp_data)
{
    float temp, press, hum;
    FILE *file = fopen(nome_do_arquivo, "a");

#ifdef BME280_FLOAT_ENABLE
    temp = comp_data->temperature;
    press = 0.01 * comp_data->pressure;
    hum = comp_data->humidity;
#else
#ifdef BME280_64BIT_ENABLE
    temp = 0.01f * comp_data->temperature;
    press = 0.0001f * comp_data->pressure;
    hum = 1.0f / 1024.0f * comp_data->humidity;
#else
    temp = 0.01f * comp_data->temperature;
    press = 0.01f * comp_data->pressure;
    hum = 1.0f / 1024.0f * comp_data->humidity;
#endif
#endif

    char *dataHora;
    defineDataHoraAtual(&dataHora);
    fprintf(file, "%0.2lf,%0.2lf,%0.2lf,%s\r\n", temp, press, hum, dataHora);
    fclose(file);
}

void cria_arquivo(char nome_do_arquivo[])
{
    FILE *file = fopen(nome_do_arquivo, "w");
    fprintf(file, "Temperatura,Pressao,Umidade,DataHora\n");
    fclose(file);
}

void calculaMediaValores(float temperaturas[], float pressoes[], float umidades[], struct bme280_data *comp_data)
{
    float temp = 0.0f, press = 0.0f, umi = 0.0f;
    for (int i = 0; i < NUMERO_VALORES_PARA_MEDIA; i++)
    {
        temp += temperaturas[i];
        press += pressoes[i];
        umi += umidades[i];
    }

    comp_data->temperature = temp / NUMERO_VALORES_PARA_MEDIA;
    comp_data->pressure = press / NUMERO_VALORES_PARA_MEDIA;
    comp_data->humidity = umi / NUMERO_VALORES_PARA_MEDIA;
}

void print_sensor_data(struct bme280_data *comp_data)
{
    float temp, press, hum;

#ifdef BME280_FLOAT_ENABLE
    temp = comp_data->temperature;
    press = 0.01 * comp_data->pressure;
    hum = comp_data->humidity;
#else
#ifdef BME280_64BIT_ENABLE
    temp = 0.01f * comp_data->temperature;
    press = 0.0001f * comp_data->pressure;
    hum = 1.0f / 1024.0f * comp_data->humidity;
#else
    temp = 0.01f * comp_data->temperature;
    press = 0.01f * comp_data->pressure;
    hum = 1.0f / 1024.0f * comp_data->humidity;
#endif
#endif
    printf("%0.2lf deg C, %0.2lf hPa, %0.2lf%%\n", temp, press, hum);
}

int8_t stream_sensor_data_normal_mode(struct bme280_dev *dev)
{
    int8_t rslt;
    uint8_t settings_sel;
    struct bme280_data comp_data;

    /* Recommended mode of operation: Indoor navigation */
    dev->settings.osr_h = BME280_OVERSAMPLING_1X;
    dev->settings.osr_p = BME280_OVERSAMPLING_16X;
    dev->settings.osr_t = BME280_OVERSAMPLING_2X;
    dev->settings.filter = BME280_FILTER_COEFF_16;
    dev->settings.standby_time = BME280_STANDBY_TIME_62_5_MS;

    settings_sel = BME280_OSR_PRESS_SEL;
    settings_sel |= BME280_OSR_TEMP_SEL;
    settings_sel |= BME280_OSR_HUM_SEL;
    settings_sel |= BME280_STANDBY_SEL;
    settings_sel |= BME280_FILTER_SEL;
    rslt = bme280_set_sensor_settings(settings_sel, dev);
    rslt = bme280_set_sensor_mode(BME280_NORMAL_MODE, dev);

    int contaSegundos = 1;
    char nome_do_arquivo[] = "dados_sensor.csv";
    float temperaturas[NUMERO_VALORES_PARA_MEDIA], pressoes[NUMERO_VALORES_PARA_MEDIA], umidades[NUMERO_VALORES_PARA_MEDIA];
    cria_arquivo(nome_do_arquivo);

    printf("Gravando dados do sensor no arquivo: %s\r\n", nome_do_arquivo);
    while (1)
    {
        /* Espera 1 segundo para realizar a medição */
        dev->delay_us(1000000, dev->intf_ptr);
        rslt = bme280_get_sensor_data(BME280_ALL, &comp_data, dev);
        temperaturas[contaSegundos - 1] = comp_data.temperature;
        pressoes[contaSegundos - 1] = comp_data.pressure;
        umidades[contaSegundos - 1] = comp_data.humidity;
        // print_sensor_data(&comp_data);
        if (contaSegundos == NUMERO_VALORES_PARA_MEDIA)
        {
            calculaMediaValores(temperaturas, pressoes, umidades, &comp_data);
            salva_dados(nome_do_arquivo, &comp_data);
            contaSegundos = 1;
        }
        else
        {
            contaSegundos++;
        }
    }

    return rslt;
}