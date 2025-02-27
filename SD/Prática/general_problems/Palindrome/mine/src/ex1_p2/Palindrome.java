package ex1_p2;

import java.util.Scanner;

public class Palindrome {
    public static void main(String[] args) {
        Scanner scanner = new Scanner(System.in);
        System.out.print("Insira uma palavra: ");
        String entrada = scanner.nextLine();
        int comprimento = entrada.length();

        // cria instâncias de FifoObj e StackObj com base no comprimento da entrada
        FifoObj<Character> fifo = new FifoObj<>(comprimento);
        StackObj<Character> pilha = new StackObj<>(comprimento);

        // enfileira e empilha cada char da entrada
        for (char c : entrada.toCharArray()) {
            fifo.enfileirar(c);
            pilha.empilhar(c);
        }

        boolean ePalindromo = true;
        // compara os chars desenfileirados e desempilhados
        while (!fifo.estaVazia() && !pilha.estaVazia()) {
            if (!fifo.desenfileirar().equals(pilha.desempilhar())) {
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

        scanner.close();
    }
}
