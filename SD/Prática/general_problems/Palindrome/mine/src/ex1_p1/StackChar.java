package ex1_p1;

public class StackChar {
    private char[] pilha;
    private int topo;

    // construtor para inicializar a pilha com a capacidade dada
    public StackChar(int capacidade) {
        pilha = new char[capacidade];
        topo = -1;
    }

    // adiciona um char ao topo da pilha
    public void empilhar(char c) {
        if (topo < pilha.length - 1) {
            pilha[++topo] = c;
        }
    }

    // remove e retorna o char do topo da pilha
    public char desempilhar() {
        if (topo >= 0) {
            return pilha[topo--];
        }
        return '\0'; // retorna um char nulo se a pilha estiver vazia
    }

    // verifica se a pilha está vazia
    public boolean estaVazia() {
        return topo == -1;
    }
}
