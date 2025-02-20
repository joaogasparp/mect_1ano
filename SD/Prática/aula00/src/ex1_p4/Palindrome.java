package ex1_p4;

import java.util.Scanner;

public class Palindrome {
    public static void main(String[] args) {
        Scanner scanner = new Scanner(System.in);
        System.out.print("Insira uma palavra: ");
        String entrada = scanner.nextLine();
        int comprimento = entrada.length();

        // cria instâncias de MemFifo e MemStack com base no comprimento da entrada
        MemFifo<Character> fifo = new MemFifo<>(comprimento);
        MemStack<Character> pilha = new MemStack<>(comprimento);

        // enfileira e empilha cada char da entrada
        try {
            for (char c : entrada.toCharArray()) {
                fifo.write(c);
                pilha.write(c);
            }

            boolean ePalindromo = true;
            // compara os chars desenfileirados e desempilhados
            while (!fifo.estaVazia() && !pilha.estaVazia()) {
                if (!fifo.read().equals(pilha.read())) {
                    ePalindromo = false;
                    break;
                }
            }

            // imprime o resultado
            if (ePalindromo) {
                System.out.println(entrada + " é um palíndromo.");
            } else {
                System.out.println(entrada + " não é um palíndromo.");
            }
        } catch (MemException e) {
            System.out.println("Erro: " + e.getMessage());
        }

        scanner.close();
    }
}
