#!/bin/bash

# Configurações do servidor de Exit Poll
PORT=22001

echo "Configurando o nó do Exit Poll."

# Apagar diretório anterior
rm -rf test/ExitPoll
mkdir -p test/ExitPoll

# Copiar e descomprimir
cp dirExitPoll.zip test/ExitPoll/
cd test/ExitPoll || exit 1
unzip -o dirExitPoll.zip

# Executar o servidor
echo "A iniciar o servidor Exit Poll..."
cd dirExitPoll || exit 1
java serverSide.main.ServerExitPollMain $PORT

read -p "Pressione Enter para sair..."
