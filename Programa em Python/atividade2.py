import smbus2
import bme280
import time
import os
import RPi_I2C_driver

def requesitaDadosSensor(bus, endereco, calibracao_paramentros):
    dado = bme280.sample(bus, endereco, calibracao_paramentros)

    return round(dado.temperature, 2), round(dado.humidity, 2), round(dado.pressure, 2)

def imprimeLCD(temperatura, umidade, pressAtm):
    # mylcd.lcd_clear()
    mylcd.lcd_display_string("T:" + str(temperatura), 1)
    mylcd.lcd_display_string("U:" + str(umidade) + " P:" + str(pressAtm), 2)

if __name__ == "__main__":
    porta_i2c = 1
    endereco = 0x76
    bus = smbus2.SMBus(porta_i2c)

    calibracao_paramentros = bme280.load_calibration_params(bus, endereco)
    mylcd = RPi_I2C_driver.lcd()

    while True:
        temperatura, umidade, pressAtm = requesitaDadosSensor(bus, endereco, calibracao_paramentros)
        imprimeLCD(temperatura, umidade, pressAtm)

        time.sleep(1)