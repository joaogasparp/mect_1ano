package ex1_p3;

public class MemFifo<T> extends MemObject<T> {
    private int frente;
    private int traseira;
    private int tamanho;

    // construtor para inicializar a fila com a capacidade dada
    public MemFifo(int capacidade) {
        super(capacidade);
        frente = 0;
        traseira = -1;
        tamanho = 0;
    }

    // escreve um objeto na fila
    @Override
    public void write(T obj) throws Exception {
        if (tamanho == capacidade) {
            throw new Exception("Fila cheia");
        }
        traseira = (traseira + 1) % capacidade;
        buffer[traseira] = obj;
        tamanho++;
    }

    // lê e remove um objeto da frente da fila
    @Override
    public T read() throws Exception {
        if (tamanho == 0) {
            throw new Exception("Fila vazia");
        }
        T obj = buffer[frente];
        frente = (frente + 1) % capacidade;
        tamanho--;
        return obj;
    }

    // verifica se a fila está vazia
    public boolean estaVazia() {
        return tamanho == 0;
    }
}
