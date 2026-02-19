# Trabalho Prático 2 - Dia das Eleições

## Scripts Disponíveis

- `PollingStationDeployAndRun.sh`: Inicia o servidor da estação de votação.
- `ExitPollDeployAndRun.sh`: Inicia o servidor da pesquisa de saída (exit poll).
- `deployAndRun.sh`: Inicia ambos os servidores em terminais separados, com base na capacidade da estação de votação.

---

## Compilação e Preparação

Para compilar o projeto e distribuir os ficheiros pelas pastas necessárias, corre:
```bash
./buildAndGenerateGlobal.sh
```

---

## Execução

### 1. Iniciar os servidores

Utiliza o script `deployAndRun.sh` com a capacidade da estação:
```bash
./deployAndRun.sh <capacidadeEstacao>
```

- `<capacidadeEstacao>` deve ser um número entre **2** e **5**.
- Exemplo:
```bash
./deployAndRun.sh 3
```

Este comando abrirá dois terminais:
- Um com o servidor da **Polling Station**.
- Outro com o servidor do **Exit Poll**.

---

### 2. Iniciar os clientes

Num terminal à parte, corre o cliente com o número de votantes:
```bash
java clientSide.main.ClientElectionDay <numVotantes>
```

- `<numVotantes>` deve ser entre **3** e **10**.
- Exemplo:
```bash
java clientSide.main.ClientElectionDay 8
```

---

## Eliminar ficheiros desnecessários

```bash
cd Desktop/SD/sd_proj2/code/src
find . -type f -name "*.class" -exec rm -f {} +
find . -type d -name "dir*" -exec rm -rf {} +
find . -type f -name "*.zip" -exec rm -f {} +
rm -rf test
```
