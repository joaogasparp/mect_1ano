package ex1_p3;

public abstract class MemObject<T> {
    protected T[] buffer;
    protected int capacidade;

    // construtor para inicializar o buffer com a capacidade dada
    @SuppressWarnings("unchecked")
    public MemObject(int capacidade) {
        this.capacidade = capacidade;
        buffer = (T[]) new Object[capacidade];
    }

    // métodos abstratos para leitura e escrita
    public abstract void write(T obj) throws Exception;

    public abstract T read() throws Exception;
}
