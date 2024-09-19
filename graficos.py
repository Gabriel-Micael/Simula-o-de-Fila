import pandas as pd
import matplotlib.pyplot as plt

# Percentuais de ocupação
percentuais = [0.85, 0.90, 0.95, 0.99]

# Lista de arquivos
arquivos = [
    "dados_simulacao_1.txt",  # 85%
    "dados_simulacao_2.txt",  # 90%
    "dados_simulacao_3.txt",  # 95%
    "dados_simulacao_4.txt"   # 99%
]

# Inicializa listas para armazenar dados
ocupacao = []
en_final = []
ew_final = []
lambda_values = []
erro_de_little = []
tempos = []

# Lê os dados de cada arquivo
for arquivo in arquivos:
    # Lê o arquivo em um DataFrame
    df = pd.read_csv(arquivo, delim_whitespace=True)
    
    # Armazena os dados em listas
    ocupacao.append(df['Ocupacao'].tolist())
    en_final.append(df['E[N]'].tolist())
    ew_final.append(df['E[W]'].tolist())
    lambda_values.append(df['Lambda'].tolist())
    erro_de_little.append(df['Erro_de_Little'].tolist())
    tempos.append(df['Tempo_Decorrido'].tolist())

# Cria uma figura para os gráficos
plt.figure(figsize=(12, 10))

# Plota a ocupação
plt.subplot(5, 1, 1)
for i, oc in enumerate(ocupacao):
    plt.plot(tempos[i], oc, label=f'Ocupação {percentuais[i] * 100:.0f}%')
plt.title('Taxa de Ocupação')
plt.xlabel('Tempo Decorrido')
plt.ylabel('Ocupação')
plt.legend()

# Plota E[N]
plt.subplot(5, 1, 2)
for i, en in enumerate(en_final):
    plt.plot(tempos[i], en, label=f'E[N] {percentuais[i] * 100:.0f}%')
plt.title('E[N]')
plt.xlabel('Tempo Decorrido')
plt.ylabel('E[N]')
plt.legend()

# Plota E[W]
plt.subplot(5, 1, 3)
for i, ew in enumerate(ew_final):
    plt.plot(tempos[i], ew, label=f'E[W] {percentuais[i] * 100:.0f}%')
plt.title('E[W]')
plt.xlabel('Tempo Decorrido')
plt.ylabel('E[W]')
plt.legend()

# Plota Lambda
plt.subplot(5, 1, 4)
for i, lamb in enumerate(lambda_values):
    plt.plot(tempos[i], lamb, label=f'Lambda {percentuais[i] * 100:.0f}%')
plt.title('Lambda')
plt.xlabel('Tempo Decorrido')
plt.ylabel('Lambda')
plt.legend()

# Plota Erro de Little
plt.subplot(5, 1, 5)
for i, erro in enumerate(erro_de_little):
    plt.plot(tempos[i], erro, label=f'Erro de Little {percentuais[i] * 100:.0f}%')
plt.title('Erro de Little')
plt.xlabel('Tempo Decorrido')
plt.ylabel('Erro de Little')
plt.legend()

# Ajusta layout
plt.tight_layout()
plt.show()
