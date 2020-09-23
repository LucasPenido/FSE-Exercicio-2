# Exercício 2 - FSE
aluno: Lucas Penido Antunes  
matrícula: 16/0013143

Repositório com a solução da atividade 2.

## Programa em C

Implementa o código em C que utiliza a biblioteca oficial do fabricante [(Bosch)](https://github.com/BoschSensortec/BME280_driver) para realizar a
medida das 3 grandezas (Temperatura, Umidade e Pressão) a cada 1 segundo e registrar, a cada 10 segundos, a média das amostras em um arquivo em formato CSV registrando data e hora de cada registro.

### Executando o programa

Entre na pasta "Programa em C" e execute o comando 

`make` 

Após a criação do executável

`./bme280`

Obs: Os dados serão armazenados no arquivo "dados_sensor.csv" na mesma pasta.


## Programa em Python

Implementa o programa em Python que lê as 3 grandezas (Temperatura, Umidade e Pressão) do
sensor BME280 e as apresenta no display utilizando 2 casas decimais e sendo atualizadas a cada
1 segundo.

### Executando o programa

Entre na pasta "Programa em Python" e execute o comando 

`python3 atividade2.py`
