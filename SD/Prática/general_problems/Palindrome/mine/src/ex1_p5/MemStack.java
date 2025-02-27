package ex1_p5;

public class MemStack<T> extends MemObject<T> {
    private int topo;

    // construtor para inicializar a pilha com a capacidade dada
    public MemStack(int capacidade) {
        super(capacidade);
        topo = -1;
    }

    // escreve um objeto no topo da pilha
    @Override
    public void write(T obj) throws MemException {
        if (topo == capacidade - 1) {
            throw new MemException("Pilha cheia");
        }
        buffer[++topo] = obj;
    }

    // lê e remove um objeto do topo da pilha
    @Override
    public T read() throws MemException {
        if (topo == -1) {
            throw new MemException("Pilha vazia");
        }
        return buffer[topo--];
    }

    // verifica se a pilha está vazia
    public boolean estaVazia() {
        return topo == -1;
    }
}
