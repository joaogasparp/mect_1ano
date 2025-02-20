package ex1_p2;

public class FifoObj<T> {
    private T[] fila;
    private int frente;
    private int traseira;
    private int tamanho;

    // construtor para inicializar a fila com a capacidade dada
    @SuppressWarnings("unchecked")
    public FifoObj(int capacidade) {
        fila = (T[]) new Object[capacidade];
        frente = 0;
        traseira = -1;
        tamanho = 0;
    }

    // adiciona um objeto à fila
    public void enfileirar(T obj) {
        if (tamanho < fila.length) {
            traseira = (traseira + 1) % fila.length;
            fila[traseira] = obj;
            tamanho++;
        }
    }

    // remove e retorna o objeto da frente da fila
    public T desenfileirar() {
        if (tamanho > 0) {
            T obj = fila[frente];
            frente = (frente + 1) % fila.length;
            tamanho--;
            return obj;
        }
        return null; // retorna null se a fila estiver vazia
    }

    // verifica se a fila está vazia
    public boolean estaVazia() {
        return tamanho == 0;
    }
}
