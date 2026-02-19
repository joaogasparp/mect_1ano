package ex1_p1;

public class FifoChar {
    private char[] fila;
    private int frente;
    private int traseira;
    private int tamanho;

    // construtor para inicializar a fila com a capacidade dada
    public FifoChar(int capacidade) {
        fila = new char[capacidade];
        frente = 0;
        traseira = -1;
        tamanho = 0;
    }

    // adiciona um char à fila
    public void enfileirar(char c) {
        if (tamanho < fila.length) {
            traseira = (traseira + 1) % fila.length;
            fila[traseira] = c;
            tamanho++;
        }
    }

    // remove e retorna o char da frente da fila
    public char desenfileirar() {
        if (tamanho > 0) {
            char c = fila[frente];
            frente = (frente + 1) % fila.length;
            tamanho--;
            return c;
        }
        return '\0'; // retorna um char nulo se a fila estiver vazia
    }

    // verifica se a fila está vazia
    public boolean estaVazia() {
        return tamanho == 0;
    }
}
